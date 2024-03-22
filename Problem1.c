#include "mpi.h"
#include <stdio.h>

const int N = 100005;

int isPrime[100005];
int pre[100005];

void sieve() {
    for (int i = 2; i <= N; ++i) {
        isPrime[i] = 1;
    }
    isPrime[0] = 0, isPrime[1] = 0;
    for (int i = 2; i * i <= N; ++i) {
        if (isPrime[i]) {
            for (int j = i * i; j <= N; j += i) {
                isPrime[j] = 0;
            }
        }
    }
    for (int i = 2; i <= N; ++i) {
        pre[i] = pre[i - 1] + isPrime[i];
    }
}

int countPrimes(int l, int r) {
    return pre[r - 1] - pre[l - (l != 0)];
}

int main(int argc, char *argv[]) {
    int my_rank; /* rank of process */
    int p; /* number of process */
    MPI_Status status; /* return status for */

    /* Start up MPI */
    MPI_Init(&argc, &argv);
    /* Find out process rank */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    /* Find out number of process */
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    sieve();
    if (my_rank == 0) {
        int a, b;
        scanf("%d %d", &a, &b);
        int x = a;
        int r = (b - a) / (p - 1);
        int ans = 0;
        for (int i = 1; i < p; ++i) {
            // printf("Sending task to processor %d\n", i);
            int pair[2];
            pair[0] = x;
            if (i == p - 1) pair[1] = x + r;
            else pair[1] = x + r + (r % (p - 1));

            MPI_Send(pair, 2, MPI_INT, i, 0, MPI_COMM_WORLD);
            x += r;
        }
        char messages[100][100];
        for (int i = 1; i < p; ++i) {
            int curCnt;
            MPI_Recv(&curCnt, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            sprintf(messages[i], "Total Primes in P%d: %d\n", i, curCnt);
            ans += curCnt;
        }
        sprintf(messages[0], "Total number of primes between A and B [%d, %d]: %d\n", a, b, ans);

        for (int i = 0; i < p; ++i) {
            printf("%s", messages[i]);
        }

    } else {
        int pair[2];
        MPI_Recv(pair, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        int curCnt = countPrimes(pair[0], pair[1]);
        MPI_Send(&curCnt, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    /* shutdown MPI */
    MPI_Finalize();
    return 0;
}
