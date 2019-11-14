#include <stdio.h>
#include <math.h>

void get_stats( double a[], int n, double *mean, double *std) {
    double sum = 0;
    double sum2 = 0;

    if (n == 0) { *mean = 0; *std = 0; return; }

    for ( int i = 0; i < n; i++) {
        sum += a[i];
        sum2 += a[i] * a[i];
    }

    *mean = sum / n;
    *std = sqrt(sum2 / n - (*mean) * (*mean));
} 

int main () {
    #define MAX 10
    printf("Please enter at most %d numbers, terminated by -1:\n", MAX);
    double nums[MAX];
    int n = 0;

    for ( n = 0; n < MAX; n++ ) {
        scanf("%lf", &nums[n]);

        if (nums[n] == -1) {
            break;
        }
    }

    double mean, std;
    get_stats(nums, n, &mean, &std);
    printf("Mean = %f, std = %f\n", mean, std);
}