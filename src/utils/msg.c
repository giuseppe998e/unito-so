#include "msg.h"

/**
 * Crea una coda di messaggi o si aggancia ad una c.d.m. già esistente
 *
 * @param msgKey
 * @return
 */
int msg_attach(int msgKey) {
    key_t ftokey = ftok(_FTOK_DIR, msgKey);
    int cmsgID = msgget(ftokey, _PROG_PRMS | IPC_CREAT);

    if(cmsgID == -1){
        printf("ERRORE: Impossibile creare la coda di messaggi con chiave '%d'\n", msgKey);
        exit(EXIT_FAILURE);
    }
    return cmsgID;
}

/**
 * Ritorna la dimenzione in byte del tipo messaggio
 */
int message_size(){
    static int size = -1;
    if(size == -1) size = sizeof(message) - sizeof(long);
    return size;
}

/**
 * Invia un messaggio di tipo "messaggio" sulla coda di messaggi msgID
 *
 * @param msgID
 * @param mtype
 * @param mtext
 * @return
 */
int msg_send(int msgID, long mtype, const int *mtext) {
    message msg;
    msg.mtype = mtype;
    msg.msender = getpid();
    msg.mtext[0] = mtext[0];
    msg.mtext[1] = mtext[1];
    msg.mtext[2] = mtext[2];

    int rval = msgsnd(msgID, &msg, message_size(), 0);
    if(rval == -1){
        printf("[%d]ATTENZIONE: Messaggio[Type:%ld] non inviato!\n", getpid(), msg.mtype);
    }
    return rval;
}

/**
 * Chiude una coda di messaggi con ID = msgID
 *
 * @param msgID
 * @return
 */
int msg_cclose(int msgID) {
    int rval = msgctl(msgID, IPC_RMID, NULL);
    if(rval == -1){
        printf("ATTENZIONE: Non è stato possibile chiudere la coda di messaggi ID=%d\n", msgID);
    }
    return rval;
}
