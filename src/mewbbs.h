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


/*---フォーム欄のサイズ---*/
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

    char	aptitle[MAX_CONFIG_LEN];		/*表示するタイトル名*/
    char	maintitle[MAX_CONFIG_LEN];		/*表示するタイトル名*/
    char	subtitle[MAX_CONFIG_LEN];		/*サブタイトル名*/
    int		max;							/*タイトル表示数*/
    int		tag;							/*タグの使用（許可(1)・不許可(0)）*/
    char	background[MAX_CONFIG_LEN];		/*背景(BODYタグのBACKGROUND)*/
    char	bgcolor[MAX_CONFIG_LEN];		/*背景色(BODYタグのBGCOLOR)*/
    char	text[MAX_CONFIG_LEN];			/*文字色(BODYタグのTEXT)*/
    char	link_color[MAX_CONFIG_LEN];		/*リンク文字色(BODYタグのLINK)*/
    char	vlink[MAX_CONFIG_LEN];			/*既リンク文字色(BODYタグのVLINK)*/
    char	alink[MAX_CONFIG_LEN];			/*リンク中文字色(BODYタグのALINK)*/
    char	title_color[MAX_CONFIG_LEN];	/*タイトルの色*/
    int		flag_write;						/*書き込み（許可(0)・不許可(1)）*/
    int		fweight;						/*フォントの太さ*/
    int		fsize;							/*フォントのサイズ*/
    int		regmax;							/*最大登録数*/
    char	home_url[MAX_CONFIG_LEN];		/*掲示板終了後ジャンプするURL*/
    int		proxy;				/**/
    int		hostchk;				/**/

} CF;

char	empty[] = "\0";		/*空の文字列*/
char	untitled[] = "（無題）";	/*タイトルが入力されなかった場合に使うタイトル*/
