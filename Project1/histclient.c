#include <stdlib.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "shareddefs.h"

int main(int argc, char *argv[])
{

	mqd_t mq, mq2;
	struct item item;
	struct item *itemptr;
	struct mq_attr mq_attr;
	int n;
	char *bufptr;
	char arr3[256];
	int buflen;
	int a = 0;
	item.start = atoi(argv[3]);
	item.width = atoi(argv[2]);
	item.cnt = atoi(argv[1]);
	mq = mq_open(MQNAME, O_RDWR);
	if (mq == -1)
	{
		perror("can not open msg queue\n");
		exit(1);
	}
	int i = 0;

	while (i < 1)
	{
		item.id = i;

		n = mq_send(mq, (char *)&item, sizeof(struct item), 0);

		if (n == -1)
		{
			perror("mq_send failed\n");
			exit(1);
		}

		printf("\n");
		i++;
		item.cnt++;
		
	}
	mq2 = mq_open(MQNAME, O_RDWR | O_CREAT, 0666, NULL);
	if (mq2 == -1)
	{
		perror("can not create msg 2queue\n");
		exit(1);
	}

	mq_getattr(mq2, &mq_attr);

	/* allocate large enough space for the buffer to store
		an incoming message */
	buflen = mq_attr.mq_msgsize;
	bufptr = (char *)malloc(buflen);
	while (a < 1)
	{
		n = mq_receive(mq2, (char *)bufptr, buflen, NULL);
		if (n == -1)
		{
			perror("mq2_receive failed\n");
			exit(1);
		}


		itemptr = (struct item *)bufptr;
		const int s = itemptr->cnt;
		strcpy(arr3, itemptr->data);
		
		printf("received item->str = %s\n", itemptr->data);
		int rr[s];
		char *token = strtok(arr3, "-");
		int index = 0;
		while (token != NULL)
		{	
			rr[index] = atoi(token);
			index++;
			token = strtok(NULL, "-");
		}
		for (int i = 0; i < s; i++)
		{
			printf("(%d, %d) = %d\n", itemptr->start + (i * itemptr->width), itemptr->start + ((i + 1) * itemptr->width), rr[i]);
		}
		a = 1;
	}
	mq_close(mq);
	mq_close(mq2);
	return 0;
}