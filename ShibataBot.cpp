#include <cmath>
#include <iostream>

#include "Control.hpp"
#include "Force.hpp"
#include "Params.hpp"
#include "ShibataBot.hpp"
#include "Simulation.hpp"

ShibataBot::ShibataBot() : pd(ROTATION_P, ROTATION_D) {

}

ShibataBot::~ShibataBot() {

}

void ShibataBot::Process() {
	static int shootTimer = 10, ticksFromGoodShoot = 0;
	Eigen::Vector2d resultantHeading = Eigen::Vector2d::Zero();
	// Possibly dangerous sources we should aim (lasers, rocks). Will also
	// influence movement
	Eigen::Vector2d resultantDanger = Eigen::Vector2d::Zero();
	// Actions that cause movement (avoid walls, move away from rocks and lasers
	// that could cause collisions, approach players)
	Eigen::Vector2d resultantMovement = Eigen::Vector2d::Zero();
	Eigen::Vector2d unitPosition = myShip->pos.normalized();
	GameObject* closestShip = NULL;

	shoot = 0;

	double minDistance = 99999999999999.;
	// search closest ship
	for (auto ship : gamestate->ships) {
		if (ship.second->uid != myShip->uid) {
			double distance = (ship.second->pos - myShip->pos).squaredNorm();
			if (distance < minDistance) {
				minDistance = distance;
				closestShip = ship.second;
			}
		}
	}

	if (unitPosition.dot(myShip->vel) > -0.1)
		// approaching wall, add repulsive from wall
		resultantMovement = Force::Repulsive(unitPosition *
			(gamestate->arenaRadius - myShip->pos.norm() - myShip->radius), WALL_MULTIPLIER, WALL_INFLUENCE);

	if (closestShip != NULL) {
		Eigen::Vector2d closestShipRelPos = closestShip->pos - myShip->pos;
		// attractive to closest ship with a limit influence
		if (minDistance < SHIP_INFLUENCE)
			resultantMovement += Force::Attractive(closestShipRelPos, SHIP_MULTIPLIER);
		else
			resultantMovement += Force::Attractive(closestShipRelPos.normalized() * SHIP_INFLUENCE, SHIP_MULTIPLIER);
	}

	for (auto laser : gamestate->lasers) {
		// search hostile lasers (own lasers don't cause damage)
		if (laser.second->owner != myShip->uid) {
			Eigen::Vector2d laserApproachSpeed, laserRelativePos;
			double radius = laser.second->radius + myShip->radius;
			laserApproachSpeed = laser.second->vel - myShip->vel;
			laserRelativePos = laser.second->pos - myShip->pos;

			// discount sum of radius
			if (radius > laserRelativePos.norm()) {
				std::cerr << "radius > laserRelativePos.norm() - collision?\n";
				radius /= 2;
			}
			laserRelativePos -= laserRelativePos.normalized() * radius;

			// repulsive force from lasers
			resultantDanger += Force::Repulsive(laserRelativePos, LASER_MULTIPLIER, LASER_INFLUENCE);

			// repulsive force if a collision is predicted
			resultantMovement += LASER_COLLISION_MULTIPLIER * Force::Collision(laserRelativePos,
				laserApproachSpeed, radius, LASER_COLLISION_INFLUENCE, laser.second->lifetime);
		}
	}

	for (auto rock : gamestate->rocks) {
		Eigen::Vector2d rockApproachSpeed, rockRelativePos;
		double radius = rock.second->radius + myShip->radius;
		rockApproachSpeed = rock.second->vel - myShip->vel;
		rockRelativePos = rock.second->pos - myShip->pos;

		// discount sum of radius
		if (radius > rockRelativePos.norm()) {
			std::cerr << "radius > rockRelativePos.norm() - collision?\n";
			radius /= 2;
		}
		resultantDanger += Force::Repulsive(rockRelativePos - rockRelativePos.normalized() * radius,
			ROCK_MULTIPLIER, ROCK_INFLUENCE);

		resultantMovement += ROCK_COLLISION_MULTIPLIER * Force::Collision(rockRelativePos,
			rockApproachSpeed, radius, ROCK_COLLISION_INFLUENCE, 100);
	}

	resultantMovement += resultantDanger;
	// resultantHeading = -resultantDanger;

	GameObject* closest = NULL;
	if (resultantDanger.squaredNorm() < ATTACK_THRESHOLD || !closestShip) {
		// No danger close, attack hostile lasers and rocks
		for (auto rock : gamestate->rocks) {
			double distance = (rock.second->pos - myShip->pos).squaredNorm();
			if (distance < minDistance) {
				minDistance = distance;
				closest = rock.second;
			}
		}
		for (auto laser : gamestate->lasers) {
			if (laser.second->owner != myShip->uid) {
				double distance = (laser.second->pos - myShip->pos).squaredNorm();
				if (distance < minDistance) {
					minDistance = distance;
					closest = laser.second;
				}
			}
		}
	} else
		closest = closestShip;

	// World to bot transformation
	Eigen::Rotation2D<double> transformation(-myShip->ang);
	Eigen::Vector2d unitDirection = {1, 0};
	Eigen::Vector2d speedDiff;
	// Unit ship direction
	unitDirection = Eigen::Rotation2D<double>(myShip->ang) * unitDirection;
	// Difference between desired and actual speed in bot coordinates
	speedDiff = transformation * (resultantMovement - SPEED_THRUST_GAIN * myShip->vel);
	double speedDiffAbs = abs(speedDiff[1]);	// lateral speed
	thrust = speedDiff[0];						// forward speed

	double shootSpeed;
	if (shootTimer >= 30)
		shootSpeed = 3.75;
	else if (shootTimer >= 20)
		shootSpeed = 2.5;
	else
		shootSpeed = 1.25;

	if (closest != NULL) {

		Eigen::Vector3d closestD, closestV;
		Eigen::Vector2d unitClosestD;
		unitClosestD << (closest->pos - myShip->pos).normalized();

		// Build a 3D vector and store relative tangential speed in Z with cross product
		closestD << unitClosestD, 0;
		closestV << closest->vel - myShip->vel, 0;
		double tangentSpeed = closestD.cross(closestV)[2];	// Right hand rule = positive if counter clockwise

		// Build heading by desired shoot heading.
		// Shoot speed = target tangential speed + shoot approach speed.
		double approachSpeed;
		double possibleShootSpeed = shootSpeed;
		for (approachSpeed = 0.; approachSpeed == 0. && possibleShootSpeed < 4.; possibleShootSpeed += 1.25) {
			approachSpeed = possibleShootSpeed * possibleShootSpeed - tangentSpeed * tangentSpeed;
		}
		if (approachSpeed < 0.)
			approachSpeed = 0.;
		approachSpeed = std::sqrt(approachSpeed);

		// The loop might exit with approachSpeed = 0 if abs(tangentSpeed) > max. shootSpeed.
		// Enemy tangential speed is too fast and the shoot won't reach, so we will position perpendicular to target.

		// Eigen::Vector2d{-unitClosestD[1], unitClosestD[0]} = unit direction rotated +90°
		resultantHeading = unitClosestD * approachSpeed +
			Eigen::Vector2d{-unitClosestD[1], unitClosestD[0]} * tangentSpeed;
	}

	double thrustDiff, resultantDiffHeading;
	resultantDiffHeading = fmod(myShip->ang - atan2(resultantHeading[1], resultantHeading[0]), 2. * M_PI);
	if (resultantDiffHeading < -M_PI)
		resultantDiffHeading += 2 * M_PI;
	else if (resultantDiffHeading > M_PI)
		resultantDiffHeading -= 2 * M_PI;

	double correction = pd(resultantDiffHeading, myShip->velAng);
	thrustDiff = (myShip->velAng - correction);	// @todo Revisar, -correction parece certo mas não funciona
	double thrustDiffAbs = abs(thrustDiff);

	if (speedDiffAbs + thrustDiffAbs > 1.) {
		if (thrustDiffAbs > THRUST_PRIORITY) {
			if (speedDiffAbs > 1 - THRUST_PRIORITY) {
				speedDiffAbs = 1 - THRUST_PRIORITY;
				thrustDiffAbs = THRUST_PRIORITY;
			} else
				thrustDiffAbs = 1. - speedDiffAbs;
		} else if (speedDiffAbs > 1 - THRUST_PRIORITY) {
			speedDiffAbs = 1. - thrustDiffAbs;
		}
	}

	if (thrustDiff > 0)
		thrustDiff = thrustDiffAbs;
	else
		thrustDiff = -thrustDiffAbs;

	if (speedDiff[1] > 0)
		speedDiff[1] = speedDiffAbs;
	else
		speedDiff[1] = -speedDiffAbs;

	sideThrustFront = thrustDiff - speedDiff[1];
	sideThrustBack = -thrustDiff - speedDiff[1];

	GameObject shootObject;
	if (shootTimer >= 10) {
		// If shooting is possible, check if it's worth it
		shootObject.pos = myShip->pos;
		shootObject.vel = unitDirection * shootSpeed + myShip->vel +
			myShip->velAng * 2 * (Eigen::Rotation2D<double>(M_PI_2) * unitDirection);
		shootObject.radius = .5;

		Simulation::Collision collision(shootObject, *gamestate);
		if (collision.exists())
			ticksFromGoodShoot = 0;
		if ((minDistance < 20. || ticksFromGoodShoot < 8))
			shoot = 3;
	}
	if (!shoot)
		shootTimer++;
	else
		shootTimer = 1;
	ticksFromGoodShoot++;
}
