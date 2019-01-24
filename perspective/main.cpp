#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "lib/perspective.h"

#define FRAME_W 640
#define FRAME_H 360

using namespace cv;

const String keys =
        "{ h help |      | Print this message }"
        "{ p path |      | Input file path }"
        "{ w write |      | Write video to file }"
        ;


float widthRatio;
float heightRatio;

cv::Point toSize(cv::Point point) {
    return cv::Point(point.x * widthRatio, point.y * heightRatio);
}
int toIntSize(int size) {
    return (int) (size * widthRatio);
}

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

    // writer
    int fcc = CV_FOURCC('M', 'J', 'P', 'G');
    cv::Size frameSize(video.get(CV_CAP_PROP_FRAME_WIDTH), video.get(CV_CAP_PROP_FRAME_HEIGHT));
    widthRatio = float(frameSize.width) / FRAME_W;
    heightRatio = float(frameSize.height) / FRAME_H;

    VideoWriter writer;

    if(parser.has("w")) {
        writer.open("./out.avi", fcc, video.get(cv::CAP_PROP_FPS), frameSize);

    }

    Scalar mainColor(142, 57, 58);
    pp::reset();

    for(;;)
    {
        Mat frame;
        video >> frame;
        if(frame.empty())
            break;

        //resize(frame, frame, cv::Size(FRAME_W, FRAME_H), 0, 0, cv::INTER_AREA);

        Mat segmentsFrame = frame.clone();
        Mat linesFrame;
        linesFrame = pp::prepareFrame(frame);
        std::vector<pp::Line> lineSegments = pp::findLineSegments(&linesFrame);


        if(lineSegments.empty()) continue;

        for(auto segment : lineSegments) {
            cv::line(segmentsFrame, segment.getPoint1(), segment.getPoint2(), Scalar(0, 0, 255), 1, CV_AA);
        }

        pp::estimateVanishingPoint(lineSegments);


        Mat segments(cv::Size(FRAME_W, FRAME_H), CV_8UC1, Scalar(0));
        for(auto segment : lineSegments) {
            cv::line(segments, segment.getPoint1(), segment.getPoint2(), Scalar(255), 1);
        }

        Point vanishingPoint;

        if (!pp::vanishingPoint.get(vanishingPoint)) {
            continue;
        }

        std::vector<pp::Line> vanishingLines = pp::findVanishingLines(vanishingPoint, lineSegments);

        for(auto line : vanishingLines) {
            if(line.getPoint1().x <= vanishingPoint.x) {
                cv::line(frame, toSize(cv::Point(0, line.getY(0))), toSize(vanishingPoint), mainColor, toIntSize(2), CV_AA);
            } else {
                cv::line(frame, toSize(vanishingPoint), toSize(cv::Point(640, line.getY(640))), mainColor, toIntSize(2), CV_AA);
            }
        }

        // vertical line
        cv::line(frame,
                 toSize(cv::Point(vanishingPoint.x, 0)),
                 toSize(cv::Point(vanishingPoint.x, frame.cols)),
                 cv::Scalar(255, 255, 255), toIntSize(2), CV_AA);
        // horizontal line
        cv::line(frame,
                 toSize(cv::Point(0, vanishingPoint.y)),
                 toSize(cv::Point(frame.rows, vanishingPoint.y)),
                 cv::Scalar(94, 180, 167), toIntSize(2), CV_AA);
        // vanishing point
        //circle(frame, toSize(vanishingPoint), toIntSize(15), mainColor, cv::FILLED);
        //circle(frame, toSize(pp::vanishingPoint), toIntSize(15),  Scalar(0, 0, 255), cv::FILLED);


        if(writer.isOpened()) {
            writer.write(frame);
        } else {
            Mat show;
            resize(frame, show, cv::Size(FRAME_W, FRAME_H), 0, 0, cv::INTER_AREA);
            imshow("Lines", show);
            //imshow("Segments", segmentsFrame);

            char key = cv::waitKey(10);
            if (key == 'p') {
                while (cv::waitKey(10) != 'p');
            }
            if (key == ' ') {
                break;
            }

        }
    }

    writer.release();

    return 0;
}