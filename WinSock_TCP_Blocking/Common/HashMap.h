#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "DataStructs.h"

#define singleDigitMultFactor 50
#define doubleDigitMultFactor 5

#define TABLE_SIZE 100
typedef struct keyValuePair
{
    subscriber sub;
    char* key;
    int value;
    struct keyValuePair* next;
}keyValuePair;

void InitHashMap(keyValuePair** hashMap) {
    for (int i = 0; i < TABLE_SIZE; i++)
        hashMap[i] = NULL;
}

int SimpleHash(char* key)
{
    int hashedKey = 0;
    for (int i = 0; key[i] != '\0'; i++)
    {
        hashedKey += key[i];
        hashedKey = (hashedKey * key[i]) % TABLE_SIZE;
    }
    return hashedKey;
}


void Insert(keyValuePair* kvp, keyValuePair** hashMap)
{
    //Dodavanje na pocetak liste
    kvp->next = NULL;
    printf("Insert : %s\n",kvp->key);
    kvp->next = hashMap[SimpleHash(kvp->key)];
    hashMap[SimpleHash(kvp->key)] = kvp;
}

//Dobra implementacija za koliziju sa vise razlicitih kljuceva
keyValuePair* FindKey(keyValuePair** hashMap, char* key)
{
    keyValuePair* tmp = hashMap[SimpleHash(key)];
    return tmp;
}


bool RemoveKey(keyValuePair** hashMap,char* key ,subscriber subs)
{
    keyValuePair* tmp = hashMap[SimpleHash(key)];
    keyValuePair* prev = NULL;
    if (tmp == NULL)
        return false;

    while ((tmp != nullptr))
    {
        if (tmp->sub.acceptedSocket == subs.acceptedSocket)
        {
            break;
        }

        prev = tmp;
        tmp = tmp->next;
    }
    if (tmp == NULL)
        return false;
    
    if (tmp == hashMap[SimpleHash(key)]) {
        hashMap[SimpleHash(key)] = hashMap[SimpleHash(key)]->next;
    }
    else {
        prev->next = tmp->next;
    }
    return true;
}

void PrintHashMap(keyValuePair** hashMap)
{
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        if (hashMap[i] == NULL)
        {
            continue;
        }
        else
        {
            keyValuePair* tmp = hashMap[i];
            printf("\t[%d]\tkey: [%d]\t", i,tmp->key);
            while (tmp != NULL)
            {
                printf("%d -> ", tmp->value);

                tmp = tmp->next;
            }
            printf("\n");
        }
    }
}

