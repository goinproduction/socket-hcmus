#pragma once

#include "resource.h"
#include <iostream> 
#include <Windows.h> 
#include <process.h> 
#include <string.h> 
#include <string>
#include <tchar.h>
#include "afxsock.h"
#include <fstream>
#pragma comment(lib,"Ws2_32.lib") 
using namespace std;
#define BLACKLIST "blacklist.conf"
#define MAXSIZE 65507 
#define HTTP_PORT 80
#define CACHE_MAXSIZE 100

SOCKET ProxyServer;//ProxyServer là biến toàn cục
sockaddr_in ProxyServerAddr;//địa chỉ proxy
const int ProxyPort = 8888;//cổng mặc định proxy

class httpRequest//thông tin gói tin request
{
public:
	char method[4];
	char url[1024];
	char host[1024];
	char cookie[1024 * 10];
	httpRequest()
	{
		ZeroMemory(this, sizeof(httpRequest));
	}
};
bool parsehttpRequest(char *buffer, httpRequest * httpRequest) //phân tích buffer(request từ client) thành httpRequest
{
	char *p;
	char *ptr;
	bool change = false;
	const char * delim = "\r\n";

	p = strtok_s(buffer, delim, &ptr);
	printf("%s\n", p);
	if (p[0] == 'G'&&p[1] == 'E'&&p[2] == 'T')//kiểm tra là giao thức GET, và tách url
	{//GET 
		memcpy(httpRequest->method, "GET", 3);
		httpRequest->method[3] = '\0';
		memcpy(httpRequest->url, &p[4], strlen(p) - 13);

	}
	else
	{
		if (p[0] == 'P'&&p[1]=='O'&&p[2]=='S'&&p[3]=='T')//kiểm tra giao thức POST, và tách url
		{//POST  
			memcpy(httpRequest->method, "POST", 4);
			httpRequest->method[3] = '\0';
			memcpy(httpRequest->url, &p[5], strlen(p) - 14);
		}
	}
	printf("%s\n", httpRequest->url);
	p = strtok_s(NULL, delim, &ptr);

	//lần lượt phân tích request để lấy host và cookie vào struct httpRequest
	while (p)
	{
		//printf("-------%s\n", p);
		switch (p[0])
		{
		case 'H'://Host 

		{
			memcpy(httpRequest->host, &p[6], strlen(p) - 6);
		}
		break;
		case 'C'://Cookie 
			if (strlen(p) > 8)
			{
				char header[8];
				ZeroMemory(header, sizeof(header));
				memcpy(header, p, 6);
				if (!strcmp(header, "Cookie"))
				{
					memcpy(httpRequest->cookie, &p[8], strlen(p) - 8);
				}
			}
			break;
		default:
			break;
		}
		p = strtok_s(NULL, delim, &ptr);
	}
	return change;
}
struct cache_httpHeaderRequest//phần request được lưu trong bộ đệm
{
	char method[4]; 
	char url[1024];  
	char host[1024]; 
	cache_httpHeaderRequest()//khởi tạo
	{
		ZeroMemory(this, sizeof(cache_httpHeaderRequest));
	}
};
bool Isequal(cache_httpHeaderRequest cache_rq, httpRequest rq)//kiểm tra request đã được yêu cầu trước đó rq và lưu trong bộ đệm chưa
{
	if (strcmp(cache_rq.method, rq.method)) return false;
	if (strcmp(cache_rq.url, rq.url)) return false;
	if (strcmp(cache_rq.host, rq.host)) return false;
	return true;
}
struct CACHE//cấu trúc bộ đệm cho 1 request từ client
{
	cache_httpHeaderRequest cache_req ;//phần thông tin request
	char buffer[MAXSIZE];//nội dung response từ server
	char dateTime[40];//date from lastModifiedSince
	CACHE() //khởi tạo
	{
		ZeroMemory(this->buffer, MAXSIZE);
		ZeroMemory(this->dateTime, sizeof(dateTime));
	}
};
CACHE cache[CACHE_MAXSIZE];//biến toàn cục cache là mảng gồm 100 biến kiểu CACHE,mỗi CACHE lưu thông tin 1 trang web đã được yêu cầu
int numberCache = 0;//số lượng CACHE hiện tại có trong mảng cache
int Cache_find(CACHE *cache, httpRequest rq)//tìm xem rq có nằm trong mảng cache
{
	int i = 0;
	for (i = 0; i < CACHE_MAXSIZE; i++)
	{
		if (Isequal(cache[i].cache_req, rq)) return i;
	}
	return -1;
}
string add_IfModifiedSince(string buffer, string dateTime)//thêm dòng If-Modified-Since: dateTime vào chuỗi buffer(request)
{
	string res = "";
	string temp = "If-Modified-Since: ";
	temp = temp + dateTime +"\r\n";
	res = res + buffer;
	int find = res.find("Accept");//thêm vào trước dòng Accept 
	res.insert(find, temp);
	return res;
}
string getdateTime_fromLastModified(string buffer)//lấy dateTime  từ dòng LastModified:    của response
{
	string dateTime="";
	int pos1 = buffer.find("Last-Modified: ");
	if (pos1 != string::npos)
	{
		int pos2 = buffer.find("\r\n", pos1);
		dateTime = buffer.substr(pos1 + 15, (pos2 - pos1 - 15));//tách dateTime
	}
	return dateTime;
}
void insert(CACHE C)//insert 1 CACHE C vào mảng cache
{
	memcpy(&(cache[numberCache%CACHE_MAXSIZE].cache_req.host), C.cache_req.host, strlen(C.cache_req.host));
	memcpy(&(cache[numberCache%CACHE_MAXSIZE].cache_req.method), C.cache_req.method, strlen(C.cache_req.method));
	memcpy(&(cache[numberCache%CACHE_MAXSIZE].cache_req.url), C.cache_req.url, strlen(C.cache_req.url));
	memcpy(&(cache[numberCache%CACHE_MAXSIZE].buffer), C.buffer, strlen(C.buffer));
	memcpy(&(cache[numberCache%CACHE_MAXSIZE].dateTime), C.dateTime, strlen(C.dateTime));
	numberCache++;
}
void set(CACHE& C)
{

}
struct ProxyParam //cấu trúc 1 proxy server, gồm 2 socket đóng vai trò client và server
{
	SOCKET clientSocket;
	SOCKET serverSocket;
};

string message_403()//tạo chuỗi html xuất thông báo HTTP response 403 (Forbidden) Khi Client truy cập các trang web bị cấm
{
	string body = "";
	body = body +
		"<html>\r\n" +
		"<head><title>403 Forbidden</title><head>\r\n"
		"<body>\r\n" +
		"<h1> 403 Forbidden</h1>\r\n" +
		"<p>You don't have permission to access / on this server.</p>\r\n" +
		"</body>\r\n" +
		"</html>\r\n";
	string header = "";
	header = header +
		"HTTP/1.0 403 Forbidden\r\n" +
		"Content-type: text/html\r\n" +
		"Connection: close\r\n" +
		"Content-Length: 163 ";//163=body.length()
	string message = header + "\r\n\r\n" + body;
	return message;
}
bool goodWeb(char* host, ifstream &fIn)//kiểm tra host có tồn tại trong file "blacklist.conf" 
{
	string list;
	fIn.open(BLACKLIST);
	if (!fIn)//nếu chưa có blacklist thì cho truy cập mọi trang web
		return true;
	fIn.seekg(0);
	int count = 0;
	while (!fIn.eof())
	{
		getline(fIn, list);
		unsigned int srch_index = list.find((string)host);
		if (srch_index >= 0 && srch_index <= list.length()) count++;
		if (count > 0) return false;
	}
	return true;
}
bool intialSocket()//khởi tạo socket( ProxyServer với port 8888)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		/* Tell the user that we could not find a usable */
		/* WinSock DLL */
		cout << "we could not find a usable WinSock DLL(1): %d\n" << WSAGetLastError();
		return FALSE;
	}
	/* Confirm that the WinSock DLL supports 2.2.*/
	/* Note that if the DLL supports versions greater    */
	/* than 2.2 in addition to 2.2, it will still return */
	/* 2.2 in wVersion since that is the version we      */
	/* requested.                                        */
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		cout << "we could not find a usable WinSock DLL(1)\n";
		WSACleanup();
		return FALSE;
	}
	ProxyServer = socket(AF_INET, SOCK_STREAM, 0);//AF_INET :IPv4,SOCK_STREAM : TCP
	if (INVALID_SOCKET == ProxyServer)
	{
		cout << "socket failed", WSAGetLastError();
		return FALSE;
	}
	ProxyServerAddr.sin_family = AF_INET;
	ProxyServerAddr.sin_port = htons(ProxyPort);// gắn cổng 8888 vào
	ProxyServerAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(ProxyServer, (SOCKADDR*)&ProxyServerAddr, sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		cout << "bind failed\n";
		return FALSE;
	}
	if (listen(ProxyServer, SOMAXCONN) == SOCKET_ERROR)
	{
		cout << "listen";
		return FALSE;
	}
	return TRUE;
}
bool ConnectToServer(SOCKET *serverSocket, char *host)//tạo kết nối từ serverSocket tới Server với host truyền vào
{
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(HTTP_PORT);
	HOSTENT *hostent = gethostbyname(host);
	if (!hostent)
	{
		return false;
	}
	in_addr Inaddr = *((in_addr*)*hostent->h_addr_list);
	serverAddr.sin_addr.s_addr = inet_addr(inet_ntoa(Inaddr));
	*serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (*serverSocket == INVALID_SOCKET)
	{
		return false;
	}
	if (connect(*serverSocket, (SOCKADDR *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		closesocket(*serverSocket);
		return false;
	}
	return true;
}
unsigned int __stdcall ProxyThread(void* lpParameter)//tạo ra 1 luồng xử lí từ lúc nhận request từ client , thông qua proxy , xong trả response về client.
{
	char Buffer[MAXSIZE];//chuỗi buffer để nhận và gửi yêu cầu
	char *cacheBuffer;
	memset(Buffer, 0, sizeof(Buffer));
	ZeroMemory(Buffer, MAXSIZE);
	int length = sizeof(SOCKADDR_IN);
	int recvSize;//số byte nhận được
	int ret;
	ifstream fIn;
	recvSize = recv(((ProxyParam*)lpParameter)->clientSocket, Buffer, MAXSIZE, 0);//nhập yêu cầu từ client, chuỗi yêu cầu trong Buffer
	if (recvSize <= 0)//nếu số byte nhận được<=0
	{
		goto error;
	}
	//nếu là phương thức GET hay POST mới xử lí
	if ((Buffer[0] == 'G'&&Buffer[1] == 'E'&&Buffer[2] == 'T') || (Buffer[0] =='P'&&Buffer[1] == 'O'&&Buffer[2] == 'S'&&Buffer[3]=='T'))
	{
		httpRequest* header = new httpRequest();//header là cấu trúc gói request
		cacheBuffer = new char[recvSize + 1];//gói tin request tạm để xử lí phân tích các thành phần 
		ZeroMemory(cacheBuffer, recvSize + 1);
		memcpy(cacheBuffer, Buffer, recvSize);
		bool change = parsehttpRequest(cacheBuffer, header);//tách các thành phần trong gói tin request và đưa vào cấu trúc header
		delete cacheBuffer;
		
		if (!goodWeb(header->host, fIn))//kiểm tra host cần truy cập có bị cấm không
		{
			//--chặn tất cả các truy cập trùng với các domain trong file blacklist.conf
			string error403 = message_403();//tạo html HTTP response 403 (Forbidden)
			ret = send(((ProxyParam*)lpParameter)->clientSocket, error403.c_str(), error403.length(), 0);//trả về HTTP response vừa được tạo
			fIn.close();
			goto error;//kết thúc
		}
		//không có trong danh sách blacklist nên gửi yêu cầu lên server
		if (strcmp(header->method, "GET") == 0 || strcmp(header->method, "POST") == 0)
		{
			//tạo kết nối đến server tại server socket
			if (!ConnectToServer(&((ProxyParam*)lpParameter)->serverSocket, header->host))
			{
				//kết nối đến server không thành công thì kết thúc
				goto error;
			}
			//connect to server Successfully
			cout << "Successfully Connected...." << endl;
			int indexSearch = Cache_find(cache, *header);//kiểm tra xem request đó đã có trong mảng cache chưa
			if (indexSearch >= 0)//nếu có trong mảng cache
			{
				//thêm dòng If-Modified-Since: dateTime vào chuỗi bufferEXTRA để gửi lên server
				string bufferEXTRA = add_IfModifiedSince((string)Buffer, (string)cache[indexSearch].dateTime);
				//gửi request lên server với If-Modified-Since: dateTime được thêm vào để kiểm tra trang web có được cập nhật lại chưa
				ret = send(((ProxyParam *)lpParameter)->serverSocket, bufferEXTRA.c_str(), strlen(bufferEXTRA.c_str()) + 1, 0);
				//nhận response trong bufferTemp
				char bufferTemp[MAXSIZE];
				recvSize = recv(((ProxyParam*)lpParameter)->serverSocket, bufferTemp, MAXSIZE, 0);
				if (recvSize <= 0) {
					goto error;
				}
				//kiểm tra được cập nhật chưa qua thông báo "304"
				string notModified = "304";
				unsigned int srch = ((string)bufferTemp).find(notModified);
				if (srch >= 0 && srch < strlen(bufferTemp))//timg thấy "304" nghĩa là trang web chưa được cập nhật, trả về client phần response trong bộ đệm
				{
					//từ porxy trả về client
					ret = send(((ProxyParam*)lpParameter)->clientSocket, cache[indexSearch].buffer, strlen(cache[indexSearch].buffer) + 1, 0);
					goto error;
				}

			}
			//request mới và chưa có trong mảng cache
			//proxy đóng vai trò server socket gửi yêu cầu đó tới server
				ret = send(((ProxyParam *)lpParameter)->serverSocket, Buffer, strlen(Buffer) + 1, 0);
			//nhận response trong Buffer
				recvSize = recv(((ProxyParam*)lpParameter)->serverSocket, Buffer, MAXSIZE, 0);
				//cout << "\n---------------------HTTP RECIEVE"<<recvSize<<"----------\n" << Buffer << endl<<"------------------------------------"<<endl;
				if (recvSize <= 0)
				{
					goto error;//không nhận được-->kết thúc
				}
				//luu lai dateTime cua Last-Modified.
				string dateTime = getdateTime_fromLastModified((string)Buffer);
				if (dateTime != "")
				{
					if (indexSearch >= 0)
					{
						//nếu trang web đã có trong cache thì cập nhật lại
						memcpy(&(cache[indexSearch].buffer), Buffer, strlen(Buffer));
						memcpy(&(cache[indexSearch].dateTime), dateTime.c_str(), strlen(dateTime.c_str()));
						//cout << "da cap nhat" << endl;
					}
					else
					{
						//nếu chưa có thì insert vào
						CACHE C;
						memcpy(&(C.cache_req.method), header->method, strlen(header->method));
						memcpy(&(C.cache_req.url), header->url, strlen(header->url));
						memcpy(&(C.cache_req.host), header->host, strlen(header->host));
						memcpy(&(C.buffer), Buffer, strlen(Buffer));
						memcpy(&(C.dateTime), dateTime.c_str(), strlen(dateTime.c_str()));
						insert(C);
						//cout << "da luu" << endl;
					}
				}
				//từ proxy trả về cho client
				ret = send(((ProxyParam*)lpParameter)->clientSocket, Buffer, sizeof(Buffer), 0);
		}
	}
	else
	{
		goto error;
	}

error:
	Sleep(200);
	closesocket(((ProxyParam*)lpParameter)->clientSocket);//đóng kết nối
	closesocket(((ProxyParam*)lpParameter)->serverSocket);//đóng kết nối
	delete   lpParameter;
	_endthreadex(0);
	return 0;
}
