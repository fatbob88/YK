// YK.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "YK.h"
#include "ServerSocket.h"
#include <direct.h>
#include <atlimage.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;

using namespace std;
void Dump(BYTE* pData, size_t nSize)
{
    std::string strOut;
    for (size_t i = 0; i < nSize; i++) {
    
        char buf[8] = "";
        if (i > 0 && (i % 16 == 0))strOut += "\n";
        snprintf(buf,sizeof(buf), "%02X", pData[i] & 0xFF);
        strOut += buf;
    
    
    
    }
    strOut += "\n";
    OutputDebugStringA(strOut.c_str());

}

int MakeDriverInfo() {
    std::string result;
    for (int i = 1; i <= 26; i++) {
        if (_chdrive(i) == 0) {
            if (result.size() > 0)result += ',';
            result += 'A' + i - 1;
        
        }
    }
    CPacket pack(1, (BYTE*)result.c_str(), result.size());
    Dump((BYTE*)pack.Data(), pack.Size());
   //CServerSocket::getInstance()->Send(pack);
    return 0;

}
#include<io.h>
#include<stdio.h>
#include<list>
 
typedef struct file_info {
	file_info() {
		IsInvalid = FALSE;
		IsDirectory = -1;
		HasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL IsInvalid;// 
	BOOL IsDirectory;// 
	BOOL HasNext;//  是否还有下一个文件
	char szFileName[256];//
}FILEINFO, * PFILEINFO;

int MakeDirInfo() {
    std::string strPath;
    std::list<FILEINFO> listFileInfos;
    if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
        OutputDebugString(_T("当前命令不是获取文件信息"));
        return -1;
    }
    if (_chdir(strPath.c_str()) != 0) {
		FILEINFO finfo;
		finfo.HasNext = FALSE;
		CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
		CServerSocket::getInstance()->Send(pack);
		OutputDebugString(_T("没有权限访问当前目录"));
		return -2;
    }
    _finddata_t fdata;
    int hfind = 0;
    if ((_findfirst("*", &fdata) )== -1) {
		OutputDebugString(_T("没有找到任何文件"));
		FILEINFO finfo;
		finfo.HasNext = FALSE;
		CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
		CServerSocket::getInstance()->Send(pack);
		return -3;
    }
	int count = 0;
    do {
		FILEINFO finfo;
		finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
		memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
		TRACE("%s\r\n", finfo.szFileName);
		CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
		CServerSocket::getInstance()->Send(pack);
		count++;
    } while (!_findnext(hfind, &fdata));
	TRACE("server: count = %d\r\n", count);
	//发送信息到控制端
	FILEINFO finfo;
	finfo.HasNext = FALSE;
	CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
	CServerSocket::getInstance()->Send(pack);
	return 0;
}

int RunFile() {
	std::string strPath;
	CServerSocket::getInstance()->GetFilePath(strPath);
	ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
	CPacket pack(3, NULL, 0);
	CServerSocket::getInstance()->Send(pack);
	return 0;
}

int DownloadFile() {
	std::string strPath;
	CServerSocket::getInstance()->GetFilePath(strPath);
	long long data = 0;
	FILE* pFile = NULL;
	errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");
	if (err != 0) {
		CPacket  pack(4, (BYTE*)&data, 8);
		CServerSocket::getInstance()->Send(pack);
		return -1;
	}
	if (pFile != NULL) {
		fseek(pFile, 0, SEEK_END);
		data = _ftelli64(pFile);
		CPacket head(4, (BYTE*)&data, 8);
		CServerSocket::getInstance()->Send(head);
		fseek(pFile, 0, SEEK_SET);
		char buffer[1024] = "";
		size_t rlen = 0;
		do {
			rlen = fread(buffer, 1, 1024, pFile);
			CPacket pack(4, (BYTE*)buffer, rlen);
			CServerSocket::getInstance()->Send(pack);
		} while (rlen >= 1024);
		fclose(pFile);
	}
	CPacket pack(4, NULL, 0);
	CServerSocket::getInstance()->Send(pack);
	return 0;
}

int MouseEven() {

	MOUSEEV mouse;
	if (CServerSocket::getInstance()->GetMouseEvent(mouse)) {
		DWORD nFlags = 0;
		switch (mouse.nButton) {
		case 0://左键
			nFlags = 1;
			break;
		case 1://右键
			nFlags = 2;
			break;
		case 2://中键
			nFlags = 4;
			break;
		case 4://没有按键
			nFlags = 8;
			break;
		}
		if (nFlags != 8)SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
		switch (mouse.nAction)
		{
		case 0://单击
			nFlags |= 0x10;
			break;
		case 1://双击
			nFlags |= 0x20;
			break;
		case 2://按下
			nFlags |= 0x40;
			break;
		case 3://放开
			nFlags |= 0x80;
			break;
		default:
			break;
		}
		TRACE("mouse event : %08X x %d y %d\r\n", nFlags, mouse.ptXY.x, mouse.ptXY.y);
		switch (nFlags)
		{
		case 0x21://左键双击
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x11://左键单击
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x41://左键按下
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x81://左键放开
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x22://右键双击
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x12://右键单击
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x42://右键按下
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x82://右键放开
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x24://中键双击
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x14://中键单击
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x44://中键按下
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x84://中键放开
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x08://单纯的鼠标移动
			mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
			break;
		}
		CPacket pack(4, NULL, 0);
		CServerSocket::getInstance()->Send(pack);
	}
	else {
		OutputDebugString(_T("获取鼠标操作参数失败！！"));
		return -1;
	}
	return 0;
}

int SendScreen() 
{    
	CImage screen;//GDI
	HDC hScreen = ::GetDC(NULL);
	int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);//24   ARGB8888 32bit RGB888 24bit RGB565  RGB444
	int nWidth = GetDeviceCaps(hScreen, HORZRES);
	int nHeight = GetDeviceCaps(hScreen, VERTRES);
	screen.Create(nWidth, nHeight, nBitPerPixel);
	BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hScreen, 0, 0, SRCCOPY);
	ReleaseDC(NULL, hScreen);
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
	if (hMem == NULL)return -1;
	IStream* pStream = NULL;
	HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
	if (ret == S_OK) {
		screen.Save(pStream, Gdiplus::ImageFormatPNG);
		LARGE_INTEGER bg = { 0 };
		pStream->Seek(bg, STREAM_SEEK_SET, NULL);
		PBYTE pData = (PBYTE)GlobalLock(hMem);
		SIZE_T nSize = GlobalSize(hMem);
		CPacket pack(6, pData, nSize);
		CServerSocket::getInstance()->Send(pack);
		GlobalUnlock(hMem);
	}
	//screen.Save(_T("test2020.png"), Gdiplus::ImageFormatPNG);
	/*
	TRACE("png %d\r\n", GetTickCount64() - tick);
	for (int i = 0; i < 10; i++) {
		DWORD tick = GetTickCount64();
		screen.Save(_T("test2020.png"), Gdiplus::ImageFormatPNG);
		TRACE("png %d\r\n", GetTickCount64() - tick);
		tick = GetTickCount64();
		screen.Save(_T("test2020.jpg"), Gdiplus::ImageFormatJPEG);
		TRACE("jpg %d\r\n", GetTickCount64() - tick) ;
	}*/
	pStream->Release();
	GlobalFree(hMem);
	screen.ReleaseDC();
	return 0;



	return 0;
}

#include "LockDialog.h"
CLockDialog dlg;
unsigned threadid=0;

unsigned __stdcall threadLockDlg(void* arg) {


	dlg.Create(IDD_DIALOG_INFO, NULL);
	dlg.ShowWindow(SW_SHOW);
	CRect rect;
	rect.top = 0;
	rect.left = 0;
	rect.right = GetSystemMetrics(SM_CXFULLSCREEN);
	rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
	rect.bottom *= 1.04;
	dlg.MoveWindow(rect);
	dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	ShowCursor(false);
	::ShowWindow(::FindWindow(_T("Shell_TryWnd"), NULL), SW_HIDE);//隐藏任务栏
	//dlg.GetWindowRect(rect);
	ClipCursor(rect);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_KEYDOWN) {
			if (msg.wParam == 0x1B)break;
		}
	}
	
	ShowCursor(true);
	::ShowWindow(::FindWindow(_T("Shell_TryWnd"), NULL), SW_SHOW);
	dlg.DestroyWindow();
	_endthreadex(0);
	return 0;
}

int LockMachine() {
	if ((dlg.m_hWnd == NULL) || (dlg.m_hWnd == INVALID_HANDLE_VALUE)) {
		//_beginthread(threadLockDlg, 0, NULL);
		_beginthreadex(NULL, 0, threadLockDlg, NULL, 0, &threadid);
		TRACE("threadid=%d\r\n", threadid);
	}
	CPacket pack(7, NULL, 0);
	CServerSocket::getInstance()->Send(pack);
	return 0;

}




int UnLockMachine() {
	//dlg.SendMessage(WM_KEYDOWN, 0x41, 0x01E0001);
	//::SendMessage(dlg.m_hWnd, WM_KEYDOWN, 0x41, 0x01E0001);
	PostThreadMessage(threadid, WM_KEYDOWN, 0x41, 0);
	CPacket pack(8, NULL, 0);
	CServerSocket::getInstance()->Send(pack);
	return 0;

}


int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {
   //         // TODO: 在此处为应用程序的行为编写代码。
   //          //TODO：返回值处理。
   //         CServerSocket* pserver = CServerSocket::getInstance();
   //         int count = 0;
			//if (pserver->InitSocket() == false) {
			//	MessageBox(NULL, _T("网络初始化异常,请检查网络"), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
			//	exit(0);
			//}
   //         while (CServerSocket::getInstance() != nullptr) {
   //               if (pserver->AcceptClient() == false) {
   //                     if (count >= 3) { 
   //                         MessageBox(NULL, _T("多次无法接入用户,程序退出"), _T("接入用户失败"), MB_OK | MB_ICONERROR);
   //                         exit(0);
   //                     }
   //                     MessageBox(NULL, _T("无法接入用户,自动重试"), _T("接入用户失败"), MB_OK | MB_ICONERROR);
   //                     count++;
   //                 }
   //               int ret = pserver->DealCommand();
   //         }
		
            int nCmd =7;
            switch (nCmd) {
            case 1:
                MakeDriverInfo();
                break;
            case 2:
                MakeDirInfo();
            case 3:
                RunFile();
            case 4:
                DownloadFile();
                break;
			case 5:
				MouseEven();
				break;
			case 6:
				SendScreen();
				break;
			case 7:
				LockMachine();
				break;
			case 8:
				UnLockMachine();
				break;
            }
			

        }
    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
