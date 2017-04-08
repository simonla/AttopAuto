// TestDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Test.h"
#include "TestDlg.h"

#include "HttpClient.h"
#include "Func.h"
#include "code.h"
#include "LoginDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define POSTTKTIME 3000

CString str;		//返回值
CString PostData;	//懒得在每个函数里定义了，直接定义成全局变量
CString Header;		//协议头而已
CString DWRSESSIONID;	//每次访问都需要GetScriptSessionId，DWRSESSIONID是必不可少的
HttpClient w;


CStringArray AllCourseID;	//所有的课程代码
CStringArray AllCourseName;	//所有的课程名称
CStringArray Quene_Section;	//留给刷时间的章节，在刷题时被赋值，在刷时间时使用
CString CourseAnswer;		//该课程的答案

BOOL Endthread;			//结束刷题线程

extern CString Username;
extern CString Password;
extern BOOL isConfirm;		//判断是否是确定按钮来关闭的对话框

BOOL login();
DWORD WINAPI ThreadNextCourse(LPVOID pParam);
void ThreadNextSection(CString CourseID, CString CourseName, CStringArray &AllSectionID, CStringArray &AllSectionName);
void ThreadPostExes(CString CourseID, CString CourseName, CString SectionID, CString SectionName);
void InitPostTime();
void PostTime(CString CourseID, CString CourseName, CString SectionID, CString SectionName);	//刷时间
void MediaPj(CString allinfo, CString CourseName, CString SectionName);				//媒体评价

BOOL WriteAnswerToFile(CString path, CString Answer);
CString ReadAnswerFromFile(CString path);
BOOL RefreshAnswerFile();			//从网站上下载答案
void EndCourse();				//结束刷题
BOOL RefreshCourse();				//刷新课程
CString GetSectionStatus(CString CourseID, CString SectionID, DWORD batchId);
BOOL RefreshCourseGrade(CString courseid);	//刷新某课程的成绩
BOOL GetSection(CString Courseid, CStringArray &Sectionid, CStringArray &Sectionname);//获取Courseid对应的章节
CString GetScriptSessionId();			//获取ScriptSessionId
CString OrderAnswer(CString Answer, DWORD n);

void OutPut(CString s)
{
	/*在编辑框中输出内容*/
	AfxGetMainWnd()->GetDlgItem(IDC_EDIT_PUT)->SetWindowText(s);
	UpdateWindow(AfxGetMainWnd()->GetDlgItem(IDC_EDIT_PUT)->m_hWnd);
}
void OutPutAppend(CString s)
{
	/*在编辑框中附加内容s*/
	CEdit* edit = (CEdit*)(AfxGetMainWnd()->GetDlgItem(IDC_EDIT_PUT));
	CString str;
	edit->GetWindowText(str);
	str = str + s;
	edit->SetWindowText(str);
	edit->LineScroll(edit->GetLineCount());//滚动到最后一行
	UpdateWindow(AfxGetMainWnd()->GetDlgItem(IDC_EDIT_PUT)->m_hWnd);
}

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};
CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}
BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()
CTestDlg::CTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}
void CTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PUT, m_Disp);
	DDX_Control(pDX, IDC_LIST3, m_List);
	DDX_Control(pDX, IDC_LOGIN, m_Login);
	DDX_Control(pDX, IDC_PASS, m_Pass);
}
BEGIN_MESSAGE_MAP(CTestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_LOGIN, &CTestDlg::OnBnClickedLogin)
	ON_BN_CLICKED(IDC_PASS, &CTestDlg::OnBnClickedPass)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST3, &CTestDlg::OnLvnItemchangedList3)
END_MESSAGE_MAP()
BOOL CTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu = TEXT("关于");
		//strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_Disp.SetWindowText(GetString(12));

	m_List.SetExtendedStyle(LVS_EX_FULLROWSELECT);//整行选择
	m_List.InsertColumn(0, TEXT("课程代码"), LVCFMT_LEFT, 80);
	m_List.InsertColumn(1, TEXT("课程名称"), LVCFMT_LEFT, 270);
	m_List.InsertColumn(2, TEXT("成绩"), LVCFMT_LEFT, 70);
	m_List.InsertColumn(3, TEXT("学习期限"), LVCFMT_LEFT, 110);
	m_List.InsertColumn(4, TEXT("状态"), LVCFMT_LEFT, 70);
	
	m_Login.GetFocus();

	srand((unsigned)time(NULL));
	

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}
void CTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}
void CTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}
HCURSOR CTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CTestDlg::OnBnClickedLogin()
{
	RefreshAnswerFile();
	do 
	{
		LoginDlg dlg;
		dlg.DoModal();
		if (!isConfirm)return;
	} while (!login());

#ifndef ISDEBUG
	//设置一台电脑最多两个用户使用
	if (TestUserName(Username, 1000) == FALSE) {
		AfxMessageBox(GetString(16), MB_ICONERROR);
		return;
	}
#endif

	/*登录成功*/
	OutPut(TEXT("登陆成功，开始获取你正在进行的课程...没有的话要去NPC那领任务"));

	if (!RefreshCourse())
	{
		OutPut(TEXT("登录成功，但查询有哪些课程失败，请重新登录！"));
		AfxMessageBox(TEXT("登录成功，但查询有哪些课程失败，请重新登录！"), MB_OK | MB_ICONWARNING);
		return ;
	}
	if (m_List.GetItemCount() == 0)
	{
		AfxMessageBox(TEXT("登录成功，但是伙计，\n你好像没有任何课程也，快去领取点任务再来撒！"), MB_OK | MB_ICONWARNING);
	}
	else
	{
		m_Pass.EnableWindow(TRUE);
		m_Pass.SetFocus();
		OutPut(TEXT("登录完成了，点上面的刷课按钮开始刷课呗！吼吼吼~\r\n提示：如果长时间没有开始刷课，那么会需要重新登录！"));
		AfxMessageBox(TEXT("登录完成了，快点开始刷课啦！...洗刷刷洗刷刷！"), MB_OK | MB_ICONINFORMATION);
	}
}
BOOL login()
{
	CString str;
	/*登录，并返回登录成功后的httpinfo和DWRSESSIONID*/

	OutPut(TEXT("正在登录..."));
	/*模拟第一次打开网页并获取到JSESSIONID，有点慢*/
	str = w.Get(TEXT("http://www.attop.com/"));
	/*如果没有没法获得就手动生成一个JSESSIONID*/
	if (w.GetCookie().IsEmpty())
	{
		CString JSESSIONID;
		JSESSIONID = TEXT("AAFE06E597DA8BCE86043B3099E") + GetRandStr(10000, 99999);//构建随机JSESSIONID
		OutputDebugString(TEXT("手动生成的JSESSIONID：") + JSESSIONID + TEXT("\n"));
		w.AddCookie(TEXT("JSESSIONID"), JSESSIONID);
	}
	else
	{
		OutputDebugString(TEXT("获得") + w.GetCookie() + TEXT("\n"));
	}

	/*获取DWRSESSIONID*/
	w.AddSendHeader(TEXT("Referer"), TEXT("http://www.attop.com/index.htm"));
	str = w.Post(GetString(101), GetString(102));

	DWRSESSIONID = SubString(str, TEXT("r.handleCallback(\"0\",\"0\",\""), TEXT("\");"));
	OutputDebugString(TEXT("获得的DWRSESSIONID：") + DWRSESSIONID + TEXT("\n"));
	if (DWRSESSIONID.IsEmpty())
	{
		AfxMessageBox(TEXT("获取DWRSESSIONID错误,你联网没有哟！"), MB_OK | MB_ICONERROR);
		return FALSE;
	}
	else
	{
		w.AddCookie(TEXT("DWRSESSIONID"), DWRSESSIONID);
	}
	OutputDebugString(TEXT("获得的Cookies：") + w.GetCookie() + TEXT("\n"));

	/*获取图片验证码*/
	w.Get(GetString(LOGIN_IMAGE));
	CString authorCode = w.GetCookie(TEXT("rand"));
	w.AddSendHeader(GetString(105));


	/*接下来开始登录*/
	w.AddSendHeader(GetString(LOGIN105));
	w.AddCookie(TEXT("rand"), GetRandStr(1000, 9999));
	PostData = GetString(LOGIN104) + GetScriptSessionId();
	PostData.Replace(TEXT("Username"), Username);
	PostData.Replace(TEXT("Password"), Password);
	PostData.Replace(TEXT("AuthorCode"), authorCode);
	str = w.Post(GetString(LOGIN103), PostData);

	OutputDebugString(TEXT("登陆后返回的内容：\n") + str + TEXT("\n"));

	if (str.Find(TEXT("flag:-4")) != -1|| str.Find(TEXT("flag:20")) != -1)
	{
		AfxMessageBox(TEXT("你已多次输入错误的账号或密码，请重启软件！"), MB_OK | MB_ICONERROR);
		return FALSE;
	}
	if (str.Find(TEXT("flag:1")) == -1)
	{
		AfxMessageBox(TEXT("登录失败，你的用户名或密码是不是输错了，检查一下！"), MB_OK | MB_ICONWARNING);
		return FALSE;
	}
	return TRUE;
}

BOOL pressed;
void CTestDlg::OnBnClickedPass()
{
	if (pressed)
	{
		if (AfxMessageBox(TEXT("软件正在刷课，你确定要停止吗？"), MB_ICONQUESTION | MB_OKCANCEL) != IDOK)return;
		m_Pass.SetWindowText(TEXT("停止中"));
		m_Pass.EnableWindow(FALSE);

		Endthread = TRUE;
		pressed = FALSE;
	}
	else
	{
		OutputDebugString(TEXT("需要刷的课程：\n"));
		for (int i = 0; i < m_List.GetItemCount(); i++)
		{
#ifdef ISDEBUG
			OutputDebugString(m_List.GetItemText(i, 0) + TEXT(".") + m_List.GetItemText(i, 1) + TEXT("\n"));
			AllCourseID.Add(m_List.GetItemText(i, 0));
			AllCourseName.Add(m_List.GetItemText(i, 1));
#else
			if (m_List.GetItemText(i, 4) == TEXT("学习中") && m_List.GetItemText(i, 2) != TEXT("100"))
			{
				/*
				pszFileName = TEXT("TK\\") + m_List.GetItemText(i, 0) + TEXT(".txt");
				if (!myFile.Open(pszFileName, CFile::modeNoTruncate | CFile::typeBinary | CFile::modeRead | CFile::shareDenyNone))
				{
					AfxMessageBox(m_List.GetItemText(i, 0) + TEXT(" ") + m_List.GetItemText(i, 1)+TEXT("\n该课程没有题库，不能自动做题，请到作者网站下载！") , MB_ICONINFORMATION);
				}
				else
				{
					myFile.Close();
				}
				*/
				OutputDebugString(m_List.GetItemText(i, 0) + TEXT(".") + m_List.GetItemText(i, 1) + TEXT("\n"));
				AllCourseID.Add(m_List.GetItemText(i, 0));
				AllCourseName.Add(m_List.GetItemText(i, 1));
			}
#endif
		}
#ifndef ISDEBUG
		if (AllCourseID.GetCount()==0)
		{
			AfxMessageBox(TEXT("大兄弟，你的所有课程中没有需要刷的课，你可真是学霸，学完了呢！\n如果学完了快去结业吧！"),MB_OK);
			return;
		}
#endif
		m_Pass.SetWindowText(TEXT("停止刷课"));
		pressed = TRUE;
		Endthread = FALSE;
		OutPut(TEXT("开始刷课：\r\n"));

		AfxBeginThread((AFX_THREADPROC)ThreadNextCourse, NULL);//刷题启动
		//AfxBeginThread((AFX_THREADPROC)ThreadPostTime, NULL);//刷时间启动
	}
	
}

DWORD WINAPI ThreadNextCourse(LPVOID pParam)//刷题总流程,换课程
{
	/*主要是换课程，一个课程刷完了，刷下一个课程，并取出课程所有章节*/

	CString CourseID;//当前进行的课程代码
	CString CourseName;//当前进行的课程名字
	CStringArray AllSectionID;//每个课程里面每节的ID
	CStringArray AllSectionName;//每个课程里面每节的名字

#ifndef ISDEBUG
	if (!RefreshAnswerFile()) {
		//程序有更新，没更新不能继续使用
		return 0;
	}
#endif
	for (int i = 0; i <= AllCourseID.GetCount(); i++)
	{
		if (i== AllCourseID.GetCount())//所有课程题和评价都刷完了
		{
#ifndef ISDEBUG 
			if (Endthread == TRUE)EndCourse();
			InitPostTime();//刷时间启动
#else
			EndCourse();
#endif
			return 0;
		}
		else//还有新的课程
		{
			CourseID = AllCourseID.GetAt(i);
			CourseName = AllCourseName.GetAt(i);
		}

		if (GetSection(CourseID,AllSectionID,AllSectionName))//获取所有章节
		{
#ifdef ISDEBUG 
			CString tmp;
			tmp.Format(TEXT("%lld"), GetCurrentTime());
			tmp = TEXT("<Version>") + tmp + TEXT("</Version>\r\n");
			WriteAnswerToFile(TEXT("TK\\") + CourseID + TEXT(".txt"), tmp);
#else
			CourseAnswer.Empty();
			CourseAnswer = ReadAnswerFromFile(TEXT("TK\\") + CourseID + TEXT(".txt"));//读取课程答案
#endif
			if (Endthread == TRUE)EndCourse();
			ThreadNextSection(CourseID, CourseName, AllSectionID, AllSectionName);//开始当前课程
			if (Endthread == TRUE)EndCourse();
		}
		else
		{
			OutPut(CourseID + TEXT(" ") + CourseName + TEXT("\r\n获取课程章节失败！\r\n"));
		}
	}
	return 0;
}
void ThreadNextSection(CString CourseID, CString CourseName, CStringArray &AllSectionID, CStringArray &AllSectionName)
{
	/*切换章节，一个课程的某一节刷完后，需要切换到下一节*/
	CString SectionID;//当前进行节的ID
	CString SectionName;//当前进行节的名字

	for (int i = 0; i <= AllSectionID.GetCount(); i++)
	{
		if (i == AllSectionID.GetCount())//所有章节都刷完了
		{
			RefreshCourseGrade(CourseID);
			OutputDebugString(TEXT("已经刷完该课程\n"));
			return ;//下一课程
		}
		else//还有章节
		{
			SectionID = AllSectionID.GetAt(i);
			SectionName = AllSectionName.GetAt(i);
		}
		OutputDebugString(TEXT("开始刷该节课程：") + SectionID + TEXT(".") + SectionName + TEXT("\n"));
		ThreadPostExes(CourseID, CourseName, SectionID, SectionName);
		RefreshCourseGrade(CourseID);
		if (Endthread == TRUE)EndCourse();
	}
}
void ThreadPostExes(CString CourseID, CString CourseName, CString SectionID, CString SectionName)
{
	/*将某章的题一起提交或者提取某章的所有答案*/

	CString Exes;//一道完整的题
	CString ExesName;//题的类型和第几题
	CString Question;//问题和备选答案
	CString Answer;//待提交的答案
	CString AnswerID;//题的ID
	CString Selection;//某个选项
	CString allinfo;
	CString tmp;
	DWORD Pos = 0;
	DWORD Pos1 = 0;

	/*获取本章状态*/
	CString Status;
	BOOL Status_xiti=FALSE;
	BOOL Status_meiti=FALSE;
	Status = GetSectionStatus(CourseID, SectionID, 1);
	if (Status.Find(TEXT("OK</strong> \\r\\n                 <span class=\\\"explain_rate\\\"><a href=\\\"javascript:;\\\" onclick=\\\"atPage(\\\'时间说明")) == -1)
	{
		/*时间没刷完，加入待刷队列*/
		Quene_Section.Add(TEXT("|") + CourseID + TEXT("|") + CourseName + TEXT("|") + SectionID + TEXT("|") + SectionName + TEXT("|"));
	}
	if (Status.Find(TEXT("OK</strong> \\r\\n                 <span class=\\\"explain_rate\\\"><a href=\\\"javascript:;\\\" onclick=\\\"atPage(\\\'习题说明")) != -1)
	{
		/*习题刷完了*/
		Status_xiti = TRUE;
	}
	if (Status.Find(TEXT("OK</strong> \\r\\n             <span class=\\\"explain_rate\\\"><a href=\\\"javascript:;\\\" onclick=\\\"atPage(\\\'媒材说明")) != -1)
	{
		/*媒体刷完了*/
		Status_meiti = TRUE;
	}
#ifndef ISDEBUG
	if (Status_xiti == TRUE&&Status_meiti == TRUE)
	{
		OutPutAppend(TEXT("本节已完成：") + CourseName + TEXT(" ") + SectionName + TEXT("\r\n"));
		return;//全部刷完了就返回
	}
#endif
	
	/*开始获取本节的所有内容，包括媒体评价和习题*/

	Header = GetString(117) + GetString(11);
	Header.Replace(TEXT("CourseID"), CourseID);
	Header.Replace(TEXT("SectionID"), SectionID);
	w.AddSendHeader(Header);
	PostData = GetString(116) + GetScriptSessionId();
	PostData.Replace(TEXT("CourseID"), CourseID);
	PostData.Replace(TEXT("SectionID"), SectionID);
	str = w.Post(GetString(115), PostData);

#ifndef ISDEBUG
	if(Status_meiti!=TRUE)MediaPj(str, CourseName, SectionName);//先媒体评价
#endif

	str = SubString(str, TEXT("<span class=\\\"delNum"), TEXT("</dd>"));
	str = HexStrToWChars(str);

	if (Endthread == TRUE)EndCourse();
#ifdef ISDEBUG
	/*将本章的题全部提取出来，用于后台的提取*/
	WriteAnswerToFile(TEXT("TK\\") + CourseID + TEXT(".")+ CourseName+TEXT(".txt"), TEXT("\r\n\r\n《")+CourseID + TEXT(".") + CourseName + TEXT("  ") + SectionName+TEXT("》\r\n"));
	while (1)
	{
		Exes.Empty(); ExesName.Empty(); Question.Empty(); AnswerID.Empty(); tmp.Empty(); Selection.Empty();
		Pos1 = 0;
		Exes = SubString(str, TEXT("<li name=\\\"xt\\\""), TEXT("\\r\\n                   \\r\\n                 </li>"), Pos);//每一道题
		if (Exes.IsEmpty())
		{
			Exes = SubString(str, TEXT("<li>"), TEXT("\\r\\n\\r\\n                  \\r\\n"), Pos);//两种可能的结尾
		}
		if (Exes.IsEmpty())//这一节的题完了
		{
			Answer = Answer.Left(Answer.GetLength() - 3);//去掉末尾的&;&
			Answer = TEXT("<") + SectionID + TEXT(">") + Answer + TEXT("</") + SectionID + TEXT(">\r\n");
			WriteAnswerToFile(TEXT("TK\\") + CourseID + TEXT(".txt"),Answer);
			OutputDebugString(TEXT("\n该节的题已经刷完，下一节！") + SectionName + TEXT("\r\n"));
			return ;//到下一节！
		}
		//if (Exes.Find(TEXT("ed-ans")) == -1)continue;//判断是否回答正确，如果不正确就不要
		ExesName = SubString(Exes, TEXT("<h5>"), TEXT("\\r\\n"), Pos1);//第几题
		AnswerID = SubString(Exes, TEXT("id=\\\""), TEXT("\\\""));
		AnswerID = AnswerID.Right(AnswerID.GetLength() - 3);//xt_4260 dx_2182 mx_2180 pd_2187只要后面的数字
		Answer = Answer + AnswerID + TEXT("&=&");

		if (ExesName.Find(TEXT("填空题")) != -1)
		{
			DWORD Last = 0;
			DWORD Pos2 = 0;
			Question = SubString(Exes, TEXT("<p>\\r\\n\\t\\t\\t\\t\\t\\t"), TEXT("\\r\\n                   </p>"), Pos1);
			while (1)//取出有几个空
			{
				Pos2 = Question.Find(TEXT("\\r\\n                             "), Last);
				if (Pos2 == -1)
				{
					Pos2 = Question.Find(TEXT("\\r\\n\\t\\t\\t\\t\\t "), Last);
				}
				if (Pos2 == -1)
				{
					tmp = tmp + Question.Right(Question.GetLength() - Last);
					break;
				}
				tmp = tmp + Question.Mid(Last, Pos2 - Last) + TEXT("【   】");
				Last = Pos2;
				Answer = Answer + SubString(Question, TEXT("value=\\\""), TEXT("\\\" />）"), Last) + TEXT("&,&");
				if (Question.Find(TEXT("\\r\\n\\t\\t\\t\\t\\t\\t"), Last) == -1)
				{
					Last = Last + 6;//  \" />）的长度
				}
				else
				{
					Last = Question.Find(TEXT("\\r\\n\\t\\t\\t\\t\\t\\t"), Last);
					Last = Last + 16;// \r\n\t\t\t\t\t\t的长度
				}
			}
			if(Answer.Right(3)=TEXT("&,&"))
				Answer = Answer.Left(Answer.GetLength() - 3);//去掉末尾的&,&
			Question = TEXT("\r\nQuestion：") + tmp + TEXT("\r\nAnswer：") + Answer + TEXT("\r\n");
			//OutputDebugString(TEXT("\nQuestion：") + Question);
		}
		else if (ExesName.Find(TEXT("单选题")) != -1 || ExesName.Find(TEXT("多选题")) != -1 || ExesName.Find(TEXT("判断题")) != -1)
		{
			CString xuanze;
			int n = 0;
			Question = SubString(Exes, TEXT("<p>"), TEXT("</p>"), Pos1);
			//OutputDebugString(TEXT("\nQuestion：") + Question + TEXT("\nAnswer：\n"));
			Question = TEXT("\r\nQuestion：") + Question + TEXT("\r\nAnswer：\r\n");
			while (1)
			{
				Selection = SubString(Exes, TEXT("<li"), TEXT("\\r\\n"), Pos1);
				if (Selection.IsEmpty())break;
				if ((Selection.Find(TEXT("checked=\\\"checked\\\"")) != -1))
				{
					xuanze = xuanze + SubString(Selection, TEXT("value=\\\""), TEXT("\\\"")) + TEXT(",");
					n++;
					//Answer = Answer + SubString(Selection, TEXT("value=\\\""), TEXT("\\\"")) + TEXT(",");
					Question = Question + TEXT("√");
					//OutputDebugString(TEXT("√"));
				}
				Question = Question + SubString(Selection, TEXT("/>"), TEXT("</li>")) + TEXT("\r\n");
				//OutputDebugString(SubString(Selection, TEXT("/>"), TEXT("</li>")) + TEXT("\n"));
			}

			Answer = Answer + OrderAnswer(xuanze, n);
		}
		OutPut(CourseID + TEXT(".") + CourseName + TEXT("  ") + SectionName + TEXT("\r\n") + ExesName + TEXT("：") + Question);
		WriteAnswerToFile(TEXT("TK\\") + CourseID + TEXT(".") + CourseName + TEXT(".txt"), TEXT("\r\n【") + ExesName + TEXT("】") + Question);
		Answer = Answer + TEXT("&;&");
		//c0-e3=string:4287&=&2&;&4276&=&1,2&;&4277&=&2
	}//一道题完了
#else
	/*提交本章的题库*/
	Answer = SubString(CourseAnswer, TEXT("<") + SectionID + TEXT(">"), TEXT("</") + SectionID + TEXT(">"));
	if (str.Find(TEXT("正确率：100%")) != -1|| Status_xiti==TRUE)
	{
		OutPutAppend(TEXT("本节题已做完：") + CourseName + TEXT(" ") + SectionName + TEXT("\r\n"));
		return;
	}
	if (!Answer.IsEmpty())//有题库，且有题没答对
	{
		OutPutAppend(TEXT("提交题库：") + CourseName + TEXT(" ") + SectionName + TEXT("\r\n"));
		Header = GetString(117);
		Header.Replace(TEXT("CourseID"), CourseID);
		Header.Replace(TEXT("SectionID"), SectionID);
		w.AddSendHeader(Header);
		PostData = GetString(119) + GetScriptSessionId();
		CString t = UrlDecodeUTF8(Answer);
		PostData.Replace(TEXT("Answer"), t);
		PostData.Replace(TEXT("CourseID"), CourseID);
		PostData.Replace(TEXT("SectionID"), SectionID);
		str = w.Post(GetString(118), PostData);
#ifndef ISDEBUG
		OutPutAppend(TEXT("等待下一章节...\r\n"));
		Sleep(POSTTKTIME);//延时一段时间，保险一点
#endif
	}
	else
	{
		OutPutAppend(TEXT("无题库：") + CourseName + TEXT(" ") + SectionName + TEXT("\r\n"));
	}
#endif
}
void MediaPj(CString allinfo, CString CourseName, CString SectionName)
{
	/*对课程中的媒体进行评价*/
	CString MediaID;
	DWORD pos = 0;
	CString s;
		
	OutPutAppend(TEXT("提交媒体评论：") + CourseName + TEXT(" ") + SectionName + TEXT("\r\n"));
	while (1)
	{
		if (Endthread == TRUE)EndCourse();
		MediaID = SubString(allinfo, TEXT("parent.showMediaRight("), TEXT(")"), pos);
		if (MediaID.IsEmpty())
		{
			OutputDebugString(TEXT("本节媒体已经评价完：") + CourseName + TEXT(" ") + SectionName + TEXT("\n"));
			break;
		}
		OutputDebugString(TEXT("正在提交媒体评论：") + CourseName + TEXT(" ") + SectionName + TEXT(" ") + MediaID + TEXT(" "));
		Header = GetString(122);
		Header.Replace(TEXT("MediaID"), MediaID);
		w.AddSendHeader(Header);
		PostData = GetString(121) + GetScriptSessionId();
		PostData.Replace(TEXT("MediaID"), MediaID);
		s = w.Post(GetString(118), PostData);//提交
		OutputDebugString(TEXT(" 已提交\n"));
	}
}

void InitPostTime()
{
	/*
	这是刷时间的线程
	客户端每15秒像服务器发送一段内容，每次发送，batchId++，当发送第四次时
	客户端会再次发送内容来获取当前课程的剩余时间

	这里使用N个线程来刷时间，可以大幅度加快刷时间
	*/
	OutPut(TEXT("开始刷时间：\r\n"));

	CString section;
	CString CourseID;//当前进行的课程代码
	CString CourseName;//当前进行的课程名字
	CString SectionID;
	CString	SectionName;
	DWORD iPos;

	int NumSection = Quene_Section.GetCount();
	for (int i = 0; i <NumSection; i++)
	{
		/*取出一个*/
		iPos = 0;
		section = Quene_Section.GetAt(0);
		Quene_Section.RemoveAt(0);
		CourseID = SubString(section, TEXT("|"), TEXT("|"), iPos);
		CourseName = SubString(section, TEXT("|"), TEXT("|"), iPos);
		SectionID = SubString(section, TEXT("|"), TEXT("|"), iPos);
		SectionName = SubString(section, TEXT("|"), TEXT("|"), iPos);

		PostTime( CourseID, CourseName, SectionID, SectionName);
		if (Endthread == TRUE)EndCourse();
		RefreshCourseGrade(CourseID);
	}
}
void PostTime(CString CourseID, CString CourseName, CString SectionID, CString SectionName)
{
	CString str;
	DWORD begin = 1;
	DWORD batchId = begin;
	CString LastTime=TEXT("---");//上次剩余时间

	CString t;//一个没用的变量
	while (1)
	{
		/*每刷一次时间都获取一次剩余时间，这样可以多个应用程序刷的时候快速刷完时间*/
		if (Endthread == TRUE)EndCourse();
		str = GetSectionStatus(CourseID, SectionID, batchId);
		if (str.Find(TEXT("OK</strong> \\r\\n                 <span class=\\\"explain_rate\\\"><a href=\\\"javascript:;\\\" onclick=\\\"atPage(\\\'时间说明")) != -1)
		{
			/*刷完了*/
			OutPutAppend(TEXT("该章节的时间刷完了：") + CourseName + TEXT(" ") + SectionName+TEXT("\r\n"));
			OutputDebugString(TEXT("该章节的时间刷完了：") + CourseName +TEXT(" ")+ SectionName + TEXT("\n"));
			return;
		}
		else
		{
				CString RemainTime = SubString(str, TEXT("已学时间\\\">"), TEXT("</span>"));
				RemainTime = RemainTime + TEXT("/") + SubString(str, TEXT("总学习时间\\\">"), TEXT("</span>"));

				//if ((batchId - begin) % 4 == 0)//每4次
				if (LastTime.CollateNoCase(RemainTime) != 0)//当剩余时间有改变时刷新
				{
					LastTime = RemainTime;
					OutPutAppend(TEXT("剩余时间：") + CourseName + TEXT(" ") + SectionName + TEXT(" ") + RemainTime + TEXT("\r\n"));
					OutputDebugString(TEXT("剩余时间：") + CourseName + TEXT(" ") + SectionName + TEXT(" ") + RemainTime + TEXT("\n"));
					batchId++;
					begin = batchId;
				}
		}
		for (int j = 1; j <= 15; j++)/*延时15秒*/
		{
			Sleep(1000);
			if (Endthread == TRUE)EndCourse();
		}
		Header = GetString(117);
		Header.Replace(TEXT("CourseID"), CourseID);
		Header.Replace(TEXT("SectionID"), SectionID);
		w.AddSendHeader(Header);
		PostData = GetString(110) + GetScriptSessionId();
		PostData.Replace(TEXT("CourseID"), CourseID);
		PostData.Replace(TEXT("SectionID"), SectionID);
		t.Format(TEXT("%d"), batchId);
		PostData.Replace(TEXT("BatchID"), t);
		str = w.Post(GetString(109), PostData);
		if (str.Find(TEXT("flag:1")) == -1)
		{
			OutputDebugString(TEXT("POSS时间出错：") + CourseName + TEXT(" ") + SectionName + TEXT("\n"));
		}
		else
		{
			OutPutAppend(TEXT("刷了15秒：") + CourseName + TEXT(" ") + SectionName + TEXT("\r\n"));
			OutputDebugString(TEXT("POSS了一次时间：") + CourseName + TEXT(" ") + SectionName + TEXT("\n"));
		}
		//OutputDebugString(TEXT("刷时间返回的内容：\n") + str);
		batchId++;	
	}
	return ;
}

void EndCourse()
{
	OutPutAppend(TEXT("\r\n刷课已停止！"));
	PostData.Empty();
	str.Empty();	//返回值

	CourseAnswer.Empty();
	Quene_Section.RemoveAll();
	AllCourseID.RemoveAll();
	AllCourseName.RemoveAll();

	CButton* pass = (CButton*)(AfxGetMainWnd()->GetDlgItem(IDC_PASS));
	pass->SetWindowText(TEXT("开刷"));
	pass->EnableWindow(TRUE);

	AfxEndThread(0, TRUE);
}
BOOL RefreshCourse()
{
	/*刷新所有课程并更新在窗口上*/
	w.AddSendHeader(GetString(108));
	PostData = GetString(107) + GetScriptSessionId();
	str = w.Post(GetString(106), PostData);
	str = HexStrToWChars(str);
	if (str.Find(TEXT("flag:0")) == -1)
	{
		return FALSE;
	}
	//m_Disp.SetWindowTextA(str);

	CString TD;//一列
	DWORD TDpos = 0;
	DWORD index = -1;

	CString TR;//一行
	DWORD TRpos = 0;

	CListCtrl* list = (CListCtrl*)(AfxGetMainWnd()->GetDlgItem(IDC_LIST3));

	list->DeleteAllItems();
	while (1)
	{
		TR = SubString(str, TEXT("<tr>\\r\\n                <td>"), TEXT("</tr>\\r\\n"), TRpos);//取出一行
		if (TR.IsEmpty()) break;
		TD = SubString(TR, TEXT(""), TEXT("</td>\\r\\n"), TDpos);
		index = list->InsertItem(++index, TD);//插入课程代码
		list->SetItem(index, 0, LVIF_TEXT, TD, NULL, NULL, NULL, NULL);//课程名称
		TD = SubString(TR, TEXT("target=\\\"_blank\\\">"), TEXT("</a></td>"), TDpos);
		list->SetItem(index, 1, LVIF_TEXT, TD, NULL, NULL, NULL, NULL);//课程名称
		TD = SubString(TR, TEXT("<td>"), TEXT("</td>\\r\\n"), TDpos);
		list->SetItem(index, 2, LVIF_TEXT, TD, NULL, NULL, NULL, NULL);//课程成绩
		TD = SubString(TR, TEXT("<td>"), TEXT("</td>\\r\\n"), TDpos);
		list->SetItem(index, 3, LVIF_TEXT, TD, NULL, NULL, NULL, NULL);//学习期限
		TD = SubString(TR, TEXT("<td>"), TEXT("</td>\\r\\n"), TDpos);
		list->SetItem(index, 4, LVIF_TEXT, TD, NULL, NULL, NULL, NULL);//状态
		TDpos = 0;
	}
	return TRUE;
}
BOOL RefreshCourseGrade(CString courseid)
{
	str = w.Get(TEXT("http://www.attop.com/wk/index.htm?id=") + courseid);
	CString Grade = SubString(str, TEXT("class=\"markNum\">"), TEXT("</strong>"));
	//OutputDebugString(TEXT("获取分数：") + courseid + TEXT("  ") + Grade + TEXT("\n"));
	if (Grade.IsEmpty())return FALSE;
	CListCtrl* list = (CListCtrl*)(AfxGetMainWnd()->GetDlgItem(IDC_LIST3));
	for (int i = 0; i < list->GetItemCount(); i++)
	{
		if (list->GetItemText(i, 0).Compare(courseid) == 0)
		{
			list->SetItem(i, 2, LVIF_TEXT, Grade, NULL, NULL, NULL, NULL);//课程成绩
			return TRUE;
		}
	}

	return FALSE;
}
BOOL GetSection(CString Courseid, CStringArray &Sectionid, CStringArray &Sectionname)
{
	/*根据CourseID取出该课程中的所有章节*/
	str = w.Get(TEXT("http://www.attop.com/wk/learn.htm?id=") + Courseid);

	Sectionid.RemoveAll();
	Sectionname.RemoveAll();

	DWORD pos = 0;
	DWORD pos1 = 0;
	CString zhang;
	CString jie;
	OutputDebugString(TEXT("取出某课程的所有章节：\n"));
	while (1)
	{
		zhang = SubString(str, TEXT("<dd>"), TEXT("</dd>"), pos);
		if (zhang.IsEmpty())
		{
			zhang = SubString(str, TEXT("<dt name=\"zj\""), TEXT("</dd>"), pos);//两种情况
		}
		if (zhang.IsEmpty())break;

		while (1)
		{
			jie = SubString(zhang, TEXT("<li"), TEXT("</li>"), pos1);
			if (jie.IsEmpty())break;
			CString tmp;
			tmp = SubString(zhang, TEXT("span title=\""), TEXT("</span>"));
			tmp = tmp.Left(4);//只要第几章
			Sectionid.Add(SubString(jie, TEXT("id=\"j_"), TEXT("\">")));
			Sectionname.Add(tmp + TEXT(" ") + SubString(jie, TEXT("title=\""), TEXT("\"")));
			OutputDebugString(tmp + TEXT(" ") + SubString(jie, TEXT("id=\"j_"), TEXT("\">")) + TEXT(" ") + SubString(jie, TEXT("title=\""), TEXT("\"")) + TEXT("\n"));
		}
		pos1 = 0;
	}
	return TRUE;
}
CString GetSectionStatus(CString CourseID, CString SectionID, DWORD batchId)
{
	/*获取章节的章台，例如时间完成多少，题目完成多少，媒体评价完成多少*/
	CString t;
	CString result;
	Header = GetString(117);
	Header.Replace(TEXT("CourseID"), CourseID);
	Header.Replace(TEXT("SectionID"), SectionID);
	w.AddSendHeader(Header);
	t.Format(TEXT("%d"), batchId);
	PostData = GetString(112) + GetScriptSessionId();
	PostData.Replace(TEXT("CourseID"), CourseID);
	PostData.Replace(TEXT("SectionID"), SectionID);
	PostData.Replace(TEXT("BatchID"), t);
	result = w.Post(GetString(109), PostData);
	result = HexStrToWChars(result);
	return result;
}
CString tokenify(long long number)
{
	//CString strTime;
	//strTime.Format(_T("%lldms"), number);  //总毫秒数  
	//OutputDebugString(strTime);
	/*就是获取GScriptSessionId中一段代码，根据原网页JS改的*/
	CString tokenbuf;
	CString charmap(TEXT("1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ*$"));
	long long  remainder = number;
	while (remainder > 0)
	{
		tokenbuf = tokenbuf + charmap.GetAt(remainder & 0x3F);
		remainder = floor(remainder / 64);
	}
	return tokenbuf;
}
CString GetScriptSessionId()
{
	/*传入一个DWRSESSIONID，根据相应的算法返回完整的ScriptSessionId
	妈蛋，这个必须根据算法来写，不然总报Invalid Page，这个问题折磨了我好多好多天！！！
	而且，通过多次抓包才发现，这个ScriptSessionId是一次性的，每次POST都必须使用新的ScriptSessionId，不然总错，多次之后才发现，真蠢！
	_pageId = tokenify(new Date().getTime()) + "-" + tokenify(Math.random() * 1E16);
	ScriptSessionId=DWRSESSIONID+"/"+_pageId;
	tokenify: function(number) {
	var tokenbuf = [];
	var charmap = "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ*$";
	var remainder = number;
	while (remainder > 0)
	{
	tokenbuf.push(charmap.charAt(remainder & 0x3F));
	remainder = Math.floor(remainder / 64);
	}
	return tokenbuf.join("");
	}
	*/
	CString ScriptSessionId;
	ScriptSessionId = DWRSESSIONID + TEXT("/");
	ScriptSessionId = ScriptSessionId + tokenify(getCurrentTime()) + TEXT("-") + tokenify(GetRandZeroToOne()* 1E16);
	return ScriptSessionId;
}

CString ReadAnswerFromFile(CString path)
{
	CStdioFile File;
	CString result;
	if (File.Open(path, CFile::modeNoTruncate | CFile::typeBinary | CFile::modeRead | CFile::shareDenyNone))
	{
		ULONGLONG count = File.GetLength();
		char *p = new char[count + 1];
		memset(p, 0, count + 1);
		File.Read(p, count);
		result = p;
		delete[]p;
		File.Close();
	}
	return result;
}
BOOL WriteAnswerToFile(CString path,CString Answer)
{
	CStdioFile File;
	if (File.Open(path, CFile::modeNoTruncate|CFile::modeCreate | CFile::typeBinary | CFile::modeReadWrite | CFile::shareDenyNone))
	{
		File.SeekToEnd();
		char *p = UnicodeToANSI(Answer);
		File.Write(p, strlen(p));
		delete[]p;
		File.Close();
		return TRUE;
	}
	return FALSE;
}
BOOL RefreshAnswerFile()
{
	/*从网站上下载答案,并返回程序能否继续运行*/
	//CString LocalAnswer;
	//CString LocaVersion;
	//CString OnlineVersion;
	//CString OnlineAnswer;
	//CString path;
	//CString LackCourse;
	//CString tmp;

	//HttpClient MyWeb;
	//OutPut(TEXT("正在检查文件更新...\r\n"));
	//CreateDirectory(TEXT("TK"),NULL);
	//str = MyWeb.Get(GetString(14));
	//OnlineVersion = SubString(str, TEXT("<Version>"), TEXT("</Version>"));
	//if (OnlineVersion.IsEmpty())return TRUE;
	//if (OnlineVersion.CompareNoCase(GetString(1001))!=0)
	//{
	//	AfxMessageBox(SubString(str, TEXT("<VersionContent>"), TEXT("</VersionContent>")),MB_ICONINFORMATION);
	//	return FALSE;
	//}
	//if (!SubString(str, TEXT("<Tips>"), TEXT("</Tips>")).IsEmpty())
	//{
	//	AfxMessageBox(SubString(str, TEXT("<Tips>"), TEXT("</Tips>")), MB_ICONINFORMATION);
	//}
	//for (int i = 0; i < AllCourseID.GetCount(); i++)
	//{
	//	path = TEXT("TK\\") + AllCourseID.GetAt(i) + TEXT(".txt");
	//	LocalAnswer = ReadAnswerFromFile(path);
	//	LocaVersion = SubString(LocalAnswer, TEXT("<Version>"), TEXT("</Version>"));
	//	OnlineVersion = SubString(str, TEXT("<") + AllCourseID.GetAt(i) + TEXT(">"), TEXT("</") + AllCourseID.GetAt(i) + TEXT(">"));
	//	if (OnlineVersion.IsEmpty())continue;
	//	if (OnlineVersion.CompareNoCase(LocaVersion) == 0)continue;//版本相同
	//	CString url = GetString(15);
	//	url.Replace(TEXT("CourseID"), AllCourseID.GetAt(i));
	//	OnlineAnswer = MyWeb.Get(url);
	//	if (OnlineAnswer.IsEmpty())
	//	{
	//		LackCourse = LackCourse + AllCourseID.GetAt(i) + TEXT(" ") + AllCourseName.GetAt(i) + TEXT("\n");
	//		continue;
	//	}
	//	DeleteFile(path);
	//	WriteAnswerToFile(path, OnlineAnswer);
	//	OutPutAppend(TEXT("获取到题库文件更新...")+ AllCourseID.GetAt(i)+TEXT(" ")+ AllCourseName.GetAt(i)+ TEXT("\r\n"));
	//}
	//OutPutAppend(TEXT("更新完毕！\r\n"));
	//if (!LackCourse.IsEmpty())AfxMessageBox(LackCourse + TEXT("\n这些课程缺少题库文件，不能刷题！"), MB_ICONWARNING);
	return TRUE;
}

CString OrderAnswer(CString Answer,DWORD n)
{
	/*将  2,3,1, 这样的答案排序成 1,2,3 */
	int *a= new int[n];
	DWORD iPos = 0;
	CString Num;
	if (Answer.IsEmpty())return TEXT("");
	if (Answer.Left(1).Compare(TEXT(",")) != 0)Answer = TEXT(",") + Answer;//设置哨兵
	if (Answer.Right(1).Compare(TEXT(",")) != 0)Answer = Answer + TEXT(",");//设置哨兵
	for (int k = 0; k < n; k++)
	{
		Num = SubString(Answer, TEXT(","), TEXT(","), iPos);
		a[k] = _ttoi(Num);
	}
	int i, j, temp;
	for (j = 0; j < n - 1; j++)
		for (i = 0; i < n - 1 - j; i++)
		{
			if (a[i] > a[i + 1])
			{
				temp = a[i];
				a[i] = a[i + 1];
				a[i + 1] = temp;
			}
		}
	Answer.Empty();
	CString t;
	for (int k = 0; k < n; k++)
	{
		t.Format(TEXT("%d"), a[k]);
		Answer = Answer + t+TEXT(",");
	}
	if (Answer.Right(1) = TEXT(","))
		Answer = Answer.Left(Answer.GetLength() - 1);//去掉末尾的,
	delete[]a;
	return Answer;
}

void CTestDlg::OnLvnItemchangedList3(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}
