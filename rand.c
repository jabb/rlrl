
#include "mt19937ar.h"

static unsigned int seed = 0;

void rand_init_time(void);
void rand_init_seed(unsigned int s);

unsigned int rand_seed(void);

unsigned int rand_int(void);
