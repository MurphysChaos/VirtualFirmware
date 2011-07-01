CC = gcc
AR = ar rcs
CFLAGS = -g -O2 #-fPIC
OBJECTS = panel.o sockerr.o
LIBS = libpanel.a libsockerr.a libmessage.a
LDFLAGS = -L. -lpanel -lsockerr

lib%.a : %.o
	$(AR) $@ $<

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJECTS) $(TTO) $(LIBS) *~ 
