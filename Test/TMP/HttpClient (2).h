/*
这里定义了一个Web类，用来get或者post网页
Visit函数使用wininet获取网页内容，并自动转码，懒人专用
*/
#ifndef WEBCLIENT_H
#define WEBCLIENT_H

#include <afx.h>
#include <Wininet.h>  

typedef struct HttpInfo
{
	CString url;//链接
	CString Header;//协议头
	CString PostData;//POST专用，提交信息（实际上Cookies也会加入到这里）
	CString Cookie;//提交Cookies,本参数传递变量时会自动回传返回的Cookies
}HttpInfo;

class HttpClient
{
public:
	CString strServerName;//域名
	INTERNET_PORT dwPort;//端口

	HttpClient(CString strServerName, INTERNET_PORT dwPort);//可以通过getServerName和getPort获取 http:80 https:443
	~HttpClient();

	CString Get(HttpInfo &httpinfo);
	CString Post(HttpInfo &httpinfo);
	
	static DWORD getPort(CString url);//取端口
	static CString getServerName(CString url, bool isPort);//取域名,并选择是否保留端口
	static CString GetPageName(CString url);//取页面地址
	static CString UpdateCookie(CString New, CString Old);//将A和B中相同的Cookie合并
	static CString GetCookie(CString Headers);
private:
	HINTERNET hOpen;
	HINTERNET hConnect;
	HINTERNET hRequest;
	CString Visit(HttpInfo &httpinfo, CString Verb);//傻瓜式自动转码获取网页内容
};
#endif