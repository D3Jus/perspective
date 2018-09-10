#include <opencv2/imgproc/imgproc.hpp>
#include "perspective.hpp"
#include "Line.hpp"

#define FRAME_W 640
#define FRAME_H 360
#define MIN_SEGMENT_LENGTH 30

namespace pp {
    cv::Point vanishingPoint;

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


        cv::Rect quadrant1(vanishingPoint, cv::Point(FRAME_W, 0));
        cv::Rect quadrant2(vanishingPoint, cv::Point(0, 0));
        cv::Rect quadrant3(vanishingPoint, cv::Point(0, FRAME_H));
        cv::Rect quadrant4(vanishingPoint, cv::Point(FRAME_W, FRAME_H));

        std::vector<Line> quadrant1Lines;
        std::vector<Line> quadrant2Lines;
        std::vector<Line> quadrant3Lines;
        std::vector<Line> quadrant4Lines;

        ls->detect(*input, lineSegments);

        for (auto segment : lineSegments) {
            cv::Point point(segment[0], segment[1]);
            Line line(point, cv::Point(segment[2], segment[3]));

            if (line.getAngle() > 0.2 && line.getAngle() < 1.2) {
                if (point.inside(quadrant2)) {
                    quadrant2Lines.push_back(line);
                } else if (point.inside(quadrant4)) {
                    quadrant4Lines.push_back(line);
                }
            } else if (line.getAngle() < -0.2 && line.getAngle() > -1.2) {
                if (point.inside(quadrant1)) {
                    quadrant1Lines.push_back(line);
                } else if (point.inside(quadrant3)) {
                    quadrant3Lines.push_back(line);
                }
            }
        }

        quadrant1Lines = mergeLineSegments(quadrant1Lines, false);
        quadrant2Lines = mergeLineSegments(quadrant2Lines, true);
        quadrant3Lines = mergeLineSegments(quadrant3Lines, true);
        quadrant4Lines = mergeLineSegments(quadrant4Lines, false);

        std::vector<Line> output = quadrant1Lines;
        output.insert(output.end(), quadrant2Lines.begin(), quadrant2Lines.end());
        output.insert(output.end(), quadrant3Lines.begin(), quadrant3Lines.end());
        output.insert(output.end(), quadrant4Lines.begin(), quadrant4Lines.end());

        return output;
    }

    std::vector<Line> mergeLineSegments(std::vector<Line> input, bool leftSide) {
        if (input.empty()) {
            return input;
        }

        int d = 10;
        const cv::Point dPoint(0, d);

        std::sort(input.begin(), input.end(),
                  [](Line a, Line b) { return a.getSegmentLength() > b.getSegmentLength(); });

        int currentX = (leftSide ? 0 : FRAME_W);

        for (std::vector<Line>::size_type i = 0; i < input.size() - 1; i++) {
            std::vector<cv::Point> boundary = {
                    cv::Point(currentX, (int) input.at(i).getY(currentX)) + dPoint,
                    cv::Point(vanishingPoint.x, (int) input.at(i).getY(vanishingPoint.x)) + dPoint,
                    cv::Point(vanishingPoint.x, (int) input.at(i).getY(vanishingPoint.x)) - dPoint,
                    cv::Point(currentX, (int) input.at(i).getY(currentX)) - dPoint,
            };

            for (std::vector<Line>::size_type j = i + 1; j < input.size(); j++) {

                int isIn = (int) cv::pointPolygonTest(boundary, input.at(j).getPoint1(), false);

                if (isIn >= 0) {

                    Line lineI = input.at(i);
                    Line lineJ = input.at(j);

                    int maxX = std::max(lineI.getPoint1().x, lineJ.getPoint1().x);
                    int minX = std::min(lineI.getPoint2().x, lineJ.getPoint2().x);

                    input.at(i) = Line(cv::Point(maxX, lineI.getY(maxX)), cv::Point(minX, lineI.getY(minX)));
                    input.erase(input.begin() + j);
                    j -= 1;
                }
            }
        }


        input.erase(std::remove_if(input.begin(), input.end(), [](Line line) {
            return line.getSegmentLength() < MIN_SEGMENT_LENGTH;
        }), input.end());

        return input;
    }

    /**
     * Estimates vanishing point
     * @param input Line segments
     */
    void estimateVanishingPoint(std::vector<Line> &input) {

        std::vector<cv::Point2f> mbCoordinates;
        cv::Vec4f output;

        for (auto line : input) {
            mbCoordinates.emplace_back(line.getM(), line.getB());
        }

        cv::fitLine(mbCoordinates, output, CV_DIST_L1, 0, 0.01, 0.01);

        Line outputLine(cv::Point2f(0, (-output[2] * output[1] / output[0]) + output[3]),
                        cv::Point2f(100, ((100 - output[2]) * output[1] / output[0]) + output[3]));

        cv::Point newVanishingPoint = cv::Point2f(outputLine.getM() * -1, outputLine.getB());

        vanishingPoint = newVanishingPoint;
    }

    std::vector<Line> findVanishingLines(cv::Point2f vanishingPoint, std::vector<Line> lines) {

        const float delta = 15;

        std::vector<Line> output;

        for (auto line : lines) {
            if (std::fabs(line.getY(vanishingPoint.x) - vanishingPoint.y) < delta) {
                output.push_back(line);
            }
        }

        return output;

    }
}