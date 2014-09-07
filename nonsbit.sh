#!/bin/sh
#
# Installer(nonsbit) for mewBBS
# Copyright(C) 1997-99  Yuto Ikeno  All rights reserved.

MAKE=`which make`

echo "mewBBS installing..."
echo "Changing permission(non sbit)..."
	chmod 755 *.cgi
	chmod 777 file
	chmod 666 ./counter.dat
	chmod 666 ./conf/mewbbs.cf
	chmod 666 ./conf/passwd
	chmod 666 ./conf/denied
	chmod 600 ./src/*.c
	chmod 600 ./src/*.h

echo "\n\n"
echo "Installation complete. Thanks for using mewBBS.\n"
echo "Copyright(C) 1997-99  Myu's Lab (Yuto Ikeno)  All rights reserved.\n"
