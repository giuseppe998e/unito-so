#ifndef _CONFIG_H_
#define _CONFIG_H_

// Librerie
#include "../global.h"
#include <string.h>

// Constanti
#define LMAX_CHARS 32

// Funzioni
void load_config_file(struct _CONFIG *CONFIG);
int get_max_members();

#endif