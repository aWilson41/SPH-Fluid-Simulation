#include <iostream>

#include "SPH.h"

int main()
{
	SPH* sph = new SPH();
	sph.start();
	delete sph;

	return 0;
}