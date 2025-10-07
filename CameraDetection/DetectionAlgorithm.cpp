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
		// ��ͼ���BGRת����HSV��ɫ�ռ�
		cv::Mat hsvImage;
		cv::cvtColor(image, hsvImage, cv::COLOR_BGR2HSV);

		// ���嵭��ɫ��HSV��Χ
		//cv::Scalar lower_green(50, 100, 100);
		//cv::Scalar upper_green(70, 255, 255);

		// ������Ĥ
		cv::Mat mask;
		cv::inRange(hsvImage, hsvLowerRange, hsvUpperRage, mask);

		// ��̬ѧ����ȥ������
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
		cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);

		// �����Ĥ�е�����
		std::vector<std::vector<cv::Point>> contours;
		cv::findContours(mask.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		// ��������
		bool isGlitchy = false;
		for (const auto& contour : contours) {
			double area = cv::contourArea(contour);
			if (getArea) {
				*getArea = area;
			}
			if (area > maxArea) { // ��������������100������Ϊͼ����
				isGlitchy = true;
				break;
			}
		}

		if (isGlitchy) {
			break;
		}
		// ������
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
