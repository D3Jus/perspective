#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cv.hpp>
#include "lib/perspective.hpp"

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
    // -------------- kalman -----------------
    int stateSize = 6;
    int measSize = 4;
    int contrSize = 0;

    unsigned int type = CV_32F;
    cv::KalmanFilter kf(stateSize, measSize, contrSize, type);

    cv::Mat state(stateSize, 1, type);  // [x,y,v_x,v_y,w,h]
    cv::Mat meas(measSize, 1, type);    // [z_x,z_y,z_w,z_h]
    cv::setIdentity(kf.transitionMatrix);
    kf.measurementMatrix = cv::Mat::zeros(measSize, stateSize, type);
    kf.measurementMatrix.at<float>(0) = 1.0f;
    kf.measurementMatrix.at<float>(7) = 1.0f;
    kf.measurementMatrix.at<float>(16) = 1.0f;
    kf.measurementMatrix.at<float>(23) = 1.0f;
    kf.processNoiseCov.at<float>(0) = 1e-2;
    kf.processNoiseCov.at<float>(7) = 1e-2;
    kf.processNoiseCov.at<float>(14) = 5.0f;
    kf.processNoiseCov.at<float>(21) = 5.0f;
    kf.processNoiseCov.at<float>(28) = 1e-2;
    kf.processNoiseCov.at<float>(35) = 1e-2;
    cv::setIdentity(kf.measurementNoiseCov, cv::Scalar(1e-1));
    double ticks = 0;
    // -------------- !kalman -----------------
    bool first = true;
    for(;;)
    {
        double precTick = ticks;
        ticks = (double) cv::getTickCount();

        double dT = (ticks - precTick) / cv::getTickFrequency(); //seconds

        Mat frame;
        video >> frame;
        if(frame.empty())
            break;

        kf.transitionMatrix.at<float>(2) = dT;
        kf.transitionMatrix.at<float>(9) = dT;
        state = kf.predict();

        cv::Point center;
        center.x = state.at<float>(0);
        center.y = state.at<float>(1);




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
        std::vector<pp::Line> vanishingLines = pp::findVanishingLines(pp::vanishingPoint, lineSegments);


        meas.at<float>(0) = pp::vanishingPoint.x;
        meas.at<float>(1) = pp::vanishingPoint.y;
        meas.at<float>(2) = 0;
        meas.at<float>(3) = 0;

        if (first) // First detection!
        {
            // >>>> Initialization
            kf.errorCovPre.at<float>(0) = 1; // px
            kf.errorCovPre.at<float>(7) = 1; // px
            kf.errorCovPre.at<float>(14) = 1;
            kf.errorCovPre.at<float>(21) = 1;
            kf.errorCovPre.at<float>(28) = 1; // px
            kf.errorCovPre.at<float>(35) = 1; // px

            state.at<float>(0) = meas.at<float>(0);
            state.at<float>(1) = meas.at<float>(1);
            state.at<float>(2) = 0;
            state.at<float>(3) = 0;
            state.at<float>(4) = meas.at<float>(2);
            state.at<float>(5) = meas.at<float>(3);
            // <<<< Initialization

            kf.statePost = state;

            first = false;
        }
        else
            kf.correct(meas);

        // ---- draw ----
        Mat segments(cv::Size(FRAME_W, FRAME_H), CV_8UC1, Scalar(0));
        for(auto segment : lineSegments) {
            cv::line(segments, segment.getPoint1(), segment.getPoint2(), Scalar(255), 1);
        }

        for(auto line : vanishingLines) {
            if(line.getPoint1().x <= pp::vanishingPoint.x) {
                cv::line(frame, toSize(cv::Point(0, line.getY(0))), toSize(pp::vanishingPoint), mainColor, toIntSize(2), CV_AA);
            } else {
                cv::line(frame, toSize(pp::vanishingPoint), toSize(cv::Point(640, line.getY(640))), mainColor, toIntSize(2), CV_AA);
            }
        }

        // vertical line
        cv::line(frame,
                 toSize(cv::Point(pp::vanishingPoint.x, 0)),
                 toSize(cv::Point(pp::vanishingPoint.x, frame.cols)),
                 cv::Scalar(255, 255, 255), toIntSize(2), CV_AA);
        // horizontal line
        cv::line(frame,
                 toSize(cv::Point(0, pp::vanishingPoint.y)),
                 toSize(cv::Point(frame.rows, pp::vanishingPoint.y)),
                 cv::Scalar(94, 180, 167), toIntSize(2), CV_AA);
        // vanishing point
        circle(frame, toSize(pp::vanishingPoint), toIntSize(15), mainColor, cv::FILLED);
        cv::circle(frame, toSize(center), toIntSize(10), CV_RGB(255,0,0), -1);

        if(writer.isOpened()) {
            writer.write(frame);
        } else {
            imshow("Lines", frame);
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