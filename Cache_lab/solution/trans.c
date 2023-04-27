/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void transpose_32_32(int M, int N, int A[N][M], int B[M][N]);
void transpose_64_64(int M, int N, int A[N][M], int B[M][N]);
void transpose_61_67(int M, int N, int A[N][M], int B[M][N]);
void trans(int M, int N, int A[N][M], int B[M][N]);
/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if(M == 32 && N == 32)
    {
        transpose_32_32(M, N, A, B);
    }
    else if(M == 64 && N == 64)
    {
        transpose_64_64(M, N, A, B);
    }
    else if(M == 61 && N == 67)
    {
        transpose_61_67(M, N, A, B);
    }
    else
    {
        trans(M, N, A, B);
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

void transpose_32_32(int M, int N, int A[N][M], int B[M][N])
{
    for(int i = 0; i < 32; i += 8)
    {
        for(int j = 0; j < 32; j += 8)
        {
            for(int k = i; k < i + 8; ++k)
            {
                int a = A[k][j];
                int a1 = A[k][j + 1];
                int a2 = A[k][j + 2];
                int a3 = A[k][j + 3];
                int a4 = A[k][j + 4];
                int a5 = A[k][j + 5];
                int a6 = A[k][j + 6];
                int a7 = A[k][j + 7];

                B[j][k] = a;
                B[j + 1][k] = a1;
                B[j + 2][k] = a2;
                B[j + 3][k] = a3;
                B[j + 4][k] = a4;
                B[j + 5][k] = a5;
                B[j + 6][k] = a6;
                B[j + 7][k] = a7;
            }
        }
    }
}

void transpose_64_64(int M, int N, int A[N][M], int B[M][N])
{
    int a;
    int a1;
    int a2;
    int a3;
    int a4;
    int a5;
    int a6;
    int a7;
    
    for(int i = 0; i < N; i += 8)
    {
        for(int j = 0; j < M; j += 8)
        {
            for(int k = i; k < i + 4; ++k)
            {
                a = A[k][j];
                a1 = A[k][j + 1];
                a2 = A[k][j + 2];
                a3 = A[k][j + 3];
                a4 = A[k][j + 4];
                a5 = A[k][j + 5];
                a6 = A[k][j + 6];
                a7 = A[k][j + 7];

                B[j][k] = a;
                B[j + 1][k] = a1;
                B[j + 2][k] = a2;
                B[j + 3][k] = a3;
                B[j][k + 4] = a4;
                B[j + 1][k + 4] = a5;
                B[j + 2][k + 4] = a6;
                B[j + 3][k + 4] = a7;
            }
            for(int k = j; k < j + 4; ++k)
            {
                a = B[k][i + 4];
                a1 = B[k][i + 5];
                a2 = B[k][i + 6];
                a3 = B[k][i + 7];

                a4 = A[i + 4][k];
                a5 = A[i + 5][k];
                a6 = A[i + 6][k];
                a7 = A[i + 7][k];

                B[k][i + 4] = a4;
                B[k][i + 5] = a5;
                B[k][i + 6] = a6;
                B[k][i + 7] = a7;

                B[k + 4][i] = a;
                B[k + 4][i + 1] = a1;
                B[k + 4][i + 2] = a2;
                B[k + 4][i + 3] = a3;
            
            }
            for(int k = i + 4; k < i + 8; ++k)
            {
                a4 = A[k][j + 4];
                a5 = A[k][j + 5];
                a6 = A[k][j + 6];
                a7 = A[k][j + 7];

                B[j + 4][k] = a4;
                B[j + 5][k] = a5;
                B[j + 6][k] = a6;
                B[j + 7][k] = a7;
            }
        }
    }
}

void transpose_61_67(int M, int N, int A[N][M], int B[M][N])
{
    for(int i = 0; i < N; i += 16)
    {
        for(int j = 0; j < M; j += 16)
        {
            for(int k = i; k < i + 16 && k < N; ++k)
            {
                for(int s = j; s < j + 16 && s < M; ++s)
                {
                    B[s][k] = A[k][s];
                }
            }
        }
    }
}
/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

