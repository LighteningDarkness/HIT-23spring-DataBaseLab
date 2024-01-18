#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "extmem.h"
#include "extmem.c"
#define BASE_ADDR 1000
void createRelation(char name,int num_1,int num_2,int low_1,int up_1,int low_2,int up_2,Buffer* buf)
{
    int r[up_1-low_1+1][up_2-low_2+1];
    unsigned int addr = name * BASE_ADDR;
    int i1,i2;
    for (int i = 0; i < num_1*num_2; i+=7) {
        
    
        unsigned char* pblock = getNewBlockInBuffer(buf);
        //初始化块
        memset(pblock, 0, 64);
        unsigned int *ptr = (unsigned int *) pblock;
        for (int j = 0; j < 7; j++) {
            srand((unsigned)time(NULL));
            i1=rand() % (up_1-low_1+1);
            i2=rand() % (up_2-low_2+1);
            while (r[i1][i2])
            {
                srand((unsigned)time(NULL));
                i1=rand() % (up_1-low_1+1);
                i2=rand() % (up_2-low_2+1);
            }
            r[i1][i2]=1;
            *ptr = i1 + low_1;
            *(ptr + 1) = i2 + low_2;
            printf("%d,%d\n",*(ptr),*(ptr+1));
            //元组大小为8
            ptr += 2;
        }
        if (i+7<num_1*num_2)
        {
            *(ptr + 1) = addr + 1;
        }
        
        writeBlockToDisk(pblock, addr, buf);
        addr++;
    }
}
int main()
{
    Buffer buf; /* A buffer */
    unsigned char *blk; /* A pointer to a block */
    int i = 0;

    /* Initialize the buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    createRelation('R',16,7,1,40,1,1000,&buf);
    createRelation('S',32,7,20,60,1,1000,&buf);
}