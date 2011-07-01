CC = gcc
AR = ar rcs
CFLAGS = -g -O2 #-fPIC
OBJECTS = panel.o sockerr.o
LIBS = libpanel.a libsockerr.a
LDFLAGS = -L. -lpanel -lsockerr
TTO = tinytalk.o

tinytalk: $(TTO) $(LIBS) $(OBJECTS)
	$(CC) $(CFLAGS) $(TTO) $(LDFLAGS) -o $@
	
lib%.a : %.o
	$(AR) $@ $<

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJECTS) $(TTO) $(LIBS) *~ tinytalk tinytalk.exe
