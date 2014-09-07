/*-----------------------------------------------------------------------------
	libinet.c
		�C���^�[�l�b�g�ėp���C�u����

	�쐬��			�r��
	�쐬����		1999/1/25
	�ŏI�X�V����	1999/2/20	�r��
-----------------------------------------------------------------------------*/
#include <string.h>		/*memset()*/
#include <unistd.h>		/*close()*/
#include <netdb.h>
#ifdef SOLARIS
#include <sys/types.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "libinet.h"

/*-----------------------------------------------------------------------------
�v���g�^�C�v�Fint lConnect( const char *srv, unsigned long int port );
����		�Fconst char *srv ... �ڑ���̃T�[�o���B�܂���IP�A�h���X
			�@unsigned long int port ... �ڑ���̃|�[�g�ԍ�
�Ԓl		�Fint ... ����F�\�P�b�g�f�B�X�N���v�^, �ُ�F-1
����		�Fsrv��port�Ŏw�肳�ꂽ�T�[�o�֐ڑ����Asocket descripter��Ԃ��B
�쐬��		�F�r��
�쐬����	�F1999/2/7
�ŏI�X�V�����F1999/2/7
-----------------------------------------------------------------------------*/
int lConnect( const char *srv, unsigned long int port ){

	int		sd;
	unsigned long int	ipaddr;
	struct	sockaddr_in	sv_addr;
	struct	hostent		*shost;

	if( (sd = socket( AF_INET, SOCK_STREAM, 0 )) < 0)	/*�\�P�b�g�쐬*/
		return -1;

	if( inet_addr(srv) == -1 ){		/*���O�Ŏw�肵���ꍇ*/
		if((shost = gethostbyname(srv)) == NULL){
			close( sd );
			return -1;
		}
	}else{							/*IP�A�h���X�̏ꍇ*/
		ipaddr = inet_addr(srv);
		if ((shost = gethostbyaddr ((char *)&ipaddr, 4, AF_INET)) == NULL){
			close( sd );
			return -1;
		}
	}

	memset ( (char *)&sv_addr, 0, sizeof(sv_addr));
	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons(port);
	memcpy(&sv_addr.sin_addr, (char *)shost->h_addr, shost->h_length);

	if( connect( sd, (struct sockaddr*)&sv_addr, sizeof(sv_addr) ) < 0){		/*�\�P�b�g�̐ڑ��v��*/
		close( sd );
		return -1;
	}

	return sd;

}

void lClose( int sd ){

	shutdown( sd, 2 );
	close( sd );

	return;

}

/*-----------------------------------------------------------------------------
�v���g�^�C�v�Fchar *nresolve( const char *hostname );
����		�Fconst char *hostname ... ����������z�X�g��
�Ԓl		�Fchar * �i�����j... hostname��IP�A�h���X
			�@       �i���s�j... NULL
����		�Fhostname�𐳈������AIP�A�h���X��Ԃ��B
			�@IP�A�h���X�������ɓn���ꂽ�ꍇ�����̂܂ܕԂ��B
�쐬��		�F�r��
�쐬��		�F1999/2/5
-----------------------------------------------------------------------------*/

char *nresolve( const char *hostname ){

	struct hostent *host;		/*host infomation*/
	struct in_addr addr;		/*IP Address*/
	char	*ipaddr;

	host = gethostbyname( hostname );
	if(host == NULL)
		return NULL;
	memcpy(&addr.s_addr, (char *)host->h_addr, host->h_length);
	ipaddr = inet_ntoa(addr);

	return ipaddr;

}


/*-----------------------------------------------------------------------------
�v���g�^�C�v�Fchar *iresolve( const char *ipaddr );
����		�Fconst char *ipaddr ... �t��������IP�A�h���X
�Ԓl		�Fchar * �i�����j... ipaddr�̃z�X�g��
			�@       �i���s�j... NULL
����		�FIP�A�h���X���t�������A�z�X�g����Ԃ��B
�쐬��		�F�r��
�쐬��		�F1999/2/5
-----------------------------------------------------------------------------*/

char *iresolve( const char *ipaddr ){

	struct				hostent *host;
	unsigned long int	naddr;			/*network byte order*/

	naddr = inet_addr( ipaddr );
	host = gethostbyaddr( (char *)&naddr, 4, AF_INET );
	if(host == NULL)
		return NULL;

	return host->h_name;

}

/*-----------------------------------------------------------------------------
�v���g�^�C�v�Fchar *getpeerip( int sd );
����		�Fint sd ... �ڑ����̃\�P�b�g�f�B�X�N���v�^
�Ԓl		�Fchar * �i�����j... �ڑ�����IP�A�h���X
			�@       �i���s�j... NULL
����		�F�ڑ�����IP�A�h���X��Ԃ��B
�쐬��		�F�r��
�쐬��		�F1999/2/5
-----------------------------------------------------------------------------*/

char *getpeerip( int sd ){

	struct sockaddr_in addr;
	int namelen;
	char *s_ipaddr;

	namelen = sizeof( addr );
	memset((char*)&addr, 0, sizeof(addr));
	if(getpeername( sd, (struct sockaddr *)&addr, &namelen ) == -1)
		return NULL;
	s_ipaddr = inet_ntoa(addr.sin_addr);	/*network byte order -> IP Address*/

	return s_ipaddr;

}
