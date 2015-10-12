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

	Collision::Collision(const Eigen::Vector2d& object, const Eigen::Vector2d& speed, const double radius, const double influence, const int lifetime) {
		Eigen::Vector2d zero = Eigen::Vector2d::Zero();
		if (object.dot(speed) >= 0. || object.squaredNorm() > influence * influence)
			*this << zero;
		else {
			Eigen::ParametrizedLine<double, 2> line(object, speed.normalized());
			Eigen::Vector2d projection = line.projection(zero);
            // Eigen::Vector2d perpendicular;
            // perpendicular << -speed[1], speed[0];
            // if (perpendicular.dot(object) < 0)
            //    perpendicular = -perpendicular;

			double radiusSubtract = radius * radius - projection.squaredNorm();
			if (radiusSubtract < 0.)
				radiusSubtract = 0.;
			else
				radiusSubtract = std::sqrt(radiusSubtract);
			double ticsToCollide = ((projection - object).norm() - radiusSubtract) / speed.norm();
			// double ticsToCollide = ((projection - object).norm()) / speed.norm();
			if (ticsToCollide < 0.)
				ticsToCollide = 0.;
			// std::cerr << "Pos " << object[0] << ' ' << object[1] << '\n';
			// std::cerr << "Vel " << speed[0] << ' ' << speed[1] << '\n';
			// std::cerr << "Projection " << projection[0] << ' ' << projection[1] << '\n';
			// std::cerr << "tics = " << ticsToCollide << "\n";
			// std::cerr << ticsToCollide << std::endl;
			// std::cerr << projection.squaredNorm() << ' ' << 40. * ticsToCollide * ticsToCollide << '\n';
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
