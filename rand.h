
#ifndef RAND_H
#define RAND_H

void rand_init_time(void);
void rand_init_seed(unsigned int s);

unsigned int rand_seed(void);

unsigned int rand_int(void);
double rand_double(void);

int rand_one_in(unsigned int num);

/* Inclusive. */
unsigned int rand_range(unsigned int lo, unsigned int hi);

#endif
