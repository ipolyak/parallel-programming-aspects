#include <iostream>
#include <ctime>
#include <cmath>
#include <windows.h>
#include <omp.h> 
#include <thread>

using namespace std;

double* Matr;// ������� � ���� �������
double* Vec;
double* Res_pp;
double* Res_seq;


int Size; // ������ ������� ����� Size x Size, � � ������� Size
int num_of_threads;
int Work_for_threads;


// ������� � ������������������� ������ (�������� ����� ������������������� �������, �������������� � ���� �������)
double* Create_and_init_arr(int size_arr)
{
    if (size_arr < 1)
        return NULL;

    double* arr;

    arr = new double[size_arr];
    srand((unsigned)time(NULL));
    if (size_arr < 5)// ���� �����������, �� ������ �������
        for (int i = 0; i < size_arr; i++)
        {
            cout << "Enter element at the " << i << " position:" << endl;
            cin >> arr[i];
            cout << endl;
        }
    else
        for (int i = 0; i < size_arr; i++)
            arr[i] = rand() % 100 - 50 + (double)(rand()) / RAND_MAX + 0.00000001; // [-50, 51] - �������������� �����

    return arr;
}

// ���������� ������
void Show_arr(double* arr, int size_arr)
{
    if (arr == NULL || size_arr < 1)
        return;

    cout << "Vector: " << endl;
    for (int i = 0; i < size_arr; i++)
        cout << arr[i] << " ";

    cout << endl;
}

// ���������� ���������� �������, �������������� � ���� ������� (size_Matr_pp - ������ ������ ��� �������)
void Show_Matr_pp(double* Matr_pp, int size_Matr_pp)
{
    if (Matr_pp == NULL || size_Matr_pp < 1)
        return;

    cout << "Matrix: " << endl;
    int counter = 0;
    for (int i = 0; i < size_Matr_pp * size_Matr_pp; i++)
    {
        counter++;
        cout << " " << Matr_pp[i];
        if (counter == size_Matr_pp)
        {
            cout << endl;
            counter = 0;
        }
    }
    cout << endl;
}

// ��������� ������� �� ������
void Multiplication_seq()
{
    double SumJ = 0.0;
    for (int i = 0; i < Size; i++)
    {
        for (int j = 0; j < Size; j++)
            SumJ += Matr[i * Size + j] * Vec[j];

        Res_seq[i] = SumJ;
        SumJ = 0.0;
    }
}

// ��������� ������� �� ������
void Multiplication_pp(void *thr_param)
{
    int index = *((int*)thr_param);

    double SumJ = 0.0; // ��� �������� ���������� ������������

    int start = index * Work_for_threads; // ��������� ������ ��� ������ � ������� index
    int end = index == num_of_threads - 1 ? Size : start + Work_for_threads; // �������� ������ ��� ������ � ������� index

    for (int i = start; i < end; i++)
    {
        for (int j = 0; j < Size; j++)
            SumJ += Matr[i * Size + j] * Vec[j];

        Res_pp[i] = SumJ;
        SumJ = 0.0;
    }

}

bool IsCorrect()
{
    for (int i = 0; i < Size; i++)
        if (Res_seq[i] != Res_pp[i])
            return false;

    return true;
}

int main(int argc, char* argv[])
{
    int* index_thread; // ������� �������
    thread* array_of_threads;

    double pp_alg_time_start = 0.0;
    double pp_alg_time_end = 0.0;
    double pp_work_time = 0.0;

    double seq_alg_time_start = 0.0;
    double seq_alg_time_end = 0.0;
    double seq_work_time = 0.0;
    int Show;


    //cin >> num_of_threads;
    //cin >> Size;
    num_of_threads = atoi(argv[1]);
    Size = atoi(argv[2]);

    array_of_threads = new thread[num_of_threads];
    Res_pp = new double[Size];
    Res_seq = new double[Size];
    Vec = Create_and_init_arr(Size);
    Matr = Create_and_init_arr(Size * Size);

    index_thread = new int[num_of_threads];

    cout << "Show Matrix and Vector? <1 - yes, 0 - no> " << endl;
    cin >> Show;

    if (Show)
    {
        Show_Matr_pp(Matr, Size);
        Show_arr(Vec, Size);
    }

    Work_for_threads = Size / num_of_threads;

    for (int i = 0; i < num_of_threads; i++)
        index_thread[i] = i;


    /* ������������ ������ */
    pp_alg_time_start = omp_get_wtime();

    for (int i = 0; i < num_of_threads; i++)
        array_of_threads[i] = thread::thread(Multiplication_pp, &index_thread[i]);

    for (int i = 0; i < num_of_threads; i++)
        array_of_threads[i].join();

    pp_alg_time_end = omp_get_wtime();
    pp_work_time = pp_alg_time_end - pp_alg_time_start;

    cout << "Show Result of multiplication (Parallel version)? <1 - yes, 0 - no> " << endl;
    cin >> Show;

    if (Show)
    {
        cout << "Result of parallel version: " << endl;
        Show_arr(Res_pp, Size);
    }

    /* ���������������� ������ */
    cout << "Sequence version is working... " << endl;
    seq_alg_time_start = omp_get_wtime();
    Multiplication_seq();
    seq_alg_time_end = omp_get_wtime();
    seq_work_time = seq_alg_time_end - seq_alg_time_start;

    if (IsCorrect())// ���������� ����������
        cout << "Results sequence and parallel versions are indentical " << endl;
    else
        cout << "Results sequence and parallel versions are not indentical " << endl;


    cout << "Sequence version is worked:" << seq_work_time << endl;
    cout << "<thread> library version of algorithm is worked: " << pp_work_time << endl;

    delete[] Vec;
    delete[] Matr;
    delete[] array_of_threads;
    delete[] index_thread;
    delete[] Res_pp;
    delete[] Res_seq;


    //system("pause");
    return 0;
}