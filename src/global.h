#ifndef _GLOBAL_H_
#define _GLOBAL_H_

// Librerie
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

// Costanti
#define READ  0
#define WRITE 1

#define _PROG_PRMS   0660

#define _FTOK_DIR    "opt.conf"
#define _K_STDS_SHM  123456
#define _K_GRPS_SHM  612345
#define _K_SIM_SHM   561234
#define _K_SIM_SEM   456123
#define _K_LCK_SEM   345612
#define _K_SIM_MSG   234561

// Strutture
struct _SHARED{
    struct {
        int STDS;
        int GRPS;
        int SIM;
    } SHM; // ID memorie condivise
    struct {
        int SIM;
        int LCK;
    } SEM; // ID semafori
    struct {
        int SIM;
    } MSG; // ID code di messaggi
};

struct _CONFIG {
    int POP_SIZE;
    int SIM_TIME;
    int NOF_ELEM1; // 2 persone
    int NOF_ELEM2; // 3 persone
    int NOF_ELEM3; // 4 persone
    int NOF_INVITES;
    int MAX_REJECT;
};

// Tipi definiti
typedef struct {
    int std_id;
    short max_members;
    short ade_vote;
    int group_leader;
} student;

typedef struct {
    int members[4];
    short max_members;
    short vote;
    short closed;
} group;

#endif
