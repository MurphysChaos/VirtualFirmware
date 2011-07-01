CC = gcc
CFLAGS = -g -O2
OBJECTS = api.o panel.o sock_error.o

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(OBJECTS)
