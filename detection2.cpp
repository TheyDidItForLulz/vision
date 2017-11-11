#include"opencv2/core.hpp"
#include"opencv2/imgproc.hpp"
#include"opencv2/imgcodecs.hpp"
#include"opencv2/highgui.hpp"

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<string>
#include<stdexcept>
#include<vector>
#include<utility>
#include<algorithm>

struct Point2d
{
	int r, c;
};

struct Vertex
{
	int r = -1, c = -1;
	int dir = -1;
};

struct Elem
{
	int vc = 0;
	Vertex v1, v2;
	bool valid = false;
	bool operator==(const Elem & other) {
		return memcmp(this, &other, sizeof(Elem)) == 0;
	}
};

typedef std::vector<std::vector<bool> > vb2;
typedef std::vector<std::vector<int> > vi2;

void find_borders(Point2d s, vb2& src, vb2& dest)
{
	for(int i = 1; i < s.r - 1; i++)
	{
		for(int j = 1; j < s.c - 1; j++)
		{
			if(src[i][j])
			{
				if(!(src[i + 1][j] && src[i - 1][j] && src[i][j + 1] && src[i][j - 1]
							&& src[i + 1][j + 1] && src[i - 1][j - 1] && src[i + 1][j - 1] && src[i - 1][j + 1]))
				{
					dest[i][j] = true;
				}
			}
		}
	}
}

int find_dist_to_border(vb2& borders, int r, int c)
{
	if(borders[r][c]) return 0;
	int cnt = 1;
	while(1)
	{
		for(int i = r - cnt; i < r + cnt + 1; i++)
		{
			for(int j = c - cnt; j < c + cnt + 1; j++)
			{
				try {
					if(borders.at(i).at(j)) return cnt;
				} catch (...) {}
			}
		}
		cnt++;
	}
}

bool is_w_even(vb2 p)
{
	int maxlen = -1, r, c;
	for(int i = 0; i < p.size(); i++)
	{
		int len = 0;
		for(int j = 0; j < p[i].size(); j++)
		{
			if(p[i][j]) len++;
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
	c -= maxlen / 3;
	int w = 0;
	while(1)
	{
		if(!p[r][c]) break;
		w++;
		r++;
	}
	printf("w: %i\n", w);
	if(w % 2 == 0) return true;
	else return false;
}

int find_dists(vb2& src, vb2& borders, vi2& dists, Point2d size)
{
	int max_dist = -1;
	for(int i = 0; i < size.r; i++)
	{
		for(int j = 0; j < size.c; j++)
		{
			if(src[i][j])
			{
				int dist = find_dist_to_border(borders, i, j);
				dists[i][j] = dist;
				if(dist > max_dist) max_dist = dist;
			}
		}
	}
	return max_dist;
}

void skeletonizate(Point2d size, vb2& src, vb2& dest, vb2& borders, cv::Mat& out, vi2& dists, int max_dist)
{
	printf("md: %i\n", max_dist);
	if(!(max_dist == 0))
	{
		for(int i = 1; i < size.r - 1; i++)
		{
			for(int j = 1; j < size.c - 1; j++)
			{
				int curr = dists[i][j];
				if(curr)
				{
					if(curr > dists[i - 1][j] && curr > dists[i + 1][j])
					{
						dest[i][j] = true;
					}
					else if(curr > dists[i][j - 1] && curr > dists[i][j + 1])
					{
						dest[i][j] = true;
					}
/*					if (dest[i][j] && dists[i][j] == max_dist - 1) {
						int a = 2;
						a = 3;
						a = 666;
					}
					if(curr > dists[i - 1][j - 1] && curr > dists[i + 1][j + 1])
					{
						dest[i][j] = true;
						continue;
					}
					if(curr > dists[i - 1][j + 1] && curr > dists[i + 1][j - 1])
					{
						dest[i][j] = true;
						continue;
					}*/
				}
			}
		}
	}
	else
	{
		for(int i = 0; i < size.r; i++)
		{
			for(int j = 0; j < size.c; j++)
			{
				dest[i][j] = src[i][j];
			}
		}
	}
}

void shift_pic(vb2& pic)
{
	for(int i = pic.size() - 1; i >= 1; i--)
	{
		for(int j = pic[i].size() - 1; j >= 1 ; j--)
		{
			if(pic[i][j] && !pic[i - 1][j]) pic[i][j] = false;
			if(pic[i][j] && !pic[i][j - 1]) pic[i][j] = false;
		}
	}
}

void find_elements(std::vector<Elem>& elements, int dir1, int dir2, int i, int j, int eps)
{
	bool start_vertex = true;
	for(int _i = 0; _i < elements.size(); _i++)
	{
		Elem tmp = elements[_i];
		if(tmp.vc == 1 && tmp.v1.dir == dir2)
		{
			if((dir1 < 3 && tmp.v1.r >= i - eps && tmp.v1.r <= i + eps) ||
					(dir1 >= 3 && tmp.v1.c >= j - eps && tmp.v1.c <= j + eps))
			{
				elements[_i].vc++;
				elements[_i].v2.r = i;
				elements[_i].v2.c = j;
				elements[_i].v2.dir = dir1;
				elements[_i].valid = true;
				start_vertex = false;
				break;
			}
		}
	}
	if(start_vertex)
	{
		Elem tmp;
		tmp.vc++;
		tmp.v1.r = i;
		tmp.v1.c = j;
		tmp.v1.dir = dir1;
		elements.push_back(tmp);
	}
}

void recognize(Elem e, vb2& pic, cv::Mat& out)
{
	int diameter2 = ((e.v1.r - e.v2.r) * (e.v1.r - e.v2.r) + (e.v1.c - e.v2.c) * (e.v1.c - e.v2.c));
	float diameter = sqrt(diameter2);
	int radius2 = diameter2 / 4;
	float radius = diameter / 2;

	Point2d center;
	center.r = 0;
	center.c = 0;
	int dots_count = 0;
	int max_r = std::max(e.v1.r, e.v2.r), min_r = std::min(e.v1.r, e.v2.r);
	int max_c = std::max(e.v1.c, e.v2.c), min_c = std::min(e.v1.c, e.v2.c);
	if(e.v1.dir < 3)
	{
		for(int i = min_r - radius - 2; i < max_r + radius + 2; i++)
		{
			for(int j = min_c - 2; j < max_c + 2; j++)
			{
				try
				{
					if(pic.at(i).at(j))
					{
						center.r += i;
						center.c += j;
						dots_count++;
					}
				}
				catch(...){}
			}
		}
	}
	else
	{
		for(int i = min_r - 2; i < max_r + 2; i++)
		{
			for(int j = min_c - radius - 2; j < max_c + radius + 2; j++)
			{
				try
				{
					if(pic.at(i).at(j))
					{
						center.r += i;
						center.c += j;
						dots_count++;
					}
				}
				catch(...){}
			}
		}
	}
	if(dots_count != 0)
	{
		center.r /= dots_count;
		center.c /= dots_count;
	}
//	center.r = (e.v1.r + e.v2.r) / 2;
//	center.c = (e.v1.c + e.v2.c) / 2;
	float eps = 0.325;
//	dots_count = 0;
	float round_prob = 0;
	float rect_prob = 0;
	for(int i = center.r - radius - 1; i <= center.r + radius + 1; i++)
	{
		int row_dots_count = 0;
		for(int j = center.c - radius - 1; j <= center.c + radius + 1; j++)
		{
			try
			{
				if(pic.at(i).at(j))
				{
					row_dots_count++;
					dots_count++;
					pic[i][j] = false;
					// round check
					int l2 = (center.r - i) * (center.r - i) + (center.c - j) * (center.c - j);
					if(std::abs(l2 / (float)radius2 - 1) < eps)
					{
						round_prob++;
					}
				}
			}
			catch(...){}
		}
		// rect check
		if(row_dots_count == 2 || row_dots_count >= radius)
		{
			rect_prob += row_dots_count;
		}
	}
	round_prob /= dots_count;
	rect_prob /= dots_count;
	if(rect_prob > round_prob)
	{
		cv::line(out, cv::Point(min_c, min_r), cv::Point(max_c, max_r), cv::Scalar(255, 255, 255));
	}
	else
	{
		cv::circle(out, cv::Point(center.c, center.r), radius, cv::Scalar(255, 255, 255));
	}
	printf("dots_count: %i round prob: %f rect prob: %f\n", dots_count, round_prob, rect_prob);
}

int main(int argc, char** argv)
{
	cv::Mat image;
	if(argc == 2)
	{
		image = cv::imread(argv[1]);
	}
	else return 1;
	cv::Mat gray;
	cv::cvtColor(image, gray, cv::COLOR_RGB2GRAY);
	Point2d size;
	size.r = gray.rows;
	size.c = gray.cols;
	vb2 pic(size.r, std::vector<bool>(size.c));
	for(int i = 0; i < size.r; i++)
	{
		for(int j = 0; j < size.c; j++)
		{
			if(gray.at<uint8_t>(i, j) < 128) pic[i][j] = true;
//			printf("%i", (int)pic[i][j]);
		}
//		printf("\n");
	}
	bool even_width = is_w_even(pic);
	if(even_width)
	{
		shift_pic(pic);
	}
	vb2 borders(size.r, std::vector<bool>(size.c));
	find_borders(size, pic, borders);
	vb2 dest_array(size.r, std::vector<bool>(size.c));
	cv::Mat out(size.r, size.c, CV_8U);
	vi2 dists(size.r, std::vector<int>(size.c));
	int max_dist = find_dists(pic, borders, dists, size);
	skeletonizate(size, pic, dest_array, borders, out, dists, max_dist);
	cv::imshow("jj", gray);

	std::vector<Elem> elements;
	int eps = 2;

	for(int i = 0; i < size.r; i++)
	{
		for(int j = 0; j < size.c; j++)
		{
//			if(dists[i][j])
//			{
//				printf("1");
				out.at<uint8_t>(i, j) = static_cast<uint8_t>(255 - 255 / (dest_array[i][j] + 1));
//			}
//			else printf("0");
		}
//		printf("\n");
	}

	if(!even_width)
	{
		for(int i = 1; i < size.r - 1; i++)
		{
			for(int j = 1; j < size.c - 1; j++)
			{
				if(dest_array[i][j])
				{
					if(dest_array[i][j - 1] && dest_array[i + 1][j + 1] && dest_array[i - 1][j + 1])
					{
						find_elements(elements, 1, 2, i, j, eps);
					}
					else if(dest_array[i][j + 1] && dest_array[i - 1][j - 1] && dest_array[i + 1][j - 1])
					{
						find_elements(elements, 2, 1, i, j, eps);
					}
					else if(dest_array[i + 1][j] && dest_array[i - 1][j - 1] && dest_array[i - 1][j + 1])
					{
						find_elements(elements, 3, 4, i, j, eps);
					}
					else if(dest_array[i - 1][j] && dest_array[i + 1][j + 1] && dest_array[i + 1][j - 1])
					{
						find_elements(elements, 4, 3, i, j, eps);
					}
				}
			}
		}
	}
	else
	{
		for(int i = 2; i < size.r - 2; i++)
		{
			for(int j = 2; j < size.c - 2; j++)
			{
				if(dest_array[i][j])
				{
					if(dest_array[i][j - 1] && dest_array[i + 1][j + 1] && dest_array[i - 1][j + 1] || 
							(dest_array[i][j - 2] && dest_array[i + 2][j + 2] && dest_array[i - 2][j + 2]))
					{
						find_elements(elements, 1, 2, i, j, eps);
					}
					else if(dest_array[i][j + 1] && dest_array[i - 1][j - 1] && dest_array[i + 1][j - 1] || 
							(dest_array[i][j + 2] && dest_array[i - 2][j - 2] && dest_array[i + 2][j - 2]))
					{
						find_elements(elements, 2, 1, i, j, eps);
					}
					else if(dest_array[i + 1][j] && dest_array[i - 1][j - 1] && dest_array[i - 1][j + 1] ||
							(dest_array[i + 2][j] && dest_array[i - 2][j - 2] && dest_array[i - 2][j + 2]))
					{
						find_elements(elements, 3, 4, i, j, eps);
					}
					else if(dest_array[i - 1][j] && dest_array[i + 1][j + 1] && dest_array[i + 1][j - 1] ||
							(dest_array[i - 2][j] && dest_array[i + 2][j + 2] && dest_array[i + 2][j - 2]))
					{
						find_elements(elements, 4, 3, i, j, eps);
					}
				}
			}
		}
	}
	for(int i = 0; i < elements.size(); i++)
	{
		Elem e = elements[i];
		if(e.valid)
		{
			printf("i: %i vc: %i v1r: %i v1c: %i v1dir: %i v2r: %i v2c: %i v2dir: %i\n", 
					i, e.vc, e.v1.r, e.v1.c, e.v1.dir, e.v2.r, e.v2.c, e.v2.dir);
			recognize(elements[i], dest_array, out);
		}
		else
		{
			elements.erase(std::find(elements.begin(), elements.end(), elements[i]));
			--i;
		}
	}



	cv::imshow("kk", out);

	cv::waitKey(0);
	return 0;
}
