#include <mpi.h>
#include <fmt/core.h>
#include <vector>
#include <cmath>

#define MATRIX_DIM 25

void imprimir_matriz(const std::vector<double>& A, int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            fmt::print("{:.2f} ", A[i * cols + j]);
        }
        fmt::println();
    }
}


int main(int argc, char** argv) {

    MPI_Init(&argc, &argv); // Inicializa el entorno MPI

    int nproc;
    int rank;
    //rank y nproc
    MPI_Comm_size(MPI_COMM_WORLD, &nproc); // Obtiene el número total de procesos
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);   // Obtiene el ID del proceso

    
    if(rank == 0) {

        std::vector<double> A(MATRIX_DIM * MATRIX_DIM);
        std::vector<double> b(MATRIX_DIM);
        std::vector<double> x(MATRIX_DIM);

        //inicializar la matriz A y el vector b
        for(int i=0; i<MATRIX_DIM;i++){
            for(int j=0; j<MATRIX_DIM;j++){
                int index = i*MATRIX_DIM + j;
                A[index] = index;
            }
        }

        for(int i=0; i<MATRIX_DIM;i++){
            b[i] = i;
        }

        //numero de filas para cada rank(proceso)
        int rows_per_rank = std::ceil(MATRIX_DIM *1.0 / nproc); // Número de filas que cada proceso manejará
        int padding = rows_per_rank * nproc - MATRIX_DIM; // Padding necesario para que el número total de filas sea divisible por nproc

        fmt::println("MATRIX_DIM: {}, nprocs: {}, rows_per_rank: {}, padding: {}\n",
        MATRIX_DIM, nproc, rows_per_rank, padding);
    };

    for(int i=1; i<nprocs;i++){
        int filas = rows_per_rank;
        if(i==nprocs-1){
            filas = rows_per_rank - padding; // El último proceso maneja las filas restantes sin el padding
        }
        std::vector<int> data = {MATRIX_DIM, filas};
        MPI_Send(
            data.data(), // Puntero al inicio del vector de datos //buffer
            2, // Número de elementos a enviar //count
            MPI_INT, // Tipo de dato de los elementos a enviar //tipo de datos
            i, // Destino: el proceso con rank i //rank de destino
            0, // Etiqueta del mensaje (puede ser cualquier entero) //tag
            MPI_COMM_WORLD  // Enviar datos a cada proceso //grupo
        );
        
        const double* buffer = A.data();

        MPI_Send(
            &buffer[i * rows_per_rank * MATRIX_DIM], // Puntero al inicio del vector de datos //buffer
            filas* MATRIX_DIM, // Número de elementos a enviar //count
            MPI_DOUBLE, // Tipo de dato de los elementos a enviar //tipo de datos
            i, // Destino: el proceso con rank i //rank de destino
            0, // Etiqueta del mensaje (puede ser cualquier entero) //tag
            MPI_COMM_WORLD  // Enviar datos a cada proceso //grupo
        );
    }
    fmt::println("RANK_{}, {} x {}\n", rank, rows_per_rank, MATRIX_DIM);
}
else{
    std::vector<int> data_rec(2);

    MPI_Recv(
        data_rec.data(), // Puntero al inicio del vector de datos //buffer
        2, // Número de elementos a recibir //count
        MPI_INT, // Tipo de dato de los elementos a recibir //tipo de datos
        0, // Fuente: el proceso con rank 0 //rank de origen
        0, // Etiqueta del mensaje (debe coincidir con la etiqueta usada por el proceso emisor) //tag
        MPI_COMM_WORLD, // Recibir datos del proceso con rank 0 //grupo
        MPI_STATUS_IGNORE // Ignorar el estado del mensaje recibido
    ); 

    int matrix_dim = data_rec[0];
    int rows = data_rec[1];

    fmt::println("RANK_{}, {} x {}\n", rank, rows_per_rank, MATRIX_DIM);
    std::vector<double> A_local(rows * matrix_dim);

    MPI_Recv(
        A_local.data(), // Puntero al inicio del vector de datos //buffer
        rows * matrix_dim, // Número de elementos a recibir //count
        MPI_DOUBLE, // Tipo de dato de los elementos a recibir //tipo de datos
        0, // Fuente: el proceso con rank 0 //rank de origen
        0, // Etiqueta del mensaje (debe coincidir con la etiqueta usada por el proceso emisor) //tag
        MPI_COMM_WORLD, // Recibir datos del proceso con rank 0 //grupo
        MPI_STATUS_IGNORE // Ignorar el estado del mensaje recibido
    );
    if(rank == 1){
        fmt::println("Matriz local recibida por RANK_{}:\n", rank);
        imprimir_matriz(A_local, rows, matrix_dim);
    }
}

    MPI_Finalize(); // Finaliza el entorno MPI
    return 0;
}   
