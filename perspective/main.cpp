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
    pp::reset();

    for(;;)
    {
        Mat frame;
        video >> frame;
        if(frame.empty())
            break;

        resize(frame, frame, cv::Size(FRAME_W, FRAME_H), 0, 0, cv::INTER_AREA);

        Mat segmentsFrame = frame.clone();
        Mat linesFrame;
        linesFrame = pp::prepareFrame(frame);
        std::vector<pp::Line> lineSegments = pp::findLineSegments(&linesFrame);


printf("Size: %d \n", lineSegments.size());
        if(lineSegments.empty()) continue;

        for(auto segment : lineSegments) {
            cv::line(segmentsFrame, segment.getPoint1(), segment.getPoint2(), Scalar(0, 0, 255), 1, CV_AA);
        }



        pp::estimateVanishingPoint(lineSegments);
        std::vector<pp::Line> vanishingLines = pp::findVanishingLines(pp::vanishingPoint, lineSegments);

        Mat segments(cv::Size(FRAME_W, FRAME_H), CV_8UC1, Scalar(0));
        for(auto segment : lineSegments) {
            cv::line(segments, segment.getPoint1(), segment.getPoint2(), Scalar(255), 1);
        }


        for(auto &line : vanishingLines) {

            if(line.getPoint1().x <= pp::vanishingPoint.x) {
                cv::line(frame, Point(0, line.getY(0)), Point(pp::vanishingPoint.x, line.getY(pp::vanishingPoint.x)), Scalar(0, 0, 255), 1, CV_AA);
            } else {
                cv::line(frame, Point(pp::vanishingPoint.x, line.getY(pp::vanishingPoint.x)), Point(640, line.getY(640)), Scalar(0, 0, 255), 1, CV_AA);
            }
           // cv::line(frame, Point(0, line.getY(0)), Point(640, line.getY(640)), Scalar(0, 0, 255), 1, CV_AA);
        }



        circle(frame, pp::vanishingPoint, 15, Scalar(255, 0, 0), 6);



        imshow("Lines", frame);
        imshow("Segments", segmentsFrame);

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