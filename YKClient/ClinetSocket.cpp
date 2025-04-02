#include "pch.h"
#include "ClinetSocket.h"


CClientSocket* CClientSocket::m_instance = nullptr;

CClientSocket::CHelper CClientSocket::m_helper;

CClientSocket* pclient = CClientSocket::getInstance();