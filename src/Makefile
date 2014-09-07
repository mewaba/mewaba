CC = gcc
CFLAGS = -Wall -O2
MBOBJS = mewbbs.o libcgi.o libstr.o libsec.o libpr.o libinet.o debugl.o
VWOBJS = viewer.o libcgi.o libstr.o libpr.o

## FreeBSD -------------
LIB = -lcrypt
MACRO = 

## Solaris -------------
#LIB = -lnsl -lsocket -lresolv
#MACRO =  -DSOLARIS

##Linux (Slackware, PlamoLinux)------
#LIB = 
#MACRO =

##Linux (RedHatLinux, TurboLinux, VineLinux)-----
#LIB = -lcrypt
#MACRO = -DLINUX2

## IRIX ----------------
## 

all: mewbbs.cgi viewer.cgi

mewbbs.cgi: $(MBOBJS)
	$(CC) -o $@ $(MBOBJS) $(CFLAGS) $(LIB) $(MACRO)
viewer.cgi: $(VWOBJS)
	$(CC) -o $@ $(VWOBJS) $(CFLAGS) $(MACRO)

mewbbs.o: mewbbs.c
	$(CC) -c mewbbs.c $(CFLAGS) $(MACRO)
viewer.o: viewer.c
	$(CC) -c viewer.c $(CFLAGS) $(MACRO)
libcgi.o: libcgi.c
	$(CC) -c libcgi.c $(CFLAGS) $(MACRO)
libstr.o: libstr.c
	$(CC) -c libstr.c $(CFLAGS) $(MACRO)
libsec.o: libsec.c
	$(CC) -c libsec.c $(CFLAGS) $(MACRO)
libpr.o: libpr.c
	$(CC) -c libpr.c $(CFLAGS) $(MACRO)
libinet.o: libinet.c
	$(CC) -c libinet.c $(CFLAGS) $(MACRO)
debugl.o: debugl.c
	$(CC) -c debugl.c $(CFLAGS) $(MACRO)

install:
	cp *.cgi ../
	chmod 755 ../*.cgi
	chmod u+s ../*.cgi

clean:
	rm -rf *.o *core *.cgi
