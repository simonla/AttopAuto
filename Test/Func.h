/*
这里定义的是一些功能性函数
*/

#ifndef FUNC_H
#define FUNC_H

#include <afx.h>
#include<time.h>
#include<sys/timeb.h>

CString SubString(CString s, CString head, CString tail);		//截取字符串
CString SubString(CString s, CString head, CString tail,DWORD &t);	//从T开始找head，tail位置返回到t，用于持续寻找字符串
DWORD GetRand(DWORD m, DWORD n);	//取随机数
DOUBLE GetRandZeroToOne();		//0到1的随机数
CString GetString(DWORD id);		//从资源文件获取字符串
CString GetRandWord(DWORD n);		//取长度为N的随机字母字符串
CString GetRandStr(DWORD m, DWORD n);	//取随机数，只不过是返回值是字符串类型
long long getCurrentTime();		//获取现在到1970年1月1日的毫秒数
BOOL TestUserName(CString User, int n);	//设置一台电脑最多登录n的用户，如果超出，则返回假
#endif