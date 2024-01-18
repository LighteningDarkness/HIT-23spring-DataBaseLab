#include <stdlib.h>
#include <stdio.h>
#include "extmem.h"
#include "extmem.c"

#define NJL_ADDR 4000
#define R_BLOCK 16
#define S_BLOCK 32
#define BASE_ADDR 1000
void NJL_join(Buffer* buf)
{
    unsigned int* Rbuf[6];
    unsigned int* res=(unsigned int*)getNewBlockInBuffer(buf);
    memset(res,0,64);
    *((unsigned char*)res-1)=1;
    unsigned int* Sbuf;
    int addr1='R'*BASE_ADDR;
    int addr2='S'*BASE_ADDR;
    int i,j,k,m,n;
    int res_counter=0;
    int res_addr=NJL_ADDR;
    for (i=0;i<R_BLOCK/6;i++)
    {
        for (j=0;j<6;j++)
        {
            Rbuf[j]=(unsigned int*)readBlockFromDisk(addr1+6*i+j,buf);
        }
        for (j=0;j<S_BLOCK;j++)
        {
            Sbuf=(unsigned int*)readBlockFromDisk(addr2+j,buf);
            for (k=0;k<7;k++)
            {
                for (m=0;m<6;m++)
                {
                    for (n=0;n<7;n++)
                    {
                        if (*(Sbuf+2*k)==*(Rbuf[m]+2*n))
                        {
                            *(res+4*res_counter)=*(Rbuf[m]+2*n);
                            *(res+4*res_counter+1)=*(Rbuf[m]+2*n+1);
                            *(res+4*res_counter+2)=*(Sbuf+2*k);
                            *(res+4*res_counter+3)=*(Sbuf+2*k+1);
                            res_counter++;
                            if (res_counter==3)
                            {
                                *(res+15)=res_addr+1;
                                writeBlockToDisk((unsigned char*)res,res_addr,buf);
                                memset(res,0,64);
                                *((unsigned char*)res-1)=1;
                                res_counter=0;
                                res_addr++;
                            }
                        }
                    }
                }
            }
            freeBlockInBuffer(Sbuf,buf);
        }
        for (j=0;j<6;j++)
        {
            freeBlockInBuffer(Rbuf[j],buf);
        }
    }
    int r=R_BLOCK%6;
    for (j=0;j<r;j++)
    {
        Rbuf[j]=(unsigned int*)readBlockFromDisk(addr1+6*i+j,buf);
    }
    for (j=0;j<S_BLOCK;j++)
    {
        Sbuf=(unsigned int*)readBlockFromDisk(addr2+j,buf);
        for (k=0;k<7;k++)
        {
            for (m=0;m<r;m++)
            {
                for (n=0;n<7;n++)
                {
                    if (*(Sbuf+2*k)==*(Rbuf[m]+2*n))
                    {
                        *(res+4*res_counter)=*(Rbuf[m]+2*n);
                        *(res+4*res_counter+1)=*(Rbuf[m]+2*n+1);
                        *(res+4*res_counter+2)=*(Sbuf+2*k);
                        *(res+4*res_counter+3)=*(Sbuf+2*k+1);
                        res_counter++;
                        if (res_counter==3)
                        {
                            *(res+15)=res_addr+1;
                            if (j==S_BLOCK-1&&k==6&&m==r-1&&n==6)
                            {
                                *(res+15)=0;
                            }
                            writeBlockToDisk((unsigned char*)res,res_addr,buf);
                            memset(res,0,64);
                            *((unsigned char*)res-1)=1;
                            res_counter=0;
                            res_addr++;
                        }
                    }
                }
            }
        }
        freeBlockInBuffer(Sbuf,buf);
    }
    for (j=0;j<r;j++)
    {
        freeBlockInBuffer(Rbuf[j],buf);
    }
    if (res_counter)
    {
        *(res+15)=0;
        writeBlockToDisk((unsigned char*)res,res_addr,buf);
        res_addr++;
    }
    res=(unsigned int*)readBlockFromDisk(res_addr-1,buf);
    *(res+15)=0;
    writeBlockToDisk((unsigned char*)res,res_addr-1,buf);
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

    // LinearSearch('R','A',40,&buf);
    // LinearSearch('S','C',60,&buf);

    //mergesort('R','A',&buf);
    //project('R','A',&buf);
    //mergelink('R','A','S','C',&buf);
    NJL_join(&buf);
    freeBuffer(&buf);
}