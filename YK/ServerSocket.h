#pragma once

#include "pch.h"
#include "framework.h"
class CServerSocket
{

public:
	static CServerSocket* getInstance() {
		if (m_instance == nullptr) {//静态函数没有this指针
			m_instance = new CServerSocket();
		}
		return m_instance;
	}
	bool InitSocket()
	{
		
		if (m_serv_sock == -1)return false;
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = INADDR_ANY;
		serv_adr.sin_port = htons(9527);
		if(bind(m_serv_sock, (sockaddr*)&serv_adr, sizeof(serv_adr))==-1)return false;
		if(listen(m_serv_sock, 1)==-1)return false;
		return true;
	}

	bool AcceptClient() {
		sockaddr_in client_adr;
		int client_size = sizeof(client_adr);
		m_client_sock = accept(m_serv_sock, (sockaddr*)&client_adr, &client_size);
		if (m_client_sock == -1)return false;
		return true;
	}

	int DealCommand() {
		if (m_client_sock == -1)return -1;
		char buffer[1024]="";
		while (1) {
			int len=recv(m_client_sock, buffer, sizeof(buffer), 0);
			if (len <= 0)return -1;
			
		
		
		}
	}
	bool Send(const char*pData,int nSize) {
	
		return send(m_client_sock, pData, nSize, 0)>0;
	
	
	}
private:
	SOCKET m_serv_sock;
	SOCKET m_client_sock;
	CServerSocket(const CServerSocket&) = delete;
	CServerSocket& operator=(const CServerSocket& ss) = delete;
	CServerSocket() {
		m_client_sock= INVALID_SOCKET;
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("无法初始化套接字环境,请检查网络设置"), _T("初始化错误"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		 m_serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	
	}
	~CServerSocket() {
		closesocket(m_serv_sock);
		WSACleanup();
	}
	BOOL InitSockEnv() {
	
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0){

			return FALSE;

		}
		return TRUE;
	}
	static void releaseInstance() {
	
		if (m_instance != nullptr)
		{
			CServerSocket* tmp = m_instance;
			m_instance = nullptr;
			delete tmp;

		}
	}
	static CServerSocket* m_instance;

	class CHelper
	{
	public:
		CHelper() {
		
			CServerSocket::getInstance();
		
		}
		~CHelper() {

			CServerSocket::releaseInstance();
		}
	};
	static CHelper m_helper;

};

extern CServerSocket server;
