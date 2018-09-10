#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "lib/perspective.hpp"
#include "lib/Tracker.cpp"

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
    Scalar mainColor(142, 57, 58);

    Tracker tracker;
    Point2f a;
    bool b = false;
    for(;;)
    {
        Mat frame;
        video >> frame;
        if(frame.empty())
            break;

        resize(frame, frame, cv::Size(FRAME_W, FRAME_H), 0, 0, cv::INTER_AREA);

        tracker.processImage(frame);

        Mat segmentsFrame = frame.clone();
        Mat linesFrame;
        linesFrame = pp::prepareFrame(frame);
        std::vector<pp::Line> lineSegments = pp::findLineSegments(&linesFrame);


        if(lineSegments.empty()) continue;

        for(auto segment : lineSegments) {
            cv::line(segmentsFrame, segment.getPoint1(), segment.getPoint2(), Scalar(0, 0, 255), 1, CV_AA);
        }

        pp::estimateVanishingPoint(lineSegments);


/*        // vertical line
        cv::line(frame, Point(pp::vanishingPoint.x, 0), Point(pp::vanishingPoint.x, FRAME_H), Scalar(255, 255, 255), 2, CV_AA);
        // horizontal line
        cv::line(frame, Point(0, pp::vanishingPoint.y), Point(FRAME_W, pp::vanishingPoint.y), Scalar(94, 180, 167), 2, CV_AA);
        // vanishing point
        circle(frame, pp::vanishingPoint, 15, Scalar(142, 57, 58), FILLED);*/



        Mat invTrans = tracker.rigidTransform;
        //warpAffine(frame,frame,invTrans.rowRange(0,2),Size());


        if(!b) {
            a = pp::vanishingPoint;
            b = true;
        }

        std::vector<cv::Point2f> notTrans = {a};
        std::vector<cv::Point2f> trans;
        cv::perspectiveTransform(notTrans, trans, invTrans);
        pp::vanishingPoint = trans.at(0);
        std::vector<pp::Line> vanishingLines = pp::findVanishingLines(pp::vanishingPoint, lineSegments);

        Mat segments(cv::Size(FRAME_W, FRAME_H), CV_8UC1, Scalar(0));
        for(auto segment : lineSegments) {
            cv::line(segments, segment.getPoint1(), segment.getPoint2(), Scalar(255), 1);
        }

        // main lines
        for(auto &line : vanishingLines) {
            if(line.getPoint1().x <= pp::vanishingPoint.x) {
                cv::line(frame, Point(0, line.getY(0)), Point(pp::vanishingPoint.x, line.getY(pp::vanishingPoint.x)), mainColor, 2, CV_AA);
            } else {
                cv::line(frame, Point(pp::vanishingPoint.x, line.getY(pp::vanishingPoint.x)), Point(640, line.getY(640)), mainColor, 2, CV_AA);
            }
        }

        // vertical line
        cv::line(frame, Point(trans.at(0).x, 0), Point(trans.at(0).x, FRAME_H), Scalar(255, 255, 255), 2, CV_AA);
        // horizontal line
        cv::line(frame, Point(0, trans.at(0).y), Point(FRAME_W, trans.at(0).y), Scalar(94, 180, 167), 2, CV_AA);
        // vanishing point
        circle(frame, trans.at(0), 15, Scalar(142, 57, 58), FILLED);

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