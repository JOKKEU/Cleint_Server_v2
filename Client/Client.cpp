#define _WIN32_LEAN_AND_MEAN // требование MS


#include <iostream>
#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <windows.h>




#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4005)

using namespace std;

void check(SOCKET& socket, ADDRINFO* addrResult)
{
    closesocket(socket); // Закрываем соккет
    socket = INVALID_SOCKET; // обнуляем
    freeaddrinfo(addrResult);
}

int main()
{
    WSADATA wsaData;
    ADDRINFO hints; // продсказки для клиента о сервере
    ADDRINFO* addrResult = NULL; // куда получим данные о сервере
    SOCKET ConnectSocket = INVALID_SOCKET; // пока не создали соккет

    int result = 0; // код значения

    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        cout << "WSAStartup failed, resilt " << result << endl;
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints)); // зануляем все поля hints (все параметры hints равны 0)
    hints.ai_family = AF_INET; // ethernet
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP; // протокол TCP/IP

    result = getaddrinfo("localhost", "666", &hints, &addrResult);  // информация и сервере
    if (result != 0)
    {
        cout << "getaddrinfo failed with error: " << result << endl;
        WSACleanup();
        return 1;
    }

    ConnectSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET)
    {
        cout << "Socket creation failed" << endl;
        freeaddrinfo(addrResult); // освобождаем addrResult
        WSACleanup();
        return 1;
    }

    result = connect(ConnectSocket, addrResult->ai_addr, static_cast<int>(addrResult->ai_addrlen));

    if (result == SOCKET_ERROR)
    {
        cout << "Enable connect to server" << endl;
        check(ConnectSocket, addrResult);
        return 1;
    }

    char sendBuffer[512];
    char recvBuffer[512];
    int recvbuflen = sizeof(recvBuffer);

    while (true) {
        cout << "Enter message: ";
        cin.getline(sendBuffer, sizeof(sendBuffer));
        cout << "Your enter: " << sendBuffer << endl;
        result = send(ConnectSocket, sendBuffer, static_cast<int>(strlen(sendBuffer)), 0);
        if (result == SOCKET_ERROR) {
            cout << "Send failed: " << WSAGetLastError() << endl;
            check(ConnectSocket, addrResult);
            WSACleanup();
            return 1;
        }

        result = recv(ConnectSocket, recvBuffer, recvbuflen, 0);
        if (result > 0) {
            cout << "Received: " << recvBuffer << endl;
            ZeroMemory(recvBuffer, sizeof(recvBuffer));
            continue;
        }
        else if (result == 0) {
            cout << "Connection closed by server." << endl;
            break;
        }
        else 
        {
            cout << "Receive failed: " << WSAGetLastError() << endl;
            break;
        }
    }

    check(ConnectSocket, addrResult);
    WSACleanup();
    return 0;
}
