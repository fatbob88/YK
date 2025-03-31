#pragma once

#include "pch.h"
#include "framework.h"




class CPacket
{
public:
	CPacket():
		sHead(0),
		nLength(0),
		sCmd(0),
		sSum(0)
	{
	
	
	
	}
	CPacket(const CPacket& pack) {
	
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}

	CPacket& operator=(const CPacket& pack) {
		if (this != &pack) {
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;
	
	
	}
	CPacket(const BYTE* pData, size_t& nSize) {
	
		size_t i = 0;
		for (; i < nSize; i++) {

			if (*(WORD*)(pData + i) == 0xFEFF) {

				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}

		}
		if (i+8 >=nSize) {//包数据可能不全,包头可能未全部解析到

			nSize = 0;
			return;
		}
		nLength=*(DWORD*)(pData + i);
		i += 4;
		if (nLength + i > nSize) {//包未完全接收到
			nSize = 0;
			return;
		}
		sCmd= *(WORD*)(pData + i);
		i += 2;
		if (nLength > 4) {
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 2 - 2);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i);
		i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++) {


			sum += BYTE(strData[j]) & 0xFF;

		}
		if (sum == sSum) {
		
			nSize = i;
			return;
		}
		nSize = 0;
	
	}
	~CPacket() {}
public:
	WORD sHead;
	DWORD nLength;
	WORD sCmd;
	std::string strData;
	WORD sSum;
private:

};

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
#define BUFFER_SIZE 4096
	int DealCommand() {
		if (m_client_sock == -1)return -1;
		char *buffer= new char[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;
		while (1) {
			size_t len=recv(m_client_sock, buffer+index, sizeof(buffer), 0);
			if (len <= 0)return -1;
			index += len;
			len = index;
			m_packet=CPacket ((BYTE*)buffer, index);
			if (len > 0) {
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index-= len;
				return m_packet.sCmd;
			}
		}
		return -1;
	}

	bool Send(const char*pData,int nSize) {
	
		return send(m_client_sock, pData, nSize, 0)>0;
	
	
	}
private:
	SOCKET m_serv_sock;
	SOCKET m_client_sock;
	CPacket m_packet;
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
