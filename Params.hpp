#ifndef _PARAMS_HPP_
#define _PARAMS_HPP_

// Old parameters
// #define OMEGA_MAX		(534.378 / 180. * M_PI / 20.)	// 534.378°/s
// #define OMEGA_DERIV_MAX	(85. / 180. * M_PI / 20.)		// 85°/(s * tick)
//
// // Ku = .4
// // Tu = 9.78 / 5 / 20
// // Kp = .8 * Ku
// // Kd = Kp * Tu / 8
// #define ROTATION_P		(.8 * .4)
// #define ROTATION_D		(3.5 * (.8 * .4) * (9.78 / 5. / 20.) / 8. * 100.)
//
// #define ORIGIN_ATTRACT			3.
// #define COLLISION_TIMESTEP_MULTIPLIER	1.
//
// #define WALL_MULTIPLIER			500.
// #define WALL_INFLUENCE			50.
//
// #define ROCK_MULTIPLIER			8000.
// #define ROCK_INFLUENCE			100.
// #define ROCK_COLLISION_MULTIPLIER	1.
// #define ROCK_COLLISION_INFLUENCE	2.
//
// #define SHIP_MULTIPLIER			500.
// #define SHIP_INFLUENCE			50.
//
// #define LASER_MULTIPLIER		500.
// #define LASER_INFLUENCE			50.
// #define LASER_COLLISION_MULTIPLIER	1.
// #define LASER_COLLISION_INFLUENCE	2.
//
// #define ATTACK_THRESHOLD		3.
//
// #define SIMULATION_END	20.	// max 40 ticks (2s)


#define ROTATION_P		.2//((.8 * .4) * 1.3)
#define ROTATION_D		.123//((3.5 * (.8 * .4) * (9.78 / 5. / 20.) / 8. * 100.) * 1.8 / 20)
#define SIMULATION_END	60.
#define THRUST_PRIORITY	.3
#define SPEED_THRUST_GAIN	2.
#define ATTACK_THRESHOLD 0.

#define WALL_MULTIPLIER 80.076
#define WALL_INFLUENCE 15.8839

#define ROCK_MULTIPLIER 6.2043e+04
#define ROCK_INFLUENCE 20.
#define ROCK_COLLISION_MULTIPLIER 8.40966e+02
#define ROCK_COLLISION_INFLUENCE 60.147

#define SHIP_MULTIPLIER 2.5
#define SHIP_INFLUENCE 70.

#define LASER_MULTIPLIER 4.29935e+02
#define LASER_INFLUENCE 30.33
#define LASER_COLLISION_MULTIPLIER 3.02549e+3
#define LASER_COLLISION_INFLUENCE 80.78

#endif
