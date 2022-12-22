all: libdma.a  app

libdma.a:  dma.c
	gcc -Wall -g -c -lpthread -lm -lrt dma.c 
	ar rcs libdma.a dma.o

app: app.c
	gcc -Wall -g -o app app.c -L. -ldma -lm -lpthread -lrt

clean: 
	rm -fr *.o *.a *~ a.out  app
