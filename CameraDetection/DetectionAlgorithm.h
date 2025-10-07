#pragma once
#include <ImageProcess/ImageProcess.h>
class DetectionAlgorithm
{
public:
	DetectionAlgorithm();
	~DetectionAlgorithm();
	bool checkImageCrash(const cv::Mat& image,
		const cv::Scalar& hsvLowerRange,
		const cv::Scalar& hsvUpperRage,
		int maxArea,
		int* getArea = nullptr);
};

