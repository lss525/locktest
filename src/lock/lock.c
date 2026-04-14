#include "lock.h"
#include <stdio.h>
#include <pthread.h>

void amountInit(lock_t* account) {
    if (account == NULL) {
        perror("Invalid account pointer");
        return;
    }
    
    if (pthread_mutex_init(&account->mutex, NULL) != 0) {
        perror("Mutex initialization failed");
        return;
    }
    
    account->amount = 0;
}

void Income(lock_t* account, int amount) {
    if (account == NULL) {
        perror("Invalid account pointer");
        return;
    }
    
    if (amount <= 0) {
        perror("Invalid income amount");
        return;
    }
    
    if (pthread_mutex_lock(&account->mutex) != 0) {
        perror("Mutex lock failed");
        return;
    }
    
    account->amount += amount;

    if (pthread_mutex_unlock(&account->mutex) != 0) {
        perror("Mutex unlock failed");
    }
}

void Expend(lock_t* account, int amount) {
    if (account == NULL) {
        perror("Invalid account pointer");
        return;
    }
    
    if (amount <= 0) {
        perror("Invalid expend amount");
        return;
    }
    
    if (pthread_mutex_lock(&account->mutex) != 0) {
        perror("Mutex lock failed");
        return;
    }
    
    if (account->amount >= amount) {
        account->amount -= amount;
    } 
    else {
        perror("Insufficient balance");
    }
    
    if (pthread_mutex_unlock(&account->mutex) != 0) {
        perror("Mutex unlock failed");
    }
}