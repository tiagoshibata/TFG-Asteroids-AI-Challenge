#include <cmath>
#include <iostream>

#include "Control.hpp"

namespace Control {
    PD::PD(double newP, double newD) {
        p = newP;
        d = newD;
    }

    double PD::operator()(double error, double angularSpeed) {
        return - error * p - (angularSpeed) * d;
    }
}
