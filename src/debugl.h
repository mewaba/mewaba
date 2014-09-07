/*-----------------------------------------------------------------------------
	debugl.h
		�ǥХå������ϥ饤�֥��

	������			����
	��������		1999/3/23
	�ǽ���������	1999/3/23
-----------------------------------------------------------------------------*/

void dInitl( char *fname );
void dPrintl( char *msg, const char *srcname, const unsigned int srcline );

#ifdef DEBUG
#define DINITL(fname) {dInitl(fname);}
#define DPRINTL(msg) {dPrintl(msg, __FILE__, __LINE__);}
#else
#define DINITL(fname) /* DINITL */
#define DPRINTL(msg) /* DPRINTL */
#endif
