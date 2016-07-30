// Calculate_pi_omp.cpp: ���������� ����� ����� ��� ����������� ����������.
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

    // ������ ������������ ������ ����. ����� num_threads(num_of_threads) ������������� ����� �������, ����������� ��� � ���� ���������
    // shared(...) - ����������, ������� ��������� ������������ ����� ����������
    // private(...) - ��� ������ ��������� ����� ���������� ��������� ����� ��� ������� ������ (�.�. ��� ������ ��������� ������)

#pragma omp parallel num_threads(num_of_threads) private(arg) firstprivate(sum)
    {
        // ����������������� �����. ����� schedule(static, chunk) ������������ chunk �������� ����� ����� ��������
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