#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

#include "../Common/Queue.h"
#include "../Common/HashMap.h"

#define DEFAULT_BUFLEN 512
#define SUBSCRIBER_PORT "28000"
#define PUBLISHER_PORT "27000"

bool InitializeWindowsSockets();
bool SocketIsReady(SOCKET*);
bool SocketIsReadyForSend(SOCKET*);
DWORD SendMessageToSubsribers();

QUEUE queue;
keyValuePair *keyValueSocket[TABLE_SIZE];
char* teme[5];
int brojTeme = 0;

CRITICAL_SECTION cs; 
CRITICAL_SECTION cs2; 
CRITICAL_SECTION cs3; 

DWORD WINAPI doWorkPublisher(LPVOID lpParam)
{
    SOCKET acceptedSocket = *(SOCKET*)lpParam;
    int iResult = 0;
    char recvbuf[DEFAULT_BUFLEN];
    
   
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(acceptedSocket, (struct sockaddr*)&sin, &len) == -1)
        perror("getsockname");

    int ulaz = 0;

    do
    {
        
        if (!SocketIsReady(&acceptedSocket))
        {
            Sleep(100);
            continue;
        }

        // Receive data until the client shuts down the connection
        iResult = recv(acceptedSocket, recvbuf, DEFAULT_BUFLEN, 0);

        if (iResult > 0)
        {
            if (atoi(recvbuf) == 5)
            {
                printf("Connection with client closed.\n");
                closesocket(acceptedSocket);
                break;
            }
                if ((int)strlen(recvbuf) == 2)
                {
                    printf("Connection with client closed.\n");
                    closesocket(acceptedSocket);
                    break;
                }

                EnterCriticalSection(&cs);
                MESSAGE* message = (MESSAGE*)recvbuf;
                MESSAGE pom;
                memset(&pom, 0, sizeof(MESSAGE));
                strcpy((char*)pom.topic, (char*)message->topic);
                strcpy((char*)pom.text, (char*)message->text);
  
                Enqueue(&queue, pom);
                LeaveCriticalSection(&cs);
                SendMessageToSubsribers();
     
                if ((int)strlen(recvbuf) == 2)
                {
                    printf("Connection with client closed.\n");
                    closesocket(acceptedSocket);
                    break;
                }

                printf("Message received from client: %s.\n", recvbuf);

        }
        else if (iResult == 0)
        {
            // connection was closed gracefully
            printf("Connection with client closed.\n");
            closesocket(acceptedSocket);
            break;
        }
        else
        {
            // there was an error during recv
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(acceptedSocket);
        }
    } while (iResult >= 0);
    return 0;
}

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

DWORD WINAPI doWorkSubscriber(LPVOID lpParam)
{
    SOCKET acceptedSocket = *(SOCKET*)lpParam;
    int iResult = 0;
    char recvbuf[DEFAULT_BUFLEN];


    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(acceptedSocket, (struct sockaddr*)&sin, &len) == -1)
        perror("getsockname");

    int ulaz = 0;

    do
    {

        if (!SocketIsReady(&acceptedSocket))
        {
            Sleep(100);
            continue;
        }
        iResult = recv(acceptedSocket, recvbuf, DEFAULT_BUFLEN, 0);
        if (iResult > 0)
        {
           
                if (atoi(recvbuf) == 5)
                {

                    subscriber sub;
                    memset(&sub, 0, sizeof(subscriber));
                    sub.acceptedSocket = acceptedSocket;

                    sub.address = inet_ntoa(sin.sin_addr);
                    sub.port = sin.sin_port;
                    for(int i=0;i<brojTeme;i++)
                        RemoveKey(keyValueSocket,teme[i],sub);
                    printf("\nBrisanje\n");
                    PrintHashMap(keyValueSocket);
                    break;
                }
                bool trebaDodatiTemu = true;
                for (int i = 0; i < brojTeme; i++) {
                    if (teme[i] == findTopicByNumber(atoi(recvbuf)))
                        trebaDodatiTemu = false;
                }
                if (trebaDodatiTemu) {
                    teme[brojTeme] = findTopicByNumber(atoi(recvbuf));
                    brojTeme++;
                }

                keyValuePair* pom = (keyValuePair*)malloc(sizeof(keyValuePair));
                subscriber sub;
                memset(&sub, 0, sizeof(subscriber));
                sub.acceptedSocket = acceptedSocket;

                sub.address = inet_ntoa(sin.sin_addr);
                sub.port = sin.sin_port;
                pom->sub = sub;
                pom->key = findTopicByNumber(atoi(recvbuf));
                pom->value = atoi(recvbuf);
                pom->next = NULL;
                if (findTopicByNumber(atoi(recvbuf)) != "Error") {
                    EnterCriticalSection(&cs2);
                    Insert(pom, keyValueSocket);
                    printf("\nHasmap\n");
                    PrintHashMap(keyValueSocket);
                    LeaveCriticalSection(&cs2);
                    
                }
                else {
                    printf("Unknown topic\n");
                }
                printf("Message received from client: %s.\n", recvbuf);

        }
        else if (iResult == 0)
        {
            Sleep(100);
            
        }
        else
        {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(acceptedSocket);
        }
    } while (iResult >= 0);
    return 0;
}

DWORD WINAPI AlertConcreteSubscriber(LPVOID pair) {
    SOCKET acceptedSocekt = ((subscriber*)pair)->acceptedSocket;
    char mess[DEFAULT_BUFLEN];
    sprintf(mess, "%s", ((subscriber*)pair)->message);
    while (!SocketIsReadyForSend(&acceptedSocekt)) {
        Sleep(100);
    }
    // salji svakom pojedinacno
    int iResult = send(acceptedSocekt, (char*)mess, (int)strlen(mess), 0);

    // Check result of send function
    if (iResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(acceptedSocekt);
        WSACleanup();
        return 1;
    }

    return 0;
}

DWORD SendMessageToSubsribers() {
    MESSAGE* message=(MESSAGE*)malloc(sizeof(MESSAGE));
    keyValuePair* par;
    char* topics[5] = { "Music\0","Films\0","Sports\0","Politics\0","History\0" };
    for (int i = 0; i < 5; i++) {
        while (!isEmpty(&queue)) {
            EnterCriticalSection(&cs3);
            if (Dequeue(&queue,message)) {
                par = FindKey(keyValueSocket, (char*)message->topic);
                while (par != NULL) {
                    subscriber* sub = (subscriber*)malloc(sizeof(subscriber));
                    sub->acceptedSocket = (par->sub).acceptedSocket;
                    DWORD concreteSubscriberID;
                    strcpy(sub->message, (char*)message->text);
                    
                    HANDLE threadSendToSub= CreateThread(NULL, 0, &AlertConcreteSubscriber, (LPVOID)sub, 0, &concreteSubscriberID);
                    par = par->next;
                }
            }
            LeaveCriticalSection(&cs3);
        }
    }
    free(message);
    return 0;
}

int  main(void) 
{

    SOCKET listenSocketSub = INVALID_SOCKET;
    SOCKET acceptedSocketSub = INVALID_SOCKET;

    SOCKET listenSocketPub = INVALID_SOCKET;
    SOCKET acceptedSocketPub = INVALID_SOCKET;

    int iResultSub;
    int iResultPub;
    
    if(InitializeWindowsSockets() == false)
    {
		return 1;
    }
    
    InitializeQUEUE(&queue);
    //*keyValueSocket = (keyValuePair*)malloc(TABLE_SIZE * sizeof(keyValuePair));
    InitHashMap(keyValueSocket);
    InitializeCriticalSection(&cs);
    InitializeCriticalSection(&cs2);
    InitializeCriticalSection(&cs3);
    addrinfo *resultingAddressSub = NULL;
    addrinfo hintsSub;

    addrinfo* resultingAddressPub = NULL;
    addrinfo hintsPub;

    memset(&hintsSub, 0, sizeof(hintsSub));
    hintsSub.ai_family = AF_INET;       
    hintsSub.ai_socktype = SOCK_STREAM; 
    hintsSub.ai_protocol = IPPROTO_TCP; 
    hintsSub.ai_flags = AI_PASSIVE;     

    memset(&hintsPub, 0, sizeof(hintsPub));
    hintsPub.ai_family = AF_INET;       
    hintsPub.ai_socktype = SOCK_STREAM; 
    hintsPub.ai_protocol = IPPROTO_TCP; 
    hintsPub.ai_flags = AI_PASSIVE;     

    iResultSub = getaddrinfo(NULL, SUBSCRIBER_PORT, &hintsSub, &resultingAddressSub);
    if ( iResultSub != 0 )
    {
        printf("getaddrinfo failed with error: %d\n", iResultSub);
        WSACleanup();
        return 1;
    }

    iResultPub = getaddrinfo(NULL, PUBLISHER_PORT, &hintsPub, &resultingAddressPub);
    if (iResultPub != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResultPub);
        WSACleanup();
        return 1;
    }

    listenSocketSub = socket(AF_INET,      
                          SOCK_STREAM,  
                          IPPROTO_TCP);

    if (listenSocketSub == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(resultingAddressSub);
        WSACleanup();
        return 1;
    }

    listenSocketPub = socket(AF_INET,   
        SOCK_STREAM,  
        IPPROTO_TCP); 

    if (listenSocketPub == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(resultingAddressPub);
        WSACleanup();
        return 1;
    }

    iResultSub = bind( listenSocketSub, resultingAddressSub->ai_addr, (int)resultingAddressSub->ai_addrlen);
    if (iResultSub == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(resultingAddressSub);
        closesocket(listenSocketSub);
        WSACleanup();
        return 1;
    }

    iResultPub = bind(listenSocketPub, resultingAddressPub->ai_addr, (int)resultingAddressPub->ai_addrlen);
    if (iResultPub == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(resultingAddressPub);
        closesocket(listenSocketPub);
        WSACleanup();
        return 1;
    }


    freeaddrinfo(resultingAddressSub);
    freeaddrinfo(resultingAddressPub);

    unsigned long int nonBlockingMode = 1;
    iResultSub = ioctlsocket(listenSocketSub, FIONBIO, &nonBlockingMode);

    if (iResultSub == SOCKET_ERROR)
    {
        printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
        return 1;
    }
    
    iResultPub = ioctlsocket(listenSocketPub, FIONBIO, &nonBlockingMode);

    if (iResultPub == SOCKET_ERROR)
    {
        printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
        return 1;
    }

    iResultSub = listen(listenSocketSub, SOMAXCONN);
    if (iResultSub == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocketSub);
        WSACleanup();
        return 1;
    }

    iResultPub = listen(listenSocketPub, SOMAXCONN);
    if (iResultPub == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocketPub);
        WSACleanup();
        return 1;
    }
	printf("Server initialized, waiting for clients.\n");

    do
    {
        if ((!SocketIsReady(&listenSocketSub)) && (!SocketIsReady(&listenSocketPub)))
        {
            Sleep(1000);
            continue;
        }

        if (!SocketIsReady(&listenSocketPub)) {
            acceptedSocketSub = accept(listenSocketSub, NULL, NULL);

            if (acceptedSocketSub == INVALID_SOCKET)
            {
                printf("accept failed with error: %d\n", WSAGetLastError());
                closesocket(listenSocketSub);
                WSACleanup();
                return 1;
            }
            iResultSub = ioctlsocket(acceptedSocketSub, FIONBIO, &nonBlockingMode);

            if (iResultSub == SOCKET_ERROR)
            {
                printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
                return 1;
            }

            DWORD dwPrint1ID;
            HANDLE handle = CreateThread(NULL, 0, &doWorkSubscriber, &acceptedSocketSub, 0, &dwPrint1ID);

        }
        else {

            acceptedSocketPub = accept(listenSocketPub, NULL, NULL);

            if (acceptedSocketPub == INVALID_SOCKET)
            {
                printf("accept failed with error: %d\n", WSAGetLastError());
                closesocket(listenSocketPub);
                WSACleanup();
                return 1;
            }
            iResultPub = ioctlsocket(acceptedSocketPub, FIONBIO, &nonBlockingMode);

            if (iResultPub == SOCKET_ERROR)
            {
                printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
                return 1;
            }


            DWORD dwPrint2ID;
            HANDLE handle2 = CreateThread(NULL, 0, &doWorkPublisher, &acceptedSocketPub, 0, &dwPrint2ID);

        }


    } while (1);

    iResultSub = shutdown(acceptedSocketSub, SD_SEND);
    if (iResultSub == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(acceptedSocketSub);
        WSACleanup();
        return 1;
    }

    iResultSub = shutdown(acceptedSocketPub, SD_SEND);
    if (iResultSub == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(acceptedSocketPub);
        WSACleanup();
        return 1;
    }

    closesocket(listenSocketSub);
    closesocket(acceptedSocketSub);
    closesocket(listenSocketPub);
    closesocket(acceptedSocketPub);

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

bool SocketIsReady(SOCKET *socket)
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

bool SocketIsReadyForSend(SOCKET* socket)
{
    FD_SET set;
    timeval timeVal;

    FD_ZERO(&set);
    FD_SET(*socket, &set);

    timeVal.tv_sec = 0;
    timeVal.tv_usec = 0;

    int iResult;

    iResult = select(0, NULL, &set, NULL, &timeVal);

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
