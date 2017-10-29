#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

#include <iostream>
#include <math.h>
#include <string.h>

using namespace cv;
using namespace std;

static void help()
{
	printf("\tProgram for detecting shapes.\n\tAdded features:round and rectangle.\n\tDefault pictures are sample pictures stored in ../data.\n");
}

const int MIN_AREA = 100;

int thresh = 50, N = 10;
const char* wndname = "test";

// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
static double angle( Point pt1, Point pt2, Point pt0 )
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

// function to find distance between the points
static double find_dist(Point p1, Point p2)
{
	double dx = p2.x - p1.x;
	double dy = p2.y - p1.y;
	return sqrt(dx * dx + dy * dy);
}

// function to check, is given point vector actually round area.
static bool check_round(vector<Point> approx)
{
	if(approx.size() < 8) return false;
	Point center;
	center.x = (approx[0].x + approx[approx.size() / 2].x) / 2;
	center.y = (approx[0].y + approx[approx.size() / 2].y) / 2;
	double epsmax = 1.05;
	double epsmin = 0.95;
	for(int i = 0; i < approx.size() - 2; i += 2)
	{
		double ratio = find_dist(center, approx[i]) / find_dist(center, approx[i + 1]);
		if(ratio < epsmax && ratio > epsmin && find_dist(center, approx[i]) > sqrt(MIN_AREA))
		{
			continue;
		}
		else return false;
	}
	return true;
}

static void findAngles(const Mat& image, vector<vector<Point> > &angles)
{
	
}


// returns sequence of squares detected on the image.
// the sequence is stored in the specified memory storage
static void findShapes(const Mat& image, vector<vector<Point> >& shapes, vector<vector<Point> > &angles)
{
	shapes.clear();
	angles.clear();

	Mat pyr, timg, gray0(image.size(), CV_8U), gray;

	// down-scale and upscale the image to filter out the noise
	pyrDown(image, pyr, Size(image.cols/2, image.rows/2));
	pyrUp(pyr, timg, image.size());
	vector<vector<Point> > contours;

	// find shapes in every color plane of the image
	for( int c = 0; c < 3; c++ )
	{
		int ch[] = {c, 0};
		mixChannels(&timg, 1, &gray0, 1, ch, 1);

		// try several threshold levels
		for( int l = 0; l < N; l++ )
		{
			// hack: use Canny instead of zero threshold level.
			// Canny helps to catch squares with gradient shading
			if( l == 0 )
			{
				// apply Canny. Take the upper threshold from slider
				// and set the lower to 0 (which forces edges merging)
				Canny(gray0, gray, 0, thresh, 5);
				// dilate canny output to remove potential
				// holes between edge segments
				dilate(gray, gray, Mat(), Point(-1,-1));
			}
			else
			{
				// apply threshold if l!=0:
				//     tgray(x,y) = gray(x,y) < (l+1)*255/N ? 255 : 0
				gray = gray0 >= (l+1)*255/N;
			}
			
	//		findAngles(gray, angles);
			// find contours and store them all as a list
			findContours(gray, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

			vector<Point> approx;
			//			vector<Point> more_approx;
			//			vector<Point> one_more_approx;

			// test each contour
			for( size_t i = 0; i < contours.size(); i++ )
			{
				// approximate contour with accuracy proportional
				// to the contour perimeter

				approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.0002, true);

				if(approx.size() >= 3)
				{
					for(int i = 0; i < approx.size() - 3; i++)
					{
						double cosine = angle(approx[i], approx[i + 2], approx[i + 1]);

						if(fabs(cosine) < 0.3)
						{
							vector<Point> v_angle;
							v_angle.push_back(approx[i]);
							v_angle.push_back(approx[i + 1]);
							v_angle.push_back(approx[i + 2]);
							angles.push_back(v_angle);
						}
					}
				}

//			approxPolyDP(Mat(contours[i]), more_approx, arcLength(Mat(contours[i]), true)*0.002, true);
//			approxPolyDP(Mat(contours[i]), one_more_approx, arcLength(Mat(contours[i]), true)*0.09, true);
			// square contours should have 4 vertices after approximation
			// relatively large area (to filter out noisy contours)
			// and be convex.
			// Note: absolute value of an area is used because
			// area may be positive or negative - in accordance with the
			// contour orientation
//			for(int j = 0; j < less_approx.size(); j++)
//			{
//				printf("i: %i x: %i y: %i\n", j, (int)less_approx[j].x, (int)less_approx[j].y);
//				shapes.push_back(less_approx);
//			}
//			continue;
			if( approx.size() == 4 &&
				fabs(contourArea(Mat(approx))) > MIN_AREA &&
				isContourConvex(Mat(approx)) )
			{
				double maxCosine = 0;
				for( int j = 2; j < 5; j++ )
				{
					// find the maximum cosine of the angle between joint edges
					double cosine = fabs(angle(approx[j % 4], approx[j - 2], approx[j -1]));
					maxCosine = MAX(maxCosine, cosine);
				}
				// if cosines of all angles are small
				// (all angles are ~90 degree) then write quandrange
				// vertices to resultant sequence
				if( maxCosine < 0.3 )
					shapes.push_back(approx);
			}
			/*else if(check_round(more_approx))
			shapes.push_back(more_approx);
					else if( checkLamp(one_more_approx))
						shapes.push_back(one_more_approx); */
			}
		}
	}
}


static void drawAngles(Mat &image, const vector<vector<Point> > &angles)
{
	for(int i = 0; i < angles.size(); i++)
	{
		line(image, angles[i][0], angles[i][1], Scalar(0, 255, 0), 1, LINE_AA);
		line(image, angles[i][1], angles[i][2], Scalar(255, 0, 0), 1, LINE_AA);
	}
}

// the function draws all the squares in the image
static void drawShapes( Mat& image, const vector<vector<Point> >& shapes, vector<vector<Point> > &angles)
{
	for( size_t i = 0; i < shapes.size(); i++ )
	{
		const Point* p = &shapes[i][0];
		int n = (int)shapes[i].size();
		polylines(image, &p, &n, 1, true, Scalar(0, 0, 255), 1, LINE_AA);
	}
	drawAngles(image, angles);

	imshow(wndname, image);
}

int main(int argc, char** argv)
{
	static const char* names[] = { "../data/pic1.png", "../data/pic2.png", "../data/pic3.png",
	"../data/pic4.png", "../data/pic5.png", "../data/pic6.png", 0 };
	help();

	if( argc > 1)
	{
		names[0] =  argv[1];
		names[1] =  "0";
	}

	if(argc == 3)
	{
		N = stoi(argv[2]);
		//		printf("%i\n", N);
	}

	namedWindow( wndname, 1 );
	vector<vector<Point> > shapes;
	vector<vector<Point> > angles;

	for( int i = 0; names[i] != 0; i++ )
	{
		Mat image = imread(names[i], 1);
		if( image.empty() )
		{
			cout << "Couldn't load " << names[i] << endl;
			continue;
		}
		
		findShapes(image, shapes, angles);
		drawShapes(image, shapes, angles);

		char c = (char)waitKey();
		if( c == 27 )
			break;
	}

	return 0;
}
