#define WIN32_LEAN_AND_MEAN //

#include <Windows.h>
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

using namespace std;

int main() {
    WSADATA wsaData;
    ADDRINFO hints;
    ADDRINFO* addrResult;
    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ConnectSocket = INVALID_SOCKET;
    char recvBuffer[512];//будет использоваться для приема данных

    const char* sendBuffer = "Hello from server";

    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);//инициализирует библиотеку Winsock. Параметры MAKEWORD(2, 2) указывают на версию Winsock, а &wsaData передает указатель на структуру wsaData для сохранения информации
    if (result != 0) {//если инициализация завершилась неудачно выводится сообщение об ошибке 
        cout << "WSAStartup failed with result: " << result << endl;
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));//строка для очистки памяти 
    hints.ai_family = AF_INET;//устонавливает семейство адреов 
    hints.ai_socktype = SOCK_STREAM;//устанавливает тип сокета
    hints.ai_protocol = IPPROTO_TCP;//установление протокола 
    hints.ai_flags = AI_PASSIVE;//сокет для входящих соединений 

    result = getaddrinfo(NULL, "666", &hints, &addrResult);//ищет информацию об адресе и порте для использования при соединении NUll (что означает использование локального адреса) и порт "666"
    if (result != 0) {
        cout << "getaddrinfo failed with error: " << result << endl;
        freeaddrinfo(addrResult); //освобождается выделенная память 
        WSACleanup();//завершение использования библиотеки 
        return 1;
    }

    ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);//создается новый сокет 
    if (ListenSocket == INVALID_SOCKET) {//проверется будет ли успешно создан сокет 
        cout << "Socket creation failed" << endl;//если сокет не создался выводится сообщение об ошибке 
        freeaddrinfo(addrResult);//освобождение памяти
        WSACleanup();
        return 1;
    }

    result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);//связать адресс сокета с IP адресом 
    if (result == SOCKET_ERROR) {
        cout << "Bind failed, error: " << result << endl;
        closesocket(ListenSocket);//сокет закрывается освобождая ресурсы
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }
      
    result = listen(ListenSocket, SOMAXCONN);//начинает прослушивание входящих соединений на сокет
    if (result == SOCKET_ERROR) {
        cout << "Listen failed, error: " << result << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    ConnectSocket = accept(ListenSocket, NULL, NULL);//принимает входящие соединения и создает новый сокет для взаимодействия с клиентом 
    if (ConnectSocket == INVALID_SOCKET) {
        cout << "Accept failed, error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    closesocket(ListenSocket);

    do {
        ZeroMemory(recvBuffer, 512); //буфер очищается перед приемом новых данных.
        result = recv(ConnectSocket, recvBuffer, 512, 0);//функция принимет данные от клиента на сокете. 
        if (result > 0) {
            cout << "Received " << result << " bytes" << endl;
            cout << "Received data: " << recvBuffer << endl;

            result = send(ConnectSocket, sendBuffer, (int)strlen(sendBuffer), 0);//отправить данные клиенту, данные беруться из буфера
            if (result == SOCKET_ERROR) {
                cout << "Send failed, error: " << result << endl;
                closesocket(ConnectSocket);
                freeaddrinfo(addrResult);
                WSACleanup();
                return 1;
            }
        }
        else if (result == 0) {
            cout << "Connection closing" << endl;
        }
        else {
            cout << "Recv failed, error: " << WSAGetLastError() << endl;
            closesocket(ConnectSocket);
            freeaddrinfo(addrResult);
            WSACleanup();
            return 1;
        }
    } while (result > 0);

    result = shutdown(ConnectSocket, SD_SEND);//закрывает отправку данных на сокете 
    if (result == SOCKET_ERROR) {
        cout << "Shutdown failed, error: " << result << endl;
        closesocket(ConnectSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    closesocket(ConnectSocket);
    freeaddrinfo(addrResult);
    WSACleanup();
    return 0;
}