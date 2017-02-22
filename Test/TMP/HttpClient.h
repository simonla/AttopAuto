/*
一个Http客户端
采用不同的session，可以对同一个网站同时登录不同的账号，而不会出现串号的现象
*/

#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H
#include <afxinet.h>

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
	CString Post(HttpInfo &httpinfo);
	CString Get(HttpInfo &httpinfo);
	static CString UpdateCookie(CString New, CString Old);
	static CString GetCookie(CString Headers);
private:
	CInternetSession sess;//每个session的独立标志，用于对同一网站建立多个连接
	CString Visit(HttpInfo &httpinfo, CString Verb);
};
#endif 