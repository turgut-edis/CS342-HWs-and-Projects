#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "shareddefs.h"

void *calc_hist(void *arg);

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

const unsigned MAX_LENGTH = 256;

char *total; 

typedef struct funcParams
{
    char name[256];
    int start;
    int width;
    int count;
    int *sum;
} funcParams;

int main(int argc, char *argv[])
{
    total = malloc(200);
    const int fileCnt = atoi(argv[1]);
    pthread_t thread_id[fileCnt];
    mqd_t mq, mq2;
    struct mq_attr mq_attr;
    struct item *itemptr;
    funcParams f;
    ssize_t n;
    char *bufptr;
    int buflen;
    int argCnt = atoi(argv[1]);
    int arrCnt = 0;
    int done = 0;
    int s = 0;
    int w = 0;
    int *thTotal;

    mq = mq_open(MQNAME1, O_RDWR | O_CREAT, 0666, NULL);
    if (mq == -1)
    {
        perror("can not create msg queue\n");
        exit(1);
    }
    printf("mq created, mq id = %d\n", (int)mq);

    mq_getattr(mq, &mq_attr);
    buflen = mq_attr.mq_msgsize;
    bufptr = malloc(buflen);
    if (bufptr == NULL)
    {
        printf("Could not create buffer");
    }

    {

        n = mq_receive(mq, bufptr, buflen, 0);
        if (n != -1)
        {
            itemptr = (struct item *)bufptr;

            for (int i = 0; i < argCnt; i++)
            {
                f.count = itemptr->cnt;
                f.start = itemptr->start;
                f.width = itemptr->width;
                strcpy(f.name, argv[i + 2]);
                char xx[256];
                
                pthread_create(&thread_id[i], NULL, calc_hist, (void *)&f);
                pthread_join(thread_id[i], NULL);
                
                printf("i: %d", i);

                int rr[f.count];
                char *token = strtok(total, "-");
                int index = 0;
                
                while (token != NULL)
                {
                    rr[index] = atoi(token);
                    printf("%s-", token);
                    index++;
                    token = strtok(NULL, "-");
                }
                printf("rr: ");
                free(total);
            }
            int *results[fileCnt];

            for (int i = 0; i < argCnt; i++)
            {
                for (int j = 0; j < f.count; j++)
                {
                    printf("%d a", results[i][j]);
                }
            }
            int tot[f.count];
            for (int x = 0; x < f.count; x++)
            {
                tot[x] = 0;
            }
            for (int i = 0; i < argCnt; i++)
            {
                for (int j = 0; j < f.count; j++)
                {
                    tot[j] += results[i][j];
                }
            }

            char strTot[] = "";
            for (int i = 0; i < arrCnt; i++)
            {
                int num = tot[i];
                char str[MAX_LENGTH];
                sprintf(str, "%d", num);
                strcat(str, "-");
                strcat(strTot, str);
            }
            strcpy(itemptr->data, strTot);
            mq2 = mq_open(MQNAME2, O_RDWR);
            if (mq2 == -1)
            {
                printf("MQ is not created");
                return 0;
            }
            n = mq_send(mq2, (char *)itemptr, sizeof(sizeof(struct item)), 0);
            
        }
    }
    
    free(bufptr);
    mq_close(mq);
    mq_close(mq2);
    return 0;
}

void *calc_hist(void *x)
{
    char* out;
    funcParams *f = (funcParams *)x;
    printf("file: %d", f->count);
    int *arr = malloc(f->count * sizeof(int *));
    for (int i = 0; i < f->count; i++)
    {
        arr[i] = 0;
    }
    FILE *fp = fopen(f->name, "r");
    if (fp == NULL)
    {
        printf("Error: could not open file %s", f->name);
        return (void *)1;
    }
    char buffer[MAX_LENGTH];
    int cntt = 1;
    while (fgets(buffer, MAX_LENGTH, fp))
    {
        int num = atoi(buffer);

        if (num > f->start && num < (f->start + (f->width * f->count)))
        {
            int a = f->start;
            int isVisited = 0;
            while (cntt <= f->count)
            {

                if (num < a + (cntt * f->width) && (!isVisited))
                {

                    arr[cntt - 1]++;

                    isVisited = 1;
                }
                else
                {
                    cntt++;
                }
            }
            cntt = 1;
        }
    }
    char strTot[] = "";
    for (int i = 0; i < f->count; i++)
    {
        int num = arr[i];
        char str[MAX_LENGTH];
        sprintf(str, "%d", num);
        strcat(str, "-");
        strcat(strTot, str);
    }
    printf("aaaa");
    
    strcpy(total, strTot);
    
    printf("\n");
    printf("%s", total);
    fclose(fp);
    free(arr);
    pthread_exit(NULL);
}