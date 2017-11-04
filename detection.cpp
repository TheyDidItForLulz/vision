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

double find_cosine(Point p1, Point p0, Point p2)
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
			int maxdiffcont = -1;
			cv::Point p1 = contours[i][0];
			cv::Point p2;
			for(int j = 1; j < contours[i].size(); j++)
			{
				cv::Point t_p2 = contours[i][j];
				int diffcont = sqrt((p1.x - t_p2.x) * (p1.x - t_p2.x) + (p1.y - t_p2.y) * (p1.y - t_p2.y));
				if(diffcont > maxdiffcont)
				{
					p2 = t_p2;
					maxdiffcont = diffcont;
				}
			}
			cv::Point center;
			center.x = (p1.x + p2.x) / 2;
			center.y = (p1.y + p2.y) / 2;
			cv::circle(thresh, center, maxdiffcont, (128, 128, 128), 1);
			areas.push_back({center, maxdiffcont});
		}
	}
}

bool find_rect(cv::Point center, int radius)
{
	
}

void find_shapes(std::vector<std::pair<cv::Point, int> > areas)
{
	for(int i = 0; i < areas.size(); i++)
	{
		if(find_rect(areas[i].first, areas[i].second))
		{
			printf("Rect found\n");
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

	cv::imshow("TEST", gray);
	cv::imshow("TEST2", blur);
	cv::imshow("TEST3", thresh);

	char ch = (char)cv::waitKey();
	clean();
	return 0;
}
