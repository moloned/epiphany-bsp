#include <host_bsp.h>

#include <stdlib.h>
#include <stdio.h>

#include "common.h"

#define DEBUG

// information on matrix and procs
char N = -1;
char M = -1;

// always choose multiple of 4 such that we dont have to worry
// about heterogeneous distributions, too much,
// which makes a lot of things much easier
char dim = 40;

// "local to global" index
int ltg(int* i, int* j, int l, int s, int t)
{
    (*i) = s + (l / (dim / N)) * N;
    (*j) = t + (l % (dim / M)) * M;
}

// "global to local" index
int gtl(int i, int j, int* l, int* s, int* t)
{
    (*s) = i % N;
    (*t) = j % M;
    (*l) = (i / M) * (dim / M) + (j / M);
}

int proc_id(int s, int t)
{
    return s * M + t;
}

int main(int argc, char **argv)
{
    // allocate and zero-initialize matrix
    float* mat = malloc(sizeof(float) * dim * dim);

    // construct the matrix
    int i = 0; 
    int j = 0;
    for(i = 0; i < dim; ++i) {
        for(j = 0; j < dim; ++j) {
            if(i > j) 
                mat[dim*i + j] = (float)i / j;
            else 
                mat[dim*i + j] = (float)j / i;
        }
    }

    // initialize the BSP system
    bsp_init("bin/e_lu_decomposition.srec", argc, argv);
    bsp_begin(bsp_nprocs());

    // distribute the matrix
    switch (bsp_nprocs()) {
        case 16:
            N = 4;
            M = 4;
            break;

        case 64:
            N = 8;
            M = 8;
            break;

        default:
            fprintf(stderr, "Unsupported processor count, please add values\
                    for N and M in the host program.");
            break;
    }

    printf("LUD: Writing info on procs and matrix \n");
    // Write M, N and dim to every processor such that they can figure out 
    // the (s,t) pair, and gtl / ltg functions
    for(i = 0; i < bsp_nprocs(); ++i) {
        co_write(i, &M, (off_t)LOC_M, sizeof(char));
        co_write(i, &N, (off_t)LOC_N, sizeof(char));
        co_write(i, &dim, (off_t)LOC_DIM, sizeof(char));
    }

    int s = 0;
    int t = 0;
    int l = 0;
    for (i = 0; i < dim; ++i) {
        for (j = 0; j < dim; ++j) {
            gtl(i, j, &l, &s, &t);
            co_write(proc_id(s, t),
                    &mat[dim*i + j],
                    LOC_MATRIX + sizeof(float) * l,
                    sizeof(float));
        }
    }

    // test global to local and local to global function for random processor
#ifdef DEBUG
    s = 2;
    t = 3;
    printf("i.e. (s,t) = (2,3): \n");
    for (l = 0; l < (dim * dim) / bsp_nprocs(); ++l) {
            ltg(&i, &j, l, s, t);
            float val;
            co_read(proc_id(s, t),
                    LOC_MATRIX + sizeof(float) * l,
                    &val,
                    sizeof(float));
            printf("%i \t (%i, %i) \t %f \t 0x%x\n",
                    l, i, j, val,
                    LOC_MATRIX + sizeof(float) * l);
    }
#endif

    ebsp_spmd();

    printf("----------------------------: \n");
    printf("Matrix: \n");
    for (i = 0; i < dim; ++i) {
        for (j = 0; j < dim; ++j) {
            printf("%f ", mat[dim * i + j]);
        }
        printf("\n");
    }

    printf("----------------------------: \n");
    printf("LU decomposition: \n");

    for (s = 0; s < N; ++s) {
        for (t = 0; t < M; ++t) {
            for (l = 0; l < (dim * dim) / bsp_nprocs(); ++l) {
                    ltg(&i, &j, l, s, t);
                    co_read(proc_id(s, t),
                            LOC_MATRIX + sizeof(float) * l,
                            &mat[dim*i + j], sizeof(float));
            }
        }
    }

    for (i = 0; i < dim; ++i) {
        for (j = 0; j < dim; ++j) {
            printf("%f ", mat[dim * i + j]);
        }
        printf("\n");
    }

    printf("----------------------------: \n");

    // finalize
    bsp_end();

    return 0;
}
