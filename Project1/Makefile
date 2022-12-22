all: histserver histclient histserver_th histclient_th

histclient: histclient.c
	gcc -Wall -o histclient histclient.c -lrt

histserver: histserver.c
	gcc -Wall -o histserver histserver.c -lrt

histclient_th: histclient_th.c
	gcc -Wall -g -o histclient_th histclient_th.c -lrt -lpthread

histserver_th: histserver_th.c
	gcc -Wall -g -o histserver_th histserver_th.c -lrt -lpthread

clean: 
	rm -fr *~ histserver histclient histclient_th histserver_th


