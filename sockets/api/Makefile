CC = gcc
AR = ar rcs
CFLAGS = -g -O2 #-fPIC
OBJECTS = panel.o sockerr.o tinymsg.o opt.o
LIBS = libpanel.a libsockerr.a libmessage.a libopt.a
LDFLAGS = -L. -lmessage -lpanel -lsockerr -lopt

tinymsg: $(LIBS) tinymsg.o
	$(CC) $(CFLAGS) tinymsg.o $(LDFLAGS) -o $@

lib%.a : %.o
	$(AR) $@ $<

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJECTS) $(TTO) $(LIBS) *~ tinymsg
