#include <omp.h>
#include <iostream>

using namespace std;

// Вычисление значения функции f(x) = 4 / (1 + x ^ 2)
double value_of_function(double x)
{
    return 4.0 / (1.0 + x * x);
}

double Calculate_pi(int num_of_intervals)
{
    double pi = 0.0;
    double step = 1.0 / num_of_intervals;
    double curr_arg = 0;
    double sum = 0.0;

    for (int i = 0; i < num_of_intervals; i++)
    {
        curr_arg = step * (double)(i + 0.5);// Берем аргумент интервала ( i-1 ; i )
        sum += value_of_function(curr_arg);
    }

    pi = sum * step;

    return pi;
}

void main(int argc, char* argv[])
{
    int num_of_intervals = 1;
    double start_time = 0;
    double end_time = 0;
    double time_work = 0;

    double pi;

    num_of_intervals = atoi(argv[1]);

    start_time = omp_get_wtime();

    pi = Calculate_pi(num_of_intervals);

    end_time = omp_get_wtime();

    time_work = end_time - start_time;

    cout.precision(15);
    cout << "Value of PI: " << pi << endl;
    cout << "Calculating PI is worked:" << time_work << " seconds" << endl;

}