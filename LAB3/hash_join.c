#include <stdlib.h>
#include <stdio.h>
#include "extmem.h"
#include "extmem.c"

#define R_HASH_ADDR 100
#define S_HASH_ADDR 200
#define HASH_JOIN_ADDR 90000
#define BASE_ADDR 1000
#define R_BLOCK 16
#define S_BLOCK 32
#define N 6
int hashFunc(int x)
{
    return x%7;
}
int R_counter[7];
int S_counter[7];
void hashBlock(char relation,char field,Buffer* buf)
{
    int *blk_counter=(relation=='R')?R_counter:S_counter;
    int blk=(relation=='R')?R_BLOCK:S_BLOCK;
    int offset=0;
    if ((relation=='R'&&field=='B')||(relation=='S'&&field=='D'))
    {
        offset=1;
    }
    int addr=(relation-'R'+1)*100;
    int blkaddr=relation*BASE_ADDR;
    int num[7];
    unsigned int* bucket[7];
    int i,j,h;
    unsigned int* r_buf;
    for (i=0;i<7;i++)
    {
        num[i]=0;
        bucket[i]=(unsigned int*)getNewBlockInBuffer(buf);
        memset(bucket[i],0,64);
    }
    for (i=0;i<blk;i++)
    {
        r_buf=(unsigned int*)readBlockFromDisk(blkaddr+i,buf);
        for (j=0;j<7;j++)
        {
            h=hashFunc(*(r_buf+2*j+offset));
            if (blk_counter[h]==N)
            {
                printf("不满足内存约束");
                exit(0);
            }
            *(bucket[h]+2*num[h])=*(r_buf+2*j);
            *(bucket[h]+2*num[h]+1)=*(r_buf+2*j+1);
            num[h]++;
            if (num[h]==7)
            {
                writeBlockToDisk(bucket[h],addr+10*h+blk_counter[h],buf);
                *((unsigned char*)bucket[h]-1)=1;
                memset(bucket[h],0,64);
                blk_counter[h]++;
                num[h]=0;
            }
        }
        freeBlockInBuffer(r_buf,buf);
    }
    for (i=0;i<7;i++)
    {
        if (blk_counter[i]==N)
        {
            printf("不满足内存约束");
            exit(0);
        }
        writeBlockToDisk(bucket[i],addr+10*i+blk_counter[i],buf);
        blk_counter[i]++;
    }
}
void hashJoin(Buffer* buf)
{
    int i,j,k,m,n;
    int res_addr=HASH_JOIN_ADDR;
    int r_addr=R_HASH_ADDR;
    int s_addr=S_HASH_ADDR;
    unsigned int* Rbuf[6];
    unsigned int* Sbuf;
    unsigned int* res=(unsigned int*)getNewBlockInBuffer(buf);
    memset(res,0,64);
    int res_counter=0;
    for (i=0;i<7;i++)
    {
        for (j=0;j<R_counter[i];j++)
        {
            Rbuf[j]=(unsigned int*)readBlockFromDisk(r_addr+10*i+j,buf);
        }
        for (j=0;j<S_counter[i];j++)
        {
            Sbuf=(unsigned int*)readBlockFromDisk(s_addr+10*i+j,buf);
            for (k=0;k<7;k++)
            {
                for (m=0;m<R_counter[i]-1;m++)
                {
                    for (n=0;n<7;n++)
                    {
                        if (*(Rbuf[m]+2*n)==*(Sbuf+2*k))
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
                n=0;
                while (*(Rbuf[m]+2*n))
                {
                    if (*(Rbuf[m]+2*n)==*(Sbuf+2*k))
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
                    n++;
                }
            }
            freeBlockInBuffer(Sbuf,buf);
        }
        for (j=0;j<R_counter[i];j++)
        {
            freeBlockInBuffer(Rbuf[j],buf);
        }
    }
    if (res_counter)
    {
        *(res+15)=0;
        writeBlockToDisk((unsigned char*)res,res_addr,buf);
        res_addr++;
    }
    freeBlockInBuffer(res,buf);
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
    //NJL_join(&buf);
    hashBlock('R','A',&buf);
    hashBlock('S','C',&buf);
    hashJoin(&buf);
    freeBuffer(&buf);
}