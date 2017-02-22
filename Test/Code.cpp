#include "stdafx.h"
#include "Code.h"
#include "Func.h"
#include <string>

long Wchart2Hex(wchar_t wch)
{
	return (long)wch;
}
wchar_t Hex2Wchart(long hex)//这个是UNICODE编码上的
{
	wchar_t wch = -1;
	if (hex <0)	return wch;
	wch = (wchar_t)hex;

	return wch;
}
long MIndex(int num, int index)
{
	long s = 1;
	int i = 0;
	while (i<index)
	{
		s *= num;
		i++;
	}

	return s;
}
int Char2Hex(char ch)
{
	int n = -1;
	switch (ch)
	{
	case '0':	n = 0;	break;
	case '1':	n = 1;	break;
	case '2':	n = 2;	break;
	case '3':	n = 3;	break;
	case '4':	n = 4;	break;
	case '5':	n = 5;	break;
	case '6':	n = 6;	break;
	case '7':	n = 7;	break;
	case '8':	n = 8;	break;
	case '9':	n = 9;	break;
	case 'A':
	case 'a':	n = 10;	break;
	case 'B':
	case 'b':	n = 11;	break;
	case 'C':
	case 'c':	n = 12;	break;
	case 'D':
	case 'd':	n = 13;	break;
	case 'E':
	case 'e':	n = 14;	break;
	case 'F':
	case 'f':	n = 15;	break;
	default:	break;
	}
	return n;
}
long String2Hex(char* string, int strlen)
{
	long hex = -1;
	int i = 0, n = 0;
	char *p = string;
	p += strlen - 1;
	if (string == NULL)	return hex;
	if (strlen <= 0 || strlen > 10)	return hex;

	hex = 0;
	do
	{
		n = Char2Hex(*p--);
		hex += n*MIndex(16, i++);
	} while (i<strlen);

	return hex;
}
CString HexStrToWChars(CString hexStr)
{
	CString WChars;
	CString s;
	CString str;
	char *ch;
	DWORD pos = 0;
	DWORD tmp = 1;
	DWORD Add = 1;
	long hex;
	s = hexStr;
	s.Replace(TEXT("\\u"), TEXT("0x"));
	if (s.GetLength() < 4)return s;
	s = s + TEXT("0x");//设置哨兵
					   //int wchsLen= s.GetLength() / 4 + (hexstrlen%eachchar>0 ? 1 : 0);
	while (1)
	{
		if (pos != 0)pos = pos + Add;
		tmp = pos;
		pos = s.Find(TEXT("0x"), pos);
		//if(pos==-1)
		//if(pos==0)
		if ( pos == s.GetLength() - 2)//寻找到最后一个
		{
			str=str+ s.Right(s.GetLength() - tmp);
			break;
		}
		else
		{
			str = str + s.Mid(tmp, pos - tmp);
			WChars = s.Mid(pos + 2, 4); //0x5B66长度0x是2  5B66是4
		}
		ch = UnicodeToANSI(WChars);
		hex = String2Hex(ch, strlen(ch));
		delete []ch;
		if (hex < 0)//不是正确的
		{
			str = str + TEXT("0x");//懒得判断是0x还是\u了
			Add = 2;
			continue;
		}
		else
		{
			Add = 6;
			str = str + Hex2Wchart(hex);
		}
	}
	return str.Left(str.GetLength()-2);
}

char * UnicodeToANSI(const wchar_t* str)
{
	char* result;
	int textlen;
	textlen = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	result = (char *)malloc((textlen + 1)*sizeof(char));
	memset(result, 0, sizeof(char) * (textlen + 1));
	WideCharToMultiByte(CP_ACP, 0, str, -1, result, textlen, NULL, NULL);
	return result;
}
wchar_t * UTF8ToUnicode(const char* str)
{
	int textlen;
	wchar_t * result;
	textlen = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	result = (wchar_t *)malloc((textlen + 1)*sizeof(wchar_t));
	memset(result, 0, (textlen + 1)*sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, str, -1, (LPWSTR)result, textlen);
	return result;
}
char* UTF8ToANSI(const char* str)
{
	wchar_t * s;
	char * result;
	s = UTF8ToUnicode(str);
	result = UnicodeToANSI(s);
	delete[]s;
	return result;
}
char* UnicodeToUtf8(const char* unicode)
{
	int len;
	len = WideCharToMultiByte(CP_UTF8, 0, (const wchar_t*)unicode, -1, NULL, 0, NULL, NULL);
	char *szUtf8 = (char*)malloc(len + 1);
	memset(szUtf8, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, (const wchar_t*)unicode, -1, szUtf8, len, NULL, NULL);
	return szUtf8;
}

CString  UrlDecodeUTF8(CString str)
{
	std::string tt;
	std::string dd;
	CString ret;
	char * p;
	p = UnicodeToUtf8((LPSTR)(LPCTSTR)str);
	tt = p;
	delete[]p;
	//GB2312ToUTF_8(tt, str, (int)strlen(str));

	size_t len = tt.length();
	for (size_t i = 0; i<len; i++)
	{
		if (isalnum((BYTE)tt.at(i)))
		{
			char tempbuff[2] = { 0 };
			sprintf(tempbuff, "%c", (BYTE)tt.at(i));
			dd.append(tempbuff);
		}
		else if (isspace((BYTE)tt.at(i)))
		{
			dd.append("+");
		}
		else
		{
			char tempbuff[4];
			sprintf(tempbuff, "%%%X%X", ((BYTE)tt.at(i)) >> 4, ((BYTE)tt.at(i)) % 16);
			dd.append(tempbuff);
		}

	}
	ret= dd.c_str();
	return ret;
}
