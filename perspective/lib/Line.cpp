//
// Created by jus on 10.7.2018.
//
#include <opencv2/imgproc/imgproc.hpp>
#include "Line.hpp"
namespace pp {
    Line::Line(cv::Point point1, cv::Point point2) {

        // point 1 has bigger x
        if (point1.x > point2.x) {
            this->point1 = point1;
            this->point2 = point2;
        } else {
            this->point1 = point2;
            this->point2 = point1;
        }

        int rise = point2.y - point1.y;
        int run = point2.x - point1.x;

        this->m = float(rise) / run;
        this->b = this->point1.y - this->m * this->point1.x;
        this->angle = std::atan(this->m);
    }

    cv::Point Line::getPoint1() {
        return this->point1;
    }

    cv::Point Line::getPoint2() {
        return this->point2;
    }

    float Line::getY(float x) {
        return this->m * x + this->b;
    }

    float Line::getM() {
        return this->m;
    }

    float Line::getAngle() {
        return this->angle;
    }

    int Line::getB() {
        return this->b;
    }

    float Line::getSegmentLength() {
        cv::Point2f diff = this->point1 - this->point2;

        return cv::sqrt(diff.x * diff.x + diff.y * diff.y);
    }
}
