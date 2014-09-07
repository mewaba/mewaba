#!/bin/sh
#
# Installer for mewBBS
# Copyright(C) 1997-99  Yuto Ikeno  All rights reserved.

MAKE=`which make`

echo "mewBBS installing..."
echo "removing old version files..."
	if [ -r "mewbbs.c" ]
	then
		rm mewbbs.c
	fi
	if [ -r "cgi.c" ]
	then
		rm cgi.c
	fi
	if [ -r "cgi.h" ]
	then
		rm cgi.h
	fi
	if [ -r "libcgi.c" ]
	then
		rm libcgi.c
	fi
	if [ -r "libcgi.h" ]
	then
		rm libcgi.h
	fi
	if [ -r "res.gif" ]
	then
		rm res.gif
	fi
	if [ -r "Makefile" ]
	then
		rm Makefile
	fi

echo "moving old version config files..."
	if [ -r "mewbbs.cf" ]
	then
		mv ./mewbbs.cf ./conf/mewbbs.cf
	fi
	if [ -r "denied" ]
	then
		mv ./denied ./conf/denied
	fi
	if [ -r "passwd" ]
	then
		mv ./passwd ./conf/passwd
	fi
	if [ -r "mewbbs.prf" ]
	then
		mv ./mewbbs.prf ./conf/mewbbs.cf
	fi

echo "Creating file..."
	if [ ! -d "file" ]
	then
		mkdir file
	fi

	if [ ! -d "conf" ]
	then
		mkdir conf
	fi

	if [ ! -r "passwd" ]
	then
		touch ./conf/passwd
	fi

	if [ ! -r "mewbbs.prf" ]
	then
		touch ./conf/mewbbs.cf
	fi

	if [ -r "file/denied" ]
	then
		mv ./file/denied ./conf/denied
	else
		touch ./conf/denied
	fi

	if [ ! -r "counter.dat" ]
	then
		echo '00000' > counter.dat
	fi

echo "Changing permission..."
	chmod 755 file
	chmod 644 ./counter.dat
	chmod 600 ./conf/mewbbs.cf
	chmod 600 ./conf/passwd
	chmod 600 ./conf/denied
	chmod 600 ./src/*.c
	chmod 600 ./src/*.h

echo "Making mewBBS executables (Please wait a few minute)..."
	cd ./src
	$MAKE clean
	$MAKE all
	$MAKE install

echo "\n\n"
echo "Installation complete. Thanks for using mewBBS.\n"
echo "Copyright(C) 1997-99  Myu's Lab (Yuto Ikeno)  All rights reserved.\n"
