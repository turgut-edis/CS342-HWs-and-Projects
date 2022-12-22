#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include "shareddefs.h"

int main(int argc, char *argv[])
{
	pid_t p_n;
	pid_t parentid;
	mqd_t mq, mq2, mq3;
	struct mq_attr mq_attr;
	struct mq_attr mq3_attr;
	struct item *itemptr;
	int n;
	char *bufptr, *bufptr3;
	int buflen, buflen3;
	const unsigned MAX_LENGTH = 256;
	int argCnt = atoi(argv[1]);
	int arrCnt = 0;
	int done = 0;
	int s = 0;
	int w = 0;
	int *arr;

	parentid = getpid();

	printf("I am parent and my pid is: %d\n", parentid);

	mq = mq_open(MQNAME, O_RDWR | O_CREAT, 0666, NULL);
	if (mq == -1)
	{
		perror("can not create msg queue\n");
		exit(1);
	}

	mq_getattr(mq, &mq_attr);

	/* allocate large enough space for the buffer to store
		an incoming message */
	buflen = mq_attr.mq_msgsize;
	bufptr = (char *)malloc(buflen);

	mq3 = mq_open(MQNAME, O_RDWR | O_CREAT, 0666, NULL);
	if (mq3 == -1)
	{
		perror("can not create msg3 queue\n");
		exit(1);
	}
	

	mq_getattr(mq3, &mq3_attr);

	/* allocate large enough space for the buffer to store
		an incoming message */
	buflen3 = mq3_attr.mq_msgsize;
	bufptr3 = (char *)malloc(buflen3);
	clock_t s_time = clock();
	while (!done)
	{
		n = mq_receive(mq, (char *)bufptr, buflen, NULL);
		if (n == -1)
		{
			perror("mq_receive failed\n");
			exit(1);
		}

		itemptr = (struct item *)bufptr;
		arrCnt = itemptr->cnt;
		s = itemptr->start;
		w = itemptr->width;
		arr = (int *)malloc(arrCnt * sizeof(int));
		int c_id;
		for (int i = 0; i < arrCnt; i++)
		{
			arr[i] = 0;
		}
		for (int i = 0; i < argCnt; i++)
		{
			if(i < argCnt)
				p_n = fork();
			if (p_n == 0)
			{
				printf("I am child=%d and mypid=%d\n", i, getpid());
				c_id = getpid();
				FILE *fp = fopen(argv[i + 2], "r");
				if (fp == NULL)
				{
					printf("Error: could not open file %s", argv[i + 2]);
					return 1;
				}
				char buffer[MAX_LENGTH];
				int cntt = 1;
				while (fgets(buffer, MAX_LENGTH, fp))
				{
					int num = atoi(buffer);

					if (num > s && num < (s + (w * arrCnt)))
					{
						int a = s;
						int isVisited = 0;
						while (cntt <= arrCnt)
						{

							if (num < a + (cntt * w) && (!isVisited))
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
				for (int i = 0; i < arrCnt; i++)
				{
					int num = arr[i];
					char str[MAX_LENGTH];
					sprintf(str, "%d", num);
					strcat(str, "-");
					strcat(strTot, str);
				}
				printf("%s", strTot);
				strcpy(itemptr->data, strTot);
				mq3 = mq_open(MQNAME, O_RDWR);
				n = mq_send(mq3, (char *)itemptr, sizeof(struct item), 0);
				fclose(fp);
				_exit(0);
			}
			else
			{

				n = mq_receive(mq3, (char *)bufptr, buflen, NULL);
				if (n == -1)
				{
					perror("mq_receive failed\n");
					exit(1);
				}

				printf("mq_receive success, message size = %d\n", n);

				itemptr = (struct item *)bufptr;
				
				mq2 = mq_open(MQNAME, O_RDWR);
				strcpy(itemptr->astr, "done");
				n = mq_send(mq2, (char *)itemptr, sizeof(struct item), 0);
				printf("\n");
				
			}
		}

		int return_stat;
		waitpid(c_id, &return_stat, 0);
		done = 1;
	}
	clock_t e_time = clock();
	printf("total time: %f", (double) (e_time - s_time));
	free(bufptr);
	mq_close(mq);
	mq_close(mq2);
	mq_close(mq3);
	return 0;
}