#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void RandomDataInitialization(double* pArray, int Size) {
    int i;
    srand(1);
    for (i = 0; i < Size; i++) {
        pArray[i] = rand() / double(1000);
    }
}

void ProcessInitialization(double*& pArray, int& Size) {
    do {
        printf("\nEnter the size of the array: ");
        scanf("%d", &Size);
        printf("\nChosen array size = %d\n", Size);
        if (Size <= 0)
            printf("\nSize of array must be greater than 0!\n");
    } while (Size <= 0);

    pArray = new double[Size];
    RandomDataInitialization(pArray, Size);
}

double ResultCalculation(double* pArray, int Size) {
    double Sum = 0.0;
    int i;
    for (i = 0; i < Size; i++) {
        Sum += pArray[i];
    }
    return Sum;
}

void ProcessTermination(double* pArray) {
    delete[] pArray;
}

int main(int argc, char* argv[]) {
    double* pArray;
    int Size;
    double Sum;
    time_t start, finish;
    double duration;

    printf("Serial array sum calculation program\n");

    if (argc > 1) {
        Size = atoi(argv[1]);
        pArray = new double[Size];
        RandomDataInitialization(pArray, Size);
        printf("Array size = %d\n", Size);
    } else {
        ProcessInitialization(pArray, Size);
    }

    start = clock();
    Sum = ResultCalculation(pArray, Size);
    finish = clock();
    duration = (finish - start) / double(CLOCKS_PER_SEC);

    printf("\nSum of array elements = %f\n", Sum);
    printf("Time of execution: %f\n", duration);

    ProcessTermination(pArray);

    return 0;
}