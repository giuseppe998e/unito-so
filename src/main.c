#include "global.h"
#include "config/config.h"
#include "student/student.h"
#include "utils/sem.h"
#include "utils/shm.h"
#include "utils/msg.h"

/**
 * Strutture contenenti le informazioni
 * delle memorie condivise
 */
struct _SHARED SHARED;

/**
 * Struttura contenente le configurazioni
 * dal file opt.conf
 */
struct _CONFIG CONFIG;

/**
 * Array contenente i pids dei processi student
 */
int *pid_stds;

/**
 * Immette i dati per il funzionamento della simulazione
 * in due PIPE per il figlio appena generato
 *
 * @param fp1
 * @param fp2
 */
void send_data(int *fp1, int *fp2) {
    int f_max_members = get_max_members();

    close(fp1[READ]);
    close(fp2[WRITE]);

    int readval = 0;
    while (readval > -1) {
        switch (readval) {
            case 0:
                write(fp1[WRITE], &f_max_members, sizeof(int));
                break;
            case 1:
                write(fp1[WRITE], &SHARED, sizeof(struct _SHARED));
                break;
            case 2:
                write(fp1[WRITE], &CONFIG, sizeof(struct _CONFIG));
                break;
            default:
                break;
        }
        read(fp2[READ], &readval, sizeof(int));
    }

    close(fp1[WRITE]);
    close(fp2[READ]);
}

/**
 * Fa il fork dei processi student
 * e chiama il metodo di invio dati
 */
void generate_students() {
    for (int x = CONFIG.POP_SIZE; x--;) {
        int fp1[2];
        int fp2[2];

        if (pipe(fp1) == -1) {
            printf("ATTENZIONE: Pipe per il processo figlio fallito.");
            continue;
        }
        if (pipe(fp2) == -1) {
            printf("ATTENZIONE: Pipe per il processo figlio fallito.");
            close(fp1[READ]);
            close(fp1[WRITE]);
            continue;
        }

        switch (pid_stds[x] = fork()) {
            case 0:
                std_main(fp1, fp2);
                exit(EXIT_SUCCESS);
            case -1:
                printf("ERRORE: Creazione del fork() PID#%d fallito\n", pid_stds[x]);
                exit(EXIT_FAILURE);
            default:
                send_data(fp1, fp2);
                break;
        }
    }
}

/**
 * Crea le memorie condivise e salvo l'ID
 */
void generate_shmem() {
    // Inizializzo SEM della SIM
    SHARED.SEM.SIM = sem_attach(_K_SIM_SEM, 1);

    // Inizializzo SEM del LOCK SHM
    SHARED.SEM.LCK = sem_attach(_K_LCK_SEM, 2);
    semctl(SHARED.SEM.LCK, 0, SETVAL, 0);
    semctl(SHARED.SEM.LCK, 1, SETVAL, 0);

    // Inizializzo MSG della SIM
    SHARED.MSG.SIM = msg_attach(_K_SIM_MSG);

    // Inizializzo SHM dei gruppi e degli studenti
    group grpp_default = {0};
    student std_default = {0};

    SHARED.SHM.GRPS = shm_attach(_K_GRPS_SHM, CONFIG.POP_SIZE * sizeof(group));
    SHARED.SHM.STDS = shm_attach(_K_STDS_SHM, CONFIG.POP_SIZE * sizeof(student));

    group *grpps = shmat(SHARED.SHM.GRPS, NULL, 0);
    student *stds = shmat(SHARED.SHM.STDS, NULL, 0);
    for (int x = CONFIG.POP_SIZE; x--;) {
        *(grpps++) = grpp_default;
        *(stds++) = std_default;
    }
    shmdt(grpps);
    shmdt(stds);

    // Inizializzo SHM della SIM
    SHARED.SHM.SIM = shm_attach(_K_SIM_SHM, sizeof(long));
}

/**
 * Chiudi le memorie condivise
 */
void close_shmem() {
    shm_cclose(SHARED.SHM.STDS);
    shm_cclose(SHARED.SHM.GRPS);
    shm_cclose(SHARED.SHM.SIM);

    sem_cclose(SHARED.SEM.SIM);
    sem_cclose(SHARED.SEM.LCK);

    msg_cclose(SHARED.MSG.SIM);
}

/**
 * Attendi i vari forks prima di avviare la simulazione
 */
void waitfor_students() {
    while (semctl(SHARED.SEM.SIM, 0, GETZCNT) < CONFIG.POP_SIZE) sleep(1);

    if (semctl(SHARED.SEM.SIM, 0, SETVAL, 0) == -1) {
        printf("ERRORE: main.c@waitfor_students()");
        exit(EXIT_FAILURE);
    }
}

/**
 * Salva in memoria condivisa l'unix time di fine simulazione
 */
void set_endsem() {
    long *s = shmat(SHARED.SHM.SIM, NULL, 0);
    *s = (long) time(NULL) + CONFIG.SIM_TIME;
    shmdt(s);
}

/**
 * Cerca uno student nella mem. condivisa tramite la
 * sua std_id
 *
 * @param stdpid
 * @return
 */
student search_student(int stdpid) {
    student *std = shmat(SHARED.SHM.STDS, NULL, SHM_RDONLY);
    for (int x = CONFIG.POP_SIZE; x-- && std->std_id != stdpid; std++);
    student new_std = *std;
    shmdt(std);
    return new_std;
}

/**
 * Calcola la dimenzione del group passato tramite parametro
 *
 * @param grp
 * @return
 */
int get_members_num(group *grp) {
    int i;
    for (i = 0; i < grp->max_members; i++) {
        if (grp->members[i] == 0) break;
    }
    return i;
}

/**
 * calcola il punteggio maggiore tra tutti i voti dei memrbi del group
 * @param grp
 * @return
 */
int calculate_score(group *grp) {//sono il leader del mio group
    int score = 0, size = get_members_num(grp);
    for (int i = 0; i < size; i++) {
        student std = search_student(grp->members[i]);
        int std_vote = std.ade_vote;
        if (score < std_vote) {
            score = std_vote;
        }
    }
    return score;
}

/**
 * Ritorna il vote del group dello student
 *
 * @param std
 * @return
 */
int group_score(student *std) {
    int score = 0;
    if (std->group_leader > 0) {
        group *grp = shmat(SHARED.SHM.GRPS, NULL, 0);
        while (std->group_leader != grp->members[0] && grp->members[0] > 0) grp++; //trovo il mio group
        if (grp->closed == 1) {
            if (grp->vote == -1) {
                grp->vote = score = calculate_score(grp);
            } else {
                score = grp->vote;
            }
            if (std->max_members != get_members_num(grp)) {
                score -= 3;
            }
        }
        shmdt(grp);
    }

    return score;
}

/**
 * Invia il segnale di arresto agli studenti
 * e distribuisce i rispettivi voti
 */
void stop_students() {
    student *stds = shmat(SHARED.SHM.STDS, NULL, SHM_RDONLY);
    for (int x = CONFIG.POP_SIZE; x--; stds++) {
        union sigval value;
        value.sival_int = group_score(stds);
        if (sigqueue(stds->std_id, SIGINT, value) != 0) {
            printf("ERRORE: Impossibile inviare il signal al processo %d", stds->std_id);
        }
    }
    shmdt(stds);
}

/**
 * Implementazione dell'insertion sort
 *
 * @param array
 * @param size
 */
void sort_scores(int *array, int size) {
    int key, j;
    for (int i = 1; i < size; i++) {
        key = array[i];
        j = i - 1;
        while (j >= 0 && array[j] > key) {
            array[j + 1] = array[j];
            j--;
        }
        array[j + 1] = key;
    }
}

/**
 * Stampa a video il risultato dei voti dei gruppi
 *
 * @param ris
 */
void print_groups_score(int *ris) {
    sort_scores(ris, CONFIG.POP_SIZE);
    int count = 1;
    for (int i = 0; i < CONFIG.POP_SIZE; i++) {
        if (ris[i] < 0) continue;
        if (ris[i] != ris[i + 1]) {
            printf("%d gruppo/i ha come voto: %d\n", count, ris[i]);
            count = 1;
            continue;
        }

        count++;
    }
}

/**
 * Stampa a video il risultato dei voti di Arch. degli studenti
 *
 * @param ris
 */
void print_students_score(int *ris) {
    sort_scores(ris, CONFIG.POP_SIZE);
    int count = 1;
    for (int i = 0; i < CONFIG.POP_SIZE; i++) {
        if (ris[i] != ris[i + 1]) {
            printf("%d studenti hanno come voto: %d\n", count, ris[i]);
            count = 1;
            continue;
        }

        count++;
    }
}

/**
 * Stampa il risultati e la media dei gruppi e degli studenti
 */
void final_results() {
    int scores[CONFIG.POP_SIZE];
    float sum_ade_vote = 0;
    student *stds = shmat(SHARED.SHM.STDS, NULL, SHM_RDONLY);
    for (int i = 0; i < CONFIG.POP_SIZE; i++, stds++) {
        sum_ade_vote = sum_ade_vote + stds->ade_vote;
        scores[i] = stds->ade_vote;
    }
    shmdt(stds);
    print_students_score(scores);
    printf("La media dei voti di Architettura di tutti gli studenti e': %.2f\n\n", sum_ade_vote / CONFIG.POP_SIZE);


    int vote_group[CONFIG.POP_SIZE];
    int num_groups = 0;
    float sum_project_vote = 0;
    group *grpp = shmat(SHARED.SHM.GRPS, NULL, SHM_RDONLY);
    for (int y = CONFIG.POP_SIZE; y--; grpp++) {
        if (grpp->members[0] > 0) {
            sum_project_vote = sum_project_vote + grpp->vote;
            vote_group[num_groups] = grpp->vote;
            num_groups++;
        }
    }
    shmdt(grpp);

    for (int x = num_groups; x < CONFIG.POP_SIZE; x++) {
        vote_group[x] = -1;
    }

    print_groups_score(vote_group);
    printf("La media dei voti del progetto e': %.2f\n", sum_project_vote / num_groups);
}

/**
 * Riceve e gestisce l'interruzione di esecuzione del gestore
 *
 * @param sig
 */
void handlesig(int sig) {
    signal(sig, SIG_IGN);
    printf("Chiusura forzata del gestore...\n");

    for (int i = CONFIG.POP_SIZE; i--;) kill(pid_stds[i], SIGKILL);

    close_shmem();
    exit(EXIT_SUCCESS);
}

/**
 * Funzione main del programma gestore
 *
 * @return
 */
int main() {
    // Carica configurazioni
    load_config_file(&CONFIG);

    // Alloco memoria per pid_stds
    pid_stds = malloc(sizeof(int) * CONFIG.POP_SIZE);

    // Registro i segnali
    signal(SIGINT, handlesig);
    signal(SIGTSTP, handlesig);
    signal(SIGQUIT, handlesig);
    signal(SIGKILL, handlesig);
    signal(SIGSEGV, handlesig);

    // Inizializza mem. condivise
    generate_shmem();

    // Crea gli studenti
    generate_students();

    // Simulazione
    waitfor_students();
    set_endsem();
    sleep(CONFIG.SIM_TIME);

    // Ferma la simulazione
    semctl(SHARED.SEM.LCK, 0, SETVAL, 1); // Lock SHM studenti
    semctl(SHARED.SEM.LCK, 1, SETVAL, 1); // Lock SHM gruppi
    stop_students();
    sleep(2);

    //Stampa i risultati della simulazione
    printf("\n\n");
    final_results();

    // Chiudi mem. condivise
    close_shmem();

    return 0;
}