// Calculate_pi_thread.cpp: определяет точку входа для консольного приложения.
//

#include <iostream>
#include <windows.h>
#include <omp.h>

using namespace std;

int num_of_intervals;
int num_of_threads;


DWORD* array_id_of_threads;
double* array_of_local_summ;


// Вычисление числа Пи потоками
DWORD WINAPI Calculate_pi(void *thr_param)
{
    int start = *((int*)thr_param);
    int i = 0;
    double step = 1.0 / (double)(num_of_intervals);
    double arg;
    double sum = 0.0;

    for (i = start; i < num_of_intervals; i = i + num_of_threads)
    {
        arg = step * (i + 0.5);
        sum += 4.0 / (1.0 + arg * arg); // Считаем площадь прямоугольника
    }
    array_of_local_summ[start] = sum * step;

    return 0;
}

void main(int argc, char* argv[])
{
    double pi = 0.0;

    double start_time = 0.0;
    double end_time = 0.0;
    double time_work = 0.0;

    int* index_thread; // индекс потоков

    //Создание потоков
    DWORD* array_id_of_threads;
    HANDLE* array_of_threads;

    //cin >> num_of_threads;
    //cin >> num_of_intervals;
    num_of_threads = atoi(argv[1]); // С консоли считываем введенное число потоков 
    num_of_intervals = atoi(argv[2]); // С консоли считываем введенное число интервалов

    array_id_of_threads = new DWORD[num_of_threads];
    array_of_threads = new HANDLE[num_of_threads];
    index_thread = new int[num_of_threads];
    array_of_local_summ = new double[num_of_threads];

    for (int i = 0; i < num_of_threads; i++)
    {
        array_of_local_summ[i] = 0;
        index_thread[i] = i;
    }

    start_time = omp_get_wtime();

    for (int i = 0; i < num_of_threads; i++)
        array_of_threads[i] = CreateThread(NULL, 0, Calculate_pi, &index_thread[i], 0, &array_id_of_threads[i]);

    WaitForMultipleObjects(num_of_threads, array_of_threads, true, INFINITE); // Ждем завершения работы всех потоков

    end_time = omp_get_wtime();
    time_work = end_time - start_time;

    // Удалить дескрипторы потоков и дескриптор мьютекса
    for (int i = 0; i < num_of_threads; i++)
        CloseHandle(array_of_threads[i]);

    for (int i = 0; i < num_of_threads; i++)
        pi += array_of_local_summ[i];



    cout.precision(15);
    cout << "Value of PI is eqvivalent: " << pi << endl;
    cout << "Time of work is eqvivalent: " << time_work << " seconds" << endl;

    //system("pause");

}