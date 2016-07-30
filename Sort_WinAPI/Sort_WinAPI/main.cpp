#include <iostream>
#include <ctime>
#include <omp.h>
#include <windows.h>

using namespace std;

typedef unsigned int uint;

int* mas;
int* mas_seq_bubble; // Р-т последовательной версии
int* mas_seq_qsort;
int* mas_pp_bubble; // Р-т параллельной версии
int* mas_seq_merge;
int* mas_pp_merge;

int** matr_merge_pp; // Заранее выделяем память для потоков, которые будут сортировать локальные буферы (чтобы не тратить время на выделение и освобождение памяти)
int* mas_merge_seq;

HANDLE* array_of_threads;

uint Size;
uint num_of_threads;
uint Work_for_threads;

/* Механизм синхронизации потоков */
volatile bool* ThreadIsFinish;

// Создать и проинициализировать массив
int* Create_and_init_arr(int size_arr)
{
    if (size_arr < 1)
        return NULL;

    int* arr;

    arr = new int[size_arr];
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
            arr[i] = rand() % 100 - 50; // [-50, 51] - действительные числа

    return arr;
}

// Отобразить массив
void Show_arr(int* arr, int size_arr)
{
    if (arr == NULL || size_arr < 1)
        return;

//    cout << "Vector: " << endl;
    for (int i = 0; i < size_arr; i++)
        cout << arr[i] << " ";

    cout << endl;
}

inline void Swap(int& a, int& b)
{
    int c = a;
    a = b;
    b = c;
}

bool IsCorrect(int* arr_1, int* arr_2)
{
    for (int i = 0; i < Size; i++)
        if (arr_1[i] != arr_2[i])
            return false;

    return true;
}

bool PowerOfTwo(int value)
{
    if (value <= 0)
        return false;

    if (value == 1)
        return true;

    while ((value % 2) == 0)
    {

        if ((value /= 2) == 1)
            return true;
    }

    return false;

}

// Пузырьковая сортировка методом чет-нечетной перестановки (последовательная версия)
void BubbleSort_seq(int* arr)
{
    for (int i = 0; i < Size; i++)
        if (i % 2 == 1) // нечетная итерация
        {
            for (int j = 0; j < Size / 2 - 1; j++)
                if (arr[2 * j + 1] > arr[2 * j + 2])
                    Swap(arr[2 * j + 1], arr[2 * j + 2]);

            if (Size % 2 == 1) // если нечетный размер, то нужно сравнить последнюю пару
                if (arr[Size - 2] > arr[Size - 1])
                    Swap(arr[Size - 2], arr[Size - 1]);
        }
        else // четная итерация
            for (int j = 0; j < Size / 2; j++)
                if (arr[2 * j] > arr[2 * j + 1])
                    Swap(arr[2 * j], arr[2 * j + 1]);
}


void QSortReq_seq(int lf, int rt)
{
    int i = lf;
    int j = rt;

    int x = mas_seq_qsort[lf];

    while (i <= j)
    {
        while (mas_seq_qsort[i] < x)
            i++;

        while (mas_seq_qsort[j] > x)
            j--;

        if (i <= j)
        {
            Swap(mas_seq_qsort[i], mas_seq_qsort[j]);
            i++;
            j--;
        }
    }

    if (i < rt)
        QSortReq_seq(i, rt);

    if (j > lf)
        QSortReq_seq(lf, j);
}

// Быстрая сортировка (последовательная версия)
void QSort_seq()
{
    QSortReq_seq(0, Size - 1);
}

void Merge_seq(int left, int s, int right, int* mas)
{
    int tmp1 = left;
    int tmp2 = s + 1;
    int tmp3 = 0;

    while (tmp1 <= s && tmp2 <= right)
    {
        if (mas[tmp1] < mas[tmp2])
            mas_merge_seq[tmp3++] = mas[tmp1++];
        else
            mas_merge_seq[tmp3++] = mas[tmp2++];
    }

    while (tmp1 <= s)
        mas_merge_seq[tmp3++] = mas[tmp1++];

    while (tmp2 <= right)
        mas_merge_seq[tmp3++] = mas[tmp2++];

    for (tmp3 = 0; tmp3 < right - left + 1; tmp3++)
        mas[left + tmp3] = mas_merge_seq[tmp3];
}

void Merge_pp(int index_thread, int left, int s, int right, int* mas)
{
    int tmp1 = left;
    int tmp2 = s + 1;
    int tmp3 = 0;

    while (tmp1 <= s && tmp2 <= right)
    {
        if (mas[tmp1] < mas[tmp2])
            matr_merge_pp[index_thread][tmp3++] = mas[tmp1++];
        else
            matr_merge_pp[index_thread][tmp3++] = mas[tmp2++];
    }

    while (tmp1 <= s)
        matr_merge_pp[index_thread][tmp3++] = mas[tmp1++];

    while (tmp2 <= right)
        matr_merge_pp[index_thread][tmp3++] = mas[tmp2++];

    for (tmp3 = 0; tmp3 < right - left + 1; tmp3++)
        mas[left + tmp3] = matr_merge_pp[index_thread][tmp3];
}

void MergeSort_seq(int left, int right, int* mas)
{
    int s;
    if (left < right)
    {
        s = (left + right) / 2;
        MergeSort_seq(left, s, mas);
        MergeSort_seq(s + 1, right, mas);
        Merge_seq(left, s, right, mas);
    }
}

void MergeSort_SortLocalBuffer_pp(int index_thread, int left, int right, int* mas)
{
    int s;
    if (left < right)
    {
        s = (left + right) / 2;
        MergeSort_SortLocalBuffer_pp(index_thread, left, s, mas);
        MergeSort_SortLocalBuffer_pp(index_thread, s + 1, right, mas);
        Merge_pp(index_thread, left, s, right, mas);
    }
}

void MergeSplit_right(int index_thread, int start_pos, int& end_pos, int work, int i)
{
    int start_pos_right = (index_thread + (int)pow(2, i)) * work;
    int end_pos_right = start_pos_right + work * pow(2, i) - 1;

    if (index_thread + (int)pow(2, i + 1) == num_of_threads)
        end_pos_right = Size - 1;

    int tmp_1 = start_pos;
    int tmp_2 = start_pos_right;
    int tmp_3 = 0;

    int size_merge_mas = end_pos_right - start_pos + 1;

    int* merge_mas = new int[size_merge_mas];

    /* Синхронизация */
    while (!ThreadIsFinish[index_thread + (int)pow(2, i)]); // Ожидаем поток, с которым будем сливать массивы

    /* MERGE */

    while (tmp_1 < end_pos && tmp_2 <= end_pos_right)
        if (mas_pp_bubble[tmp_1] < mas_pp_bubble[tmp_2])
            merge_mas[tmp_3++] = mas_pp_bubble[tmp_1++];
        else
            merge_mas[tmp_3++] = mas_pp_bubble[tmp_2++];

    while (tmp_1 < end_pos)
        merge_mas[tmp_3++] = mas_pp_bubble[tmp_1++];

    while (tmp_2 <= end_pos_right)
        merge_mas[tmp_3++] = mas_pp_bubble[tmp_2++];

    /* SPLIT */

    for (tmp_3 = 0; tmp_3 < size_merge_mas; tmp_3++)
        mas_pp_bubble[start_pos + tmp_3] = merge_mas[tmp_3];

    end_pos = end_pos_right + 1; // У этого потока массив увеличился до размера size_merge_mas => необходимо изменить end_pos

    if (index_thread % (int)pow(2, i + 1) == 0 && index_thread % (int)pow(2, i + 2) != 0)
        ThreadIsFinish[index_thread] = true; // То этот поток больше работать не будет, он выполнил свою задачу

    delete[] merge_mas;
}


void MergeSplit_right_merge(int index_thread, int start_pos, int& end_pos, int work, int i)
{
    int start_pos_right = (index_thread + (int)pow(2, i)) * work;
    int end_pos_right = start_pos_right + work * pow(2, i) - 1;

    if (index_thread + (int)pow(2, i + 1) == num_of_threads)
        end_pos_right = Size - 1;

    int tmp_1 = start_pos;
    int tmp_2 = start_pos_right;
    int tmp_3 = 0;

    int size_merge_mas = end_pos_right - start_pos + 1;

    int* merge_mas = new int[size_merge_mas];

    /* Синхронизация */
    while (!ThreadIsFinish[index_thread + (int)pow(2, i)]); // Ожидаем поток, с которым будем сливать массивы

    /* MERGE */

    while (tmp_1 < end_pos && tmp_2 <= end_pos_right)
        if (mas_pp_merge[tmp_1] < mas_pp_merge[tmp_2])
            merge_mas[tmp_3++] = mas_pp_merge[tmp_1++];
        else
            merge_mas[tmp_3++] = mas_pp_merge[tmp_2++];

    while (tmp_1 < end_pos)
        merge_mas[tmp_3++] = mas_pp_merge[tmp_1++];

    while (tmp_2 <= end_pos_right)
        merge_mas[tmp_3++] = mas_pp_merge[tmp_2++];

    /* SPLIT */

    for (tmp_3 = 0; tmp_3 < size_merge_mas; tmp_3++)
        mas_pp_merge[start_pos + tmp_3] = merge_mas[tmp_3];

    end_pos = end_pos_right + 1; // У этого потока массив увеличился до размера size_merge_mas => необходимо изменить end_pos

    if (index_thread % (int)pow(2, i + 1) == 0 && index_thread % (int)pow(2, i + 2) != 0)
        ThreadIsFinish[index_thread] = true; // То этот поток больше работать не будет, он выполнил свою задачу
}

// Пузырьковая сортировка методом чет-нечетной перестановки (параллельная версия)
DWORD WINAPI BubbleSort_pp(void* thr_param)
{
    int index_thread = *((int*)(thr_param));
    int work = Size / num_of_threads;
    int start_pos = work * index_thread;
    int end_pos = start_pos + work;
    int exponent = log2(num_of_threads);

    if (index_thread == num_of_threads - 1)
        end_pos = Size;

    for (int i = 0; i < end_pos - start_pos; i++)
        for (int j = start_pos; j < end_pos - i - 1; j++)
            if (mas_pp_bubble[j] > mas_pp_bubble[j + 1])
                Swap(mas_pp_bubble[j], mas_pp_bubble[j + 1]);

    if (index_thread == num_of_threads - 1)
        end_pos = Size - 1;

    if (index_thread % 2 != 0)
        ThreadIsFinish[index_thread] = true;

    for (int i = 0; i < exponent; i++)
        if (index_thread % (int)pow(2, i + 1) == 0)
            MergeSplit_right(index_thread, start_pos, end_pos, work, i);


    return 0;
}

DWORD WINAPI MergeSort_pp(void* thr_param)
{
    int index_thread = *((int*)(thr_param));
    int work = Size / num_of_threads;
    int start_pos = work * index_thread;
    int end_pos = start_pos + work;
    int exponent = log2(num_of_threads);

    if (index_thread == num_of_threads - 1)
        end_pos = Size;

    MergeSort_SortLocalBuffer_pp(index_thread, start_pos, end_pos - 1, mas_pp_merge);

    if (index_thread == num_of_threads - 1)
        end_pos = Size - 1;

    if (index_thread % 2 != 0)
        ThreadIsFinish[index_thread] = true;

    for (int i = 0; i < exponent; i++)
    {
        if (index_thread % (int)pow(2, i + 1) == 0)
        {
            MergeSplit_right_merge(index_thread, start_pos, end_pos, work, i);
        }
    }

    return 0;
}

void main(int argc, char* argv[])
{
    DWORD* array_id_of_threads;
    int* index_thread; // индексы потоков

    double pp_alg_time_start_bubble = 0.0;
    double pp_alg_time_end_bubble = 0.0;
    double pp_work_time_bubble = 0.0;
    double pp_alg_time_start_qsort = 0.0;
    double pp_alg_time_end_qsort = 0.0;
    double pp_work_time_qsort = 0.0;

    double seq_alg_time_start_bubble = 0.0;
    double seq_alg_time_end_bubble = 0.0;
    double seq_work_time_bubble = 0.0;
    double seq_alg_time_start_qsort = 0.0;
    double seq_alg_time_end_qsort = 0.0;
    double seq_work_time_qsort = 0.0;

    double seq_alg_time_start_merge = 0.0;
    double seq_alg_time_end_merge = 0.0;
    double seq_work_time_merge = 0.0;
    double pp_alg_time_start_merge = 0.0;
    double pp_alg_time_end_merge = 0.0;
    double pp_work_time_merge = 0.0;

    int Show = 0;
    bool IsEnd = false; // Если true, то выход из меню
    int num_of_operation = 0;

    cout << "Enter num of threads (this value must be power of 2):" << endl;
    cin >> num_of_threads;
    cout << "Enter Size of array:" << endl;
    cin >> Size;
    /*num_of_threads = atoi(argv[1]);
    Size = atoi(argv[2]);*/

    mas = Create_and_init_arr(Size);

    ThreadIsFinish = new bool[num_of_threads];

    array_id_of_threads = new DWORD[num_of_threads];
    array_of_threads = new HANDLE[num_of_threads];
    index_thread = new int[num_of_threads];

    for (int i = 0; i < num_of_threads; i++)
    {
        index_thread[i] = i;
        ThreadIsFinish[i] = false;
    }

    while (!IsEnd)
    {
        cout << " Enter 1 to show created array " << endl;
        cout << " Enter 2 to use BubbleSort (sequence version) " << endl;
        cout << " Enter 3 to use BubbleSort (parallel version) " << endl;
        cout << " Enter 4 to use MergeSort (sequence version) " << endl;
        cout << " Enter 5 to use MergeSort (parallel version) " << endl;
        cout << " Enter 0 to exit " << endl;
        
        cin >> num_of_operation;

        switch (num_of_operation)
        {

        case 1:

            Show_arr(mas, Size);

            break;

        case 2:
            mas_seq_bubble = new int[Size];

            for (int i = 0; i < Size; i++)
                mas_seq_bubble[i] = mas[i];

            seq_alg_time_start_bubble = omp_get_wtime();
            BubbleSort_seq(mas_seq_bubble);
            seq_alg_time_end_bubble = omp_get_wtime();
            seq_work_time_bubble = seq_alg_time_end_bubble - seq_alg_time_start_bubble;

            cout << "Show Result of BubbleSort_seq ? <1 - yes, 0 - no> " << endl;
            cin >> Show;
            if (Show)
            {
                cout << "Result of BubbleSort_seq version: " << endl;
                Show_arr(mas_seq_bubble, Size);
            }

            cout << "BubbleSort_seq is worked: " << seq_work_time_bubble << "seconds" << endl;

            delete[] mas_seq_bubble;

            break;

        case 3:
            if (PowerOfTwo(num_of_threads))
            {
                mas_seq_qsort = new int[Size];
                mas_pp_bubble = new int[Size];



                for (int i = 0; i < Size; i++)
                {
                    mas_seq_qsort[i] = mas[i];
                    mas_pp_bubble[i] = mas[i];
                }

                pp_alg_time_start_bubble = omp_get_wtime();
                for (int i = 0; i < num_of_threads; i++)
                    array_of_threads[i] = CreateThread(NULL, 0, BubbleSort_pp, &index_thread[i], 0, &array_id_of_threads[i]);
                WaitForMultipleObjects(num_of_threads, array_of_threads, true, INFINITE); // Ждем завершения работы всех потоков

                pp_alg_time_end_bubble = omp_get_wtime();
                pp_work_time_bubble = pp_alg_time_end_bubble - pp_alg_time_start_bubble;

                // Удалить дескрипторы потоков
                for (int i = 0; i < num_of_threads; i++)
                    CloseHandle(array_of_threads[i]);

                cout << "Show Result of BubbleSort_pp ? <1 - yes, 0 - no> " << endl;
                cin >> Show;
                if (Show)
                {
                    cout << "Result of BubbleSort_pp version: " << endl;
                    Show_arr(mas_pp_bubble, Size);
                }

                cout << "BubbleSort_pp is worked: " << pp_work_time_bubble << "seconds" << endl;

                QSort_seq();
                if (IsCorrect(mas_pp_bubble, mas_seq_qsort))
                    cout << " Result of parallel version (BubbleSort_pp) is correct " << endl;
                else
                    cout << " Result of parallel version (BubbleSort_pp) is not correct " << endl;
                for (int i = 0; i < num_of_threads; i++)
                    ThreadIsFinish[i] = false;

                delete[] mas_pp_bubble;
                delete[] mas_seq_qsort;

            }
            else
                cout << "Number of threads is not a power of 2" << endl;

            break;

        case 4:

            mas_seq_merge = new int[Size];
            mas_merge_seq = new int[Size];

            for (int i = 0; i < Size; i++)
                mas_seq_merge[i] = mas[i];

            seq_alg_time_start_merge = omp_get_wtime();
            MergeSort_seq(0, Size - 1, mas_seq_merge);
            seq_alg_time_end_merge = omp_get_wtime();
            seq_work_time_merge = seq_alg_time_end_merge - seq_alg_time_start_merge;

            cout << "Show Result of MergeSort_seq ? <1 - yes, 0 - no> " << endl;
            cin >> Show;

            if (Show)
            {
                cout << "Result of MergeSort_seq version: " << endl;
                Show_arr(mas_seq_merge, Size);
            }

            cout << "MergeSort_seq is worked: " << seq_work_time_merge << "seconds" << endl;

            delete[] mas_merge_seq;
            delete[] mas_seq_merge;

            break;

        case 5:

            if (PowerOfTwo(num_of_threads))
            {
                mas_pp_merge = new int[Size];
                mas_seq_qsort = new int[Size];

                matr_merge_pp = new int*[num_of_threads];

                for (int i = 0; i < num_of_threads; i++)
                    if (i == num_of_threads - 1)
                        matr_merge_pp[i] = new int[Size / num_of_threads + Size % num_of_threads];
                    else
                        matr_merge_pp[i] = new int[Size / num_of_threads];

                for (int i = 0; i < Size; i++)
                {
                    mas_seq_qsort[i] = mas[i];
                    mas_pp_merge[i] = mas[i];
                }

                pp_alg_time_start_merge = omp_get_wtime();

                for (int i = 0; i < num_of_threads; i++)
                    array_of_threads[i] = CreateThread(NULL, 0, MergeSort_pp, &index_thread[i], 0, &array_id_of_threads[i]);

                WaitForMultipleObjects(num_of_threads, array_of_threads, true, INFINITE); // Ждем завершения работы всех потоков

                pp_alg_time_end_merge = omp_get_wtime();
                pp_work_time_merge = pp_alg_time_end_merge - pp_alg_time_start_merge;

                // Удалить дескрипторы потоков
                for (int i = 0; i < num_of_threads; i++)
                    CloseHandle(array_of_threads[i]);

                cout << "Show Result of MergeSort_pp ? <1 - yes, 0 - no> " << endl;
                cin >> Show;
                if (Show)
                {
                    cout << "Result of MergeSort_pp version: " << endl;
                    Show_arr(mas_pp_merge, Size);
                }

                cout << "MergeSort_pp is worked: " << pp_work_time_merge << "seconds" << endl;

                QSort_seq();
                if (IsCorrect(mas_pp_merge, mas_seq_qsort))
                    cout << " Result of parallel version (MergeSort_pp) is correct " << endl;
                else
                    cout << " Result of parallel version (MergeSort_pp) is not correct " << endl;

                for (int i = 0; i < num_of_threads; i++)
                    ThreadIsFinish[i] = false;



                for (int i = num_of_threads - 1; i >= 0; i--)
                    delete[] matr_merge_pp[i];

                delete[] matr_merge_pp;
                delete[] mas_pp_merge;
                delete[] mas_seq_qsort;

            }
            else
                cout << "Number of threads is not a power of 2" << endl;

            break;

        case 0:

            IsEnd = true;

            break;

        default: continue;

        }

    }
    

    delete[] mas;
    delete[] array_id_of_threads;
    delete[] array_of_threads;
    delete[] index_thread;

    system("pause");
}