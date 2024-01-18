#include <stdlib.h>
#include <stdio.h>
#include "extmem.h"
#include "extmem.c"
#define R_BLOCK 16
#define S_BLOCK 32
#define BASE_ADDR 1000
#define R_TUPLE 112
#define S_TUPLE 224
#define R_SORT_ADDR 10000
#define S_SORT_ADDR 20000
#define R_PROJ_ADDR 5000
#define S_PROJ_ADDR 6000
void externalsort(char relation,char field,Buffer* buf)
{
    int offset=0;
    if ((relation=='R'&&field=='B')||(relation=='S'&&field=='D'))
    {
        offset=1;
    }
    int tuple_counter=(relation=='R')?R_TUPLE:S_TUPLE;
    int block=(relation=='R')?R_BLOCK:S_BLOCK;
    unsigned int* blk_array[8]={NULL};
    int i;
    unsigned int addr=relation*BASE_ADDR;
    int c=(block%8==0)?(block/8):(block/8+1);
    int counter=0;
    while (counter<c)
    {
        for (i=0;i<8;i++)
        {
            blk_array[i]=(unsigned int*)readBlockFromDisk(addr+i+8*counter,buf);
        }
        int n=(tuple_counter>56)?56:tuple_counter;
        for (i=0;i<n;i++)
        {
            int blk_idx1=i/7;
            int tuple_idx1=i%7;
            int j;
            for (j=i+1;j<n;j++)
            {
                int blk_idx2=j/7;
                int tuple_idx2=j%7;
                if (*(blk_array[blk_idx1]+2*tuple_idx1+offset)>*(blk_array[blk_idx2]+2*tuple_idx2+offset))
                {
                    unsigned int tmp=*(blk_array[blk_idx1]+2*tuple_idx1);
                    *(blk_array[blk_idx1]+2*tuple_idx1)=*(blk_array[blk_idx2]+2*tuple_idx2);
                    *(blk_array[blk_idx2]+2*tuple_idx2)=tmp;
                    tmp=*(blk_array[blk_idx1]+2*tuple_idx1+1);
                    *(blk_array[blk_idx1]+2*tuple_idx1+1)=*(blk_array[blk_idx2]+2*tuple_idx2+1);
                    *(blk_array[blk_idx2]+2*tuple_idx2+1)=tmp;
                }
            }
        }
        for (i=0;i<8;i++)
        {
            writeBlockToDisk((unsigned char*)blk_array[i],addr+i+8*counter,buf);
            freeBlockInBuffer((unsigned char*)blk_array[i],buf);
        }
        tuple_counter-=56;
        counter++;
    }
}
void mergesort(char relation,char field,Buffer* buf)
{
    int offset=0;
    if ((relation=='R'&&field=='B')||(relation=='S'&&field=='D'))
    {
        offset=1;
    }
    int tuple_counter=(relation=='R')?R_TUPLE:S_TUPLE;
    int block=(relation=='R')?R_BLOCK:S_BLOCK;
    int blk_num[8]={0,0,0,0,0,0,0,0};
    int i;
    unsigned int addr=relation*BASE_ADDR;
    int c=(block%8==0)?(block/8):(block/8+1);
    int counter=0;
    externalsort(relation,field,buf);
    unsigned int* res=(unsigned int*)getNewBlockInBuffer(buf);
    
    memset((unsigned char*)res,0,64);
    
    int res_counter=0;
    //最大就7个队伍
    unsigned int* q[7]={NULL};
    int rest[7];
    unsigned int res_addr=(relation=='R')?R_SORT_ADDR:S_SORT_ADDR;
    for (i=0;i<c;i++)
    {
        q[i]=(unsigned int*)readBlockFromDisk(addr+8*i,buf);
        rest[i]=7;
    }
    tuple_counter=(relation=='R')?R_TUPLE:S_TUPLE;
    while (tuple_counter>0)
    {
        //寻找最小值
        int min=INT_MAX;
        for (i=0;i<7*c;i++)
        {
            int blk_idx1=i/7;
            int tuple_idx1=i%7;
            if (blk_num[blk_idx1]<8&&tuple_idx1>=7-rest[blk_idx1])
            {

                min=((int)(*(q[blk_idx1]+2*tuple_idx1+offset))<min)?(int)(*(q[blk_idx1]+2*tuple_idx1+offset)):min;
            }
        }
        printf("%d\n",min);
        for (i=0;i<7*c;i++)
        {
            int blk_idx1=i/7;
            int tuple_idx1=i%7;
            if (blk_num[blk_idx1]<8&&tuple_idx1>=7-rest[blk_idx1])
            {
                if ((int)(*(q[blk_idx1]+2*tuple_idx1+offset))==min)
                {
                    *(res+2*res_counter)=*(q[blk_idx1]+2*tuple_idx1);
                    *(res+2*res_counter+1)=*(q[blk_idx1]+2*tuple_idx1+1);
                    res_counter++;
                    rest[blk_idx1]--;
                    tuple_counter--;
                }
                
                if (res_counter==7)
                {
                    if (tuple_counter)
                    {
                        *(res+15)=res_addr+1;
                    }
                    writeBlockToDisk((unsigned char*)res,res_addr,buf);
                    *((unsigned char*)res-1)=1;
                    memset((unsigned char*)res,0,64);
                    res_addr++;
                    res_counter=0;
                }
                if (!tuple_counter&&res_counter)
                {
                    writeBlockToDisk((unsigned char*)res,res_addr,buf);
                    break;
                }
            }
        }
        for (i=0;i<c;i++)
        {
            while (!rest[i])
            {
                unsigned int tmp=*(q[i]+15);
                //printf("%d\n",tmp);
                if (!tmp)
                {
                    rest[i]=-1;
                }
                else
                {
                    blk_num[i]++;
                    freeBlockInBuffer((unsigned char*)q[i],buf);
                    if (blk_num[i]<8)
                    {
                        q[i]=(unsigned int*)readBlockFromDisk(tmp,buf);
                        rest[i]=7;
                        int j;
                        for (j=0;j<7;j++)
                        {
                            if (*(q[i]+2*j+offset)==min)
                            {
                                *(res+2*res_counter)=*(q[i]+2*j);
                                *(res+2*res_counter+1)=*(q[i]+2*j+1);
                                res_counter++;
                                rest[i]--;
                                tuple_counter--;
                            }
                            if (res_counter==7)
                            {
                                if (res_counter)
                                {
                                    *(res+15)=res_addr+1;
                                }
                                writeBlockToDisk((unsigned char*)res,res_addr,buf);
                                *((unsigned char*)res-1)=1;
                                memset(res,0,64);
                                res_addr++;
                                res_counter=0;
                            }
                            if (!tuple_counter&&res_counter)
                            {
                                writeBlockToDisk((unsigned char*)res,res_addr,buf);
                                break;
                            }
                        }
                    }
                    else
                    {
                        rest[i]=-1;
                    }
                }
            }
        }
    }
    memset(buf->data,0,buf->bufSize);
}

void project(char relation,char field,Buffer* buf)
{
    mergesort(relation,field,buf);
    int i,j=0;
    int offset=0;
    if ((relation=='R'&&field=='B')||(relation=='S'&&field=='D'))
    {
        offset=1;
    }
    int block=(relation=='R')?R_BLOCK:S_BLOCK;
    unsigned int addr=(relation=='R')?R_SORT_ADDR:S_SORT_ADDR;
    unsigned int proj_addr=(relation=='R')?R_PROJ_ADDR:S_PROJ_ADDR;
    unsigned int* ptr=NULL;
    unsigned int* res=(unsigned int*)getNewBlockInBuffer(buf);
    memset(res,0,64);
    int res_counter=0;
    unsigned int tmp;
    for (i=0;i<block;i++)
    {
        j=0;
        ptr=(unsigned int*)readBlockFromDisk(addr,buf);
        while (i>0&&j<7&&*(ptr+2*j+offset)==tmp)
        {
            j++;
        }
        while (j<7)
        {
            *(res+res_counter)=*(ptr+2*j+offset);
            tmp=*(ptr+2*j+offset);
            res_counter++;
            if (res_counter==15)
            {
                *(res+15)=proj_addr+1;
                writeBlockToDisk((unsigned char*)res,proj_addr,buf);
                *((unsigned char*)res-1)=1;
                memset(res,0,64);
                proj_addr++;
                res_counter=0;
            }
            while (j<7&&*(ptr+2*j+offset)==tmp)
            {
                j++;
            }
        }
        addr++;
        freeBlockInBuffer(ptr,buf);
    }
    if (res_counter)
    {
        *(res+15)=0;
        writeBlockToDisk((unsigned char*)res,proj_addr,buf);
    }

}
// int main()
// {
//     Buffer buf; /* A buffer */
//     unsigned char *blk; /* A pointer to a block */
//     int i = 0;

//     /* Initialize the buffer */
//     if (!initBuffer(520, 64, &buf))
//     {
//         perror("Buffer Initialization Failed!\n");
//         return -1;
//     }
//     //createRelation('R',16,7,1,40,1,1000,&buf);
//     //createRelation('S',32,7,20,60,1,1000,&buf);

//     // LinearSearch('R','A',40,&buf);
//     // LinearSearch('S','C',60,&buf);
//     // mergesort('R','A',&buf);
//     // mergesort('S','C',&buf);
//     project('R','A',&buf);
//     freeBuffer(&buf);
// }
