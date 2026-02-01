#pragma once

#include "../welford.h"

namespace dferone::algorithms {

    /*! @brief Filtering mechanism for the Local Search */
    class Filtering {
    public:
        /*! Constructor
         * @param q Threshold on the number of standard deviations away from the estimated mean percentage improvement
         */
        explicit Filtering(double q) :
                q_(q) {
        }

        virtual ~Filtering() {};

        /*! @brief Add an element during the warm up
         *
         * @param construction Solution cost after construction
         * @param ls Solution cost after local search
         */
        void addElement(double construction, double ls) {
            double x = (construction - ls) / construction;
            wa_.addElement(x);
        }

        /// \brief Checks if the local search must be performed
        /// \param current Current solution cost
        /// \param incumbent Incumbent solution cost
        /// \return True if the local search must be performed, false otherwise
        bool check(double current, double incumbent) const{
            double imp = (current - incumbent) / current;
            return imp < wa_.getMean() + q_ * wa_.getStdDev();
        }
    private:
        double q_;
        WelfordAlgorithm wa_;
    };

}
