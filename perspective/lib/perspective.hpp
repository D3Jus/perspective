//
// Created by jus on 11.6.2018.
//
#ifndef PERSPECTIVE_PERSPECTIVE_H
#define PERSPECTIVE_PERSPECTIVE_H

#include "Line.hpp"

extern cv::Point vanishingPoint;

void reset();

cv::Mat prepareFrame(cv::Mat input);

std::vector<Line> findLineSegments(cv::Mat *input);

void estimateVanishingPoint(std::vector<Line> &input);

std::vector<Line> findVanishingLines(cv::Point2f vanishingPoint, std::vector<Line> lines);

Line getHorizonLine(cv::Point2f vanishingPoint);

#endif //PERSPECTIVE_PERSPECTIVE_H
