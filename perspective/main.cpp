#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "lib/perspective.hpp"

#define FRAME_W 640
#define FRAME_H 360

using namespace cv;

const String keys =
        "{ h help |      | Print this message }"
        "{ p path |      | Input file path }"
        ;

int main(int argc, char** argv)
{
    // parse commandline args
    CommandLineParser parser(argc, argv, keys);

    // show help
    if (parser.has("h"))
    {
        parser.printMessage();

        return 0;
    }

    // check if path is defined
    if (!parser.has("p"))
    {
        std::cout << "Path not defined!" << std::endl;

        return 0;
    }

    String path = parser.get<String>("p");

    VideoCapture video(path);
    reset();

    for(;;)
    {
        Mat frame;
        video >> frame;
        if(frame.empty())
            break;

        resize(frame, frame, cv::Size(FRAME_W, FRAME_H), 0, 0, cv::INTER_AREA);

        Mat segmentsFrame = frame.clone();
        Mat linesFrame;
        linesFrame = prepareFrame(frame);
        std::vector<Line> lineSegments = findLineSegments(&linesFrame);




        for(auto segment : lineSegments) {
            cv::line(segmentsFrame, segment.getPoint1(), segment.getPoint2(), Scalar(0, 0, 255), 1, CV_AA);
        }

        if(lineSegments.empty()) continue;

        estimateVanishingPoint(lineSegments);
        std::vector<Line> vanishingLines = findVanishingLines(vanishingPoint, lineSegments);

        Mat segments(cv::Size(FRAME_W, FRAME_H), CV_8UC1, Scalar(0));
        for(auto segment : lineSegments) {
            cv::line(segments, segment.getPoint1(), segment.getPoint2(), Scalar(255), 1);
        }


        for(auto &line : vanishingLines) {

            if(line.getPoint1().x <= vanishingPoint.x) {
                cv::line(frame, Point(0, line.getY(0)), Point(vanishingPoint.x, line.getY(vanishingPoint.x)), Scalar(0, 0, 255), 1, CV_AA);
            } else {
                cv::line(frame, Point(vanishingPoint.x, line.getY(vanishingPoint.x)), Point(640, line.getY(640)), Scalar(0, 0, 255), 1, CV_AA);
            }
           // cv::line(frame, Point(0, line.getY(0)), Point(640, line.getY(640)), Scalar(0, 0, 255), 1, CV_AA);
        }

        for(auto line : vanishingLines) {
            cv::line(frame, line.getPoint1(), line.getPoint2(), Scalar(0, 255, 0), 1, CV_AA);
        }

        circle(frame, vanishingPoint, 15, Scalar(255, 0, 0), 6);
        Line hl = getHorizonLine(vanishingPoint);

        cv::line(frame, Point(0, hl.getY(0)), Point(640, hl.getY(640)), Scalar(255, 0, 0), 1, CV_AA);

        imshow("Lines", frame);
        imshow("Segments", segments);

        char key = cv::waitKey(10);
        if(key == 'p') {
            while (cv::waitKey(10) != 'p');
        }
        if(key == ' ') {
            break;
        }


    }

    return 0;
}