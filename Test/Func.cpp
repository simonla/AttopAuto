#include "stdafx.h"
#include "Func.h"
#include "Code.h"

CString GetRandWord(DWORD n)//获取n个随机字母
{
	//srand((unsigned)time(NULL));
	int i;
	CString str;
	char c;
	int t;
	for (i = 0; i < n; i++)
	{
		t = rand() % 3;
		if (t>1)
		{
			t = 'a';
		}
		else
		{
			t = 'A';
		}
		c = t + rand() % 26;
		str = str + c;
	}
	return str;
}
DWORD GetRand(DWORD m, DWORD n)//m <= rand() % (n - m + 1) + m <= n
{
	//srand((unsigned)time(NULL));
	return rand() % (n - m + 1) + m;
}
DOUBLE GetRandZeroToOne()
{
	//srand((unsigned)time(NULL));
	return  (rand() % 10) / (float)10;//生成0-1间的随机数。
}
CString GetRandStr(DWORD m, DWORD n)//m <= rand() % (n - m + 1) + m <= n
{
	CString str;
	str.Format(TEXT("%d"), GetRand(m, n));
	return str;
}
CString GetString(DWORD id)//从资源中提取字符串
{
	CString ss;
	/*
	int c;
	c = LoadString(NULL, id, ss.GetBuffer(1024000), 1024000);
	ss.ReleaseBuffer();
	ss.Left(c);
	*/
	ss.LoadString(id);
	return ss;
}
CString SubString(CString s, CString head, CString tail)
{
	int iPos = s.Find(head);
	if (iPos == -1)return TEXT("");
	iPos += head.GetLength();
	int iPos1 = s.Find(tail, iPos);
	if (iPos1 == -1)return TEXT("");
	return s.Mid(iPos, iPos1 - iPos);
}
CString SubString(CString s, CString head, CString tail,DWORD &t)
{
	int iPos = s.Find(head, t);
	if (iPos == -1)return TEXT("");
	iPos += head.GetLength();
	int iPos1 = s.Find(tail, iPos);
	if (iPos1 == -1)return TEXT("");
	t = iPos1;
	return s.Mid(iPos, iPos1 - iPos);
}
long long getCurrentTime()
{
	long long time_last;
	time_last = time(NULL);     //总秒数  
	struct timeb t1;
	ftime(&t1);
	//CString strTime;
	//strTime.Format(_T("%lldms"), t1.time * 1000 + t1.millitm);  //总毫秒数  
	//OutputDebugString(strTime);
	return t1.time * 1000 + t1.millitm;
}
BOOL TestUserName(CString User, int n) {
	/*简单的限制每台电脑最多运行n个账号，防止有人卖软件
	还能使用返回真，不能使用返回假*/
	HKEY  hKey;
	CString KeyName;
	CStringArray ret;


	//打开注册表
	if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\SKSoftware"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) != ERROR_SUCCESS) {
		return TRUE;
	}

	//查询当前所有已经用过的用户
	CString Value;
	DWORD dwSize;
	BYTE *data;
	DWORD lpType = REG_SZ;
	for (int i = 0; i < n; i++) {
		KeyName.Format(TEXT("%d"), i);

		data = new BYTE[256];
		dwSize = 256;
		
		//获取键值内容
		if (RegQueryValueEx(hKey, KeyName, NULL, &lpType, data, &dwSize) != ERROR_SUCCESS) {
			continue;
		}
		
		//获取到一个使用过的用户名
		Value.Format(TEXT("%s"), data);
		ret.Add(Value);

		delete[]data;
	}

	//RegCloseKey(hKey);

	for (int i = 0; i < ret.GetSize(); i++) {
		//如果已经被注册过,直接返回成功
		if (User == ret[i]) {
			RegCloseKey(hKey);
			return TRUE;
		}
		//注册表中没有，这是一个新用户
		if (i == ret.GetSize() - 1) {

			if (i == n - 1) {
				//超过了能够使用的最大用户数
				RegCloseKey(hKey);
				return FALSE;
			}
			else {
				//还能接受一个新用户
				break;
			}

		}
	}

	//在注册表中写入新用户的信息
	KeyName.Format(TEXT("%d"), ret.GetSize());

	RegSetValueEx(hKey, KeyName, NULL, REG_SZ,(BYTE*)User.GetBuffer(0), User.GetLength()*2+2);
	User.ReleaseBuffer();

	RegCloseKey(hKey);
	return TRUE;
}