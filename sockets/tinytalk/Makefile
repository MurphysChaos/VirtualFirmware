CC = gcc
CFLAGS = -g -O2 -fPIC
LIBFLAGS = -shared
OBJECTS = panel.o sock_error.o
TTO = tinytalk.o

tinytalk: $(OBJECTS) $(TTO) libpanel
	$(CC) $(CFLAGS) $(TTO) -L. -lpanel -o tinytalk
	
tinytalk.exe: $(OBJECTS) $(TTO)
	$(CC) $(CFLAGS) $(OBJECTS) $(TTO) -o tinytalk.exe
	
libpanel: $(OBJECTS)
	$(CC) $(LIBFLAGS) -W1,-soname,libpanel.so.0 -o libpanel.so.0.0.1 $(OBJECTS)

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(OBJECTS)
