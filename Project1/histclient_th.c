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
	item.start = atoi(argv[3]);
	item.width = atoi(argv[2]);
	item.cnt = atoi(argv[1]);
	ssize_t n;
	char mqName[] = "/mq1";
	char mq2Name[] = "/mq2";
	mq = mq_open(MQNAME1, O_RDWR);
	if (mq == -1)
	{
		perror("can not open msg queue\n");
		exit(1);
	}

	mq2 = mq_open(MQNAME2, O_RDWR | O_CREAT, 0666, NULL);
	if (mq2 == -1)
	{
		perror("can not create msg 2queue\n");
		exit(1);
	}

	printf("mq opened, mq id = %d\n", (int)mq);
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

		printf("mq_send success, item size = %d\n",
			   (int)sizeof(struct item));
		printf("item->id   = %d\n", item.id);
		printf("item->start = %d\n", item.start);
		printf("item->width = %d\n", item.width);
		printf("item->count = %d\n", item.cnt);
		i++;
	}
	int a = 0;
	while (a < 1)
	{
		mq_getattr(mq2, &mq_attr);
		char *bufptr = malloc(mq_attr.mq_msgsize);

		n = mq_receive(mq2, bufptr, mq_attr.mq_msgsize, NULL);
		if (n == -1)
		{
			perror("mq2_receive failed\n");
			exit(1);
		}
		itemptr = (struct item *)bufptr;
		const int s = itemptr->cnt;
		char arr3[] = "";
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

		printf("\n");
		a++;
		item.cnt++;
		free(bufptr);
		sleep(1);
	}

	/* allocate large enough space for the buffer to store
		an incoming message */
	if (mq_unlink(MQNAME2) == -1)
	{
		printf("Could Not Delete Reply-Queue\n");
		return (0);
	}

	return 0;
}