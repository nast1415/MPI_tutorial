#include<stdio.h> // стандартная библиотека 
#include <ctime> // библиотека для работы с функциями времени


#define NUM_ROWS_A 5 // количество строк в матрице A
#define NUM_COLUMNS_A 5 // количество столбцов в матрице А
#define NUM_ROWS_B 5 // количество строк в матрице В
#define NUM_COLUMNS_B 5 // // количество столбцов в матрице A

void fillAB(); // функция, заполняющая значениями матрицы A и B
void printResult(); // функция, обеспечивающая красивый вывод матриц и результата их перемножения

int i, j, k; // вспомогательные переменные для использования в циклах

double mat_a[NUM_ROWS_A][NUM_COLUMNS_A]; // объявление матрицы A
double mat_b[NUM_ROWS_B][NUM_COLUMNS_B]; // объявление матрицы B
double mat_c[NUM_ROWS_A][NUM_COLUMNS_B]; // объявление результирующей матрицы C

double start_time; // переменная для отсечки времени начала алгоритма
double end_time; // переменная для отсечки времени конца алгоритма


int main(int argc, char *argv[])
{
    fillAB(); // заполняем матрицы А и В
    start_time =  clock(); // засекаем время начала работы
    for (i = 0; i < NUM_ROWS_A; i++) {// итерируемся в цикле по строкам матрицы A
        for (j = 0; j < NUM_COLUMNS_B; j++) {// итерируемся в цикле по столбцам матрицы В
            // скалярно перемножаем i-ую строку A и j-ую строку B
            for (k = 0; k < NUM_ROWS_B; k++) {// итерируемся в цикле по элементам столбца (строки)
                mat_c[i][j] += mat_a[i][k] * mat_b[k][j]; // суммируем произведения соответствующих элементов и записываем в С[i][j]
            }
        }
    }
    end_time =  clock(); // засекаем время окончания работы алгоритма
    printf("\nRunning Time = %f\n\n", end_time - start_time); // получаем итоговое время работы программы в миллисекундах
    printResult(); // выводим матрицы на печать
    return 0;

}
void fillAB() // вспомогательная функция, заполняющая матрицы А и В значениями
{
    for (i = 0; i < NUM_ROWS_A; i++) {
        for (j = 0; j < NUM_COLUMNS_A; j++) {
            mat_a[i][j] = i + j + 35;
        }
    }
    for (i = 0; i < NUM_ROWS_B; i++) {
        for (j = 0; j < NUM_COLUMNS_B; j++) {
            mat_b[i][j] = i * j + 10;
        }
    }
}
void printResult() // вспомогательная функция, позволяющая делать красивый вывод матриц
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