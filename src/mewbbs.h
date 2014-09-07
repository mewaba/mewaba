#define SALT	"MW"
#define DENIED_LIST		"./conf/denied"
#define COUNTER_FILE	"./counter.dat"
#define CONFIG_FILE		"./conf/mewbbs.cf"
#define DATA_FILE		"./mewbbs.dat"
#define ADMIN_HELP_URL	"./help/admhelp.html"
#define HELP_URL		"./help/help.html"
#define CC_URL			"./help/cc.html"
#define PASSWD_PATH		"./conf/passwd"
#define COOKIE_EXPIRE_DAYS (100)


/*---�t�H�[�����̃T�C�Y---*/
#define	TITLE_SIZE		(40)
#define	TITLE_MAXSIZE	(40)
#define	CONTENTS_ROWS	(6)
#define	CONTENTS_COLS	(70)
#define	URL_SIZE		(60)
#define	URL_MAXSIZE		(300)
#define	HANDLE_SIZE		(20)
#define	HANDLE_MAXSIZE	(30)
#define	MAIL_SIZE		(20)
#define	MAIL_MAXSIZE	(150)
#define PASSWD_LEN		(50)
/*------------------------*/

#define MAX_CONFIG_LEN	(1024)
#define FNAME_LEN (256)

typedef struct mewbbsConfig {

    char	aptitle[MAX_CONFIG_LEN];		/*�\������^�C�g����*/
    char	maintitle[MAX_CONFIG_LEN];		/*�\������^�C�g����*/
    char	subtitle[MAX_CONFIG_LEN];		/*�T�u�^�C�g����*/
    int		max;							/*�^�C�g���\����*/
    int		tag;							/*�^�O�̎g�p�i����(1)�E�s����(0)�j*/
    char	background[MAX_CONFIG_LEN];		/*�w�i(BODY�^�O��BACKGROUND)*/
    char	bgcolor[MAX_CONFIG_LEN];		/*�w�i�F(BODY�^�O��BGCOLOR)*/
    char	text[MAX_CONFIG_LEN];			/*�����F(BODY�^�O��TEXT)*/
    char	link_color[MAX_CONFIG_LEN];		/*�����N�����F(BODY�^�O��LINK)*/
    char	vlink[MAX_CONFIG_LEN];			/*�������N�����F(BODY�^�O��VLINK)*/
    char	alink[MAX_CONFIG_LEN];			/*�����N�������F(BODY�^�O��ALINK)*/
    char	title_color[MAX_CONFIG_LEN];	/*�^�C�g���̐F*/
    int		flag_write;						/*�������݁i����(0)�E�s����(1)�j*/
    int		fweight;						/*�t�H���g�̑���*/
    int		fsize;							/*�t�H���g�̃T�C�Y*/
    int		regmax;							/*�ő�o�^��*/
    char	home_url[MAX_CONFIG_LEN];		/*�f���I����W�����v����URL*/
    int		proxy;				/**/
    int		hostchk;				/**/

} CF;

char	empty[] = "\0";		/*��̕�����*/
char	untitled[] = "�i����j";	/*�^�C�g�������͂���Ȃ������ꍇ�Ɏg���^�C�g��*/
