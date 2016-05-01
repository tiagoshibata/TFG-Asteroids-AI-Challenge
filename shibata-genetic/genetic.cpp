#include <ga/GASimpleGA.h>
#include <ga/GABin2DecGenome.h>
#include <fstream>
#include <iostream>
#include <stdexcept>

#define len(array)	(sizeof(array) / sizeof(array[0]))

float genetic_objective(GAGenome &);

int main() {
	GABin2DecPhenotype map;

	float min[] = {
	//	ROTATION_P ROTATION_D SIMULATION_END THRUST_PRIORITY SPEED_THRUST_GAIN
		.1,        .05,       3.,            0.,             1.,
	//  WALL_MULTIPLIER WALL_INFLUENCE ROCK_MULTIPLIER ROCK_INFLUENCE ROCK_COLLISION_MULTIPLIER
		5.,             2.,            2000.,          40.,           100.,
	//  ROCK_COLLISION_INFLUENCE SHIP_MULTIPLIER SHIP_INFLUENCE LASER_MULTIPLIER LASER_INFLUENCE
	    40.,                     1.,             20.,           10.,             20.,
	//  LASER_COLLISION_MULTIPLIER LASER_COLLISION_INFLUENCE ATTACK_THRESHOLD
		1.,                        2.,                       .00001};

	float max[] = {
	//	ROTATION_P ROTATION_D SIMULATION_END THRUST_PRIORITY SPEED_THRUST_GAIN
		1.,        .7,        20.,           1.,             2.,
	//  WALL_MULTIPLIER WALL_INFLUENCE ROCK_MULTIPLIER ROCK_INFLUENCE ROCK_COLLISION_MULTIPLIER
		400.,           100.,          15000000.,      300.,          1500000.,
	//  ROCK_COLLISION_INFLUENCE SHIP_MULTIPLIER SHIP_INFLUENCE LASER_MULTIPLIER LASER_INFLUENCE
		400.,                    1000000.,       1000.,         15000000.,       5000.,
	//  LASER_COLLISION_MULTIPLIER LASER_COLLISION_INFLUENCE ATTACK_THRESHOLD
		15000000.,                 10000.,                   1.};
	for (unsigned int i = 0; i < len(min); i++)
		map.add(32, min[i], max[i]);

	float target[len(min)];
	for (unsigned int i = 0; i < len(min); i++)
		target[i] = GARandomFloat(min[i], max[i]);

	GABin2DecGenome genome(map, genetic_objective, (void *) target);
	GASimpleGA genetic(genome);
	genetic.populationSize(10);
	genetic.nConvergence(5);
	genetic.pConvergence(.999);
	genetic.pMutation(0.01);
	genetic.pCrossover(0.6);
	genetic.scoreFilename("ship_genetic_score.txt");
	genetic.flushFrequency(1);
	genetic.evolve();
	return 0;
}

float genetic_objective(GAGenome& raw_genome) {
	static const std::string parameterNames[] = {
		"ROTATION_P", "ROTATION_D",
		"SIMULATION_END",
		"THRUST_PRIORITY", "SPEED_THRUST_GAIN",
		"WALL_MULTIPLIER", "WALL_INFLUENCE",
		"ROCK_MULTIPLIER", "ROCK_INFLUENCE", "ROCK_COLLISION_MULTIPLIER", "ROCK_COLLISION_INFLUENCE",
		"SHIP_MULTIPLIER", "SHIP_INFLUENCE",
		"LASER_MULTIPLIER", "LASER_INFLUENCE", "LASER_COLLISION_MULTIPLIER", "LASER_COLLISION_INFLUENCE",
		"ATTACK_THRESHOLD"
	};
	GABin2DecGenome& genome = (GABin2DecGenome&) raw_genome;

	std::ofstream file("../Params.hpp");
	file << "#ifndef _PARAMS_HPP_\n#define _PARAMS_HPP_\n";

	for (unsigned i = 0; i < len(parameterNames); i++) {
		file << "#define " << parameterNames[i] << ' ' <<  genome.phenotype(i) << '\n';
	}

	file << "#endif\n";
	file.close();

	std::cout << "Genetic - Enter score:\n";

	float fitness;
	std::cin >> fitness;

	return fitness;
}
