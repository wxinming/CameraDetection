#include "DetectionAlgorithm.h"

DetectionAlgorithm::DetectionAlgorithm()
{

}

DetectionAlgorithm::~DetectionAlgorithm()
{

}

bool DetectionAlgorithm::checkImageCrash(const cv::Mat& image, 
	const cv::Scalar& hsvLowerRange, 
	const cv::Scalar& hsvUpperRage,
	int maxArea,
	int* getArea)
{
	bool result = false;
	do 
	{
		// 将图像从BGR转换到HSV颜色空间
		cv::Mat hsvImage;
		cv::cvtColor(image, hsvImage, cv::COLOR_BGR2HSV);

		// 定义淡绿色的HSV范围
		//cv::Scalar lower_green(50, 100, 100);
		//cv::Scalar upper_green(70, 255, 255);

		// 创建掩膜
		cv::Mat mask;
		cv::inRange(hsvImage, hsvLowerRange, hsvUpperRage, mask);

		// 形态学操作去除噪声
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
		cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);

		// 检测掩膜中的区域
		std::vector<std::vector<cv::Point>> contours;
		cv::findContours(mask.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		// 评估花屏
		bool isGlitchy = false;
		for (const auto& contour : contours) {
			double area = cv::contourArea(contour);
			if (getArea) {
				*getArea = area;
			}
			if (area > maxArea) { // 如果区域面积大于100，则认为图像花屏
				isGlitchy = true;
				break;
			}
		}

		if (isGlitchy) {
			break;
		}
		// 输出结果
		//if (is_glitchy) {
		//	std::cout << "The image is glitchy with light green areas." << std::endl;
		//}
		//else {
		//	std::cout << "The image is normal." << std::endl;
		//}
		result = true;
	} while (false);
	return result;
}
