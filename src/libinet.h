/*-----------------------------------------------------------------------------
	libinet.h
		�C���^�[�l�b�g�ėp���C�u�����w�b�_�[�t�@�C��

	�쐬��			�r��
	�쐬����		1999/1/25
	�ŏI�X�V����	1999/2/20
-----------------------------------------------------------------------------*/

int lConnect( const char *srv, unsigned long int port );
void lClose( int sd );
char *nresolve( const char *hostname );
char *iresolve( const char *ipaddr );
char *getpeerip( int sd );
