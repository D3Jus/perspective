#include <jni.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include "../../../../perspective/lib/perspective.h"

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

    cv::Point vp;

    if (!pp::vanishingPoint.get(vp)) {
        return;
    }

    std::vector<pp::Line> vanishingLines = pp::findVanishingLines(vp, lines);
    cv::Scalar mainColor(58, 57, 142);

    for(auto line : vanishingLines) {
        if(line.getPoint1().x <= vp.x) {
            cv::line(inputFrame, toSize(cv::Point(0, line.getY(0))), toSize(vp), mainColor, toIntSize(2), CV_AA);
        } else {
            cv::line(inputFrame, toSize(vp), toSize(cv::Point(640, line.getY(640))), mainColor, toIntSize(2), CV_AA);
        }
    }

    // vertical line
    cv::line(inputFrame,
             toSize(cv::Point(vp.x, 0)),
             toSize(cv::Point(vp.x, inputFrame.cols)),
             cv::Scalar(255, 255, 255), toIntSize(2), CV_AA);
    // horizontal line
    cv::line(inputFrame,
             toSize(cv::Point(0, vp.y)),
             toSize(cv::Point(inputFrame.rows, vp.y)),
             cv::Scalar(167, 180, 94), toIntSize(2), CV_AA);

}
}