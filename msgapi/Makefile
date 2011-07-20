CC = gcc
AR = ar rcs
CFLAGS = -g #-O2 #-fPIC
OBJECTS = panel.o sockerr.o tinymsg.o opt.o
LIBS = libpanel.a libsockerr.a libmessage.a libopt.a
LDFLAGS = -L. -lmessage -lpanel -lsockerr -lopt

linux_test_client: $(LIBS) linux_test_client.o
	$(CC) $(CFLAGS) linux_test_client.o $(LDFLAGS) -o $@

tinymsg: $(LIBS) tinymsg.o
	$(CC) $(CFLAGS) tinymsg.o $(LDFLAGS) -o $@

opt_test: $(LIBS) opt_test.o
	$(CC) $(CFLAGS) opt_test.o $(LDFLAGS) -o $@

lib%.a : %.o
	$(AR) $@ $<

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJECTS) $(TTO) $(LIBS) *~ tinymsg
