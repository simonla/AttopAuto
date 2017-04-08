// LoginDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "LoginDlg.h"
#include "afxdialogex.h"
#include "Func.h"
#include "Code.h"

CString Username;
CString Password;

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
}


BEGIN_MESSAGE_MAP(LoginDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &LoginDlg::OnBnClickedOk)
END_MESSAGE_MAP()

void LoginDlg::OnBnClickedOk()
{
	m_Username.GetWindowText(Username);
	m_Password.GetWindowText(Password);

	if (Username.IsEmpty() || Password.IsEmpty()) {
		AfxMessageBox(L"信息未输入完全，请检查！", MB_ICONERROR);
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

	return TRUE;
}