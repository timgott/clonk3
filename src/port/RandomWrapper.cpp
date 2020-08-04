#include "RandomWrapper.h"
#include <stdlib.h>
#include <time.h>

void randomize()
{
	srand(time(NULL));
}

int random(int max)
{
	if (max == 0) return 0;
	if (max < 0) return -rand() % max;
	else return rand() % max;
}
