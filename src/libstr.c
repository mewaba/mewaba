/*---------------------------------------------------------------------------
 * libstr.c Ver1.2
 * 文字列操作ライブラリ
 *
 * Last update : 1999/4/17
 *
 * Copyright(C) 1997-99  Yuto Ikeno  All rights reserved.
 * e-mail : mew@onbiz.net
 * homepage:http://www.onbiz.net/~mew/
 * compile: gcc -c libstr.c
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：int strspl (char *, char *, char);
引数　　　　：char *s1		検索文字の直前までを格納する文字列
　　　　　　　char *s2		加工対象文字列
　　　　　　　char search	検索文字
戻り値　　　：int			加工対象文字列の終端：0
説明　　　　：s2からsearchが見つかるまでs1に代入し、s2の残りを左にシフトする
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
int strspl(char *s1, char *s2, char search)
{

    int flag = 1;
    char *p = s2, *q = s2;		/*s2の位置保存*/

    if(s1 == NULL || s2 == NULL)
        return 0;

    while(*p != search && *p)	/*searchに当たる直前までの値を*outに代入 */
        *s1++ = *p++;

    if(!*p)						/*該当する文字無し*/
        flag = 0;
    *s1 = '\0';				/*s1の最後に\0代入*/
    p++;					/*区切り文字の分だけスキップ*/
    while(*p)				/*残りの文字列を、先頭にシフトさせる */
        *q++ = *p++;
    *q = '\0';
    return flag;
}
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：char *strsub (char *, const char *, int, int)
引数　　　　：char *s1		出力文字列
　　　　　　　char *s2		検索対象文字列
　　　　　　　int start		開始位置
　　　　　　　int end		終了位置
戻り値　　　：char *		s2のstartからendまでの文字列
説明　　　　：s2のstartからendまでをs1へ抜き出す。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
char *strsub(char *s1, const char *s2, int start, int end)
{

    char	*p = s1;			/*s1の位置保存*/

    if(s1 == NULL || s2 == NULL)
        return NULL;

    s2 += start;				/*s2のポインタを指定数進める*/
    while(start < end-- && *s2)	/*指定文字数分s1へコピーする*/
        *p++ = *s2++;

    *p = '\0';

    return s1;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：char *replacestring (char *, int, int)
引数　　　　：char *in		置換対象文字列
　　　　　　　char search	検索文字列
　　　　　　　char *replace	置換文字列
戻り値　　　：char *		置換終了後の文字列へのポインタ
説明　　　　：inからsearchを検索し、replaceに置き換える。
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
char *replacestring(char *strIn, char *strSearch, char *strReplace)
{

    unsigned int		lenInput = 0;
    unsigned int		lenSearch = 0;
    unsigned int		lenBuffer = 0;
    unsigned int		lenReplace = 0;
    unsigned int		sizeDest = 0;
    unsigned int		len = 0;
    char*				s;
    char*				dest = NULL;

    if(strIn == NULL || strSearch == NULL || strReplace == NULL)
        return strIn;
    else if(strlen(strIn) == 0)
        return NULL;

    lenSearch = strlen(strSearch);
    lenReplace = strlen(strReplace);

    sizeDest = (lenReplace + (lenReplace % lenSearch)) / lenSearch;
    if((dest = (char *)malloc(strlen(strIn) * (sizeDest + 1))) == NULL)
        return NULL;
    memset( (char *)dest, 0, sizeof( dest ) );

    if(lenSearch <= 0) {		/*検索文字列長が0の場合は文字列をそのまま返す*/
        strcpy(dest, strIn);
        return dest;
    }
    while(1) {
        lenInput = strlen(strIn);		/*入力文字列長*/
        if((s = strstr(strIn, strSearch)) == NULL) {
            strcat(dest, strIn);
            break;
        }
        lenBuffer = strlen(s);
        len = lenInput - lenBuffer;
        strncat(dest, strIn, len);
        strIn += len;
        strcat(dest, strReplace);
        s += lenSearch;
        strIn = s;
    }
    return dest;
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
プロトタイプ：char *trLower(char *)
引数　　　　：char *str		変換対象の文字列
戻り値　　　：char*			変換後の文字列
説明　　　　：strを全て小文字に変換する
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
char *trLower(char *str)
{

    char *p = str;

    while(*str++) {
        if('A' <= *str && *str <= 'Z')
            *str += ' ';
    }
    str = p;

    return str;
}

/*-----------------------------------------------------------------------------
ProtoType  : char *strhead( char *s1, const char *s2, int c );
Arguments  : char s1 ... 格納先変数（最低s2を格納できるサイズ分確保されている必要がある）
             const char s2 ... 複写元変数
		     int c  ... 検索文字（charでキャストする）
Returns    : char * ... s1の先頭アドレス
Description: s2からcを探し、見つかるまでs1へ代入する
-----------------------------------------------------------------------------*/
char *strhead( char *s1, char *s2, int c )
{

    char *p = s1;	/*位置保存*/

    while( *s2 != (char)c && *s2)
        *p++ = *s2++;
    *p = '\0';

    return s1;

}
