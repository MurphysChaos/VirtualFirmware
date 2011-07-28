CC = gcc
AR = ar rcs
CFLAGS = -g -O2 #-fPIC
OBJECTS = panel.o sockerr.o tinymsg.o opt.o
LIBS = libpanel.a libsockerr.a libmessage.a libopt.a libdbg.a
LDFLAGS = -L. -lmessage -lpanel -lsockerr -lopt -ldbg


sendrcv: $(LIBS) sendrcv.o
	$(CC) $(CFLAGS) sendrcv.o $(LDFLAGS) -o $@

anounceloc: $(LIBS) anounceloc.o
	$(CC) $(CFLAGS) anounceloc.o $(LDFLAGS) -o $@

demo: $(LIBS) demo.o
	$(CC) $(CFLAGS) demo.o $(LDFLAGS) -o $@

opt_test: $(LIBS) opt_test.o
	$(CC) $(CFLAGS) opt_test.o $(LDFLAGS) -o $@

lib%.a : %.o
	$(AR) $@ $<

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJECTS) $(TTO) $(LIBS) *~ tinymsg annouceloc sendrcv

