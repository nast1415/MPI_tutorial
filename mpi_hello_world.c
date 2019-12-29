# include <stdio.h> // стандартная библиотека
#include <mpi.h> // библиотека для работы с MPI

int main(int argc, char* argv[]) {
  MPI_Init(&argc, &argv); // Инициализируем среду MPI

  int mySize; // переменная для количества процессоров
  MPI_Comm_size(MPI_COMM_WORLD, &mySize); // узнаем количество процессоров и записываем в переменную

  int myRank; // переменная для ранга процесса
  MPI_Comm_rank(MPI_COMM_WORLD, &myRank); // узнаем ранг процесса

  // Выводим приветственное сообщение
  printf("Hello world from processor with rank %d! Total amount of processors: %d.\n", myRank, mySize);

  MPI_Finalize(); // Завершаем среду MPI. После этого нельзя вызывать MPI функции
  return 0;
}