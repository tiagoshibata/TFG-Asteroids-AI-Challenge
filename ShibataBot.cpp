#include <cmath>

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
	Eigen::Vector2d resultantDanger = Eigen::Vector2d::Zero();
	Eigen::Vector2d resultantMovement = Eigen::Vector2d::Zero();// Force::Attractive(-myShip->pos, ORIGIN_ATTRACT);
	Eigen::Vector2d unitPosition = myShip->pos.normalized();
	GameObject* closestShip = NULL;

	shoot = 0;

	double minDistance = 99999999999999.;
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
		resultantMovement = Force::Repulsive(unitPosition *
			(gamestate->arenaRadius - myShip->pos.norm() - myShip->radius), WALL_MULTIPLIER, WALL_INFLUENCE);
	if (closestShip != NULL) {
		Eigen::Vector2d closestShipRelPos = closestShip->pos - myShip->pos;
		if (minDistance < SHIP_INFLUENCE)
			resultantMovement += Force::Attractive(closestShipRelPos, SHIP_MULTIPLIER);
		else
			resultantMovement += Force::Attractive(closestShipRelPos.normalized() * SHIP_INFLUENCE, SHIP_MULTIPLIER);
	}

	for (auto laser : gamestate->lasers) {
		if (laser.second->owner != myShip->uid) {
			Eigen::Vector2d laserApproachSpeed, laserRelativePos;
			double radius = laser.second->radius + myShip->radius;
			laserApproachSpeed = laser.second->vel - myShip->vel;
			laserRelativePos = laser.second->pos - myShip->pos;
			resultantDanger += Force::Repulsive(laserRelativePos - laserRelativePos.normalized() * radius * .8,
				LASER_MULTIPLIER, LASER_INFLUENCE);

			Force::Collision collision(laserRelativePos, laserApproachSpeed, radius, LASER_COLLISION_INFLUENCE, laser.second->lifetime);
			resultantMovement += Force::Attractive(collision, LASER_COLLISION_MULTIPLIER);
		}
	}

	for (auto rock : gamestate->rocks) {
		Eigen::Vector2d rockApproachSpeed, rockRelativePos;
		double radius = rock.second->radius + myShip->radius;
		rockApproachSpeed = rock.second->vel - myShip->vel;
		rockRelativePos = rock.second->pos - myShip->pos;
		resultantDanger += Force::Repulsive(rockRelativePos - rockRelativePos.normalized() * radius * .8, ROCK_MULTIPLIER, ROCK_INFLUENCE);

		Force::Collision collision(rockRelativePos, rockApproachSpeed, radius, ROCK_COLLISION_INFLUENCE, 100);
		resultantMovement += Force::Attractive(collision, ROCK_COLLISION_MULTIPLIER);
	}

	resultantHeading += resultantDanger;
	resultantMovement += resultantHeading;
	resultantHeading = -resultantHeading;

	GameObject* closest = NULL;
	if (resultantDanger.squaredNorm() < ATTACK_THRESHOLD) {
		for (auto rock : gamestate->rocks) {
			double distance = (rock.second->pos - myShip->pos).squaredNorm();
			if (distance < minDistance) {
				minDistance = distance;
				closest = rock.second;
			}
		}
		for (auto laser : gamestate->lasers) {
			double distance = (laser.second->pos - myShip->pos).squaredNorm();
			if (distance < minDistance) {
				minDistance = distance;
				closest = laser.second;
			}
		}
	} else
		closest = closestShip;

	// world to bot
	Eigen::Rotation2D<double> transformation(-myShip->ang);
	Eigen::Vector2d unitDirection = {1, 0};
	Eigen::Vector2d speedDiff;
	unitDirection = Eigen::Rotation2D<double>(myShip->ang) * unitDirection;
	if (resultantMovement.squaredNorm() < 4.)
		speedDiff = transformation * (resultantMovement - .8 * myShip->vel);
	else
		speedDiff = transformation * (resultantMovement - SPEED_THRUST_GAIN * myShip->vel);
	double speedDiffAbs = abs(speedDiff[1]);
	thrust = speedDiff[0];

	if (closest != NULL) {
		Eigen::Vector3d closestD, closestV;
		Eigen::Vector2d unitClosestD;
		unitClosestD << (closest->pos - myShip->pos).normalized();
		closestD << unitClosestD, 0;
		closestV << closest->vel - myShip->vel, 0;
		Eigen::Vector3d tangentSpeed = closestD.cross(closestV);
		double shootSpeed;
		if (shootTimer >= 30)
			shootSpeed = 3.75;
		else if (shootTimer >= 20)
			shootSpeed = 2.5;
		else
			shootSpeed = 1.25;
		double straightSpeed;
		for (straightSpeed = -1.; straightSpeed < 0. && shootSpeed < 4.; shootSpeed += 1.25) {
			straightSpeed = shootSpeed * shootSpeed - tangentSpeed.dot(tangentSpeed);
		}
		if (straightSpeed < 0)
			straightSpeed = 0;
		else
			straightSpeed = std::sqrt(straightSpeed);

		resultantHeading = unitClosestD * straightSpeed +
			Eigen::Vector2d{-unitClosestD[1], unitClosestD[0]} * tangentSpeed[2];
	}

	double thrustDiff, resultantDiffHeading;
	if (resultantHeading.squaredNorm() > .2) {
		resultantDiffHeading = fmod(myShip->ang - atan2(resultantHeading[1], resultantHeading[0]), 2. * M_PI);
		if (resultantDiffHeading < -M_PI)
			resultantDiffHeading += 2 * M_PI;
		else if (resultantDiffHeading > M_PI)
			resultantDiffHeading -= 2 * M_PI;
	} else
		resultantDiffHeading = 0;

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
	double shootSpeed;
	if (shootTimer >= 30)
		shootSpeed = 75. / 20.;
	else if (shootTimer >= 20)
		shootSpeed = 50. / 20.;
	else
		shootSpeed = 25. / 20.;
	shootObject.pos = myShip->pos;
	shootObject.vel = unitDirection * shootSpeed + myShip->vel + myShip->velAng * 2 * (Eigen::Rotation2D<double>(M_PI_2) * unitDirection);
	shootObject.radius = .5;
	Simulation::Collision collision(shootObject, *gamestate);
	if (collision.exists())
		ticksFromGoodShoot = 0;

	if ((minDistance < 50. || ticksFromGoodShoot < 8) && shootTimer >= 10)
		shoot = 3;
	if (!shoot)
		shootTimer++;
	else
		shootTimer = 0;
	ticksFromGoodShoot++;
}
