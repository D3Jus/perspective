//
// Created by jus on 11.6.2018.
//
#ifndef PERSPECTIVE_PERSPECTIVE_H
#define PERSPECTIVE_PERSPECTIVE_H

#include "Line.hpp"

namespace pp {
    extern cv::Point vanishingPoint;

    void reset();

    cv::Mat prepareFrame(cv::Mat input);

    std::vector<Line> findLineSegments(cv::Mat *input);

    std::vector<Line> mergeLineSegments(std::vector<Line> input, bool leftSide);

    void estimateVanishingPoint(std::vector<Line> &input);

    std::vector<Line> findVanishingLines(cv::Point2f vanishingPoint, std::vector<Line> lines);

    cv::Point getVanigshingPoint();
}

#endif //PERSPECTIVE_PERSPECTIVE_H
