//
// Created by jus on 10.7.2018.
//

#ifndef PERSPECTIVE_LINE_H
#define PERSPECTIVE_LINE_H

class Line {
private:
    cv::Point2f point1;

    cv::Point2f point2;
    // slope
    float m;
    // y-intercept
    float b;
    // angle of line
    float angle;
public:

    Line(cv::Point point1, cv::Point point2);

    Line(cv::Point point, float angle);

    cv::Point getPoint1();

    cv::Point getPoint2();

    float getY(float x);

    float getM();

    float getAngle();

    int getB();

    float getSegmentLength();

    bool isPointOnLineSegment(cv::Point point);
};


#endif //PERSPECTIVE_LINE_H
