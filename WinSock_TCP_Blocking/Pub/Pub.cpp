#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include "../Common/DataStructs.h"
#include "../Common/HashMap.h"

using namespace std;

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27000

// Initializes WinSock2 library
// Returns true if succeeded, false otherwise.
bool InitializeWindowsSockets();

bool SocketIsReady(SOCKET*);

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

int __cdecl main(int argc, char **argv) 
{
    SOCKET connectSocket = INVALID_SOCKET;
    int iResult;
    char *messageToSend = "test message";
  
    if(InitializeWindowsSockets() == false)
    {
		return 1;
    }

    // create a socket
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

    int cnt = 0;
    printf("Odaberite opciju : \n");
    printf("1. Normalan rad aplikacije \n");
    printf("2. Stres test sa timeout-om\n");
    printf("3. Stres test bez timout-a\n");
    printf("4. Stres test beskonacnost\n");
    char brojIzbora[2];
    gets_s(brojIzbora, 2);
    while (true)
    {
        
        if (atoi(brojIzbora) == 2) {
            char tema[TOPIC_LENGTH];
            itoa(rand() % 5, tema, 10);
            printf(tema);

            char text_o_temi[474];
            
            strcpy(text_o_temi, "stres test ");
            for (int i = 0; i < 10000000; i++) {
                MESSAGE message;
                memset(&message, '\0', int(sizeof(MESSAGE)));

                strcpy((char*)message.text, text_o_temi);
                strcpy((char*)message.topic, findTopicByNumber(atoi(tema)));
                while (!SocketIsReady(&connectSocket))
                {
                    printf("Cannot send message to the server\n");
                    Sleep(100);
                }

                iResult = send(connectSocket, (char*)&message, BUFSIZ, 0);

                if (iResult == SOCKET_ERROR)
                {
                    printf("send failed with error: %d\n", WSAGetLastError());
                    closesocket(connectSocket);
                    WSACleanup();
                    return 1;
                }
                Sleep(100);
            }
        }
        if (atoi(brojIzbora) == 3) {
            char tema[TOPIC_LENGTH];
            itoa(rand() % 5, tema, 10);
            printf(tema);

            char text_o_temi[474];

            strcpy(text_o_temi, "stres test ");
            for (int i = 0; i < 10000000; i++) {
                MESSAGE message;
                memset(&message, '\0', int(sizeof(MESSAGE)));

                strcpy((char*)message.text, text_o_temi);
                strcpy((char*)message.topic, findTopicByNumber(atoi(tema)));
                while (!SocketIsReady(&connectSocket))
                {
                    Sleep(100);
                }

                iResult = send(connectSocket, (char*)&message, BUFSIZ, 0);

                if (iResult == SOCKET_ERROR)
                {
                    printf("send failed with error: %d\n", WSAGetLastError());
                    closesocket(connectSocket);
                    WSACleanup();
                    return 1;
                }
            }
        }
        if (atoi(brojIzbora) == 4) {
            char tema[TOPIC_LENGTH];
            itoa(rand() % 5, tema, 10);
            printf(tema);

            char text_o_temi[474];

            strcpy(text_o_temi, "stres test ");
            while (true) {
                MESSAGE message;
                memset(&message, '\0', int(sizeof(MESSAGE)));

                strcpy((char*)message.text, text_o_temi);
                strcpy((char*)message.topic, findTopicByNumber(atoi(tema)));
                while (!SocketIsReady(&connectSocket))
                {
                    Sleep(100);
                }

                iResult = send(connectSocket, (char*)&message, BUFSIZ, 0);

                if (iResult == SOCKET_ERROR)
                {
                    printf("send failed with error: %d\n", WSAGetLastError());
                    closesocket(connectSocket);
                    WSACleanup();
                    return 1;
                }
            }
        }
        else {
            MESSAGE message;
            char tema[50];
            memset(&message, '\0', int(sizeof(MESSAGE)));
            printf("0. Music\n1. Films\n2. Sports\n3. Politics\n4. History\n5. Exit\n");
            printf("Unesi redni broj teme : ");
            gets_s(tema, TOPIC_LENGTH);
            if (atoi(tema) == 5) {
                closesocket(connectSocket);
                WSACleanup();

                return 0;
            }
            char* topicTema = findTopicByNumber(atoi(tema));
            strcpy((char*)message.topic, topicTema);
            char text_o_temi[474];
            printf("Unesi text : ");
            gets_s(text_o_temi, TEXT_LENGTH);

            strcpy((char*)message.text, text_o_temi);

            while (!SocketIsReady(&connectSocket))
            {
                Sleep(100);
            }

            iResult = send(connectSocket, (char*)&message, BUFSIZ, 0);

            if (iResult == SOCKET_ERROR)
            {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(connectSocket);
                WSACleanup();
                return 1;
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
