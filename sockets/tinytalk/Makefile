CC = gcc
AR = ar rcs
CFLAGS = -g -O2 #-fPIC
OBJECTS = panel.o sock_error.o
TTO = tinytalk.o

tinytalk: $(TTO) libpanel.a
	$(CC) $(CFLAGS) $(TTO) -L. -lpanel -o tinytalk
	
tinytalk.exe: $(TTO) libpanel.a
	$(CC) $(CFLAGS) $(TTO) -L. -lpanel -o tinytalk.exe
	
libpanel.a: $(OBJECTS)
	$(AR) libpanel.a $(OBJECTS)

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(OBJECTS) $(TTO) tinytalk libpanel.a


