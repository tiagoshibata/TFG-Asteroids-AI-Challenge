#ifndef _SIMULATION_HPP_
#define _SIMULATION_HPP_

// based on http://algs4.cs.princeton.edu/61event/
#include <queue>

#include "bot_interface.h"

namespace Simulation {
	class CollisionObject : public GameObject {
	public:
		int type, count = 0;

		CollisionObject(const GameObject& objectData, int _type);
		double timeToHit(const CollisionObject& i) const;
	};

	class CollisionEvent {
	public:
		double myTime;
		CollisionObject *a, *b;

		CollisionEvent(double time, CollisionObject* a, CollisionObject* b);
		bool operator<(const CollisionEvent& other) const;
		bool operator>(const CollisionEvent& other) const;
		bool valid() const;
		void solve();

	private:
		int expectedCollisionsA, expectedCollisionsB;
	};

	class Collision {
	public:
		Collision(const GameObject& shoot, const GameState& state);
		bool exists() const;

	private:
		const GameState* gameState;
		std::priority_queue<CollisionEvent, std::vector<CollisionEvent>, std::greater<CollisionEvent>> events;
		vector<CollisionObject> objects;
		bool collisionExists;

		void predict(CollisionObject& object, vector<CollisionObject>::iterator start,
			const vector<CollisionObject>::iterator& end);
	};
}

#endif
