cmake_minimum_required(VERSION 3.16)
project(ejemplo-mpi)

# 1. Definir variables PRIMERO (para que estén disponibles)
set(MPT_ROOT "C:/oneAPI/mpi/latest") #ruta base
set(MPI_INCLUDE "${MPT_ROOT}/include") #archivos .h (includes)
set(MPI_LIB "${MPT_ROOT}/lib") # librerias (linker)

# 2. Configurar directorios (ahora sí conoce MPI_INCLUDE)
include_directories(${MPI_INCLUDE})

# 3. Definir los ejecutables
add_executable(ejemplo-mpi main.cpp)
add_executable(matrices_mult matrices_mult.cpp)
add_executable(matrices_mult_colectiva matrices_mult_colectiva.cpp)


# 4. Buscar librerías externas
find_package(fmt CONFIG REQUIRED)

# 5. Enlazar las librerías a cada ejecutable
#impi.lib intel mpi (arhivo con todo el codigo binario de la implementacion MPI de intel)
target_link_libraries(ejemplo-mpi PRIVATE fmt::fmt ${MPI_LIB}/impi.lib) 
target_link_libraries(matrices_mult PRIVATE fmt::fmt ${MPI_LIB}/impi.lib) 
target_link_libraries(matrices_mult_colectiva PRIVATE fmt::fmt ${MPI_LIB}/impi.lib) 
