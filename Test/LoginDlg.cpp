// LoginDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "LoginDlg.h"
#include "afxdialogex.h"
#include "Func.h"
#include "Code.h"

CString Username;
CString Password;
CString Usekey;

BOOL isConfirm;//判断是否是确定按钮来关闭的对话框
// LoginDlg 对话框

IMPLEMENT_DYNAMIC(LoginDlg, CDialogEx)

LoginDlg::LoginDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOGLOGIN, pParent)
{

}

LoginDlg::~LoginDlg()
{
}

void LoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT2, m_Username);
	DDX_Control(pDX, IDC_EDIT3, m_Password);
	DDX_Control(pDX, IDOK, m_Confirm);
	DDX_Control(pDX, IDC_EDIT4, m_Usekey);
}


BEGIN_MESSAGE_MAP(LoginDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &LoginDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &LoginDlg::OnBnClickedButton1)
END_MESSAGE_MAP()

void LoginDlg::OnBnClickedOk()
{
	m_Username.GetWindowText(Username);
	m_Password.GetWindowText(Password);
	m_Usekey.GetWindowText(Usekey);

	if (Username.IsEmpty() || Password.IsEmpty() && Usekey.IsEmpty()) {
		AfxMessageBox(L"信息未输入完全，请检查！", MB_ICONERROR);
		return;
	}
	if (Usekey.Left(1).CompareNoCase(TEXT("s")) != 0)
	{
		AfxMessageBox(GetString(11), MB_ICONERROR);
		return;
	}
	if (Usekey.Left(5).CompareNoCase(GetString(13)) != 0)
	{
		AfxMessageBox(GetString(11), MB_ICONERROR);
		return;
	}
	if (Usekey.CompareNoCase(GetString(13)+TEXT("martin")) != 0)
	{
		AfxMessageBox(GetString(11), MB_ICONERROR);
		return;
	}

	isConfirm = TRUE;
	CDialogEx::OnOK();
}


BOOL LoginDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	//Username.Empty();
	//Password.Empty();
	isConfirm = FALSE;
	m_Username.SetWindowText(Username);
	m_Usekey.SetWindowText(Usekey);
#if 0
	//叶上上
	m_Username.SetWindowText(TEXT("18816803310"));
	m_Password.SetWindowText(TEXT("133133"));
	m_Usekey.SetWindowText(TEXT("supermartin"));
#endif

#if 0
	//自己
	m_Username.SetWindowText(TEXT("18375666059"));
	m_Password.SetWindowText(TEXT("Mxiaoding"));
	m_Usekey.SetWindowText(TEXT("supermartin"));
#endif

#if 0
	//me
	m_Username.SetWindowText(TEXT("小丁")); //13399805385
	m_Password.SetWindowText(TEXT("Mxiaoding"));
	m_Usekey.SetWindowText(TEXT("supermartin"));

#endif

#if 0
	//自己
	m_Username.SetWindowText(TEXT("大笨蛋"));
	m_Password.SetWindowText(TEXT("Mxiaoding"));
	m_Usekey.SetWindowText(TEXT("supermartin"));
#endif

#if 0
	//自己
	m_Username.SetWindowText(TEXT("13881333316"));
	m_Password.SetWindowText(TEXT("Mxiaoding"));
	m_Usekey.SetWindowText(TEXT("supermartin"));
#endif

#if 0
	//自己
	m_Username.SetWindowText(TEXT("傻瓜笨蛋"));
	m_Password.SetWindowText(TEXT("Mxiaoding"));
	m_Usekey.SetWindowText(TEXT("supermartin"));
#endif

#if 0
	m_Username.SetWindowText(TEXT("15923655032"));
	m_Password.SetWindowText(TEXT("741018."));
	m_Usekey.SetWindowText(TEXT("supermartin"));
#endif

#if 0
	m_Username.SetWindowText(TEXT("田鹏"));
	m_Password.SetWindowText(TEXT("960714"));
	m_Usekey.SetWindowText(TEXT("supermartin"));
#endif 

#if 0
	//叶上上室友
	m_Username.SetWindowText(TEXT("邮差"));
	m_Password.SetWindowText(TEXT("lxp520"));
	m_Usekey.SetWindowText(TEXT("supermartin"));
#endif 

#if 0
	//李悦祎
	m_Username.SetWindowText(TEXT("15948037200"));
	m_Password.SetWindowText(TEXT("005522"));
	m_Usekey.SetWindowText(TEXT("supermartin"));

#endif

#if 0
	//李朝阳
	m_Username.SetWindowText(TEXT("vimpire"));
	m_Password.SetWindowText(TEXT("1995129li"));
	m_Usekey.SetWindowText(TEXT("supermartin"));

#endif

	return TRUE;
}



void LoginDlg::OnBnClickedButton1()
{
	ShellExecute(NULL, TEXT("open"), TEXT("http://www.supermartin.cn/"), NULL, NULL, SW_SHOWNORMAL);
}
