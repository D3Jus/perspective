#include <opencv2/imgproc/imgproc.hpp>
// DEBUG
#include <opencv2/highgui/highgui.hpp>
// DEBUG
#include "perspective.hpp"
#include "Line.hpp"

#define FRAME_W 640
#define FRAME_H 360

cv::Point vanishingPoint = cv::Point(FRAME_W / 2, FRAME_H / 2);

void reset() {
    vanishingPoint = cv::Point(FRAME_W / 2, FRAME_H / 2);
}

/**
 * Resizes and converts color of input frame
 * @param input Frame
 * @return Prepared frame
 */
cv::Mat prepareFrame(cv::Mat input) {

    cv::Mat output;

    resize(input, output, cv::Size(FRAME_W, FRAME_H), 0, 0, cv::INTER_AREA);
    cvtColor(output, output, CV_RGB2GRAY);

    return output;
}

/**
 * Finds line segments, which could lead to vanishing point
 * @param input Frame
 * @return Vector of found lines
 */
std::vector<Line> findLineSegments(cv::Mat *input) {
    cv::Ptr<cv::LineSegmentDetector> ls = cv::createLineSegmentDetector(cv::LSD_REFINE_NONE);
    std::vector<cv::Vec4i> lineSegments;
    std::vector<float> horizonAngles;
    std::vector<Line> output;

    cv::Rect quadrant1(vanishingPoint, cv::Point(FRAME_W, 0));
    cv::Rect quadrant2(vanishingPoint, cv::Point(0,0));
    cv::Rect quadrant3(vanishingPoint, cv::Point(0, FRAME_H));
    cv::Rect quadrant4(vanishingPoint, cv::Point(FRAME_W, FRAME_H));

    ls->detect(*input, lineSegments);

    for(auto segment : lineSegments) {
        cv::Point point(segment[0], segment[1]);
        Line line(point, cv::Point(segment[2], segment[3]));

        // use only segments with appropriate angle
        if(((point.inside(quadrant2) || point.inside(quadrant4)) && line.getAngle() > 0.2 && line.getAngle() < 1.2) ||
                (point.inside(quadrant1) || point.inside(quadrant3)) && line.getAngle() < -0.2 && line.getAngle() > -1.2)
        {
             output.push_back(line);
        }
    }

    cv::Mat segments(cv::Size(FRAME_W, FRAME_H), CV_8UC3, cv::Scalar(0,0,0));

    // DEBUG
    /*for(size_t i = 0; i < output.size() - 1; i++) {
        Line line1 = output[i];
        int b = rand()%256,
                g=rand()%256,
                r=rand()%256;
        for(size_t j = i + 1; j < output.size(); j++) {
            Line line2 = output[j];

            if(std::abs(line1.getAngle() - line2.getAngle()) <= 0.01 &&
               std::abs(line1.getB() - line2.getB()) <= 0.5) {
                cv::line(segments, line1.getPoint1(), line1.getPoint2(), cv::Scalar(b,g,r), 1);
                cv::line(segments, line2.getPoint1(), line2.getPoint2(), cv::Scalar(b,g,r), 1);
            }

        }
    }
    imshow("Same segments", segments);*/
    // DEBUG

    return output;
}

/**
 * Estimates vanishing point
 * @param input Line segments
 */
void estimateVanishingPoint(std::vector<Line> &input) {

    std::vector<cv::Point2f> mbCoordinates;
    cv::Vec4f output;

    for(auto line : input) {
        mbCoordinates.emplace_back(line.getM(), line.getB());
    }

    cv::fitLine(mbCoordinates, output, CV_DIST_L1, 0, 0.01, 0.01);

    Line outputLine(cv::Point2f(0,(-output[2]*output[1]/output[0])+output[3]),
                    cv::Point2f(100,((100 -output[2])*output[1]/output[0])+output[3]));

    cv::Point newVanishingPoint = cv::Point2f(outputLine.getM() * -1, outputLine.getB());

    vanishingPoint = newVanishingPoint;
}

std::vector<Line> findVanishingLines(cv::Point2f vanishingPoint, std::vector<Line> lines) {

    const float delta = 15;

    std::vector<Line> output;

    for(auto line : lines) {
        if(std::fabs(line.getY(vanishingPoint.x) - vanishingPoint.y) < delta)  {
            output.push_back(line);
        }
    }

    return output;

}

Line getHorizonLine(cv::Point2f vanishingPoint) {
    return {vanishingPoint, 0};
}