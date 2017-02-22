// TestDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"

//#define ISDEBUG//如果定义，则用于提取题库，如果不定义，则是发布版

// CTestDlg 对话框
class CTestDlg : public CDialogEx
{
// 构造
public:
	CTestDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_TEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	BOOL GetMsg(CString source, CString& phone, CString& msg, CString& other);

	CEdit m_Disp;
	CListCtrl m_List;
	CButton m_Login;
	afx_msg void OnBnClickedLogin();
	CButton m_Pass;
	afx_msg void OnBnClickedPass();

	afx_msg void OnBnClickedLogin2();
	afx_msg void OnBnClickedLogin3();
};
