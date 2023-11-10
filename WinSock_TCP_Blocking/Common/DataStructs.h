#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN

#define _LIST_H

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"


#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#define TOPIC_LENGTH 50
#define TEXT_LENGTH 462

typedef struct Message {
	char* topic[TOPIC_LENGTH];
	char* text[TEXT_LENGTH];
}MESSAGE;

typedef struct part_of_Q {
	MESSAGE data;
	part_of_Q* next;
} PART_OF_Q;

typedef struct queue {
	PART_OF_Q* front;
	PART_OF_Q* back;
	int size;
} QUEUE;

typedef struct subscriber {
	unsigned short port;
	char* address;
	SOCKET acceptedSocket;
	char message[TEXT_LENGTH];
}SUBSCRIBER;