#ifndef _FORCE_HPP_
#define _FORCE_HPP_

#include <eigen3/Eigen/Dense>

#include "bot_interface.h"

namespace Force {
	class Repulsive : public Eigen::Vector2d {
	public:
		Repulsive(const Eigen::Vector2d& object, double mult, double influenceDistance);
	};

	class Attractive : public Eigen::Vector2d {
	public:
		Attractive(const Eigen::Vector2d& object, double mult);
	};

	class Collision : public Eigen::Vector2d {
	public:
		Collision(const Eigen::Vector2d& object, const Eigen::Vector2d& speed,
			const double radius, const double influence, const int lifetime = 999999999);
	};
}

#endif
