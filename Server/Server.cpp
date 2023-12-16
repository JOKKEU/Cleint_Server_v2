#define _WIN32_LEAN_AND_MEAN // ���������� Microsoft


#include <iostream>
#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h>


#define SERVER_PORT "666"
#define BUFFER_SIZE 1024

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4005)

using namespace std;

std::atomic<int> counter(0);
std::mutex mtx;


int check(SOCKET s)
{
    if (s == SOCKET_ERROR)
    {
        cout << "Socket Error" << endl;
        closesocket(s);
        return 1;
    }
    return 0;
}

int ClientID()
{
    mtx.lock();
    ++counter;
    mtx.unlock();
    return counter;
}


//��������� �������

DWORD WINAPI HandleClient(LPVOID lpParam)
{
    pair<SOCKET, int>* params = reinterpret_cast<std::pair<SOCKET, int>*>(lpParam);
    SOCKET ClientSocket = params->first;
    int ClientID = params->second;
    char recvBuffer[BUFFER_SIZE];
    int result;
    int count_messange = 1;

    do
    {
        ZeroMemory(recvBuffer, sizeof(recvBuffer));
        result = recv(ClientSocket, recvBuffer, sizeof(recvBuffer), 0); // �������� ���������� �� ����������� �������

        if (result > 0)
        {
            cout << "(" << count_messange << ") Client [" << ClientID << "] " << recvBuffer << endl;
            result = send(ClientSocket, recvBuffer, result, 0); // ���������� ������� ��������� �� �������
            if (result == SOCKET_ERROR)
            {
                cout << "Failed to send data back" << endl;
                check(ClientSocket);
                return 1;
            }
        }
        else if (result == 0)
        {
            cout << "Connection closing..." << endl;
        }
        else
        {
            cout << "Recv failed with error" << endl;
            check(ClientSocket);
            return 1;
        }
        ++count_messange;
    } while (result > 0);

    result = shutdown(ClientSocket, SD_SEND);
    if (result == SOCKET_ERROR)
    {
        cout << "shutdown client socket failed" << endl;
        check(ClientSocket);
        return 1;
    }

    closesocket(ClientSocket);
    return 0;
}

int main(int argc, char** argv)
{
    WSADATA wsaData; // ������ ������� ���������� Winsock 2.2
    ADDRINFO hints; // ���������� ��� ������� � �������
    ADDRINFO* addrResult = NULL; // ���� ������� ������ � �������
    SOCKET ClientSocket = INVALID_SOCKET; // ���������� ������
    SOCKET ListenSocket = INVALID_SOCKET; // ��������� ������


    const char* sendBuffer = "Your enter messange to server!"; // ��� �������
    char recvBuffer[BUFFER_SIZE];

    int result; // ��� ��������
        
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        cout << "WSAStartup failed, resilt " << result << endl;
        return 1;
    }

    cout << "----Server Activated----" << endl;

    ZeroMemory(&hints, sizeof(hints)); // �������� ��� ���� hints (��� ��������� hints ����� 0)
    hints.ai_family = AF_INET; // ethernet
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP; // �������� TCP/IP
    hints.ai_flags = AI_PASSIVE; // ���������� ��� ��������� ������� (�.� ������ ��������� �������, � ������ ��������)

    result = getaddrinfo(NULL, SERVER_PORT, &hints, &addrResult);  // ���������� � �������
    if (result != 0)
    {
        cout << "getaddrinfo failed with error: " << result << endl;
        WSACleanup();
        return 1;
    }

    ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ListenSocket == INVALID_SOCKET)
    {
        cout << "Socket creation failed" << endl;
        freeaddrinfo(addrResult); // ����������� addrResult
        WSACleanup();
        return 1;
    }

    result = bind(ListenSocket, addrResult->ai_addr, static_cast<int>(addrResult->ai_addrlen));

    if (result == SOCKET_ERROR)
    {
        cout << "Binding socket failed" << endl;
        check(ListenSocket);
        freeaddrinfo(addrResult);
        return 1;
    }

    result = listen(ListenSocket, SOMAXCONN); // ������ �������� - ������� �������� ����� ������� ������


    if (result != 0)
    {
        cout << "Listen socket failed" << endl;
        check(ListenSocket);
        freeaddrinfo(addrResult);
        return 1;
    }

    cout << "Server listing on port: " << SERVER_PORT << endl;


    while (true)
    {
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (check(ClientSocket))
        {   
            cout << "Accepting socket failed" << endl;
            check(ListenSocket);
            freeaddrinfo(addrResult);
            return 1;
        }
        else
        {
            pair<SOCKET, int> parametrs = make_pair(ClientSocket, ClientID());
            cout << "Client connected!" << endl;
            HANDLE hThread = CreateThread(NULL, 0, HandleClient, &parametrs, 0, NULL);
        }

    }

    //closesocket(ListenSocket); // �.� ������ �� ����� ������� ���������� �������
    //DWORD dwThreadId;
    //HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HandleClient, &ClientSocket, 0, &dwThreadId);

    check(ClientSocket);
    freeaddrinfo(addrResult);
    closesocket(ListenSocket);
    return 0;

}
