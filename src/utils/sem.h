#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

// Librerie
#include "../global.h"
#include <sys/sem.h>

// Funzioni
int sem_attach(int semKey, int semNum);

int sem_waitfor_zero(int semID, short semNum);
int sem_checkfor_zero(int semID, short semNum);

int sem_cclose(int semID);

#endif