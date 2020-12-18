#include "sem.h"

/**
 * Crea un semaforo ed imposta il valore ad 1 oppure si aggancia ad un s. già esistente
 *
 * @param semKey
 * @param semNum
 * @return
 */
int sem_attach(int semKey, int semNum){
    key_t ftokey = ftok(_FTOK_DIR, semKey);
    int semID = semget(ftokey, semNum, IPC_EXCL | IPC_CREAT | _PROG_PRMS);

    if(semID != -1){
        for(int x = 0; x < semNum; x++){
            if(semctl(semID, x, SETVAL, 1) == -1){
                printf("ATTENZIONE: Semaforo N#%d con chiave '%d' non impostato ad 1\n", x, semKey);
            }
        }
    }else {
        semID = semget(ftokey, 0, _PROG_PRMS);
    }

    if(semID == -1){
        printf("ERRORE: Impossibile creare il semaforo con chiave '%d'\n", semKey);
        exit(EXIT_FAILURE);
    }

    return semID;
}

/**
 * Attende che il semaforo è impostato a 0 prima di continuare l'esecuzione
 *
 * @param semID
 * @param semNum
 * @return
 */
int sem_waitfor_zero(int semID, short semNum) {
    struct sembuf sb = {.sem_num=semNum, .sem_op=0, .sem_flg=0};
    return semop(semID, &sb, 1);
}

/**
 * Controlla se il semaforo è impostato a 0
 *
 * @param semID
 * @param semNum
 * @return
 */
int sem_checkfor_zero(int semID, short semNum) {
    struct sembuf sb = {.sem_num=semNum, .sem_op=0, .sem_flg=IPC_NOWAIT};
    return semop(semID, &sb, 1) == 0;
}

/**
 * Cancella il semaforo di ID = "semID"
 *
 * @param semID
 * @return
 */
int sem_cclose(int semID){
    int rval = semctl(semID, 0, IPC_RMID, NULL);
    if(rval == -1){
        printf("ATTENZIONE: Non è stato possibile chiudere il semaforo ID=%d\n", semID);
    }
    return rval;
}