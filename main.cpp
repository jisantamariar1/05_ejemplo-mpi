#include <iostream>
#include <fmt/core.h>
#include <mpi.h>

int main(int argc, char** argv) {
    // Inicializa el entorno MPI; argc y argv son obligatorios para la configuración.
    MPI_Init(&argc, &argv); //argc Argument Count // argcv Argument vector (linea de comandos)

    int nproc, rank;

    // MPI_COMM_WORLD es el comunicador que agrupa a todos los procesos ("el universo").
    // Obtenemos el total de procesos (nproc) y el ID del proceso actual (rank).
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int version, subversion;
    MPI_Get_version(&version, &subversion);

    // El rank 0 actúa como coordinador para imprimir info global.
    if(rank == 0) { 
        fmt::println("Versión de MPI: {}.{}", version, subversion);
        fmt::println("Número de procesos: {}", nproc);
    }

    fmt::println("RANK_{} de {} procesos", rank, nproc);

    // ADVERTENCIA: Este bucle infinito impide llegar a MPI_Finalize().
    // Los procesos quedarían colgados (zombis) y la red no se liberaría.
/*     while(1){ 

    } */

    // MPI_Finalize cierra la comunicación y libera recursos; es sagrado.
    MPI_Finalize(); 
    return 0;
}