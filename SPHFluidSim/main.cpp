#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iterator>

#include "SPH.h"

int main()
{
	/*std::cout << "SPH Parameters or enter default\n"
		"(<number of frames> <particle size> <reference density> <bulk modulus> <viscosity> <gravity strength>):\n";
	std::string line;
	std::getline(std::cin, line);
	std::istringstream iss(line);
	std::vector<std::string> tokens { std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{} };*/

	SPH* sph = new SPH();
	/*if (tokens[0] != "default")
	{
		if (tokens.size() == 8)
		{
			sph->nFrames = std::stoi(tokens[0]);
			sph->npFrames = std::stoi(tokens[1]);
			sph->h = std::stof(tokens[2]);
			sph->dt = std::stof(tokens[3]);
			sph->rho0 = std::stof(tokens[4]);
			sph->k = std::stof(tokens[5]);
			sph->mu = std::stof(tokens[6]);
			sph->g = std::stof(tokens[7]);
		}
		else
			std::cout << "Invalid parameters.\n";
	}

	std::cout << "Running SPH with:" << std::endl <<
		"Number of frames:          " << std::to_string(sph->nFrames) << std::endl <<
		"Number of steps per frame: " << std::to_string(sph->npFrames) << std::endl <<
		"Particle size:             " << std::to_string(sph->h) << std::endl <<
		"Time step:                 " << std::to_string(sph->dt) << std::endl <<
		"Reference density:         " << std::to_string(sph->rho0) << std::endl <<
		"Bulk modulus:              " << std::to_string(sph->k) << std::endl <<
		"Viscosity:                 " << std::to_string(sph->mu) << std::endl <<
		"Gravity strength:          " << std::to_string(sph->g) << std::endl;*/

	// Run SPH with the given parameters
	sph->start();
	sph->cleanup();
	delete sph;

	printf("\n");
	system("pause");

	return 0;
}