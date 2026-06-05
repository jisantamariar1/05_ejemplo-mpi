#include <mpi.h>        // Librería MPI para programación paralela distribuida
#include <fmt/core.h>   // Librería fmt para impresión formateada
#include <vector>       // Contenedor dinámico std::vector
#include <cmath>        // Funciones matemáticas como std::ceil

#define MATRIX_DIM 25   // Tamaño de la matriz cuadrada (25x25)

// Función para imprimir una matriz almacenada en un vector unidimensional
void imprimir_matriz(const std::vector<double> &A, int rows, int cols)
{
    // Recorre cada fila
    for (int i = 0; i < rows; ++i)
    {
        // Recorre cada columna
        for (int j = 0; j < cols; ++j)
        {
            // Convierte la posición bidimensional (i,j) a índice lineal
            fmt::print("{:.2f} ", A[i * cols + j]);
        }

        // Salto de línea al terminar cada fila
        fmt::println("");
    }
}

int main(int argc, char **argv)
{
    // Inicializa el entorno MPI
    MPI_Init(&argc, &argv);

    // Número total de procesos MPI
    int nproc;

    // Identificador único del proceso actual
    int rank;

    // Obtiene el número total de procesos ejecutándose
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    // Obtiene el rank (ID) del proceso actual
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Calcula cuántas filas le corresponden a cada proceso
    int rows_per_rank = std::ceil(MATRIX_DIM * 1.0 / nproc);

    // Calcula cuántas filas sobran debido al redondeo
    int padding = rows_per_rank * nproc - MATRIX_DIM;

    // El proceso 0 actúa como maestro
    if (rank == 0)
    {
        // Matriz completa A de tamaño MATRIX_DIM x MATRIX_DIM
        std::vector<double> A(MATRIX_DIM * MATRIX_DIM);

        // Vector b del sistema Ax=b
        std::vector<double> b(MATRIX_DIM);

        // Vector solución x
        std::vector<double> x(MATRIX_DIM);

        // Inicializa la matriz A
        for (int i = 0; i < MATRIX_DIM; i++)
        {
            for (int j = 0; j < MATRIX_DIM; j++)
            {
                // Convierte (i,j) a índice lineal
                int index = i * MATRIX_DIM + j;

                // Llena toda la fila con el valor i
                A[index] = i;
            }
        }

        // Inicializa el vector b
        for (int i = 0; i < MATRIX_DIM; i++)
        {
            b[i] = i;
        }

        // Muestra información general de la distribución
        fmt::println(
            "MATRIX_DIM: {}, nprocs: {}, rows_per_rank: {}, padding: {}\n",
            MATRIX_DIM,
            nproc,
            rows_per_rank,
            padding);

        // Envía datos a todos los procesos trabajadores
        for (int i = 1; i < nproc; i++)
        {
            // Cantidad de filas que recibirá inicialmente el proceso i
            int filas = rows_per_rank;

            // Si es el último proceso, se descuentan las filas de padding
            if (i == nproc - 1)
            {
                filas = rows_per_rank - padding;
            }

            // Vector con metadatos:
            // [0] = tamaño de la matriz
            // [1] = número de filas asignadas
            std::vector<int> data = {MATRIX_DIM, filas};

            // Envía los metadatos al proceso i
            MPI_Send(
                data.data(),      // Buffer de envío
                2,                // Cantidad de enteros
                MPI_INT,          // Tipo de dato
                i,                // Destino
                0,                // Tag
                MPI_COMM_WORLD    // Comunicador
            );

            // Puntero al inicio de la matriz A (Apunta al inicio de los datos de la matriz)
            const double *buffer = A.data();

            // Envía el bloque de filas correspondiente al proceso i
            MPI_Send(
                &buffer[i * rows_per_rank * MATRIX_DIM], // Inicio del bloque
                filas * MATRIX_DIM,                      // Cantidad de elementos
                MPI_DOUBLE,                              // Tipo de dato
                i,                                       // Destino
                0,                                       // Tag
                MPI_COMM_WORLD                          // Comunicador
            );
        }

        // Muestra la porción que teóricamente maneja el rank 0
        fmt::println(
            "RANK_{}, {} x {}\n",
            rank,
            rows_per_rank,
            MATRIX_DIM);
    }
    else
    {
        // Vector para recibir los metadatos enviados por el rank 0
        std::vector<int> data_rec(2);

        // Recibe MATRIX_DIM y número de filas asignadas
        MPI_Recv(
            data_rec.data(),    // Buffer de recepción
            2,                  // Cantidad de elementos
            MPI_INT,            // Tipo de dato
            0,                  // Origen
            0,                  // Tag
            MPI_COMM_WORLD,     // Comunicador
            MPI_STATUS_IGNORE   // Ignorar información de estado
        );

        // Extrae el tamaño de la matriz
        int matrix_dim = data_rec[0];

        // Extrae el número de filas asignadas
        int rows = data_rec[1];

        // Muestra información del bloque asignado
        fmt::println(
            "RANK_{}, {} x {}\n",
            rank,
            rows_per_rank,
            MATRIX_DIM);

        // Reserva espacio para almacenar la submatriz recibida
        std::vector<double> A_local(rows * matrix_dim);

        // Recibe el bloque de filas correspondiente
        MPI_Recv(
            A_local.data(),     // Buffer donde se guardará la submatriz
            rows * matrix_dim,  // Cantidad de elementos a recibir
            MPI_DOUBLE,         // Tipo de dato
            0,                  // Proceso origen
            0,                  // Tag
            MPI_COMM_WORLD,     // Comunicador
            MPI_STATUS_IGNORE   // Ignorar estado
        );

        // Sólo el proceso 1 imprime su bloque recibido
        if (rank == 1)
        {
            fmt::println(
                "Matriz local recibida por RANK_{}:\n",
                rank);

            // Imprime la submatriz recibida
            imprimir_matriz(
                A_local,
                rows,
                matrix_dim);
        }
    }

    // Finaliza el entorno MPI y libera recursos
    MPI_Finalize();

    // Fin correcto del programa
    return 0;
}