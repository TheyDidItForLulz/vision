#include"opencv2/core.hpp"
#include"opencv2/imgproc.hpp"
#include"opencv2/imgcodecs.hpp"
#include"opencv2/highgui.hpp"

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string>
#include<vector>
#include<fstream>
#include<utility>

//
//	Mat.ptr -> y ; m1[n] -> x
//

double find_cosine(cv::Point pt1, cv::Point pt0, cv::Point pt2)
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1 * dx2 + dy1 * dy2) / sqrt((dx1 * dx1 + dy1 * dy1) * (dx2 * dx2 + dy2 * dy2));
}

void init()
{
	cv::namedWindow("TEST");
	cv::namedWindow("TEST2");
	cv::namedWindow("TEST3");
}

void clean()
{
	cv::destroyAllWindows();
}

void find_maxlen(cv::Mat gray, int& maxlen, int& r, int& c)
{
	for(int i = 0; i < gray.rows; i++)
	{
		unsigned char* mat = gray.ptr(i);
		int len = 0;
		for(int j = 0; j < gray.cols; j++)
		{
			if((int)mat[j] < 25)
			{
				len++;
			}
			else
			{
				if(len > maxlen)
				{
					maxlen = len;
					r = i;
					c = j;
				}
				len = 0;
			}
		}
	}
//	printf("maxlen: %i\n", maxlen);
	c -= maxlen / 3;
}
// mat1.at<Point3i>(...).x = mat2.at<uint8_t>(...);
void find_width(cv::Mat gray, int& width, int r, int c)
{
	while(1)
	{
		unsigned char* mat = gray.ptr(r);
		if(mat[c] < 200)
		{
			width++;
			r++;
		}
		else break;
	}
//	printf("width: %i\n", width);
}

void draw_circles(std::vector<std::vector<cv::Point> > contours, cv::Mat& thresh, std::vector<std::pair<cv::Point, int> >& areas)
{
	for(int i = 0; i < contours.size(); i++)
	{
		if(contours[i].size() > 1)
		{
			cv::Point pmin = cv::Point(100500, 100500), pmax = cv::Point(-100500, -100500);
			for(int j = 0; j < contours[i].size(); j++)
			{
				if(contours[i][j].x < pmin.x) pmin.x = contours[i][j].x;
				else if(contours[i][j].x > pmax.x) pmax.x = contours[i][j].x;
				if(contours[i][j].y < pmin.y) pmin.y = contours[i][j].y;
				else if(contours[i][j].y > pmax.y) pmax.y = contours[i][j].y;
			}
			int diameter = sqrt((pmax.x - pmin.x) * (pmax.x - pmin.x) + (pmax.y - pmin.y) * (pmax.y - pmin.y));
			cv::Point center;
			center.x = (pmin.x + pmax.x) / 2;
			center.y = (pmin.y + pmax.y) / 2;
			cv::circle(thresh, center, diameter / 2, (128, 128, 128), 1);
//			printf("cx: %i cy: %i d: %i\n", center.x, center.y, diameter);
			areas.push_back({center, diameter});
		}
	}
}

bool find_rect(cv::Mat area)
{
	cv::imshow("test", area);
	return true;
}

void find_shapes(std::vector<std::pair<cv::Point, int> > areas, cv::Mat gray)
{
	for(int i = 0; i < areas.size(); i++)
	{
		cv::Mat area(areas[i].second, areas[i].second, CV_8U);
		for(int j = 0; j < area.rows; j++)
		{
			for(int k = 0; k < area.cols; k++)
			{
				area.at<uint8_t>(j, k) = gray.at<uint8_t>(j + areas[i].first.y - areas[i].second / 2, 
						k + areas[i].first.x - areas[i].second / 2);
			}
		}
		if(find_rect(area))
		{
			printf("Chleb found\n");
		}
	}
}

int main(int argc, char** argv)
{
	init();
	cv::Mat image = cv::imread(argv[1]);
	cv::Mat gray;
	cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
	int maxlen = -1, width = 0, r, c;
	find_maxlen(gray, maxlen, r, c);
	find_width(gray, width, r, c);
	int magic, thresh_magic = 200;
	switch(width)
	{
		case 1:
			magic = 11;
			break;
		case 2:
			magic = 25;
			thresh_magic += 5;
			break;
		case 3:
			magic = 39; // 37 - 41
			break;
		case 4:
			magic = 58; // 51 - 65
			break;
		case 5:
			magic = 90; // 81 - 151
			break;
		case 6:
			magic = 109; // 101 - 115
			break;
		default:
			magic = 110;
			thresh_magic -= 10 * (width - 6);
			break;
	}
	std::vector<std::vector<cv::Point> > contours;

	cv::Mat blur;
	if(magic % 2 == 1)
	{
		cv::GaussianBlur(gray, blur, cv::Size(magic, magic), 0);
	}
	else
	{
		cv::GaussianBlur(gray, blur, cv::Size(magic + 1, magic + 1), 0);
	}
	cv::Mat thresh;
	cv::threshold(blur, thresh, thresh_magic, 255, cv::THRESH_BINARY_INV);
	
	cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	
	std::vector<std::pair<cv::Point, int> > areas;

	draw_circles(contours, thresh, areas);
	
	find_shapes(areas, gray);

	cv::imshow("TEST", gray);
	cv::imshow("TEST2", blur);
	cv::imshow("TEST3", thresh);

	char ch = (char)cv::waitKey();
	clean();
	return 0;
}
