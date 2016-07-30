#include <iostream>
#include <thread>
#include <omp.h>

using namespace std;

int num_of_intervals;
int num_of_threads;

double* rez_of_threads;
thread* mas_of_threads;

double f(double arg)
{
    return 4.0 / (1.0 + arg * arg);
}

void CalculationPi(void* thr_param)
{
    int start = *((int*)thr_param);
    int i = 0;
    double step = 1.0 / (double)num_of_intervals;
    double arg = 0.0;
    double partial_sum = 0.0;

    for (i = start; i < num_of_intervals; i += num_of_threads)
    {
        arg = step * (i + 0.5);
        partial_sum += f(arg);
    }

    rez_of_threads[start] = partial_sum * step;
}

int main(int argc, char* argv[])
{
    double start_time = 0.0;
    double end_time = 0.0;
    double time_work = 0.0;

    double pi = 0.0;

    int* index_thread;

    num_of_threads = atoi(argv[1]);
    num_of_intervals = atoi(argv[2]);

    cout << "Input number of intervals: " << endl;
    cin >> num_of_intervals;

    cout << "Input number of threads: " << endl;
    cin >> num_of_threads;

    mas_of_threads = new thread[num_of_threads];
    rez_of_threads = new double[num_of_threads];
    index_thread = new int[num_of_threads];

    for (int i = 0; i < num_of_threads; i++)
    {
        index_thread[i] = i;
        rez_of_threads[i] = 0.0;
    }

    start_time = omp_get_wtime();

    for (int i = 0; i < num_of_threads; i++)
        mas_of_threads[i] = thread::thread(CalculationPi, &index_thread[i]);

    for (int i = 0; i < num_of_threads; i++)
        mas_of_threads[i].join();

    end_time = omp_get_wtime();

    time_work = end_time - start_time;

    for (int i = 0; i < num_of_threads; i++)
        pi += rez_of_threads[i];

    cout.precision(15);
    cout << "Result of calculating pi: " << pi << endl;
    cout << "Algorithm is worked: " << time_work << "seconds" << endl;

    delete[] rez_of_threads;
    delete[] mas_of_threads;
    delete[] index_thread;

    //system("pause");

    return 0;
}