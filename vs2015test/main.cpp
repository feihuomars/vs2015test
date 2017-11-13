#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib") //socket�����Ҫ���øÿ�

using std::cerr;
using std::cout;
using std::endl;

const char DEFAULT_PORT[] = "4000";
const int RECV_BUF_SIZE = 256;
const size_t IP_BUF_SIZE = 65;

//������
int main() {
	WSADATA wsa_data; //WSADATA����,����windows socketִ�е���Ϣ
	int i_result = 0; //���շ���ֵ
	SOCKET sock_server = INVALID_SOCKET; //�����������׽���
	SOCKET sock_client = INVALID_SOCKET; //�����ͻ����׽���
										 //addrinfo��getaddrinfo()������������������ַ��Ϣ�Ľṹ��
	addrinfo *result = nullptr; //result�Ǵ洢��ַ��Ϣ������
	addrinfo hints;
	//��ʼ��winsock��̬��(ws2_32.dll),MAKEWORD(2, 2)��������ʹ��winsock2.2�汾
	i_result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (i_result != 0) {
		cerr << "WSAStartup() function failed: " << i_result << "\n";
		system("pause");
		return 1;
	}
	//��0����ڴ�����,��ZeroMemory�ĸ���ȫ�汾
	SecureZeroMemory(&hints, sizeof(addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM; //��ʽ�׽�������TCPЭ��
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE; //socket�ĵ�ַ�ᱻ����bind()�����ĵ���
								 //ȷ���������ĵ�ַ��˿�,�������Ϣд��result��
	i_result = getaddrinfo(nullptr, DEFAULT_PORT, &hints, &result);
	if (i_result != 0) {
		cerr << "getaddrinfo() function failed with error: " << WSAGetLastError() << "\n";
		WSACleanup();
		system("pause");
		return 1;
	}
	//�����������׽���
	sock_server = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	//�׽��ִ���ʧ��
	if (sock_server == INVALID_SOCKET) {
		cerr << "socket() function failed with error: " << WSAGetLastError() << "\n";
		//��getaddrinfo()������̬�����addrinfo�еĵ�ַ��Ϣ�ͷŵ�
		freeaddrinfo(result);
		//�ͷ��׽�����Դ
		WSACleanup();
		system("pause");
		return 1;
	}
	//���������׽������ַ�����,result->ai_addr�ǽṹ���ָ��
	i_result = bind(sock_server, result->ai_addr, static_cast<int>(result->ai_addrlen));
	//��ʧ��
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
	//��ʼ����
	cout << "start listening..." << endl;
	i_result = listen(sock_server, SOMAXCONN);
	if (i_result == SOCKET_ERROR) {
		cerr << "listen() function failed with error: " << WSAGetLastError() << "\n";
		closesocket(sock_server);
		system("pause");
		return 1;
	}
	//���տͻ�������,��ȡ�ͻ���ip��ַ
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
	//ip��ַת��
	inet_ntop(AF_INET, &addr_client, ip_buf, IP_BUF_SIZE);
	cout << "client ip address: " << ip_buf << endl;
	//���պͷ�������
	char recv_buf[RECV_BUF_SIZE];
	int send_result = 0;
	do {
		//����ȱ��,�������ڴ�ռ�������������,������Ϊ���͹�������Ϣδ����256���ֽ�
		SecureZeroMemory(recv_buf, RECV_BUF_SIZE);
		//��־λһ������Ϊ0
		i_result = recv(sock_client, recv_buf, RECV_BUF_SIZE, 0);
		if (i_result > 0) {
			//exit��ʾ�ͻ�������Ͽ�����
			if (strcmp(recv_buf, "exit") == 0) {
				cout << "client requests to close the connection..." << endl;
				break;
			}
			//������յ��ֽ���
			cout << "Bytes received: " << i_result << endl;
			cout << "message received: " << recv_buf << endl;
			//��ͻ��˷��ͽ��յ�������
			send_result = send(sock_client, recv_buf, i_result, 0);
			if (send_result == SOCKET_ERROR) {
				cerr << "send() function failed with error: " << WSAGetLastError() << "\n";
				closesocket(sock_client);
				WSACleanup();
				system("pause");
				return 1;
			}
		}
		//i_result��ֵΪ0��ʾ�����Ѿ��ر�
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
	} while (i_result > 0); //do...while����ע��Ҫ�зֺ�
							//shutdown()�����׽��ֵĽ��ջ��͹���
	i_result = shutdown(sock_client, SD_SEND);
	if (i_result == SOCKET_ERROR) {
		cerr << "shutdown() function failed with error: " << WSAGetLastError() << "\n";
		closesocket(sock_client);
		WSACleanup();
		system("pause");
		return 1;
	}
	//�ر��׽���
	i_result = closesocket(sock_server);
	WSACleanup();
	cout << "socket closed..." << endl;
	system("pause");
	return 0;
}