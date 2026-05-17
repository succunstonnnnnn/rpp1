#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

int ProcNum = 0;
int ProcRank = 0;

double f(double x) {
    return sin(x);
}

void ProcessInitialization(double& a, double& b, int& N, int argc, char* argv[]) {
    if (ProcRank == 0) {
        a = 0.0;
        b = M_PI;
        N = 100000000;
        if (argc > 1) {
            N = atoi(argv[1]);
        }
    }
    MPI_Bcast(&a, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&b, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
}

void ComputeRange(int N, int& StartIdx, int& BlockSize) {
    int RestElements = N;
    int i;
    StartIdx = 0;
    for (i = 0; i < ProcRank; i++) {
        int curBlock = RestElements / (ProcNum - i);
        StartIdx += curBlock;
        RestElements -= curBlock;
    }
    BlockSize = RestElements / (ProcNum - ProcRank);
}

double ParallelRectangle(double a, double b, int N, int StartIdx, int BlockSize) {
    double h = (b - a) / N;
    double sum = 0.0;
    int i;
    for (i = StartIdx; i < StartIdx + BlockSize; i++) {
        double x = a + (i + 0.5) * h;
        sum += f(x);
    }
    return sum * h;
}

double ParallelTrapezoid(double a, double b, int N, int StartIdx, int BlockSize, int LastProc) {
    double h = (b - a) / N;
    double sum = 0.0;
    int i;

    if (ProcRank == 0) {
        sum += f(a) / 2.0;
    }
    if (ProcRank == LastProc) {
        sum += f(b) / 2.0;
    }

    int loopStart = (ProcRank == 0) ? 1 : StartIdx;
    int loopEnd = StartIdx + BlockSize;

    for (i = loopStart; i < loopEnd; i++) {
        double x = a + i * h;
        sum += f(x);
    }
    return sum * h;
}

double ParallelSimpson(double a, double b, int N, int StartIdx, int BlockSize, int LastProc) {
    double h = (b - a) / N;
    double sum = 0.0;
    int i;

    if (ProcRank == 0) {
        sum += f(a);
    }
    if (ProcRank == LastProc) {
        sum += f(b);
    }

    int loopStart = (ProcRank == 0) ? 1 : StartIdx;
    int loopEnd = StartIdx + BlockSize;

    for (i = loopStart; i < loopEnd; i++) {
        double x = a + i * h;
        if (i % 2 == 0)
            sum += 2.0 * f(x);
        else
            sum += 4.0 * f(x);
    }
    return sum * h / 3.0;
}

double SerialRectangle(double a, double b, int N) {
    double h = (b - a) / N;
    double sum = 0.0;
    int i;
    for (i = 0; i < N; i++) {
        double x = a + (i + 0.5) * h;
        sum += f(x);
    }
    return sum * h;
}

void TestResult(double ParallelResult, double SerialResult, const char* methodName) {
    if (ProcRank == 0) {
        double diff = fabs(ParallelResult - SerialResult);
        double tolerance = fabs(SerialResult) * 1e-9;
        if (tolerance < 1e-9) tolerance = 1e-9;
        if (diff < tolerance) {
            printf("[%s] Parallel and serial results are identical.\n", methodName);
        } else {
            printf("[%s] Results differ! Serial=%.15f, Parallel=%.15f\n",
                   methodName, SerialResult, ParallelResult);
        }
    }
}

int main(int argc, char* argv[]) {
    double a, b;
    int N;
    int StartIdx, BlockSize;
    double LocalResult, GlobalResult;
    double Start, Finish, Duration;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

    int LastProc = ProcNum - 1;

    if (ProcRank == 0) {
        printf("Parallel numerical integration program\n");
        printf("Number of processes: %d\n", ProcNum);
    }

    ProcessInitialization(a, b, N, argc, argv);
    ComputeRange(N, StartIdx, BlockSize);

    if (ProcRank == 0) {
        printf("Function: f(x) = sin(x), Interval: [0, pi]\n");
        printf("Number of subintervals N = %d\n", N);
        printf("Exact value: 2.0\n\n");
    }

    // Rectangle method
    MPI_Barrier(MPI_COMM_WORLD);
    Start = MPI_Wtime();
    LocalResult = ParallelRectangle(a, b, N, StartIdx, BlockSize);
    MPI_Reduce(&LocalResult, &GlobalResult, 1, MPI_DOUBLE,
               MPI_SUM, 0, MPI_COMM_WORLD);
    Finish = MPI_Wtime();
    Duration = Finish - Start;

    if (ProcRank == 0) {
        printf("Rectangle method:  %.15f\n", GlobalResult);
        printf("Error:             %.2e\n", fabs(GlobalResult - 2.0));
        printf("Time:              %f sec\n", Duration);
    }
    TestResult(GlobalResult, SerialRectangle(a, b, N), "Rectangle");
    if (ProcRank == 0) printf("\n");

    // Trapezoid method
    MPI_Barrier(MPI_COMM_WORLD);
    Start = MPI_Wtime();
    LocalResult = ParallelTrapezoid(a, b, N, StartIdx, BlockSize, LastProc);
    MPI_Reduce(&LocalResult, &GlobalResult, 1, MPI_DOUBLE,
               MPI_SUM, 0, MPI_COMM_WORLD);
    Finish = MPI_Wtime();
    Duration = Finish - Start;

    if (ProcRank == 0) {
        printf("Trapezoid method:  %.15f\n", GlobalResult);
        printf("Error:             %.2e\n", fabs(GlobalResult - 2.0));
        printf("Time:              %f sec\n\n", Duration);
    }

    // Simpson method
    MPI_Barrier(MPI_COMM_WORLD);
    Start = MPI_Wtime();
    LocalResult = ParallelSimpson(a, b, N, StartIdx, BlockSize, LastProc);
    MPI_Reduce(&LocalResult, &GlobalResult, 1, MPI_DOUBLE,
               MPI_SUM, 0, MPI_COMM_WORLD);
    Finish = MPI_Wtime();
    Duration = Finish - Start;

    if (ProcRank == 0) {
        printf("Simpson method:    %.15f\n", GlobalResult);
        printf("Error:             %.2e\n", fabs(GlobalResult - 2.0));
        printf("Time:              %f sec\n", Duration);
    }

    MPI_Finalize();
    return 0;
}