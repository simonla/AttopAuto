#include "stdafx.h"
#include "HttpClient.h"
#include "Code.h"
#include "Func.h"
#include &lt;exception&gt;
#define MAXSIZE 1024000

HttpClient::HttpClient(CString strServerName, INTERNET_PORT dwPort)//域名和端口
{
	this-&gt;strServerName = strServerName;
	this-&gt;dwPort = dwPort;
	hOpen = InternetOpen(TEXT("Mozilla/5.0 (Windows NT 6.3; WOW64; rv:32.0) Firefox/32.0"), INTERNET_OPEN_TYPE_DIRECT, NULL, 0, 0);
	
	if (hOpen == NULL)
	{
		throw 0;
	}
	if (!(hConnect = InternetConnect(hOpen, strServerName, dwPort, NULL, NULL, INTERNET_SERVICE_HTTP, 0,0)))
	{
		throw 0;
	}
}
HttpClient::~HttpClient()
{
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hOpen);
}

CString HttpClient::Post(HttpInfo &amp;httpinfo)
{
	return Visit(httpinfo, TEXT("POST"));
}
CString HttpClient::Get(HttpInfo &amp;httpinfo)
{
	return Visit(httpinfo, TEXT("GET"));
}

CString HttpClient::Visit(HttpInfo &amp;httpinfo, CString Verb)
{
	CString strHeaders = httpinfo.Header;//拷贝一下附加协议头
	CString strObject = GetPageName(httpinfo.url);//页面地址

	/*请求*/
	DWORD dwFlags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;
	if (dwPort==443)//https
	{
		dwFlags = dwFlags | INTERNET_FLAG_SECURE;
	}
	else
	{
		dwFlags = dwFlags | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS;
	}

	if (!(hRequest = HttpOpenRequest(hConnect, Verb, strObject, TEXT("HTTP_VERSION"), NULL, NULL, dwFlags, 0)))
	{
		return TEXT("");
	}

	/*开始构造报文*/
	if (strHeaders.Find(TEXT("Referer:")) == -1)
		strHeaders = strHeaders + TEXT("\r\n") + TEXT("Referer:") + httpinfo.url;
	if (strHeaders.Find(TEXT("Accept:")) == -1)
		strHeaders = strHeaders + TEXT("\r\n") + TEXT("Accept: */*");
	if (strHeaders.Find(TEXT("Accept-Language:")) == -1)
		strHeaders = strHeaders + TEXT("\r\n") + TEXT("Accept-Language: zh-cn");
	if (strHeaders.Find(TEXT("Content-Type:")) == -1)
		strHeaders = strHeaders + TEXT("\r\n") + TEXT("Content-Type: application/x-www-form-urlencoded");
	if (!httpinfo.Cookie.IsEmpty())
		strHeaders = strHeaders + TEXT("\r\n") + TEXT("Cookie: ") + httpinfo.Cookie;
	if (strHeaders.Find(TEXT("Content-Length:")) == -1&amp;&amp;Verb.CompareNoCase(TEXT("POST")) == 0)
	{
		CString tmp;
		tmp.Format(TEXT("%d"), httpinfo.PostData.GetLength());
		strHeaders = strHeaders + TEXT("\r\n") + TEXT("Content-Length: ") + tmp;
	}
	strHeaders = strHeaders + TEXT("\r\n\r\n");//报文结束

	/*发送报文*/
	if (Verb.CompareNoCase(TEXT("POST")) == 0)
	{
		char *pszData = NULL;
		pszData = UnicodeToANSI(httpinfo.PostData);
		if (!HttpSendRequest(hRequest, strHeaders, strHeaders.GetLength(), pszData, strlen(pszData)))
		{
			delete[]pszData;
			return TEXT("");
		}
		delete[]pszData;
	}
	else
	{
		if (!HttpSendRequest(hRequest, strHeaders, strHeaders.GetLength(), TEXT(""), 0))
		{
			return TEXT("");
		}
	}

	/*接收数据*/
	char buffer[1024];
	CString ReadBuffer;
	char *sReadBuffer = new char[MAXSIZE];//一个网页1000K够了吧
	char *temp;
	DWORD dwSize = 0;//每次读入的长度
	DWORD dwReadSize = 0;//已经读入的总长度
	DWORD dwMax =MAXSIZE;//能够存储最大长度
	while (1)
	{
		InternetReadFile(hRequest, buffer, 1024, &amp;dwSize);
		if (dwSize == 0)break;
		if (dwSize + dwReadSize &gt; dwMax)//内存装不下了
		{
			temp = sReadBuffer;
			sReadBuffer = new char[MAXSIZE + dwMax];//追加内存
			memcpy(sReadBuffer,temp, dwReadSize);
			delete []temp;
			dwMax = MAXSIZE + dwMax;
		}
		memcpy(&amp;sReadBuffer[dwReadSize], buffer, dwSize);
		dwReadSize += dwSize;
	}
	ReadBuffer = sReadBuffer;
	ReadBuffer = ReadBuffer.Left(dwReadSize);
	int iPos = ReadBuffer.Find(TEXT("utf-8"));//将utf-8转换成Ansi
	if (iPos != -1 &amp;&amp; iPos &lt; 500 || httpinfo.Header.Find(TEXT("utf-8")) != -1)
	{
		char *p= UTF8ToANSI(sReadBuffer);
		ReadBuffer = p;	
		delete[]p;
		ReadBuffer = ReadBuffer.Left(dwReadSize);
	}
	delete []sReadBuffer;

	/*从返回的数据头中提取Cookie并与原来的Cookie合并e*/
	HttpQueryInfo(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, NULL, &amp;dwSize, NULL);
	TCHAR* AnsiHeaders = new TCHAR[dwSize + 1];
	if (!HttpQueryInfo(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, (LPVOID)AnsiHeaders, &amp;dwSize, NULL))
	{
		delete[]AnsiHeaders;
		return ReadBuffer;
	}
	CString Headers = AnsiHeaders;
	delete[]AnsiHeaders;
	httpinfo.Cookie = UpdateCookie(GetCookie(Headers), httpinfo.Cookie);//更新Cookie

	InternetCloseHandle(hRequest);
	return ReadBuffer;
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
CString HttpClient::UpdateCookie(CString New,CString Old)
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
		if (pos != 0)pos ++;
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
DWORD HttpClient::getPort(CString url)
{
	CString port;
	int iPos;
	port = getServerName(url,true);
	iPos = port.Find(TEXT(":"));
	if (iPos != -1)
	{
		iPos = port.Find(TEXT(":"), iPos);
		if (iPos != -1)
		{
			port = port.Right(port.GetLength() - iPos);
			return _ttoi(port);//转换到整数
		}
	}
	
	if (url.Left(5) == TEXT("https"))return 443;
	return 80;
}
CString HttpClient::getServerName(CString url,bool isPort)
{
	CString ServerName = url.MakeLower();
	if (ServerName.Right(1) != TEXT("/"))ServerName = ServerName + TEXT("/");
	if (ServerName.Left(8) == TEXT("https://"))
	{
		ServerName = SubString(ServerName, TEXT("https://"),TEXT("/"));
	}
	else 
	{
		if (ServerName.Find(TEXT("http://")) == -1)ServerName = TEXT("http://") + ServerName;
		ServerName = SubString(ServerName, TEXT("http://"), TEXT("/"));
	}
	if (isPort == true)return ServerName;//是否保留端口
	int iPos = ServerName.Find(TEXT(":"));
	if (iPos == -1)return ServerName;
	return ServerName.Left(iPos-1);
}
CString HttpClient::GetPageName(CString url)
{
	CString PageName;
	CString ServerName = getServerName(url,NULL);
	int iPos = url.Find(ServerName);
	iPos = url.Find(TEXT("/"),iPos);
	if (iPos &gt; 0)
	{
		PageName = url.Right(url.GetLength() - iPos);
	}
	else
	{
		PageName = TEXT("/");
	}
	return PageName;
}