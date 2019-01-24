//
// Created by jus on 3.12.2018.
//

#ifndef PERSPECTIVE_VANISHINGPOINT_H
#define PERSPECTIVE_VANISHINGPOINT_H

#define FRAMES_SAVED 10

#include <opencv2/core/types.hpp>

namespace pp {
    class VanishingPoint {
    private:
        cv::Point pointInFrame[FRAMES_SAVED];
        cv::Point memoizedVp;
        bool isInitialized = false;
        int i = 0;
        bool vpChanged = true;
    public:
        bool get(cv::Point &point);
        void add(cv::Point point);
    };
}


#endif //PERSPECTIVE_VANISHINGPOINT_H
