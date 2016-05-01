#include <cmath>
#include <eigen3/Eigen/Geometry>

#include "Force.hpp"
#include "Params.hpp"

namespace Force {
	Repulsive::Repulsive(const Eigen::Vector2d& object, double mult, double influenceDistance) {
		double objectNorm = object.norm(), forceNorm;
		if (objectNorm > influenceDistance)
			*this << 0, 0;
		else {
			forceNorm = mult * (1 / objectNorm - 1 / influenceDistance) / (objectNorm * objectNorm);
			*this << -forceNorm * object / objectNorm;
		}
	}

	Attractive::Attractive(const Eigen::Vector2d& object, double mult) {
		*this << mult * object;
	}

	Collision::Collision(const Eigen::Vector2d& object, const Eigen::Vector2d& speed,
		const double radius, const double influence, const int lifetime) {

		// Returns a repulsive force only if a collision is predicted
		Eigen::Vector2d zero = Eigen::Vector2d::Zero();
		if (object.dot(speed) >= 0. || object.squaredNorm() > influence * influence)
			// Moving away or too far
			*this << zero;
		else {
			// Line of movement of object
			Eigen::ParametrizedLine<double, 2> line(object, speed.normalized());
			Eigen::Vector2d projection = line.projection(zero);
            // Eigen::Vector2d perpendicular;
            // perpendicular << -speed[1], speed[0];
            // if (perpendicular.dot(object) < 0)
            //    perpendicular = -perpendicular;

			// Contact happens before the above projection vector due to object's
			// radius. radiusSubtract calculates the distance from projection
			// where contact happens
			double radiusSubtract = radius * radius - projection.squaredNorm();
			double ticsToCollide;
			if (radiusSubtract < 0.)
				ticsToCollide = 0.;
			else {
				radiusSubtract = std::sqrt(radiusSubtract);
				ticsToCollide = ((projection - object).norm() - radiusSubtract) / speed.norm();
				if (ticsToCollide < 0.)
					ticsToCollide = 0.;
			}
			// std::cerr << "Pos " << object[0] << ' ' << object[1] << '\n';
			// std::cerr << "Vel " << speed[0] << ' ' << speed[1] << '\n';
			// std::cerr << "Projection " << projection[0] << ' ' << projection[1] << '\n';
			// std::cerr << "tics = " << ticsToCollide << "\n";
			// std::cerr << ticsToCollide << std::endl;
			// std::cerr << projection.squaredNorm() << ' ' << 40. * ticsToCollide * ticsToCollide << '\n';

			// If shoot will die before colliding, ignore
			if (ticsToCollide <= lifetime) {
				*this << 1 / std::sqrt(projection.squaredNorm() + (40. * ticsToCollide * ticsToCollide)) * -projection.normalized();
				// *this << (projection.normalized() + projection) * (1. + ticsToCollide);
				// std::cerr << ticsToCollide << '\n';
                // std::cerr << object[0] << ' ' << object[1] << '\n' <<
				//	speed[0] << ' ' << speed[1] << '\n' <<
				//	line.distance(zero) << '\n';
			} else
				*this << zero;
		}
	}
}
