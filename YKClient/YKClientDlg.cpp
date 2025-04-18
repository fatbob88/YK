﻿
// YKClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "YKClient.h"
#include "YKClientDlg.h"
#include "ClinetSocket.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CYKClientDlg 对话框



CYKClientDlg::CYKClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_YKCLIENT_DIALOG, pParent)
	, m_server_address(0)
	, m_nPort(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CYKClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_IPAddress(pDX, IDC_IPADDRESS_SERV, m_server_address);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nPort);
	DDX_Control(pDX, IDC_TREE_DIR, m_Tree);
	DDX_Control(pDX, IDC_LIST_FILE, m_List);
}

int CYKClientDlg::SendCommandPacket(int nCmd, bool bAutoClose, BYTE* pData, size_t nLength)
{
	UpdateData();
	CClientSocket* pClient = CClientSocket::getInstance();
	bool ret = pClient->InitSocket(m_server_address, atoi((LPCTSTR)m_nPort));
	if (!ret) {
		AfxMessageBox(_T("网络初始化失败!"));
		return -1;
	}
	CPacket pack(nCmd, pData, nLength);
	ret = pClient->Send(pack);
	TRACE("Send ret %d\r\n", ret);
	int cmd = pClient->DealCommand();
	TRACE("ack:%d\r\n", cmd);
	if (bAutoClose)
		pClient->CloseSocket();
	return cmd;
}


BEGIN_MESSAGE_MAP(CYKClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_TEST, &CYKClientDlg::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BTN_FILEINFO, &CYKClientDlg::OnBnClickedBtnFileinfo)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_DIR, &CYKClientDlg::OnNMDblclkTreeDir)
	ON_NOTIFY(NM_CLICK, IDC_TREE_DIR, &CYKClientDlg::OnNMClickTreeDir)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_FILE, &CYKClientDlg::OnNMRClickListFile)
	ON_COMMAND(ID_DOWNLOAD_FILE, &CYKClientDlg::OnDownloadFile)
	ON_COMMAND(ID_DELETE_FILE, &CYKClientDlg::OnDeleteFile)
	ON_COMMAND(ID_RUN_FILE, &CYKClientDlg::OnRunFile)
END_MESSAGE_MAP()


// CYKClientDlg 消息处理程序

BOOL CYKClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	UpdateData();
	m_server_address = 0x7f000001;
	m_nPort = _T("9527");
	UpdateData(FALSE);


	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CYKClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CYKClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
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

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CYKClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CYKClientDlg::OnBnClickedBtnTest()
{
	
	SendCommandPacket(1981);
	// TODO: 在此添加控件通知处理程序代码
}

void CYKClientDlg::OnBnClickedBtnFileinfo()
{
	int ret=SendCommandPacket(1);
	if (ret == -1) {
		AfxMessageBox("命令处理失败");
		return;
	}
	CClientSocket* pClient = CClientSocket::getInstance();
	std::string drivers = pClient->GetPacket().strData;
	std::string dr;
	m_Tree.DeleteAllItems();
	for (size_t i = 0; i < drivers.size(); i++)
	{
		if (drivers[i] == ',') {
			dr += ":";
			HTREEITEM hTemp = m_Tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);
			m_Tree.InsertItem(NULL, hTemp, TVI_LAST);
			dr.clear();
			continue;
		}
		dr += drivers[i];
	}


	// TODO: 在此添加控件通知处理程序代码
}
CString CYKClientDlg::GetPath(HTREEITEM hTree)
{
	CString strRet, strTmp;
	do {
		strTmp = m_Tree.GetItemText(hTree);
		strRet = strTmp + '\\' + strRet;
		hTree = m_Tree.GetParentItem(hTree);
	} while (hTree != NULL);
	return strRet;
}




void CYKClientDlg::OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	LoadFileInfo();


} 
void CYKClientDlg::LoadFileInfo()
{
	CPoint ptMouse;
	GetCursorPos(&ptMouse);
	m_Tree.ScreenToClient(&ptMouse);
	HTREEITEM hTreeSelected = m_Tree.HitTest(ptMouse, 0);
	if (hTreeSelected == NULL)
		return;
	if (m_Tree.GetChildItem(hTreeSelected) == NULL)
		return;
	DeleteTreeChildrenItem(hTreeSelected);
	m_List.DeleteAllItems();
	CString strPath = GetPath(hTreeSelected);
	int nCmd = SendCommandPacket(2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());
	PFILEINFO pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
	CClientSocket* pClient = CClientSocket::getInstance();
	int Count = 0;
	while (pInfo->HasNext) {
		TRACE("[%s] isdir %d\r\n", pInfo->szFileName, pInfo->IsDirectory);
		if (pInfo->IsDirectory) {
			if (CString(pInfo->szFileName) == "." || (CString(pInfo->szFileName) == ".."))
			{
				int cmd = pClient->DealCommand();
				TRACE("ack:%d\r\n", cmd);
				if (cmd < 0)break;
				pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
				continue;
			}
			HTREEITEM hTemp = m_Tree.InsertItem(pInfo->szFileName, hTreeSelected, TVI_LAST);
			m_Tree.InsertItem("", hTemp, TVI_LAST);
		}
		else {
			m_List.InsertItem(0, pInfo->szFileName);
		}
		Count++;
		int cmd = pClient->DealCommand();
		//TRACE("ack:%d\r\n", cmd);
		if (cmd < 0)break;
		pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
	}
	pClient->CloseSocket();
	TRACE("Count = %d\r\n", Count);
}

void CYKClientDlg::DeleteTreeChildrenItem(HTREEITEM hTree)
{
	HTREEITEM hSub = NULL;
	do {
		hSub = m_Tree.GetChildItem(hTree);
		if (hSub != NULL)m_Tree.DeleteItem(hSub);
	} while (hSub != NULL);
}


void CYKClientDlg::OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	LoadFileInfo();
}

void CYKClientDlg::OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	CPoint ptMouse, ptList;
	GetCursorPos(&ptMouse);
	ptList = ptMouse;
	m_List.ScreenToClient(&ptList);
	int ListSelected = m_List.HitTest(ptList);
	if (ListSelected < 0)return;
	CMenu menu;
	menu.LoadMenu(IDR_MENU_RCLICK);
	CMenu* pPupup = menu.GetSubMenu(0);
	if (pPupup != NULL) {
		pPupup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptMouse.x, ptMouse.y, this);
	}

}

void CYKClientDlg::OnDownloadFile()
{
	int nListSelected = m_List.GetSelectionMark();
	CString strFile=m_List.GetItemText(nListSelected, 0);
	CFileDialog dlg(FALSE,"*", 
		m_List.GetItemText(nListSelected, 0),OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,NULL, this);
	if (dlg.DoModal()==IDOK) {
		FILE* pFile = fopen(dlg.GetPathName(), "wb+");
		if (pFile == NULL) {
			AfxMessageBox("没有权限或文件无法创建");
		}
		HTREEITEM hSelected = m_Tree.GetSelectedItem();
		strFile = GetPath(hSelected) + strFile;
		int ret = SendCommandPacket(4, false, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
		if (ret < 0)
		{
			AfxMessageBox("下载命令失败");
			TRACE("下载失败：ret=%d\r\n", ret);
			return;
		}
		CClientSocket* pClient = CClientSocket::getInstance();
		long long nLength = *(long long*)pClient->GetPacket().strData.c_str();
		if (nLength == 0)
		{
			AfxMessageBox("文件长度为0或无法读取!");
			return;
		}
		long long nCount = 0;
	
		
		while (nCount < nLength)
		{
			ret = pClient->DealCommand();
			if (ret < 0)
			{
				AfxMessageBox("传输失败!");
				break;
			}
			fwrite(pClient->GetPacket().strData.c_str(), 1, pClient->GetPacket().strData.size(), pFile);
			nCount += pClient->GetPacket().strData.size();
		}
		fclose(pFile);
		pClient->CloseSocket();
	
	}
	
}

void CYKClientDlg::OnDeleteFile()
{
	// TODO: 在此添加命令处理程序代码
}

void CYKClientDlg::OnRunFile()
{
	// TODO: 在此添加命令处理程序代码
}
