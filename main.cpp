#include <iostream>
#include <fmt/core.h>
#include <mpi.h>

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv); // Inicializa el entorno MPI

    int nproc;
    int rank;
    //rank y nproc
    MPI_Comm_size(MPI_COMM_WORLD, &nproc); // Obtiene el número total de procesos
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);   // Obtiene el ID del

    //version
    int version, subversion;
    MPI_Get_version(&version, &subversion); // Obtiene la versión de MPI

    if(rank == 0) { // Solo el proceso con rank 0 imprime la información
        fmt::println("Versión de MPI: {}.{}", version, subversion);
        fmt::println("Number of processes: {}", nproc);
        
    }

    fmt::println("RANK_{} de {} procesos", rank, nproc);

/*     while(1){
        
    } */


    MPI_Finalize(); // Finaliza el entorno MPI
    return 0;
}