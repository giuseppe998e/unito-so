#include "config.h"

/**
 * Struttura privata
 */
struct {
    int ELEM1; // 2 persone
    int ELEM2; // 3 persone
    int ELEM3; // 4 persone
} NOF;

/**
 * Legge il file di configurazione "opt.conf"
 *
 */
void load_config_file(struct _CONFIG *CONFIG){
    FILE *fp;

    if((fp = fopen(_FTOK_DIR, "r")) == NULL){
        perror("ERRORE");
        exit(EXIT_FAILURE);
    }

    while(!feof(fp)){
        char name[LMAX_CHARS];
        int value = 0;
        fscanf(fp, "%s %d\n", name, &value);

        if(strstr(name, "POP_SIZE") != NULL){
            CONFIG->POP_SIZE = value;
        }else if(strstr(name, "SIM_TIME") != NULL){
            CONFIG->SIM_TIME = value;
        }else if(strstr(name, "NOF_ELEM1") != NULL){
            CONFIG->NOF_ELEM1 = value; //2 Persone
        }else if(strstr(name, "NOF_ELEM2") != NULL){
            CONFIG->NOF_ELEM2 = value; //3 Persone
        }else if(strstr(name, "NOF_ELEM3") != NULL){
            CONFIG->NOF_ELEM3 = value; //4 Persone
        }else if(strstr(name, "NOF_INVITES") != NULL){
            CONFIG->NOF_INVITES = value;
        }else if(strstr(name, "MAX_REJECT") != NULL){
            CONFIG->MAX_REJECT = value;
        }
    }
    fclose(fp);

    if(CONFIG->NOF_ELEM1 + CONFIG->NOF_ELEM2 + CONFIG->NOF_ELEM3 != 100){
        perror("ERRORE: La somma delle percentuali di preferenza deve corrispondere a 100\n");
        exit(EXIT_FAILURE);
    }

    NOF.ELEM1 = (CONFIG->POP_SIZE * CONFIG->NOF_ELEM1) / 100;
    NOF.ELEM2 = (CONFIG->POP_SIZE * CONFIG->NOF_ELEM2) / 100;
    NOF.ELEM3 = (CONFIG->POP_SIZE * CONFIG->NOF_ELEM3) / 100;
}


/**
 * Restituisce la preferenza dei members del group
 *
 * @return
 */
int get_max_members(){
    if(NOF.ELEM3 > 0){
        NOF.ELEM3--;
        return 4;
    }else if(NOF.ELEM2 > 0){
        NOF.ELEM2--;
        return 3;
    }else if(NOF.ELEM1 > 0){
        NOF.ELEM1--;
        return 2;
    }else{
        return 1;
    }
}