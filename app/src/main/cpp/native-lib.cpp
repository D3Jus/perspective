#include <jni.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include "../../../../perspective/lib/perspective.hpp"

extern "C"
{

float widthRatio;
float heightRatio;
cv::Point toSize(cv::Point point) {
    return cv::Point(point.x * widthRatio, point.y * heightRatio);
}
int toIntSize(int size) {
    return (int) (size * widthRatio);
}
void JNICALL Java_si_fri_jus_perspective_MainActivity_reset(JNIEnv *env, jobject instance) {
    pp::reset();
};

void JNICALL Java_si_fri_jus_perspective_MainActivity_perspective(JNIEnv *env, jobject instance,
                                                                      jlong matAddr) {
    cv::Mat &inputFrame = *(cv::Mat *) matAddr;

    cv::Mat frame;
    frame = pp::prepareFrame(inputFrame);

    widthRatio = float(inputFrame.cols) / frame.cols;
    heightRatio = float(inputFrame.rows) / frame.rows;

    std::vector<pp::Line> lines = pp::findLineSegments(&frame);

    if(lines.empty()) {
        return;
    }

    pp::estimateVanishingPoint(lines);

    std::vector<pp::Line> vanishingLines = pp::findVanishingLines(pp::vanishingPoint, lines);
    cv::Scalar mainColor(58, 57, 142);

    for(auto line : vanishingLines) {
        if(line.getPoint1().x <= pp::vanishingPoint.x) {
            cv::line(inputFrame, toSize(cv::Point(0, line.getY(0))), toSize(pp::vanishingPoint), mainColor, toIntSize(2), CV_AA);
        } else {
            cv::line(inputFrame, toSize(pp::vanishingPoint), toSize(cv::Point(640, line.getY(640))), mainColor, toIntSize(2), CV_AA);
        }
    }

    // vertical line
    cv::line(inputFrame,
             toSize(cv::Point(pp::vanishingPoint.x, 0)),
             toSize(cv::Point(pp::vanishingPoint.x, inputFrame.cols)),
             cv::Scalar(255, 255, 255), toIntSize(2), CV_AA);
    // horizontal line
    cv::line(inputFrame,
             toSize(cv::Point(0, pp::vanishingPoint.y)),
             toSize(cv::Point(inputFrame.rows, pp::vanishingPoint.y)),
             cv::Scalar(167, 180, 94), toIntSize(2), CV_AA);
    // vanishing point
    circle(inputFrame, toSize(pp::vanishingPoint), toIntSize(15), mainColor, cv::FILLED);

}
}