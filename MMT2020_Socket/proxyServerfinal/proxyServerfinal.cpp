// proxyServerfinal.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "proxyServerfinal.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;

int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // initialize MFC and print and error on failure
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: change error code to suit your needs
            wprintf(L"Fatal Error: MFC initialization failed\n");
            nRetCode = 1;
        }
        else
        {
            // TODO: code your application's behavior here.
			if (!intialSocket()) {

				cout << "SORRY!Cannot create a socket !\n";
				return -1;
			}
			cout << "ProxyServer initialization successful !!!" << endl;
			ProxyParam *lpProxyParam;
			SOCKET acceptSocket;
			sockaddr_in verAddr;
			HANDLE hThread;
			int hahaha = sizeof(SOCKADDR);
			while (1)
			{
				/* A browser request starts here */

				acceptSocket = accept(ProxyServer, (SOCKADDR*)&verAddr, (socklen_t*)&hahaha);

				if (acceptSocket <0)
				{
					cout << "ERROR! On Accepting Request ! i.e requests limit crossed \n";
				}
				lpProxyParam = new ProxyParam;
				if (lpProxyParam == NULL) {
					continue;
				}
				lpProxyParam->clientSocket = acceptSocket;
				/*	memset(lpBuffRevc, 0, sizeof(lpBuffRevc));
				int recvSize =recv(lpProxyParam->clientSocket, lpBuffRevc, sizeof(lpBuffRevc), 0);
				if (lpBuffRevc[0] == 'G' || lpBuffRevc[0] == 'P')
				{
				char *cacheBuffer;
				cacheBuffer = new char[recvSize + 1];
				ZeroMemory(cacheBuffer, recvSize + 1);
				memcpy(cacheBuffer, lpBuffRevc, recvSize);
				parsehttpRequest(cacheBuffer, head);
				delete cacheBuffer;
				if (strcmp(head->method, "GET") == 0 || strcmp(head->method, "POST") == 0)
				{

				if (ConnectToServer(&((ProxyParam*)lpProxyParam)->serverSocket, head->host))
				{
				cout << "Connected to :" << head->host << endl;
				int ret =send(((ProxyParam *)lpProxyParam)->serverSocket, lpBuffRevc, strlen(lpBuffRevc) + 1, 0);
				recvSize = recv(((ProxyParam*)lpProxyParam)->serverSocket, cacheBuffer, MAXSIZE, 0);
				if (recvSize <= 0)
				{
				cout << "ERROR" << endl;
				}
				ret = send(((ProxyParam*)lpProxyParam)->clientSocket, cacheBuffer, strlen(lpBuffRevc), 0);
				}

				}
				}
				*/
				hThread = (HANDLE)_beginthreadex(NULL, 0, &ProxyThread, (LPVOID)lpProxyParam, 0, 0);//thread
				CloseHandle(hThread);
				Sleep(200);
			}
			closesocket(ProxyServer);
			WSACleanup();
        }
    }
    else
    {
        // TODO: change error code to suit your needs
        wprintf(L"Fatal Error: GetModuleHandle failed\n");
        nRetCode = 1;
    }

    return nRetCode;
}
