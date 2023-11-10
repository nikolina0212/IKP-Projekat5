#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>


using namespace std;

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 28000

bool InitializeWindowsSockets();
bool SocketIsReady(SOCKET*);
bool SocketIsReadyForRecv(SOCKET*);
CRITICAL_SECTION cs;

char* findTopicByNumber(int num) {
    if (num == 0)
        return "Music\0";
    if (num == 1)
        return "Films\0";
    if (num == 2)
        return "Sports\0";
    if (num == 3)
        return "Politics\0";
    if (num == 4)
        return "History\0";
    else
        return "Error\0";
}

DWORD WINAPI processingPubSubEngine(LPVOID par)
{
    SOCKET connectedSocket = (SOCKET)par;

    char dataBuffer[DEFAULT_BUFLEN];

    while(true){
        while (!SocketIsReadyForRecv(&connectedSocket))
        {
            Sleep(100);
        }
        int iResult = recv(connectedSocket, dataBuffer, DEFAULT_BUFLEN, 0);
        if (iResult > 0)	
        {
            if(iResult<DEFAULT_BUFLEN)
                dataBuffer[iResult] = '\0';

            printf("\n\nPublisher je objavio : %s\n\n", dataBuffer);

        }
        else if (iResult == SOCKET_ERROR)	
        {
            printf("Connection with PubSubEngine closed!");
            break;
        }
    }

    int iResult = shutdown(connectedSocket, SD_BOTH);

    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(connectedSocket);
        WSACleanup();
        return 1;
    }


    return 0;
}


int __cdecl main(int argc, char **argv) 
{
    SOCKET connectSocket = INVALID_SOCKET;
    int iResult;
    char *messageToSend = "test message";


    if(InitializeWindowsSockets() == false)
    {
		return 1;
    }

    connectSocket = socket(AF_INET,
                           SOCK_STREAM,
                           IPPROTO_TCP);

    if (connectSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(DEFAULT_PORT);
    if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        printf("Unable to connect to server.\n");
        closesocket(connectSocket);
        WSACleanup();
    }
 
    unsigned long int nonBlockingMode = 1;
    iResult = ioctlsocket(connectSocket, FIONBIO, &nonBlockingMode);

    if (iResult == SOCKET_ERROR)
    {
        printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
        return 1;
    }
    DWORD pubSubengineThreadID;
    HANDLE hpubSubengine;

    hpubSubengine = CreateThread(NULL, 0, &processingPubSubEngine, (LPVOID)connectSocket, 0, &pubSubengineThreadID);
    int cnt = 0;
    printf("Izbaerite opciju : \n1. Normalan rad aplikacije\n2. Stres test\n");
    char opcija[2];
    gets_s(opcija, 2);
    while (true)
    {
        
        if (atoi(opcija) == 2) {
            for (int i = 0; i < 3; i++) {
                char myMessage[BUFSIZ];
                itoa(i,myMessage,10);
                while (!SocketIsReady(&connectSocket))
                {
                    printf("Cannot send message to the server\n");
                    Sleep(100);
                }

                iResult = send(connectSocket, myMessage, BUFSIZ, 0);

                if (iResult == SOCKET_ERROR)
                {
                    printf("send failed with error: %d\n", WSAGetLastError());
                    closesocket(connectSocket);
                    WSACleanup();
                    return 1;
                }
                else {
                    printf("Message sent\n");
                }
                if (i == 2) {
                    _getch();
                    closesocket(connectSocket);
                    WSACleanup();

                    return 0;
                }
            }
        }
        else {
            char myMessage[BUFSIZ];
            memset(myMessage, 0, BUFSIZ);
            printf("0. Music\n1. Films\n2. Sports\n3. Politics\n4. History\n5. Exit\n");
            printf("Unesi redni broj teme : ");

            gets_s(myMessage, BUFSIZ);

            while (!SocketIsReady(&connectSocket))
            {
                printf("Cannot send message to the server\n");
                Sleep(100);
            }

            iResult = send(connectSocket, myMessage, BUFSIZ, 0);

            if (iResult == SOCKET_ERROR)
            {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(connectSocket);
                WSACleanup();
                return 1;
            }
            else {
                printf("Message sent\n");
            }
            if (atoi(myMessage) == 5) {
                closesocket(connectSocket);
                WSACleanup();

                return 0;
            }
        }
    }
        _getch();

        printf("Bytes Sent: %ld\n", iResult);

        // cleanup

        closesocket(connectSocket);
        WSACleanup();

        return 0;
    
}

bool InitializeWindowsSockets()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return false;
    }
	return true;
}

bool SocketIsReady(SOCKET* socket)
{
    FD_SET set;
    timeval timeVal;

    FD_ZERO(&set);
    FD_SET(*socket, &set);


    timeVal.tv_sec = 0;
    timeVal.tv_usec = 0;

    int iResult;

    iResult = select(0 , NULL, &set, NULL, &timeVal);
    if (iResult == SOCKET_ERROR)
    {
        fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
        return false;
    }

    if (iResult == 0)
    {
        return false;
    }

    return true;
}
bool SocketIsReadyForRecv(SOCKET* socket)
{
    FD_SET set;
    timeval timeVal;

    FD_ZERO(&set);
    FD_SET(*socket, &set);

    timeVal.tv_sec = 0;
    timeVal.tv_usec = 0;

    int iResult;

    iResult = select(0 , &set, NULL, NULL, &timeVal);

    if (iResult == SOCKET_ERROR)
    {
        fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
        return false;
    }

    if (iResult == 0)
    {
        return false;
    }

    return true;
}