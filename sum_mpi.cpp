#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int ProcNum = 0;
int ProcRank = 0;

void RandomDataInitialization(double* pArray, int Size) {
    int i;
    srand(1);
    for (i = 0; i < Size; i++) {
        pArray[i] = rand() / double(1000);
    }
}

void ProcessInitialization(double*& pArray, double*& pProcArray,
                           int& Size, int& BlockSize, int argc, char* argv[]) {
    int RestElements;
    int i;

    setvbuf(stdout, 0, _IONBF, 0);

    if (ProcRank == 0) {
        if (argc > 1) {
            Size = atoi(argv[1]);
        } else {
            do {
                printf("\nEnter the size of the array: ");
                scanf("%d", &Size);
                if (Size < ProcNum) {
                    printf("Size of the array must be greater than "
                           "number of processes! \n");
                }
            } while (Size < ProcNum);
        }
    }

    MPI_Bcast(&Size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    RestElements = Size;
    for (i = 0; i < ProcRank; i++) {
        RestElements = RestElements - RestElements / (ProcNum - i);
    }
    BlockSize = RestElements / (ProcNum - ProcRank);

    pProcArray = new double[BlockSize];

    if (ProcRank == 0) {
        pArray = new double[Size];
        RandomDataInitialization(pArray, Size);
    }
}

void DataDistribution(double* pArray, double* pProcArray,
                      int Size, int BlockSize) {
    int* pSendNum;
    int* pSendInd;
    int RestElements = Size;
    int i;

    pSendInd = new int[ProcNum];
    pSendNum = new int[ProcNum];

    pSendNum[0] = Size / ProcNum;
    pSendInd[0] = 0;
    for (i = 1; i < ProcNum; i++) {
        RestElements -= pSendNum[i - 1];
        pSendNum[i] = RestElements / (ProcNum - i);
        pSendInd[i] = pSendInd[i - 1] + pSendNum[i - 1];
    }

    MPI_Scatterv(pArray, pSendNum, pSendInd, MPI_DOUBLE,
                 pProcArray, BlockSize, MPI_DOUBLE,
                 0, MPI_COMM_WORLD);

    delete[] pSendNum;
    delete[] pSendInd;
}

double ParallelResultCalculation(double* pProcArray, int BlockSize) {
    double ProcSum = 0.0;
    int i;
    for (i = 0; i < BlockSize; i++) {
        ProcSum += pProcArray[i];
    }
    return ProcSum;
}

double SerialResultCalculation(double* pArray, int Size) {
    double Sum = 0.0;
    int i;
    for (i = 0; i < Size; i++) {
        Sum += pArray[i];
    }
    return Sum;
}

void TestResult(double* pArray, double ParallelSum, int Size) {
    if (ProcRank == 0) {
        double SerialSum = SerialResultCalculation(pArray, Size);
        double diff = ParallelSum - SerialSum;
        if (diff < 0) diff = -diff;
        double absSum = SerialSum < 0 ? -SerialSum : SerialSum;
        double tolerance = absSum * 1e-9;
        if (tolerance < 1e-6) tolerance = 1e-6;
        if (diff < tolerance) {
            printf("\nThe results of serial and parallel algorithms are identical.\n");
        } else {
            printf("\nThe results of serial and parallel algorithms are NOT identical!\n");
            printf("Serial sum   = %f\n", SerialSum);
            printf("Parallel sum = %f\n", ParallelSum);
        }
    }
}

void ProcessTermination(double* pArray, double* pProcArray) {
    if (ProcRank == 0) {
        delete[] pArray;
    }
    delete[] pProcArray;
}

int main(int argc, char* argv[]) {
    double* pArray = nullptr;
    double* pProcArray = nullptr;
    int Size;
    int BlockSize;
    double ProcSum;
    double TotalSum = 0.0;
    double Start, Finish, Duration;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

    if (ProcRank == 0) {
        printf("Parallel array sum calculation program\n");
        printf("Number of processes: %d\n", ProcNum);
    }

    ProcessInitialization(pArray, pProcArray, Size, BlockSize, argc, argv);

    if (ProcRank == 0) {
        printf("Array size = %d\n", Size);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    Start = MPI_Wtime();

    DataDistribution(pArray, pProcArray, Size, BlockSize);

    ProcSum = ParallelResultCalculation(pProcArray, BlockSize);

    MPI_Reduce(&ProcSum, &TotalSum, 1, MPI_DOUBLE,
               MPI_SUM, 0, MPI_COMM_WORLD);

    Finish = MPI_Wtime();
    Duration = Finish - Start;

    TestResult(pArray, TotalSum, Size);

    if (ProcRank == 0) {
        printf("\nSum of array elements = %f\n", TotalSum);
        printf("Time of execution: %f\n", Duration);
    }

    ProcessTermination(pArray, pProcArray);
    MPI_Finalize();

    return 0;
}