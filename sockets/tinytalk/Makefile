CC = gcc
CFLAGS = -g -O2
OBJECTS = tinytalk.o panel.o sock_error.o

tinytalk: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o tinytalk
	
tinytalk.exe: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o tinytalk.exe

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(OBJECTS)
