#include <iostream>
#include <ctime>
#include <windows.h>
#include <omp.h> 

using namespace std;

double* Matr;// Матрица в виде вектора
double* Vec;
double* Res_pp;
double* Res_seq;

int Size; // Размер матрицы будет Size x Size, а у вектора Size
int num_of_threads;

// Создать и проинициализировать массив (возможно также проинициализировать матрицу, представленную в виде вектора)
double* Create_and_init_arr(int size_arr)
{
    if (size_arr < 1)
        return NULL;

    double* arr;

    arr = new double[size_arr];
    srand((unsigned)time(NULL));
    if (size_arr < 5)// Если выполняется, то вводим вручную
        for (int i = 0; i < size_arr; i++)
        {
            cout << "Enter element at the " << i << " position:" << endl;
            cin >> arr[i];
            cout << endl;
        }
    else
        for (int i = 0; i < size_arr; i++)
            arr[i] = rand() % 100 - 50 + (double)(rand()) / RAND_MAX + 0.00000001; // [-50, 51] - действительные числа

    return arr;
}

// Отобразить массив
void Show_arr(double* arr, int size_arr)
{
    if (arr == NULL || size_arr < 1)
        return;

    cout << "Vector: " << endl;
    for (int i = 0; i < size_arr; i++)
        cout << arr[i] << " ";

    cout << endl;
}

// Отобразить квадратную матрицу, представленную в виде вектора (size_matr - размер строки или столбца)
void Show_matr(double* matr, int size_matr)
{
    if (matr == NULL || size_matr < 1)
        return;

    cout << "Matrix: " << endl;
    int counter = 0;
    for (int i = 0; i < size_matr * size_matr; i++)
    {
        counter++;
        cout << " " << matr[i];
        if (counter == size_matr)
        {
            cout << endl;
            counter = 0;
        }
    }
    cout << endl;
}

// Умножение матрицы на вектор
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

bool IsCorrect()
{
    for (int i = 0; i < Size; i++)
        if (Res_seq[i] != Res_pp[i])
            return false;

    return true;
}

int main(int argc, char* argv[])
{

    double alg_time_start = 0.0;
    double alg_time_end = 0.0;
    double work_time = 0.0;

    double seq_alg_time_start = 0.0;
    double seq_alg_time_end = 0.0;
    double seq_work_time = 0.0;

    int Show;

    double SumJ = 0.0;
    int i = 0, j = 0;

    //cin >> num_of_threads;
    //cin >> Size;
    num_of_threads = atoi(argv[1]);
    Size = atoi(argv[2]);

    Res_pp = new double[Size];
    Res_seq = new double[Size];
    Vec = Create_and_init_arr(Size);
    Matr = Create_and_init_arr(Size * Size);

    cout << "Show Matrix and vector? <1 - yes, 0 - no> " << endl;
    cin >> Show;

    if (Show)
    {
        Show_matr(Matr, Size);
        Show_arr(Vec, Size);
    }

    alg_time_start = omp_get_wtime();

    /* OMP */
#pragma omp parallel num_threads(num_of_threads) firstprivate(SumJ)
    {
        // Распараллеливание цикла
#pragma omp for private(j)
        for (i = 0; i < Size; i++)
        {
            for (j = 0; j < Size; j++)
                SumJ += Matr[i * Size + j] * Vec[j];

            Res_pp[i] = SumJ;
            SumJ = 0;
        }

    }

    alg_time_end = omp_get_wtime();
    work_time = alg_time_end - alg_time_start;

    cout << "Show result of multiplication? <1 - yes, 0 - no> " << endl;
    cin >> Show;

    if (Show)
    {
        cout << "Result: " << endl;
        Show_arr(Res_pp, Size);
    }

    /* Последовательная версия */
    cout << "Sequence version is working... " << endl;
    seq_alg_time_start = omp_get_wtime();
    Multiplication_seq();
    seq_alg_time_end = omp_get_wtime();
    seq_work_time = seq_alg_time_end - seq_alg_time_start;


    if (IsCorrect())// Сравниваем результаты
        cout << "Results sequence and parallel versions are indentical " << endl;
    else
        cout << "Results sequence and parallel versions are not indentical " << endl;

    cout << "OMP version of algorithm is worked: " << work_time << endl;
    cout << "Sequence version is worked:" << seq_work_time << endl;

    delete[] Res_pp;
    delete[] Res_seq;
    delete[] Vec;
    delete[] Matr;

    //system("pause");

    return 0;
}