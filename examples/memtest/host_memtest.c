#include <host_bsp.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    // initialize the BSP system
    bsp_init("bin/e_memtest.srec", argc, argv);

    // initialize the epiphany system, and load the e-program
    bsp_begin(bsp_nprocs());

    // run the SPMD on the e-cores
    spmd_epiphany();

    // read messages
    int pid = 0;
    for(pid = 0; pid < bsp_nprocs(); pid++) {
        char msg;
        co_read(pid, (off_t)0x6000, &msg, 1);
        printf("%i: %c\n", pid, msg);

        void* loc[16];
        co_read(pid, (off_t)REGISTERMAP_ADDRESS, &loc, 64);
    }

    // finalize
    bsp_end();

    return 0;
}
