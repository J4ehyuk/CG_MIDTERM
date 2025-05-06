#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
	float curX, curY;	// 현재 마우스 좌표
	struct Node* next;
	struct Node* prev;	
}Node;

// 양방향 연결 리스트
//==========================================================================================
Node* create_node(float mouse_x, float mouse_y) {
	Node* newnode = (Node*)malloc(sizeof(Node));
	if (!newnode) {
		printf("mem err \n");
		exit(1);
	}

	// 노드 생성시, 눌려진 마우스 좌표로 설정
	newnode->curX = mouse_x; newnode->curY = mouse_y;
	newnode->next = NULL;
	newnode->prev = NULL;

	return newnode;
}

Node* insert_node(Node** head, float mouse_x, float mouse_y) {
	if (*head == NULL) {
		*head = create_node(mouse_x, mouse_y);
		return *head;
	}

	Node* temp = *head;
	while (temp->next != NULL) temp = temp->next;

	Node* newNode = create_node(mouse_x, mouse_y); // 마우스 이벤트때 발생한 현재 마우스 위치 전달

	newNode->next = temp->next;
	temp->next = newNode;
	newNode->prev = temp;

	return *head;
}

void delete_list(Node** head) {
	Node* temp = *head;
	Node* del = NULL;

	while (temp) {
		del = temp;
		temp = temp->next;
		free(del);
	}

	*head = NULL;
}
//==========================================================================================

#endif // !LINKEDLIST_H

