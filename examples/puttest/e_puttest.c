#include <e_bsp.h>
#include "e-lib.h"

int main()
{
    bsp_begin();

    int n = bsp_nprocs(); 
    int p = bsp_pid();

    const char* hmsg = "Hello world! BSP";

    char* a = (void*)0x7750;
    (*a) = hmsg[p];
    int regvar, kaasbroodje;
    bsp_push_reg(&regvar, sizeof(int));
    bsp_sync();
    bsp_sync();
    bsp_push_reg(&kaasbroodje, sizeof(int));
    bsp_sync();
    regvar=(int)registermap[n*0+p];
    if(p == 4) {
        int tmp=999;
        //bsp_hpput(5, &tmp, &regvar, 0, sizeof(int));
    }
    bsp_sync();
    
    int* po = (void*)0x7800;
    (*po) = regvar;

    bsp_end();
    return 0;
}
