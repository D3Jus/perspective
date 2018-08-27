#include <jni.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include "../../../../perspective/lib/perspective.hpp"

extern "C"
{
// WO
float widthRatio;
float heightRatio;
cv::Point toSize(cv::Point point) {
    return cv::Point(point.x * widthRatio, point.y * heightRatio);
}

void JNICALL Java_si_fri_jus_perspective_MainActivity_reset(JNIEnv *env, jobject instance) {
    reset();
};

void JNICALL Java_si_fri_jus_perspective_MainActivity_perspective(JNIEnv *env, jobject instance,
                                                                      jlong matAddr) {
    cv::Mat &inputFrame = *(cv::Mat *) matAddr;

    cv::Mat frame;
    frame = prepareFrame(inputFrame);

    widthRatio = float(inputFrame.cols) / frame.cols;
    heightRatio = float(inputFrame.rows) / frame.rows;

    std::vector<Line> lines = findLineSegments(&frame);

    if(lines.empty()) {
        return;
    }

    estimateVanishingPoint(lines);

    std::vector<Line> vanishingLines = findVanishingLines(vanishingPoint, lines);

    for(auto line : vanishingLines) {
        if(line.getPoint1().x <= vanishingPoint.x) {
            cv::line(inputFrame, toSize(cv::Point(0, line.getY(0))), toSize(vanishingPoint), cv::Scalar(0, 0, 255), 1, CV_AA);
        } else {
            cv::line(inputFrame, toSize(vanishingPoint), toSize(cv::Point(640, line.getY(640))), cv::Scalar(0, 0, 255), 1, CV_AA);
        }
    }

    for(auto line : lines) {
        cv::line(inputFrame, toSize(line.getPoint1()), toSize(line.getPoint2()), cv::Scalar(0, 255, 0), 1, CV_AA);
    }

    cv::circle(inputFrame, toSize(vanishingPoint), 15, cv::Scalar(255, 0, 0), 6);
    Line hl = getHorizonLine(vanishingPoint);

    cv::line(inputFrame, toSize(cv::Point(0, hl.getY(0))), toSize(cv::Point(640, hl.getY(640))), cv::Scalar(255, 0, 0), 1, CV_AA);
}
}