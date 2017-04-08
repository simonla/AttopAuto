/*
这里定义了一个Web类，用来get或者post网页
Visit函数使用wininet获取网页内容，并自动转码，懒人专用
*/
#ifndef WEBCLIENT_H
#define WEBCLIENT_H

#include <afx.h>
#include <Wininet.h>  
#pragma comment(lib,"wininet.lib")

class HttpClient
{
public:
	HttpClient();
	~HttpClient();

	//每次访问之后协议头都会置空
	CString Get(CString url);
	CString Post(CString url,CString PostData);
	
	//每次使用POST或者GET之后Header都会置空，所以每次使用POST或者GET都要增加协议头
	BOOL AddSendHeader(CString HeaderName, CString Value);
	BOOL AddSendHeader(CString Header);

	CString GetRecvHeader() { return strRecvHeader; }
	CString GetRecvHeader(CString HeaderName);

	BOOL AddCookie(CString CookieName, CString Value);

	CString GetCookie() { return strCookie; }
	CString GetCookie(CString CookieName);

private:
	HINTERNET hOpen;
	HINTERNET hConnect;
	HINTERNET hRequest;

	CString strHeader;	//提交协议头
	CString strRecvHeader;	//收到的协议头
	CString strCookie;	//提交Cookies,本参数传递变量时会自动回传返回的Cookies

	//傻瓜式自动转码获取网页内容
	CString Visit(CString url, CString PostData, CString Verb);
	//从url中取出需要使用的端口
	static DWORD GetPortFromUrl(CString url);
	//从url中取出域名,并选择是否保留端口
	static CString GetServerNameFromUrl(CString url, bool isPort);
	//从url中取出页面地址，没有也会返回一个/，例如：baidu.com/content/1.txt -> /content/1.txt 
	static CString GetPageNameFromUrl(CString url);
	//将A和B中相同的Cookie合并
	static CString UpdateCookie(CString New, CString Old);
	//从协议头中取出Cookie
	static CString GetCookieFromHeader(CString Header);
};

#endif