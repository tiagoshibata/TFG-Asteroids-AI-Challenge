#include <cmath>
#include <vector>

#include "Params.hpp"
#include "Simulation.hpp"

namespace Simulation {
	enum CollisionObjectTypes {
		SHOOT, SHIP, ROCK, LASER, INVALID
	};

	CollisionEvent::CollisionEvent(double time, CollisionObject* a, CollisionObject* b) : myTime(time), a(a), b(b) {
		if (a != NULL)
			expectedCollisionsA = a->count;
		if (b != NULL)
			expectedCollisionsB = b->count;
	}

	bool CollisionEvent::operator<(const CollisionEvent& other) const {
		return myTime < other.myTime;
	}

	bool CollisionEvent::operator>(const CollisionEvent& other) const {
		return myTime > other.myTime;
	}

	bool CollisionEvent::valid() const {
		if (a->count != expectedCollisionsA || a->type == INVALID)
			return false;
		if (b == NULL)
			return true;
		return  b->count == expectedCollisionsB && b->type != INVALID;
	}

	void CollisionEvent::solve() {
		if (b == NULL) {
			a->vel = -a->vel.norm() * a->pos;
			return;
		}

		double impulse, massA, massB;
		switch ((int) floor(a->radius + .5)) {
		case 4:
			massA = 9.;
			break;
		case 2:
			massA = 3.;
			break;
		default:
			massA = 1.;
			break;
		}
		switch ((int) floor(b->radius + .5)) {
		case 4:
			massB = 9.;
			break;
		case 2:
			massB = 3.;
			break;
		default:
			massB = 1.;
			break;
		}
		Eigen::Vector2d deltaPos = b->pos - a->pos, unitDeltaPos = deltaPos.normalized();
		impulse = 2 * (b->vel - a->vel).dot(deltaPos) / ((a->radius + b->radius) * (massA + massB));
		a->vel += unitDeltaPos * impulse;
		b->vel -= unitDeltaPos * impulse;
	}

	CollisionObject::CollisionObject(const GameObject& objectData, int _type) {
		uid = objectData.uid;
		pos = objectData.pos;
		vel = objectData.vel;
		radius = objectData.radius;
		type = _type;
	}

	double CollisionObject::timeToHit(const CollisionObject& j) const {
		Eigen::Vector2d deltaR = pos - j.pos;
		Eigen::Vector2d deltaV = vel - j.vel;
		double deltaVDotR = deltaR.dot(deltaV);

		if (deltaVDotR >= 0)
			return -1;

		double radiusSum = radius + j.radius;
		double deltaVDotV = deltaV.dot(deltaV);
		double d = deltaVDotR * deltaVDotR - deltaVDotV * (deltaR.dot(deltaR) - radiusSum * radiusSum);
		if ((type == SHOOT && j.type == SHIP) || (type == SHIP && j.type == SHOOT)) {
			if (d < -45.)
				return -1;
			if (d < 0) {
				d = 0.;
			}
		} else if (d < 0) {
			return -1;
		}

		double deltaT = -(deltaVDotR + std::sqrt(d)) / deltaVDotV;
		if (deltaT > SIMULATION_END)
			return -1;
		return deltaT;
	}

	void Collision::predict(CollisionObject& object,
		vector<CollisionObject>::iterator it,
		const vector<CollisionObject>::iterator& end) {

		for (; it != end; ++it) {
			double deltaT = object.timeToHit(*it);
			if (deltaT > 0)
				events.push(CollisionEvent(deltaT, &object, &(*it)));
		}
		double totalRadius = gameState->arenaRadius - object.radius;
		if (totalRadius > object.pos.norm())
			return;
		double a = object.vel.squaredNorm();
		double b = 2 * (object.vel[0] * object.pos[0] + object.vel[1] * object.pos[1]);
		double c = object.pos.squaredNorm() - totalRadius * totalRadius;
		if ((b * b - 4 * a * c < 0))
			return;
		double wallCollisionTime = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
		if (wallCollisionTime < SIMULATION_END)
			events.push(CollisionEvent(wallCollisionTime, &object, NULL));
	}

	Collision::Collision(const GameObject& shoot, const GameState& state) : gameState(&state) {
		objects.emplace_back(shoot, SHOOT);
		for (auto i = state.ships.begin(); i != state.ships.end(); ++i)
			if (i->second->uid != state.myShip->uid)
				objects.emplace_back(*(i->second), SHIP);
		for (auto i = state.rocks.begin(); i != state.rocks.end(); ++i)
			objects.emplace_back(*(i->second), ROCK);
		for (auto i = state.lasers.begin(); i != state.lasers.end(); ++i) {
			if (i->second->owner != state.myShip->uid)
				objects.emplace_back(*(i->second), LASER);
		}
		for (auto i = objects.begin(); i != objects.end(); ++i)
			predict(*i, i + 1, objects.end());
		// while (!events.empty()) {
		// 	cerr << (events.top().a == NULL ? "NULL" : to_string(events.top().a->type)) << ' ' <<
		// 		(events.top().b == NULL ? "NULL" : to_string(events.top().b->type)) << ' ' << events.top().myTime << '\n';
		// 	events.pop();
		// }

		double tick = 0;
		CollisionObject *collided = NULL;
		while (!events.empty()) {
			// cerr << "Evento" << '\n';
			CollisionEvent next = events.top();
			events.pop();

			if (next.valid()) {
				// cerr << "Evento Valido" << '\n';
				if (next.a->type == SHOOT) {
					// cerr << "Evento SHOOT" << '\n';
					collided = next.b;
					next.b->pos += (next.myTime - tick) * next.b->vel;
					break;
				}
				if (next.b != NULL && next.b->type == SHOOT) {
					// cerr << "Evento SHOOT" << '\n';
					collided = next.a;
					next.a->pos += (next.myTime - tick) * next.a->vel;
					break;
				}


				switch (next.a->type) {
				case ROCK:
					if (next.b == NULL || next.b->type == ROCK) {
						double deltaT = next.myTime - tick;
						for (auto i = objects.begin(); i != objects.end(); ++i)
							i->pos += deltaT * i->vel;
						next.solve();
						tick = next.myTime;
						break;
					}
					if (next.b->type == LASER) {
						next.a->type = INVALID;
						next.b->type = INVALID;
						break;
					}
				case LASER:
					if (next.b == NULL)
						break;
					if (next.b->type == LASER || next.b->type == ROCK) {
						next.a->type = INVALID;
						next.b->type = INVALID;
					}
					break;
				case SHIP:
					break;
				}
			}
		}

		if (collided == NULL) {
			collisionExists = false;
			return;
		}
		if (collided->type == SHIP) {
			collisionExists = true;
			return;
		}
		if (gameState->arenaRadius * gameState->arenaRadius < collided->pos.squaredNorm() - 2.) {
			collisionExists = false;
		} else {
			// Rock or laser moving away = not interesting
			collisionExists = (collided->vel - state.myShip->vel).dot(collided->pos - state.myShip->pos) < 0;
		}
	}

	bool Collision::exists() const {
		return collisionExists;
	}
}
