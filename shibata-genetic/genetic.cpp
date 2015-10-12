#include <ga/GASimpleGA.h>
#include <ga/GABin2DecGenome.h>
#include <iostream>
#include <stdexcept>

#define len(array)	(sizeof(array) / sizeof(array[0]))

float genetic_objective(GAGenome &);

int main() {
	GABin2DecPhenotype map;

	//             COLLISION_TIMESTEP_MULTIPLIER
	float min[] = {.001,
	//  WALL_MULTIPLIER WALL_INFLUENCE ROCK_MULTIPLIER ROCK_INFLUENCE ROCK_COLLISION_MULTIPLIER
		1.,             2.,            2000.,           40.,           10000.,
	//  ROCK_COLLISION_INFLUENCE SHIP_MULTIPLIER SHIP_INFLUENCE LASER_MULTIPLIER LASER_INFLUENCE
	    40.,                     1.,             20.,           10.,             20.,
	//  LASER_COLLISION_MULTIPLIER LASER_COLLISION_INFLUENCE ATTACK_THRESHOLD
		1.,                        2.,                       .00001};

	//             COLLISION_TIMESTEP_MULTIPLIER
	float max[] = {20.,
	//  WALL_MULTIPLIER WALL_INFLUENCE ROCK_MULTIPLIER ROCK_INFLUENCE ROCK_COLLISION_MULTIPLIER
		500.,           100.,          150000000.,     300.,          150000000.,
	//  ROCK_COLLISION_INFLUENCE SHIP_MULTIPLIER SHIP_INFLUENCE LASER_MULTIPLIER LASER_INFLUENCE
		400.,                    100000000.,     600.,          150000000.,      5000.,
	//  LASER_COLLISION_MULTIPLIER LASER_COLLISION_INFLUENCE ATTACK_THRESHOLD
		150000000.,                10000.,                   .1};
	for (unsigned int i = 0; i < len(min); i++)
		map.add(32, min[i], max[i]);

	float target[len(min)];
	for (unsigned int i = 0; i < len(min); i++)
		target[i] = GARandomFloat(min[i], max[i]);

	GABin2DecGenome genome(map, genetic_objective, (void *) target);
	GASimpleGA genetic(genome);
	genetic.populationSize(5);
	genetic.nConvergence(5);
	genetic.pConvergence(.999);
	genetic.pMutation(0.01);
	genetic.pCrossover(0.6);
	genetic.scoreFilename("ship_genetic.dat");
	genetic.flushFrequency(1);
	genetic.evolve();
	return 0;
}

float genetic_objective(GAGenome& raw_genome) {
	static const std::string parameterNames[] = {
		"COLLISION_TIMESTEP_MULTIPLIER", "WALL_MULTIPLIER", "WALL_INFLUENCE",
		"ROCK_MULTIPLIER", "ROCK_INFLUENCE", "ROCK_COLLISION_MULTIPLIER", "ROCK_COLLISION_INFLUENCE",
		"SHIP_MULTIPLIER", "SHIP_INFLUENCE", "LASER_MULTIPLIER",
		"LASER_INFLUENCE", "LASER_COLLISION_MULTIPLIER", "LASER_COLLISION_INFLUENCE",
		"ATTACK_THRESHOLD"
	};
	GABin2DecGenome& genome = (GABin2DecGenome&) raw_genome;

	for (unsigned i = 0; i < len(parameterNames); i++) {
		std::cout << "#define " << parameterNames[i] << ' ' <<  genome.phenotype(i) << '\n';
	}
	std::cout << "Score: " << std::flush;

	float fitness;
	std::cin >> fitness;

	return fitness;
}
