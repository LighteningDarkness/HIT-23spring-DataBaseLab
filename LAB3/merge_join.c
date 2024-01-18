#include <stdlib.h>
#include <stdio.h>
#include "projection.c"
#define MERGE_ADDR 3000
void mergelink(char r1,char f1,char r2,char f2,Buffer* buf)
{
    printf("mergersort");
    mergesort(r1,f1,buf);
    mergesort(r2,f2,buf);
    int offset[2]={0,0};
    if ((r1=='R'&&f1=='B')||(r1=='S'&&f1=='D'))
    {
        offset[0]=1;
    }
    if ((r2=='R'&&f2=='B')||(r2=='S'&&f2=='D'))
    {
        offset[1]=1;
    }
    int tuple_counter[2]={R_TUPLE,S_TUPLE};
    unsigned int* blk_array[8]={NULL};
    int i,j;
    unsigned int addr[2]={R_SORT_ADDR,S_SORT_ADDR};
    int c=2;
    int counter=0;
    unsigned int* res=(unsigned int*)getNewBlockInBuffer(buf);
    int maxblk[2]={1,1};
    memset((unsigned char*)res,0,64);
    
    int res_counter=0;
    //最大就7个队伍
    unsigned int* q[2][7];
    int rest[2][2];
    unsigned int res_addr=MERGE_ADDR;
    
    for (i=0;i<c;i++)
    {
        q[i][0]=(unsigned int*)readBlockFromDisk(addr[i],buf);
        rest[i][0]=-1;
        rest[i][1]=-1;
    }
    while (tuple_counter[0]>0&&tuple_counter[1]>0)
    {

        //寻找最小值
        int min=INT_MAX;
        for (i=0;i<7*c;i++)
        {
            int blk_idx1=i/7;
            int tuple_idx1=i%7;
            if (tuple_idx1>rest[blk_idx1][1])
            {

                min=((int)(*(q[blk_idx1][0]+2*tuple_idx1+offset[blk_idx1]))<min)?(int)(*(q[blk_idx1][0]+2*tuple_idx1+offset[blk_idx1])):min;
            }
        }
        printf("%d,%d,%d\n",min,tuple_counter[0],tuple_counter[1]);
        for (i=0;i<7;i++)
        {
            if (*(q[0][0]+2*i+offset[0])==min)
            {
                break;
            }
        }
        rest[0][0]=i;
        for (i=6;i>=0;i--)
        {
            if (*(q[0][0]+2*i+offset[0])==min)
            {
                break;
            }
        }
        rest[0][1]=i;
        for (i=0;i<7;i++)
        {
            if (*(q[1][0]+2*i+offset[1])==min)
            {
                break;
            }
        }
        rest[1][0]=i;
        for (i=6;i>=0;i--)
        {
            if (*(q[1][0]+2*i+offset[1])==min)
            {
                break;
            }
        }
        rest[1][1]=i;
        unsigned int* ptr=q[0][0];
        while (rest[0][1]==6)
        {
            maxblk[0]++;
            if (maxblk[0]>=7)
            {
                printf("缓冲区空间不足，无法使用归并排序");
                exit(0);
            }
            unsigned int tmp=*(ptr+15);
            if (!tmp)
            {
                rest[0][1]=6;
                maxblk[0]--;
                break;
            }
            unsigned int* tmp_ptr=(unsigned int*)readBlockFromDisk(tmp,buf);
            q[0][maxblk[0]-1]=tmp_ptr;
            ptr=tmp_ptr;

            for (j=6;j>=0;j--)
            {
                if (*(tmp_ptr+2*j+offset[0])==min)
                {
                    break;
                }
            }
            rest[0][1]=j;
        }
        ptr=q[1][0];
        while (rest[1][1]==6)
        {
            maxblk[1]++;
            if (maxblk[1]>=7-maxblk[0])
            {
                printf("缓冲区空间不足，无法使用归并排序");
                exit(0);
            }
            unsigned int tmp=*(ptr+15);
            if (!tmp)
            {
                rest[1][1]=6;
                maxblk[1]--;
                break;
            }
            unsigned int* tmp_ptr=(unsigned int*)readBlockFromDisk(tmp,buf);
            q[1][maxblk[1]-1]=tmp_ptr;
            ptr=tmp_ptr;
            for (j=6;j>=0;j--)
            {
                if (*(tmp_ptr+2*j+offset[1])==min)
                {
                    
                    break;
                }
            }
            rest[1][1]=j;
        }
        if (maxblk[0]==1&&rest[0][0]==7&&rest[0][1]==-1)
        {
            for (j=rest[1][0];j<(maxblk[1]-1)*7+rest[1][1]+1;j++)
            {
                tuple_counter[1]--;
            }
            for (j=0;j<maxblk[1]-1;j++)
            {
                freeBlockInBuffer(q[1][j],buf);
            }
            q[1][0]=q[1][maxblk[1]-1];
            int r=tuple_counter[0]%7;
            rest[0][1]=6-r;
            maxblk[1]=1;
            continue;
        }
        if (maxblk[1]==1&&rest[1][0]==7&&rest[1][1]==-1)
        {
            for (j=rest[0][0];j<(maxblk[0]-1)*7+rest[0][1]+1;j++)
            {
                tuple_counter[0]--;
            }
            for (j=0;j<maxblk[0]-1;j++)
            {
                freeBlockInBuffer(q[0][j],buf);
            }
            q[0][0]=q[0][maxblk[0]-1];
            int r=tuple_counter[1]%7;
            maxblk[0]=1;
            rest[1][1]=6-r;
            continue;
        }
        for (i=rest[0][0];i<(maxblk[0]-1)*7+rest[0][1]+1;i++)
        {
            int blk_idx1=i/7;
            int tuple_idx1=i%7;
            if (*(q[0][blk_idx1]+2*tuple_idx1+offset[0])==min)
            {
                tuple_counter[0]--;
                for (j=rest[1][0];j<(maxblk[1]-1)*7+rest[1][1]+1;j++)
                {
                    int blk_idx2=j/7;
                    int tuple_idx2=j%7;
                    if (i==(maxblk[0]-1)*7+rest[0][1])
                    {
                        tuple_counter[1]--;
                    }
                    if (*(q[1][blk_idx2]+2*tuple_idx2+offset[1])==min)
                    {
                        *(res+4*res_counter)=*(q[0][blk_idx1]+2*tuple_idx1);
                        *(res+4*res_counter+1)=*(q[0][blk_idx1]+2*tuple_idx1+1);
                        *(res+4*res_counter+2)=*(q[1][blk_idx2]+2*tuple_idx2);
                        *(res+4*res_counter+3)=*(q[1][blk_idx2]+2*tuple_idx2+1);
                        res_counter++;
                        if (res_counter==3)
                        {
                            *(res+15)=res_addr+1;
                            // if (!tuple_counter[0]||!tuple_counter[1])
                            // {
                            //     *(res+15)=0;
                            // }
                            writeBlockToDisk((unsigned char*)res,res_addr,buf);
                            *((unsigned char*)res-1)=1;
                            memset((unsigned char*)res,0,64);
                            res_addr++;
                            res_counter=0;
                        }
                    }

                }
                
            }
            
        }
        
        for (i=0;i<maxblk[0]-1;i++)
        {
            freeBlockInBuffer(q[0][i],buf);
        }
        for (i=0;i<maxblk[1]-1;i++)
        {
            freeBlockInBuffer(q[1][i],buf);
        }
        if (maxblk[0]==1)
        {
            if (tuple_counter[0]&&tuple_counter[0]<112&&tuple_counter[0]%7==0)
            {
                unsigned int t=*(q[0][0]+15);
                freeBlockInBuffer(q[0][0],buf);
                q[0][0]=(unsigned int*)readBlockFromDisk(t,buf);
                rest[0][0]=-1;
                rest[0][1]=-1;
            }
        }
        if (maxblk[1]==1)
        {
            if (tuple_counter[1]&&tuple_counter[1]<224&&tuple_counter[1]%7==0)
            {
                unsigned int t=*(q[1][0]+15);
                freeBlockInBuffer(q[1][0],buf);
                q[1][0]=(unsigned int*)readBlockFromDisk(t,buf);
                rest[1][0]=-1;
                rest[1][1]=-1;
            }
        }
        q[0][0]=q[0][maxblk[0]-1];
        q[1][0]=q[1][maxblk[1]-1];
        maxblk[0]=1;
        maxblk[1]=1;
        if (!tuple_counter[0]||!tuple_counter[1])
        {
            if (res_counter)
            {
                *(res+15)=0;
                writeBlockToDisk((unsigned char*)res,res_addr,buf);
            }
            else
            {
                freeBlockInBuffer(res,buf);
                res=(unsigned int*)readBlockFromDisk(res_addr-1,buf);
                *(res+15)=0;
                writeBlockToDisk(res,res_addr-1,buf);
            }
        }
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

    // LinearSearch('R','A',40,&buf);
    // LinearSearch('S','C',60,&buf);

    // mergesort('R','A',&buf);
    //mergesort('S','C',&buf);
    //project('R','A',&buf);
    mergelink('R','A','S','C',&buf);
    freeBuffer(&buf);
}