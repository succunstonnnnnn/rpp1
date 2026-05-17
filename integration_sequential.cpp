#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

double f(double x) {
    return sin(x);
}

double RectangleMethod(double a, double b, int N) {
    double h = (b - a) / N;
    double sum = 0.0;
    int i;
    for (i = 0; i < N; i++) {
        double x = a + (i + 0.5) * h;
        sum += f(x);
    }
    return sum * h;
}

double TrapezoidMethod(double a, double b, int N) {
    double h = (b - a) / N;
    double sum = (f(a) + f(b)) / 2.0;
    int i;
    for (i = 1; i < N; i++) {
        double x = a + i * h;
        sum += f(x);
    }
    return sum * h;
}

double SimpsonMethod(double a, double b, int N) {
    double h = (b - a) / N;
    double sum = f(a) + f(b);
    int i;
    for (i = 1; i < N; i++) {
        double x = a + i * h;
        if (i % 2 == 0)
            sum += 2.0 * f(x);
        else
            sum += 4.0 * f(x);
    }
    return sum * h / 3.0;
}

int main(int argc, char* argv[]) {
    double a = 0.0;
    double b = M_PI;
    int N = 100000000;

    if (argc > 1) {
        N = atoi(argv[1]);
    }

    printf("Serial numerical integration program\n");
    printf("Function: f(x) = sin(x)\n");
    printf("Interval: [0, pi]\n");
    printf("Number of subintervals N = %d\n", N);
    printf("Exact value: 2.0\n\n");

    time_t start, finish;
    double duration;
    double result;

    start = clock();
    result = RectangleMethod(a, b, N);
    finish = clock();
    duration = (finish - start) / double(CLOCKS_PER_SEC);
    printf("Rectangle method:  %.15f\n", result);
    printf("Error:             %.2e\n", fabs(result - 2.0));
    printf("Time:              %f sec\n\n", duration);

    start = clock();
    result = TrapezoidMethod(a, b, N);
    finish = clock();
    duration = (finish - start) / double(CLOCKS_PER_SEC);
    printf("Trapezoid method:  %.15f\n", result);
    printf("Error:             %.2e\n", fabs(result - 2.0));
    printf("Time:              %f sec\n\n", duration);

    start = clock();
    result = SimpsonMethod(a, b, N);
    finish = clock();
    duration = (finish - start) / double(CLOCKS_PER_SEC);
    printf("Simpson method:    %.15f\n", result);
    printf("Error:             %.2e\n", fabs(result - 2.0));
    printf("Time:              %f sec\n", duration);

    return 0;
}