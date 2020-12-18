#ifndef _SHAREDMEM_H_
#define _SHAREDMEM_H_

// Librerie
#include "../global.h"
#include <sys/shm.h>

// Funzioni
int shm_attach(int shmKey, size_t shmSize);
int shm_cclose(int shmID);

#endif
