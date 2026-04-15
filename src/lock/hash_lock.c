#include "hash_lock.h"
#include <stdio.h>
#include <stdlib.h>

void hashInit(hash_lock_t* bucket) {
    for (int i = 0; i < HASHNUM; i++) {
        pthread_mutex_init(&bucket->table[i].mutex, NULL);
        bucket->table[i].head = NULL;
    }
}

int getValue(hash_lock_t* bucket, int key) {
    int index = HASH(key);
    
    pthread_mutex_lock(&bucket->table[index].mutex);
    
    Hnode* p = bucket->table[index].head;
    while (p != NULL) {
        if (p->key == key) {
            int value = p->value;
            pthread_mutex_unlock(&bucket->table[index].mutex);
            return value;
        }
        p = p->next;
    }
    
    pthread_mutex_unlock(&bucket->table[index].mutex);
    return -1;
}

void insert(hash_lock_t* bucket, int key, int value) {
    int index = HASH(key);
    
    pthread_mutex_lock(&bucket->table[index].mutex);
    
    Hnode* p = bucket->table[index].head;
    while (p != NULL) {
        if (p->key == key) {
            p->value = value;
            pthread_mutex_unlock(&bucket->table[index].mutex);
            return;
        }
        p = p->next;
    }
    
    Hnode* newNode = (Hnode*)malloc(sizeof(Hnode));
    newNode->key = key;
    newNode->value = value;
    newNode->next = bucket->table[index].head;
    bucket->table[index].head = newNode;
    
    pthread_mutex_unlock(&bucket->table[index].mutex);
}

int setKey(hash_lock_t* bucket, int key, int new_key) {
    if (key == new_key) {
        return 0;
    }
    
    int old_index = HASH(key);
    int new_index = HASH(new_key);
    
    if (old_index == new_index) {
        pthread_mutex_lock(&bucket->table[old_index].mutex);
        
        Hnode* prev = NULL;
        Hnode* p = bucket->table[old_index].head;
        Hnode* target = NULL;
        
        while (p != NULL) {
            if (p->key == key) {
                target = p;
                if (prev == NULL) {
                    bucket->table[old_index].head = p->next;
                } else {
                    prev->next = p->next;
                }
                break;
            }
            prev = p;
            p = p->next;
        }
        
        if (target == NULL) {
            pthread_mutex_unlock(&bucket->table[old_index].mutex);
            return -1;
        }
        
        p = bucket->table[old_index].head;
        while (p != NULL) {
            if (p->key == new_key) {
                p->value = target->value;
                free(target);
                pthread_mutex_unlock(&bucket->table[old_index].mutex);
                return 0;
            }
            p = p->next;
        }
        
        target->key = new_key;
        target->next = bucket->table[old_index].head;
        bucket->table[old_index].head = target;
        
        pthread_mutex_unlock(&bucket->table[old_index].mutex);
        return 0;
    }
    
    if (old_index < new_index) {
        pthread_mutex_lock(&bucket->table[old_index].mutex);
        pthread_mutex_lock(&bucket->table[new_index].mutex);
    } else {
        pthread_mutex_lock(&bucket->table[new_index].mutex);
        pthread_mutex_lock(&bucket->table[old_index].mutex);
    }
    
    Hnode* prev = NULL;
    Hnode* p = bucket->table[old_index].head;
    Hnode* target = NULL;
    
    while (p != NULL) {
        if (p->key == key) {
            target = p;
            if (prev == NULL) {
                bucket->table[old_index].head = p->next;
            } else {
                prev->next = p->next;
            }
            break;
        }
        prev = p;
        p = p->next;
    }
    
    if (target == NULL) {
        if (old_index < new_index) {
            pthread_mutex_unlock(&bucket->table[new_index].mutex);
            pthread_mutex_unlock(&bucket->table[old_index].mutex);
        } else {
            pthread_mutex_unlock(&bucket->table[old_index].mutex);
            pthread_mutex_unlock(&bucket->table[new_index].mutex);
        }
        return -1;
    }
    
    p = bucket->table[new_index].head;
    while (p != NULL) {
        if (p->key == new_key) {
            p->value = target->value;
            free(target);
            if (old_index < new_index) {
                pthread_mutex_unlock(&bucket->table[new_index].mutex);
                pthread_mutex_unlock(&bucket->table[old_index].mutex);
            } else {
                pthread_mutex_unlock(&bucket->table[old_index].mutex);
                pthread_mutex_unlock(&bucket->table[new_index].mutex);
            }
            return 0;
        }
        p = p->next;
    }
    
    target->key = new_key;
    target->next = bucket->table[new_index].head;
    bucket->table[new_index].head = target;
    
    if (old_index < new_index) {
        pthread_mutex_unlock(&bucket->table[new_index].mutex);
        pthread_mutex_unlock(&bucket->table[old_index].mutex);
    } else {
        pthread_mutex_unlock(&bucket->table[old_index].mutex);
        pthread_mutex_unlock(&bucket->table[new_index].mutex);
    }
    
    return 0;
}