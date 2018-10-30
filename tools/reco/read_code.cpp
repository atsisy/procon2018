
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <zbar.h>
#include <iostream>
#include <optional>

#define CVUI_IMPLEMENTATION
#include "cvui.h"

std::optional<std::string> ask(std::string data);
void output(std::vector<std::string> data);
cv::Mat paste(const cv::Mat &src, const cv::Mat &small, const int tx, const int ty);
std::optional<std::string> select_loop(cv::Mat &frame, std::string info);

int main(int argc, char* argv[])
{
	cv::VideoCapture cap(0);
	if (!cap.isOpened())
	{
		std::cout << "CAN'T OPEN CAMERA." << std::endl;
		return -1;
	}

	zbar::ImageScanner scanner;
	std::vector<std::string> data;
	std::optional<std::string> val;
	cv::Mat back_ground(cv::Size(640, 800), CV_8UC3, cv::Scalar(0, 0, 0));
	unsigned char *raw;

	scanner.set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);
	cv::namedWindow("QR");
	cvui::init("QR");

	start:
	while (cv::waitKey(1) != 27)
	{
		cv::Mat frame, grey;

		if (!cap.read(frame))
		{
			std::cout << "FAILD TO READ A FRAME" << std::endl;
			break;
		}

		frame = paste(back_ground, frame, 0, 0);

		cv::imshow("QR", frame);
		cv::cvtColor(frame, grey, CV_BGR2GRAY);
		cvui::update();

		raw = (unsigned char *)grey.data; 
		zbar::Image image(frame.cols, frame.rows, "Y800", raw, frame.cols * frame.rows);
		scanner.scan(image);
		
		if (image.symbol_begin() != image.symbol_end())
		{
			auto select = select_loop(frame, image.symbol_begin()->get_data());
			if (select)
			{
				data.push_back(select.value());
			}
			continue;
		}
	}

	output(data);

	return 0;

}

std::optional<std::string> select_loop(cv::Mat &frame, std::string info)
{
	cvui::text(frame, 0, 600, info, 0.2);

	cv::imshow("QR", frame);
	cvui::update();
	while (cv::waitKey(10) != 'q')
	{
		if (cvui::button(frame, 0, 500, "Get"))
		{
			return info;
		}
		if (cvui::button(frame, 100, 500, "Throw Away"))
		{
			return std::nullopt;
		}
		cv::imshow("QR", frame);
		cvui::update();
	}

	return std::nullopt;
}

cv::Mat paste(const cv::Mat &src, const cv::Mat &small, const int tx, const int ty)
{
	cv::Mat dst_img;
	src.copyTo(dst_img);
	cv::Mat mat = (cv::Mat_<double>(2, 3) << 1.0, 0.0, tx, 0.0, 1.0, ty);
	cv::warpAffine(small, dst_img, mat, dst_img.size(), CV_INTER_LINEAR, cv::BORDER_TRANSPARENT);	
	return dst_img;
}

void output(std::vector<std::string> data)
{
	for (std::string str : data)
	{
		std::cout << str << std::endl;
	}
	return;
}
