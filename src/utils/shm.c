#include "shm.h"

/**
 * Crea una memoria condivisa o si aggancia ad una m.c. già esistente
 *
 * @param shmKey
 * @param shmSize
 * @return
 */
int shm_attach(int shmKey, size_t shmSize) {
    key_t ftokey = ftok(_FTOK_DIR, shmKey);
    int shmID = shmget(ftokey, shmSize, _PROG_PRMS | IPC_CREAT);

    if(shmID == -1){
        printf("ERRORE: Impossibile creare la memoria condivisa con chiave '%d'\n", shmKey);
        exit(EXIT_FAILURE);
    }
    return shmID;
}

/**
 * Chiude una memoria condivisa con ID = shmID
 *
 * @param shmID
 * @return
 */
int shm_cclose(int shmID) {
    int rval = shmctl(shmID, IPC_RMID, NULL);
    if(rval == -1){
        printf("ATTENZIONE: Non è stato possibile chiudere la memoria condivisa ID=%d\n", shmID);
    }
    return rval;
}

