#include "student.h"

/**
 * Struttura con i valori dello student corrente
 */
student me;

/**
 * Struttura con i valori del group,
 * nel caso lo student fosse il leader
 */
group my_group;

/**
 * Strutture contenenti le informazioni
 * delle memorie condivise
 */
struct _SHARED ST_SHARED;

/**
 * Struttura contenente le configurazioni
 */
struct _CONFIG ST_CONFIG;

/**
 * Restituisce 1 se il tempo rimanente è uguale
 * o minore della percentuale data, 0 altrimenti
 *
 * @param percentuale
 * @return
 */
int tempo_sim_restante() {
    long *s = shmat(ST_SHARED.SHM.SIM, NULL, SHM_RDONLY);
    long timeleft = *s - (long) time(NULL);
    shmdt(s);
    return timeleft;
}

/**
 * Salva i dati dello student corrente in memoria condivisa
 */
void save_me() {
    if (!sem_checkfor_zero(ST_SHARED.SEM.LCK, 0)) return;

    student *std = shmat(ST_SHARED.SHM.STDS, NULL, 0);
    while (std->std_id != me.std_id && std->std_id > 0) std++;
    *std = me;
    shmdt(std);
}

/**
 * Salva i dati del group dello student
 * corrente se questi è leader
 */
void save_mygroup() {
    if (my_group.members[0] != me.std_id) return;
    if (!sem_checkfor_zero(ST_SHARED.SEM.LCK, 1)) return;

    my_group.vote = -1;

    group *grpp = shmat(ST_SHARED.SHM.GRPS, NULL, 0);
    while (grpp->members[0] != me.std_id && grpp->members[0] > 0) grpp++;
    *grpp = my_group;
    shmdt(grpp);
}

/**
 * Comportamento di gestione richieste che riceve l'algoritmo
 *
 * @param msgrcvd
 * @param is_waiting
 * @param mempos
 */
void group_receive_request(message *msgrcvd, short *is_waiting, short *mempos) {
    int resp_data[3] = {0, -1, -1};
    if (msgrcvd->mtext[0] > 1) {
        if (me.group_leader > 0 || *is_waiting) {
            msg_send(ST_SHARED.MSG.SIM, msgrcvd->msender, resp_data);
            return;
        }

        if (ST_CONFIG.MAX_REJECT > 0) {
            if (tempo_sim_restante() > SHORT_WHILE) {
                if (msgrcvd->mtext[0] >= me.ade_vote) {
                    me.group_leader = msgrcvd->msender;
                    save_me();
                    resp_data[0] = 1;
                } else {
                    ST_CONFIG.MAX_REJECT--;
                }
            } else {
                if (msgrcvd->mtext[1] - msgrcvd->mtext[2] == 1 || msgrcvd->mtext[0] >= me.ade_vote) {
                    me.group_leader = msgrcvd->msender;
                    save_me();
                    resp_data[0] = 1;
                } else {
                    ST_CONFIG.MAX_REJECT--;
                }
            }
        } else {
            me.group_leader = msgrcvd->msender;
            save_me();
            resp_data[0] = 1;
        }
        msg_send(ST_SHARED.MSG.SIM, msgrcvd->msender, resp_data);

    } else {
        *is_waiting = 0;

        if (msgrcvd->mtext[0] == 1) {
            if (my_group.members[0] != me.std_id) {
                my_group.members[0] = me.group_leader = me.std_id;
                my_group.max_members = me.max_members;
                my_group.closed = 0;
                save_me();
            }
            my_group.members[*mempos] = msgrcvd->msender;
            *mempos = *mempos + 1;

            if (my_group.members[me.max_members - 1] > 0) my_group.closed = 1;
            save_mygroup();
        }
    }
}

/**
 * Comportamento di gestione delle richieste che effettua l'algoritmo
 *
 * @param is_waiting
 * @param mempos
 */
void group_send_request(short *is_waiting, const short *mempos) {
    if (me.max_members == 1) {
        if (my_group.members[0] != me.std_id) {
            my_group.members[0] = me.group_leader = me.std_id;
            my_group.max_members = me.max_members;
            my_group.closed = 1;
            save_me();
            save_mygroup();
        }
        return;
    }

    if (my_group.members[me.max_members - 1] > 0) return;

    static int old_random_std = 0;
    student *stds = shmat(ST_SHARED.SHM.STDS, NULL, SHM_RDONLY);
    while (ST_CONFIG.NOF_INVITES) {
        int random_std = rand() % ST_CONFIG.POP_SIZE;

        if (old_random_std == random_std) continue;
        else old_random_std = random_std;

        if (stds[random_std].std_id == me.std_id) continue;
        if ((stds[random_std].std_id % 2) != (me.std_id % 2)) continue;
        if (stds[random_std].group_leader > 0) continue;

        int req_data[3] = {me.ade_vote, me.max_members, *mempos};
        if (msg_send(ST_SHARED.MSG.SIM, stds[random_std].std_id, req_data) > -1) {
            *is_waiting = 1;
            ST_CONFIG.NOF_INVITES--;
            break;
        }
    }
    shmdt(stds);
}

/**
 * Creazione dei gruppi studenti
 */
void create_group() {
    short is_waiting = 0, mempos = 1;

    if (rand() % 2 == 0) group_send_request(&is_waiting, &mempos);
    else sleep(1);

    while (tempo_sim_restante()) {
        message msgrcvd;
        if (msgrcv(ST_SHARED.MSG.SIM, &msgrcvd, message_size(), me.std_id, IPC_NOWAIT) > -1) {
            group_receive_request(&msgrcvd, &is_waiting, &mempos);
        } else if (me.group_leader == 0 || me.group_leader == me.std_id) {
            if (is_waiting) continue;
            group_send_request(&is_waiting, &mempos);
        }
    }

    my_group.closed = 1;
    save_mygroup();
}

/**
 * Inizializza la struttura student me
 */
void initialize_me() {
    me.std_id = getpid();
    me.ade_vote = (short) (rand() % (31 - 18) + 18);
    me.group_leader = 0;
}

/**
 * Riceve i dati della simulazione dal gestore attraverso
 * due PIPE
 *
 * @param fp1
 * @param fp2
 */
void read_pipe_data(int *fp1, int *fp2) {
    close(fp1[WRITE]);
    close(fp2[READ]);

    int writeval = 0;
    read(fp1[READ], &me.max_members, sizeof(int));

    writeval = 1;
    write(fp2[WRITE], &writeval, sizeof(int));
    read(fp1[READ], &ST_SHARED, sizeof(struct _SHARED));

    writeval = 2;
    write(fp2[WRITE], &writeval, sizeof(int));
    read(fp1[READ], &ST_CONFIG, sizeof(struct _CONFIG));

    writeval = -1;
    write(fp2[WRITE], &writeval, sizeof(int));

    close(fp1[READ]);
    close(fp2[WRITE]);
}

/**
 * Funzione che gestisce la ricezione del segnale SIGINT
 *
 * @param sig
 * @param info
 * @param extra
 */
void std_handlesig(int sig, siginfo_t *info, void *extra) {
    signal(sig, SIG_IGN);

    int vote = info->si_value.sival_int;
    printf("Studente %d [max_members: %d, voto_ade: %d, group_leader: %d]: Ho ricevuto un vote pari a %d\n", me.std_id, me.max_members, me.ade_vote, me.group_leader, vote);

    exit(EXIT_SUCCESS);
}

/**
 * Funzione main del processo student
 *
 * @param fp1
 * @param fp2
 */
void std_main(int *fp1, int *fp2) {
    srand((unsigned int) getpid());

    struct sigaction action;
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = &std_handlesig;
    if (sigaction(SIGINT, &action, NULL) == -1) {
        printf("[%d] ERRORE: Impossibile registrare il segnale!", getpid());
        exit(EXIT_FAILURE);
    }

    read_pipe_data(fp1, fp2);
    initialize_me();
    save_me();

    sem_waitfor_zero(ST_SHARED.SEM.SIM, 0);

    create_group();

    // Wait for manager process
    while (1) sleep(1);
}
