#include "stdafx.h"
#include <afxinet.h> //定义了MFC CInternetSession类等
#include "HttpClient.h"
#include "Code.h"
#include "Func.h"
#define MAXSIZE 102400
CString HttpClient::Post(HttpInfo &httpinfo)
{
	return Visit(httpinfo,TEXT("POST"));
}
CString HttpClient::Get(HttpInfo &httpinfo)
{
	return Visit(httpinfo, TEXT("GET"));
}

CString HttpClient::Visit(HttpInfo &httpinfo,CString Verb)
{
	DWORD dwServiceType;
	unsigned short nPort;
	CString strServer, strObject,strHeader(httpinfo.Header);
	CString Content;

	try
	{
		/*分解网址*/
		if (!AfxParseURL(httpinfo.url, dwServiceType, strServer, strObject, nPort))
		{
			OutputDebugString(httpinfo.url + TEXT("不是有效有网络地址！\n"));
			return TEXT("");
		}

		/*连接服务器*/
		CHttpFile* pFile;
		CHttpConnection *pServer =sess.GetHttpConnection(strServer, nPort);
		if (pServer == NULL)
		{
			OutputDebugString(TEXT("连接服务器失败！\n"));
			return TEXT("");
		}

		if (Verb.Compare(TEXT("POST")) == 0)
		{
			pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_POST, strObject, NULL, 1, NULL, NULL, INTERNET_FLAG_EXISTING_CONNECT);
		}
		else
		{
			pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_GET, strObject, NULL, 1, NULL, NULL, INTERNET_FLAG_EXISTING_CONNECT);
		}
		if (pFile == NULL)
		{
			OutputDebugString(TEXT("找不到网络地址！\n"));
			return TEXT("");
		}

		/*开始构造报文*/
		if (strHeader.Find(TEXT("Referer:")) == -1)
			strHeader = strHeader + TEXT("\r\n") + TEXT("Referer:") + httpinfo.url;
		if (strHeader.Find(TEXT("Accept:")) == -1)
			strHeader = strHeader + TEXT("\r\n") + TEXT("Accept: */*");
		if (strHeader.Find(TEXT("Accept-Language:")) == -1)
			strHeader = strHeader + TEXT("\r\n") + TEXT("Accept-Language: zh-cn");
		if (strHeader.Find(TEXT("Content-Type:")) == -1)
			strHeader = strHeader + TEXT("\r\n") + TEXT("Content-Type: application/x-www-form-urlencoded");
		if (!httpinfo.Cookie.IsEmpty())
			strHeader = strHeader + TEXT("\r\n") + TEXT("Cookie: ") + httpinfo.Cookie;
		if (strHeader.Find(TEXT("Content-Length:")) == -1&& Verb.Compare(TEXT("POST")) == 0)
		{
			CString tmp;
			tmp.Format(TEXT("%d"), httpinfo.PostData.GetLength());
			strHeader = strHeader + TEXT("\r\n") + TEXT("Content-Length: ") + tmp;
		}
		strHeader = strHeader + TEXT("\r\n\r\n");//报文结束

		if (Verb.Compare(TEXT("POST")) == 0)
		{
			char *pszData = NULL;
			pszData = UnicodeToANSI(httpinfo.PostData);
			pFile->SendRequest(strHeader, strHeader.GetLength(), pszData, strlen(pszData));
			delete[]pszData;
		}
		else
		{
			pFile->SendRequest(strHeader, strHeader.GetLength(), NULL, 0);
		}

		/*下载网页内容*/
		CString strSentence;
		DWORD dwStatus;
		DWORD dwBuffLen = sizeof(dwStatus);
		BOOL bSuccess = pFile->QueryInfo(HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwStatus, &dwBuffLen);
		if (bSuccess && dwStatus >= 200 && dwStatus<300)
		{
			char buffer[2048];
			char *sReadBuffer = new char[MAXSIZE];
			
			char *temp;
			DWORD dwReadSize = 0;//已经读入的总长度
			DWORD dwMax = MAXSIZE;//能够存储最大长度
			DWORD nReadCount = 0;//每次读入的长度

			memset(buffer, 0, 2048);
			while ((nReadCount = pFile->Read(buffer, 2048)) > 0)
			{
				if (nReadCount == 0)break;
				if (nReadCount + dwReadSize > dwMax)//内存装不下了
				{
					temp = sReadBuffer;
					sReadBuffer = new char[MAXSIZE + dwMax];//追加内存
					memcpy(sReadBuffer, temp, dwReadSize);
					delete[]temp;
					dwMax = MAXSIZE + dwMax;
				}
				memcpy(&sReadBuffer[dwReadSize], buffer, nReadCount);
				dwReadSize += nReadCount;
				memset(buffer, 0, 2048);
			}
			Content = sReadBuffer;
			Content = Content.Left(dwReadSize);
			/*将utf-8转换成Ansi*/
			int iPos = Content.Find(TEXT("utf-8"));
			if (iPos != -1 && iPos < 500 || httpinfo.Header.Find(TEXT("utf-8")) != -1)
			{
				char *p = UTF8ToANSI(sReadBuffer);
				Content = p;
				delete[]p;
				Content = Content.Left(dwReadSize);
			}
			delete[]sReadBuffer;
		}
		else
		{
			OutputDebugString(TEXT("获取网站内容失败！\n"));
			pFile->Close();
			pServer->Close();
			//sess.Close();
			return TEXT("");
		}

		
		/*从返回的数据头中提取Cookie并与原来的Cookie合并e*/
		CString SetCookie;
		DWORD iCount = 0;
		if (pFile->QueryInfo(HTTP_QUERY_SET_COOKIE, SetCookie, &iCount))
		{
			httpinfo.Cookie = UpdateCookie(SetCookie, httpinfo.Cookie);//更新Cookie
		}
		pFile->Close();
		pServer->Close();
		//sess.Close();
		return Content;
	}
	catch (...)
	{
		OutputDebugString(TEXT("向服务器post失败！\n"));
		return TEXT("");
	}

}

CString HttpClient::GetCookie(CString Headers)
{
	CString Row;
	CString Cookie;
	DWORD pos = 0;
	DWORD tmp;

	Headers = Headers + TEXT("\r\n");
	while (1)
	{
		if (pos != 0)pos += 2;
		tmp = pos;
		pos = Headers.Find(TEXT("\r\n"), pos);
		if (pos == -1 || pos == 0 || pos == Headers.GetLength() - 2)//空串或者寻找到最后一个换行符
		{
			break;
		}
		else
		{
			Row = Headers.Mid(tmp, pos - tmp);
		}
		if (Row.Find(TEXT("Set-Cookie:")) != -1)
		{
			Cookie = Cookie + SubString(Row, TEXT("Set-Cookie:"), TEXT(";")) + TEXT("; ");
		}
	}
	Cookie = Cookie.Left(Cookie.GetLength() - 2);
	return Cookie;
}
CString HttpClient::UpdateCookie(CString New, CString Old)
{
	int pos = 0;
	int tmp;
	CString Row;
	CString Row1;
	New.Remove(TEXT(' '));
	Old.Remove(TEXT(' '));
	if (New.IsEmpty())return Old;
	if (Old.IsEmpty())return New;
	while (1)
	{
		if (pos == -1)break;
		if (pos != 0)pos++;
		tmp = pos;
		pos = Old.Find(TEXT(";"), pos);
		if (pos == -1)
		{
			Row = Old.Right(Old.GetLength() - tmp);
		}
		else
		{
			Row = Old.Mid(tmp, pos - tmp);
		}

		int pos1 = 0;
		while (1)//判断老的cookie在新的cookie中是否存在，不存在则添加进去，存在则无视老的
		{
			if (pos1 == -1)
			{
				New = New + TEXT("; ") + Row;
				break;//在新的里面没有旧的，就把旧的添加到新的里面
			}
			if (pos1 != 0)pos1++;
			tmp = pos1;
			pos1 = New.Find(TEXT(";"), pos1);
			if (pos1 == -1)
			{
				Row1 = New.Right(New.GetLength() - tmp);
			}
			else
			{
				Row1 = New.Mid(tmp, pos1 - tmp);
			}
			if (New.Right(8) == TEXT("=deleted"))break;
			int i = Row1.Find(TEXT("="));
			int j = Row.Find(TEXT("="));
			if (Row1.Left(i) == Row.Left(j))break;//存在就算了

		}
	}
	return New;
}