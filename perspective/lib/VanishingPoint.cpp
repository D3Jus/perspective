//
// Created by jus on 3.12.2018.
//

#include "VanishingPoint.h"

namespace pp {
    bool VanishingPoint::get(cv::Point &point) {

        if(!this->isInitialized) {
            return false;
        }

        if (!vpChanged) {
            point = memoizedVp;

            return true;
        }

        cv::Point sum;

        for (const auto &vanishingPoint : this->pointInFrame) {
            sum += vanishingPoint;
        }

        point = sum / FRAMES_SAVED;
        memoizedVp = point;

        vpChanged = false;

        return true;
    }

    void VanishingPoint::add(cv::Point point) {
        pointInFrame[i] = point;

        i = (i + 1) % FRAMES_SAVED;

        vpChanged = true;
        if (i == 0 && !this->isInitialized) {
            this->isInitialized = true;
        }
    }
}