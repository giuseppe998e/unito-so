#ifndef _MSGQUEUE_H_
#define _MSGQUEUE_H_

// Librerie
#include "../global.h"
#include <sys/msg.h>

// Tipi
typedef struct {
    long mtype;
    int mtext[3];
    int msender;
} message;

// Funzioni
int msg_attach(int msgKey);

int message_size();

int msg_send(int msgID, long mtype, const int *mtext);

int msg_cclose(int msgID);

#endif