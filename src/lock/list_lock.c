#include "list_lock.h"
#include <stdio.h>
#include <stdlib.h>

void listInit(list_lock_t* list){
    list->head = NULL;
    pthread_mutex_init(&list->mutex, NULL);
    pthread_cond_init(&list->cond, NULL);
}

void producer(list_lock_t* list, DataType value){
    LinkList newNode = (LinkList)malloc(sizeof(LNode));
    newNode->value = value;
    
    pthread_mutex_lock(&list->mutex);
    
    newNode->next = list->head;
    list->head = newNode;
    
    pthread_cond_signal(&list->cond);
    pthread_mutex_unlock(&list->mutex);
}

void consumer(list_lock_t* list){
    pthread_mutex_lock(&list->mutex);
    
    while(list->head == NULL){
        pthread_cond_wait(&list->cond, &list->mutex);
    }
    
    LinkList temp = list->head;
    list->head = list->head->next;
    free(temp);
    
    pthread_mutex_unlock(&list->mutex);
}

int getListSize(list_lock_t* list){
    pthread_mutex_lock(&list->mutex);
    
    int count = 0;
    LinkList p = list->head;
    while(p != NULL){
        count++;
        p = p->next;
    }
    
    pthread_mutex_unlock(&list->mutex);
    return count;
}