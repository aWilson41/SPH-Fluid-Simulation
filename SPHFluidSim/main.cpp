#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iterator>

#include "SPH.h"

int main()
{
	Parameters params;
	std::cout << "SPH Parameters or enter default\n"
		"(<number of frames> <particle size> <reference density> <bulk modulus> <viscosity> <gravity strength>):\n";
	std::string line;
	std::getline(std::cin, line);
	std::istringstream iss(line);
	std::vector<std::string> tokens { std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{} };

	if (tokens[0] != "default")
	{
		if (tokens.size() == 8)
		{
			params.nFrames = std::stoi(tokens[0]);
			params.npFrames = std::stoi(tokens[1]);
			params.h = std::stof(tokens[2]);
			params.dt = std::stof(tokens[3]);
			params.rho0 = std::stof(tokens[4]);
			params.k = std::stof(tokens[5]);
			params.mu = std::stof(tokens[6]);
			params.g = std::stof(tokens[7]);
		}
		else
			std::cout << "Invalid parameters.\n";
	}

	std::cout << "Running SPH with:" << std::endl <<
		"Number of frames:          " << std::to_string(params.nFrames) << std::endl <<
		"Number of steps per frame: " << std::to_string(params.npFrames) << std::endl <<
		"Particle size:             " << std::to_string(params.h) << std::endl <<
		"Time step:                 " << std::to_string(params.dt) << std::endl <<
		"Reference density:         " << std::to_string(params.rho0) << std::endl <<
		"Bulk modulus:              " << std::to_string(params.k) << std::endl <<
		"Viscosity:                 " << std::to_string(params.mu) << std::endl <<
		"Gravity strength:          " << std::to_string(params.g) << std::endl;
	SPH* sph = new SPH();
	sph->setParameters(params);
	sph->start();
	delete sph;

	return 0;
}