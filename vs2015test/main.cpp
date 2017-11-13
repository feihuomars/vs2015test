#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib") //socket编程需要引用该库

using std::cerr;
using std::cout;
using std::endl;

const char DEFAULT_PORT[] = "4000";
const int RECV_BUF_SIZE = 256;
const size_t IP_BUF_SIZE = 65;

//服务器
int main() {
	WSADATA wsa_data; //WSADATA变量,包含windows socket执行的信息
	int i_result = 0; //接收返回值
	SOCKET sock_server = INVALID_SOCKET; //创建服务器套接字
	SOCKET sock_client = INVALID_SOCKET; //创建客户端套接字
										 //addrinfo是getaddrinfo()函数用来保存主机地址信息的结构体
	addrinfo *result = nullptr; //result是存储地址信息的链表
	addrinfo hints;
	//初始化winsock动态库(ws2_32.dll),MAKEWORD(2, 2)用于请求使用winsock2.2版本
	i_result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (i_result != 0) {
		cerr << "WSAStartup() function failed: " << i_result << "\n";
		system("pause");
		return 1;
	}
	//用0填充内存区域,是ZeroMemory的更安全版本
	SecureZeroMemory(&hints, sizeof(addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM; //流式套接字用于TCP协议
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE; //socket的地址会被用于bind()函数的调用
								 //确定服务器的地址与端口,将相关信息写入result中
	i_result = getaddrinfo(nullptr, DEFAULT_PORT, &hints, &result);
	if (i_result != 0) {
		cerr << "getaddrinfo() function failed with error: " << WSAGetLastError() << "\n";
		WSACleanup();
		system("pause");
		return 1;
	}
	//创建服务器套接字
	sock_server = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	//套接字创建失败
	if (sock_server == INVALID_SOCKET) {
		cerr << "socket() function failed with error: " << WSAGetLastError() << "\n";
		//将getaddrinfo()函数动态分配的addrinfo中的地址信息释放掉
		freeaddrinfo(result);
		//释放套接字资源
		WSACleanup();
		system("pause");
		return 1;
	}
	//将服务器套接字与地址对象绑定,result->ai_addr是结构体的指针
	i_result = bind(sock_server, result->ai_addr, static_cast<int>(result->ai_addrlen));
	//绑定失败
	if (i_result == SOCKET_ERROR) {
		cerr << "bind() function failed with error: " << WSAGetLastError() << "\n";
		freeaddrinfo(result);
		closesocket(sock_server);
		WSACleanup();
		system("pause");
		return 1;
	}
	freeaddrinfo(result);
	cout << "server started successfully..." << endl;
	//开始监听
	cout << "start listening..." << endl;
	i_result = listen(sock_server, SOMAXCONN);
	if (i_result == SOCKET_ERROR) {
		cerr << "listen() function failed with error: " << WSAGetLastError() << "\n";
		closesocket(sock_server);
		system("pause");
		return 1;
	}
	//接收客户端请求,获取客户端ip地址
	SOCKADDR_IN addr_client;
	int len_addr = sizeof(SOCKADDR_IN);
	char ip_buf[IP_BUF_SIZE];
	SecureZeroMemory(ip_buf, IP_BUF_SIZE);
	sock_client = accept(sock_server, (SOCKADDR*)&addr_client, &len_addr);
	if (sock_client == INVALID_SOCKET) {
		cerr << "accept() function failed with error: " << WSAGetLastError() << "\n";
		closesocket(sock_server);
		WSACleanup();
		system("pause");
		return 1;
	}
	cout << "client connected..." << endl;
	//ip地址转换
	inet_ntop(AF_INET, &addr_client, ip_buf, IP_BUF_SIZE);
	cout << "client ip address: " << ip_buf << endl;
	//接收和发送数据
	char recv_buf[RECV_BUF_SIZE];
	int send_result = 0;
	do {
		//不可缺少,若不将内存空间清零会输出乱码,这是因为输送过来的信息未必有256个字节
		SecureZeroMemory(recv_buf, RECV_BUF_SIZE);
		//标志位一般设置为0
		i_result = recv(sock_client, recv_buf, RECV_BUF_SIZE, 0);
		if (i_result > 0) {
			//exit表示客户端请求断开连接
			if (strcmp(recv_buf, "exit") == 0) {
				cout << "client requests to close the connection..." << endl;
				break;
			}
			//输出接收的字节数
			cout << "Bytes received: " << i_result << endl;
			cout << "message received: " << recv_buf << endl;
			//向客户端发送接收到的数据
			send_result = send(sock_client, recv_buf, i_result, 0);
			if (send_result == SOCKET_ERROR) {
				cerr << "send() function failed with error: " << WSAGetLastError() << "\n";
				closesocket(sock_client);
				WSACleanup();
				system("pause");
				return 1;
			}
		}
		//i_result的值为0表示连接已经关闭
		else if (i_result == 0) {
			cout << "connection closed..." << endl;
		}
		else {
			cerr << "recv() function failed with error: " << WSAGetLastError() << "\n";
			closesocket(sock_client);
			WSACleanup();
			system("pause");
			return 1;
		}
	} while (i_result > 0); //do...while语句后注意要有分号
							//shutdown()禁用套接字的接收或发送功能
	i_result = shutdown(sock_client, SD_SEND);
	if (i_result == SOCKET_ERROR) {
		cerr << "shutdown() function failed with error: " << WSAGetLastError() << "\n";
		closesocket(sock_client);
		WSACleanup();
		system("pause");
		return 1;
	}
	//关闭套接字
	i_result = closesocket(sock_server);
	WSACleanup();
	cout << "socket closed..." << endl;
	system("pause");
	return 0;
}