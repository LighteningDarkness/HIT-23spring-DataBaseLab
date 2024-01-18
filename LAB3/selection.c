#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "extmem.h"
#include "extmem.c"
#define BASE_ADDR 1000
#define R_TUPLE 112
#define S_TUPLE 224
#define R_RES 1000
#define S_RES 2000
void LinearSearch(char relation,char field,int num,Buffer* buf)
{
    unsigned int res_addr=(relation=='R')?R_RES:S_RES;
    int tuple_counter=(relation=='R')?R_TUPLE:S_TUPLE;
    int res_counter=0;
    int offset=0;
    if ((relation=='R'&&field=='B')||(relation=='S'&&field=='D'))
    {
        offset=1;
    }
    unsigned int addr = relation* BASE_ADDR;
    unsigned int* res=(unsigned int*)getNewBlockInBuffer(buf);
    memset(res, 0, 64);
    unsigned int* in_block=NULL;
    int i=0;
    while (1)
    {
        in_block=(unsigned int*)readBlockFromDisk(addr+i,buf);
        int n=(tuple_counter>7)?7:tuple_counter;
        int j;
        //printf("%d\n",addr+i);
        for (j=0;j<n;j++)
        {
            //printf("%p\n",in_block+offset+2*j);
            if (*(in_block+offset+2*j)==num)
            {
                *(res+2*res_counter)=*(in_block+2*j);
                *(res+2*res_counter+1)=*(in_block+1+2*j);
                res_counter++;
                printf("%d\n",res_counter);
            }
            tuple_counter--;
            if (res_counter==7)
            {
                *(res+15)=(tuple_counter==0)?0:res_addr+1;
                writeBlockToDisk((unsigned char*)res,res_addr,buf);
                res_counter=0;
                memset(res, 0, 64);
                res_addr++;
            }
        }
        if (!tuple_counter)
        {
            *(res+15)=0;
            if (res_counter)
            {
                writeBlockToDisk((unsigned char*)res,res_addr,buf);
            }
            freeBlockInBuffer((unsigned char*)res,buf);
            break;
        }
        freeBlockInBuffer((unsigned char*)in_block,buf);
        i++;
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
    //createRelation('R',16,7,1,40,1,1000,&buf);
    //createRelation('S',32,7,20,60,1,1000,&buf);

    LinearSearch('R','A',40,&buf);
    LinearSearch('S','C',60,&buf);
    freeBuffer(&buf);
}