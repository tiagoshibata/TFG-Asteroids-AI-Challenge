#pragma once

namespace Control {
    class PD {
        public:
            PD(double newP, double newD);
            double operator()(double error, double angularSpeed);

        private:
            double p, d;
        };
}
