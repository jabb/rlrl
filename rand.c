
#include "rand.h"

#include <time.h>

#include "mt19937ar.h"

static unsigned int seed = 0;

void rand_init_time(void)
{
	seed = time((unsigned)0);
	init_genrand(seed);
}

void rand_init_seed(unsigned int s)
{
	seed = s;
	init_genrand(seed);
}

unsigned int rand_seed(void)
{
	return seed;
}

unsigned int rand_int(void)
{
	return genrand_int32();
}

double rand_double(void)
{
	return genrand_real2();
}

int rand_one_in(unsigned int num)
{
	double chance;

	if (num == 0)
		return 1;

	chance = 1.0 / num;

	return rand_double() < chance ? 1 : 0;
}

unsigned int rand_range(unsigned int lo, unsigned int hi)
{
	unsigned int tmp, range;

	if (lo > hi) {
		tmp = lo;
		lo = hi;
		hi = tmp;
	}

	range = (hi - lo) + 1;
	return rand_int() % range + lo;
}
