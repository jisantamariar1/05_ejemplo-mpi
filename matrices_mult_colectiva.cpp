#include <mpi.h>        // Librería MPI para programación paralela distribuida
#include <fmt/core.h>   // Librería fmt para impresión formateada
#include <vector>       // Contenedor dinámico std::vector
#include <cmath>        // Funciones matemáticas como std::ceil

#define MATRIX_DIM 25   // Tamaño de la matriz cuadrada (25x25)

// Función para imprimir una matriz almacenada en un vector unidimensional
void imprimir_matriz(const std::vector<double> &A, int rows, int cols)
{
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            fmt::print("{:.2f} ", A[i * cols + j]);
        }
        fmt::println("");
    }
}

void imprimir_vector(const std::vector<double> &v)
{
    for (size_t i = 0; i < v.size(); ++i) {
        fmt::print("{:.2f} ", v[i]);
    }
    fmt::println("");
}

void multiplicar_matriz_vector(const std::vector<double> &A, const std::vector<double> &b, std::vector<double> &x, int rows, int cols)
{
    for (int i = 0; i < rows; ++i) {
        double sum = 0.0;
        for (int j = 0; j < cols; ++j) {
            int index = i * cols + j;
            sum += A[index] * b[j];
        }
        x[i] = sum;
    }
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int nproc, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Todos los procesos calculan las dimensiones que les corresponden
    int rows_per_rank = std::ceil(MATRIX_DIM * 1.0 / nproc);
    int padding = rows_per_rank * nproc - MATRIX_DIM;
    int padded_rows = rows_per_rank * nproc;

    // Buffers LOCALES: Todos los procesos (incluido el maestro) los necesitan
    std::vector<double> b(MATRIX_DIM, 0.0);
    std::vector<double> A_local(rows_per_rank * MATRIX_DIM, 0.0);
    std::vector<double> x_local(rows_per_rank, 0.0);

    // Buffers GLOBALES: Solo el maestro (Rank 0) les dará un tamaño y los inicializará
    std::vector<double> A;
    std::vector<double> x;

    if (rank == 0)
    {
        A.resize(padded_rows * MATRIX_DIM, 0.0);
        x.resize(padded_rows, 0.0);

        // Inicializa la matriz A
        for (int i = 0; i < MATRIX_DIM; i++) {
            for (int j = 0; j < MATRIX_DIM; j++) {
                A[i * MATRIX_DIM + j] = i;
            }
        }

        // Inicializa el vector b
        for (int i = 0; i < MATRIX_DIM; i++) {
            b[i] = 1.0; 
        }

        fmt::println(
            "MATRIX_DIM: {}, nprocs: {}, rows_per_rank: {}, padding: {}\n",
            MATRIX_DIM, nproc, rows_per_rank, padding);
    }

    // --- COMUNICACIÓN COLECTIVA ---

    // 1. Broadcast (Bcast): Repartir la copia de 'b' desde el Rank 0 a TODOS los procesos.
    MPI_Bcast(
    b.data(),       // 1. Buffer: Puntero a los datos.
    MATRIX_DIM,     // 2. Count: Cantidad de elementos a enviar/recibir.
    MPI_DOUBLE,     // 3. Datatype: Tipo de dato de cada elemento.
    0,              // 4. Root: Rank del proceso que transmite los datos.
    MPI_COMM_WORLD  // 5. Communicator: Grupo de procesos involucrados.
);
    

// 2. SCATTER: El Maestro toma su gran matriz 'A', la corta en pedazos iguales y le da un pedazo ('A_local') a cada proceso.
    MPI_Scatter(
        A.data(),                   // [MAESTRO] De dónde saco los datos a repartir (los demás procesos lo ignoran).
        rows_per_rank * MATRIX_DIM, // [MAESTRO] Cuántos elementos le voy a enviar a CADA proceso.
        MPI_DOUBLE,                 // [MAESTRO] Tipo de dato que estoy enviando.
        
        A_local.data(),             // [TODOS] Dónde voy a guardar el pedazo que me toca.
        rows_per_rank * MATRIX_DIM, // [TODOS] Cuántos elementos voy a recibir en mi pedazo.
        MPI_DOUBLE,                 // [TODOS] Tipo de dato que estoy recibiendo.
        
        0,                          // ¿Quién es el que reparte? El Rank 0.
        MPI_COMM_WORLD              // Comunicador global.
    );

    // Cada proceso muestra qué porción va a manejar
    fmt::println("RANK_{}, {} x {}", rank, rows_per_rank, MATRIX_DIM);

    // 3. Procesamiento: Cada rank calcula su porción local
    multiplicar_matriz_vector(A_local, b, x_local, rows_per_rank, MATRIX_DIM);

// 4. GATHER: El Maestro recolecta los pedacitos de resultados ('x_local') de todos y los junta en orden en el vector final 'x'.
    MPI_Gather(
        x_local.data(),             // [TODOS] Qué pedacito de resultado quiero enviar.
        rows_per_rank,              // [TODOS] Cuántos elementos tiene mi pedacito.
        MPI_DOUBLE,                 // [TODOS] Tipo de dato que estoy enviando.
        
        x.data(),                   // [MAESTRO] Dónde voy a pegar todos los pedacitos juntos (los demás procesos lo ignoran).
        rows_per_rank,              // [MAESTRO] Cuántos elementos voy a recibir de CADA proceso.
        MPI_DOUBLE,                 // [MAESTRO] Tipo de dato que estoy recibiendo.
        
        0,                          // ¿Quién es el que junta todo? El Rank 0.
        MPI_COMM_WORLD              // Comunicador global.
    );

    // --- IMPRESIÓN DE RESULTADOS ---

    if (rank == 0)
    {
        fmt::println("\nImprime resultado final en RANK_{}:", rank);
        
        // Imprimimos el vector x (incluyendo el padding, como en la lógica original)
        imprimir_vector(x);
    }
    else if (rank == nproc - 1)
    {
        // Solo para debuggear: el último proceso imprime su bloque recibido
        fmt::println("\nMatriz local recibida por RANK_{}:", rank);
        imprimir_matriz(A_local, rows_per_rank, MATRIX_DIM);
        
        fmt::println("Vector b local recibido por RANK_{}:", rank);
        imprimir_vector(b);
    }

    // Finaliza el entorno MPI
    MPI_Finalize();
    return 0;
}