#pragma once
#include<stdio.h>
#include <stdlib.h>
#include "../Common/DataStructs.h"


void InitializeQUEUE(QUEUE* q) {
	q->front = NULL;
	q->back = NULL;
	q->size = 0;
}



void Enqueue(QUEUE* q, MESSAGE m) {
	struct part_of_Q* newOne = (struct part_of_Q*)malloc(sizeof(struct part_of_Q));
	newOne->data = m;
	newOne->next = NULL;

	if (q->back != NULL) {
		q->back->next = newOne;
	}

	q->back = newOne;

	if (q->front == NULL) {
		q->front = newOne;
	}
	q->size++;
}

bool Dequeue(QUEUE* q, MESSAGE* retVal) {
	if (q->front == NULL) {
		printf("Q je prazan\n");
		return false;
	}

	PART_OF_Q* temp = q->front;
	*retVal = temp->data;

	q->front = q->front->next;
	if (q->front == NULL) {
		q->back = NULL;
	}

	free(temp);
	q->size--;
	return true;
}

void ShowQueue(QUEUE* q) {
	PART_OF_Q* current = q->front;
	printf("Queue : \n");
	while (current != NULL) {
		printf("%s %s\n", current->data.topic, current->data.text);
		current = current->next;
	}

}

void ClearQueue(QUEUE* q) {
	MESSAGE retVal;
	while (Dequeue(q, &retVal))
	{
		continue;
	}
}

int isEmpty(QUEUE* q) {
	if (q == NULL) {
		return FALSE;
	}
	if (q->size == 0) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}