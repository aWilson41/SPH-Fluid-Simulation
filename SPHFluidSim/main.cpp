#include <iostream>

#include "SPH.h"

int main()
{
	SPH* sph = new SPH();
	sph->start(100);
	delete sph;

	return 0;
}