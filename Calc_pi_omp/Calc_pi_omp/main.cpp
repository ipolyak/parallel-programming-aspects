// Calculate_pi_omp.cpp: определяет точку входа для консольного приложения.
//

#include <omp.h>
#include <iostream>

using namespace std;

void main(int argc, char* argv[])
{
    double pi = 0.0;

    int num_of_intervals = 0;
    int num_of_threads = 1;

    double start_time = 0.0;
    double end_time = 0.0;
    double time_work = 0.0;

    int i = 0;
    double arg = 0.0;
    double step;
    double sum = 0.0;

    int chunk = 0;

    //cin >> num_of_threads;
    //cin >> num_of_intervals;
    num_of_threads = atoi(argv[1]);
    num_of_intervals = atoi(argv[2]);

    start_time = omp_get_wtime();

    step = 1.0 / num_of_intervals;

    chunk = num_of_intervals / num_of_threads;

    // Начало параллельной версии кода. Здесь num_threads(num_of_threads) устанавливает число потоков, выполняющих код в этой директиве
    // shared(...) - переменные, которые совместно используются всеми процессами
    // private(...) - для каждой указанной здесь переменной создаются копии для каждого потока (т.е. это личные параметры потока)

#pragma omp parallel num_threads(num_of_threads) private(arg) firstprivate(sum)
    {
        // Распараллеливание цикла. Здесь schedule(static, chunk) распределяет chunk итераций цикла между потоками
#pragma omp for schedule(static, chunk)
        for (i = 0; i < num_of_intervals; i++)
        {
            arg = step * (double)(i + 0.5);
            sum += 4.0 / (1.0 + arg * arg);
        }

        pi += sum * step;
    }

    end_time = omp_get_wtime();

    time_work = end_time - start_time;

    cout.precision(15);
    cout << "The value of PI is: " << pi << endl;
    cout << "Execution time is: " << time_work << endl;

    //system("pause");
}