#include<stdio.h> // стандартная библиотека
#include<mpi.h> // библиотека для работы с MPI

#define NUM_ROWS_A 5 // количество строк в матрице A
#define NUM_COLUMNS_A 5 // количество столбцов в матрице А
#define NUM_ROWS_B 5 // количество строк в матрице В
#define NUM_COLUMNS_B 5 // // количество столбцов в матрице A

#define MAIN_TO_WORKER_TAG 1 // тег (идентификатор) для сообщений, отправляемых главным процессом процессам-рабочим
#define WORKER_TO_MAIN_TAG 4 // тег (идентификатор) для сообщений, отправляемых процессами-рабочими главному процессом 

void fillAB(); // функция, заполняющая значениями матрицы A и B
void printResult(); // функция, обеспечивающая красивый вывод матриц и результата их перемножения

int rank; // переменная для хранения ранга процесса
int size; // переменная для хранения общего числа процессов
int i, j, k; // вспомогательные переменные для использования в циклах

double mat_a[NUM_ROWS_A][NUM_COLUMNS_A]; // объявление матрицы A
double mat_b[NUM_ROWS_B][NUM_COLUMNS_B]; // объявление матрицы B
double mat_c[NUM_ROWS_A][NUM_COLUMNS_B]; // объявление результирующей матрицы C

double start_time; // переменная для отсечки времени начала алгоритма
double end_time; // переменная для отсечки времени конца алгоритма

int min_row_id; // номер первой строки из блока строк матрицы А, которые выделены на обработку очередному процессу-рабочему
int max_row_id; // номер последней строки из блока строк матрицы А, которые выделены на обработку очередному процессу-рабочему
int portion; // количество строк в блоке строк матрицы А, которые выделены на обработку очередному процессу-рабочему

MPI_Status status; // хранит параметры сообщения, полученного с помощью MPI_Recv
MPI_Request request; // хранит идентификатор сообщения, отправленного с помощью MPI_Isend

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv); // инициализируем среду MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // находим ранг процесса и записываем в переменную rank
    MPI_Comm_size(MPI_COMM_WORLD, &size); // находим количество процессов и записываем в переменную size
    
    /* инициализация работы главным процессом */
    if (rank == 0) { // если процесс главный (главный тот, у кого ранг 0)
        fillAB(); // заполняем матрицы А и В значениями
        start_time = MPI_Wtime(); // запускаем таймер времени исполнения
        for (i = 1; i < size; i++) {// итерируемся в цикле по всем процессам-рабочим (кроме процесса с рангом 0)
            portion = NUM_ROWS_A / (size - 1); // вычисляем количество строк, которое каждый процесс будет обрабатывать
            min_row_id = (i - 1) * portion; // блок строк для каждого процесса начинается со строки с таким номером
            if (((i + 1) == size) && ((NUM_ROWS_A % (size - 1)) != 0)) { // если строки матрицы А нельзя равномерно поделить на всех рабочих
                max_row_id = NUM_ROWS_A; // тогда последний процесс-рабочий получит чуть больше
            } else {
                max_row_id = min_row_id + portion; // а иначе делим все поровну и тогда номер последней строки в блоке для работы будет таким
            }

            // отправляем информацию о первой строке матрицы А, с которой нужно работать, соответствующему процессу-рабочему (без блокировки)
            MPI_Isend(&min_row_id, 1, MPI_INT, i, MAIN_TO_WORKER_TAG, MPI_COMM_WORLD, &request);
            // отправляем информацию о последней строке матрицы А, с которой нужно работать, соответствующему процессу-рабочему (без блокировки)
            MPI_Isend(&max_row_id, 1, MPI_INT, i, MAIN_TO_WORKER_TAG + 1, MPI_COMM_WORLD, &request);
            // отправляем ссылку на первый элемент первой строки, с которой нужно работать процессу и указываем, сколько всего элементов нужно обработать 
            MPI_Isend(&mat_a[min_row_id][0], (max_row_id - min_row_id) * NUM_COLUMNS_A, MPI_DOUBLE, i, MAIN_TO_WORKER_TAG + 2, MPI_COMM_WORLD, &request);
        }
    }
    // делаем широковещательную рассылку матрицы В всем процессам-рабочим
    MPI_Bcast(&mat_b, NUM_ROWS_B*NUM_COLUMNS_B, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    /* что делают процессы-рабочие */
    if (rank > 0) { // рабочие процессы - это все, у кого ранг не 0
        // получаем информацию о первой строке матрицы А, с которой нужно работать
        MPI_Recv(&min_row_id, 1, MPI_INT, 0, MAIN_TO_WORKER_TAG, MPI_COMM_WORLD, &status);
        /// получаем информацию о последней строке матрицы А, с которой нужно работать
        MPI_Recv(&max_row_id, 1, MPI_INT, 0, MAIN_TO_WORKER_TAG + 1, MPI_COMM_WORLD, &status);
        // получаем сылку на первый элемент первой строки, с которой нам нужно работать и информацию о том, сколько всего элементов нужно обработать
        MPI_Recv(&mat_a[min_row_id][0], (max_row_id - min_row_id) * NUM_COLUMNS_A, MPI_DOUBLE, 0, MAIN_TO_WORKER_TAG + 2, MPI_COMM_WORLD, &status);
        
        // умножаем требуемую часть матрицы A на соответствующую часть матрицы B
        for (i = min_row_id; i < max_row_id; i++) {// итерируемся в цикле по блоку из строк матрицы A, который нам сказали обработать
            for (j = 0; j < NUM_COLUMNS_B; j++) {// итерируемся в цикле по столбцам матрицы В
                // скалярно перемножаем i-ую строку A и j-ую строку B
                for (k = 0; k < NUM_ROWS_B; k++) {// итерируемся в цикле по элементам столбца (строки)
                    mat_c[i][j] += mat_a[i][k] * mat_b[k][j]; // суммируем произведения соответствующих элементов и записываем в С[i][j]
                }
            }
        }
        // отправляем обратно информацию о первой строке, с которой мы работали
        MPI_Isend(&min_row_id, 1, MPI_INT, 0, WORKER_TO_MAIN_TAG, MPI_COMM_WORLD, &request);
        //отправляем обратно информацию о последней строке, с которой мы работали
        MPI_Isend(&max_row_id, 1, MPI_INT, 0, WORKER_TO_MAIN_TAG + 1, MPI_COMM_WORLD, &request);
        // отправляем ссылку на матрицу, которая получилась в итоге
        MPI_Isend(&mat_c[min_row_id][0], (max_row_id - min_row_id) * NUM_COLUMNS_B, MPI_DOUBLE, 0, WORKER_TO_MAIN_TAG + 2, MPI_COMM_WORLD, &request);
    }
    /* главнй процесс собирает результаты работы*/
    if (rank == 0) {
        for (i = 1; i < size; i++) {// ждем, пока все не пришлют свои результаты
            // получаем информацию о первой строке, с которой работал процесс
            MPI_Recv(&min_row_id, 1, MPI_INT, i, WORKER_TO_MAIN_TAG, MPI_COMM_WORLD, &status);
            // получаем информацию о последней строке, с которой работал процесс
            MPI_Recv(&max_row_id, 1, MPI_INT, i, WORKER_TO_MAIN_TAG + 1, MPI_COMM_WORLD, &status);
            // получаем ссылку на результат
            MPI_Recv(&mat_c[min_row_id][0], (max_row_id - min_row_id) * NUM_COLUMNS_B, MPI_DOUBLE, i, WORKER_TO_MAIN_TAG + 2, MPI_COMM_WORLD, &status);
        }
        // после получения всех результатов в mat_c будет полностью готовая матрица, которая является результатом перемножения A и B
        end_time = MPI_Wtime(); // останавливаем таймер
        printf("\nRunning Time = %f\n\n", (end_time - start_time) * 1000); // выводим время в миллисекундах (поэтому *1000)
        printResult(); // выводим матрицы
    }
    MPI_Finalize(); // завершаем работу в среде MPI
    return 0;
}
void fillAB()
{
    for (i = 0; i < NUM_ROWS_A; i++) {
        for (j = 0; j < NUM_COLUMNS_A; j++) {
            mat_a[i][j] = i + j + 35;
        }
    }
    for (i = 0; i < NUM_ROWS_B; i++) {
        for (j = 0; j < NUM_COLUMNS_B; j++) {
            mat_b[i][j] = i*j + 10;
        }
    }
}
void printResult()
{
    printf("Matrix A:\n");
    for (i = 0; i < NUM_ROWS_A; i++) {
        printf("\n");
        for (j = 0; j < NUM_COLUMNS_A; j++)
            printf("%8.2f  ", mat_a[i][j]);
    }
    printf("\n\n");
    printf("Matrix B:\n");
    for (i = 0; i < NUM_ROWS_B; i++) {
        printf("\n");
        for (j = 0; j < NUM_COLUMNS_B; j++)
            printf("%8.2f  ", mat_b[i][j]);
    }
    printf("\n\n");
    printf("Result of the multiplication:\n");
    for (i = 0; i < NUM_ROWS_A; i++) {
        printf("\n");
        for (j = 0; j < NUM_COLUMNS_B; j++)
            printf("%8.2f  ", mat_c[i][j]);
    }
    printf("\n\n");
}