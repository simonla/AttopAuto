/*定义了一些编码解码函数*/
#ifndef CODE_H
#define CODE_H
#include <afx.h>

char * UnicodeToANSI(const wchar_t* str);
wchar_t * UTF8ToUnicode(const char* str);
char* UTF8ToANSI(const char* str);

CString HexStrToWChars(CString s);//形如 \u515A\u8BFE\u6559\u7A0B -->> 党课教程
CString  UrlDecodeUTF8(CString str);
#endif