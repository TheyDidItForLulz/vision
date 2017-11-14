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
#include<queue>
#include<cassert>

#define MIN_ELEMENT_SQUARE 16

// list of functions
// find_distance_between_points
// find_borders
// find_dist_to_border
// is_w_even
// find_dists
// skeletonizate
// shift_pic
// find_elements
// recognize
// bfs
// find_connections
// main

struct Point2d
{
	int r, c;
	bool operator==(const Point2d & other)
	{
		return memcmp(this, &other, sizeof(Point2d)) == 0;
	}
};

struct Vertex
{
	int r = -1, c = -1;
	int dir = -1;
	std::vector<int> connections;
	bool operator==(const Vertex & other) 
	{
		if(r == other.r && c == other.c && dir == other.dir
				&& connections.size() == other.connections.size())
		{
			for(int i = 0; i < connections.size(); i++)
			{
				if(connections[i] != other.connections[i]) return false;
			}
			return true;
		}
		return false;
	}
};

struct Elem
{
	int id;
	std::string type = "none";
	int vc = 0;
	Vertex v1, v2;
	bool valid = false;
	bool operator==(const Elem & other) {
		if(id == other.id && type == other.type 
				&& vc == other.vc && v1 == other.v1 
				&& v2 == other.v2 && valid == other.valid)
		{
			return true;
		}
		return false;
	}
};

typedef std::vector<std::vector<bool> > vb2;
typedef std::vector<std::vector<int> > vi2;

float find_distance_between_points(Point2d b, Point2d e)
{
	return sqrt((b.r - e.r) * (b.r - e.r) + (b.c - e.c) * (b.c - e.c));
}

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
		tmp.valid = true;								// for dots
		tmp.vc++;
		tmp.v1.r = i;
		tmp.v1.c = j;
		tmp.v1.dir = dir1;
		elements.push_back(tmp);
	}
}

// codes:
// 1 - success
// 0 - error
// -1 - dots

int recognize(Elem& e, vb2& pic, cv::Mat& out)
{
	if(e.vc == 1)
	{
		e.type = "dot";
		return 1;
	}
	int diameter2 = ((e.v1.r - e.v2.r) * (e.v1.r - e.v2.r) + (e.v1.c - e.v2.c) * (e.v1.c - e.v2.c));
	float diameter = sqrt(diameter2);
	int radius2 = diameter2 / 4;
	float radius = diameter / 2;
	Point2d center;
	int dots_count = 0;
	int max_r = std::max(e.v1.r, e.v2.r), min_r = std::min(e.v1.r, e.v2.r);
	int max_c = std::max(e.v1.c, e.v2.c), min_c = std::min(e.v1.c, e.v2.c);
	center.r = (e.v1.r + e.v2.r) / 2;
	center.c = (e.v1.c + e.v2.c) / 2;
	float eps = 0.325;
	float round_prob = 0;
	float rect_prob = 0;
	bool ampermeter = false;
	bool voltmeter = false;
	bool resistor = false;
	bool battery = false;
	bool dots = false;
	int parallel_lines_count = 0;
	int voltmeter_dots = 0;
	int buff_dist_between = -1;
	for(int i = center.r - radius; i <= center.r + radius; i++)
	{
		int row_dots_count = 0;
		std::vector<Point2d> points;
		for(int j = center.c - radius; j <= center.c + radius; j++)
		{
			try
			{
				if(pic.at(i).at(j))
				{
					row_dots_count++;
					dots_count++;
					// round check
					int l2 = (center.r - i) * (center.r - i) + (center.c - j) * (center.c - j);
					if(std::abs(l2 / (float)radius2 - 1) < eps)
					{
						round_prob++;
					}
					else
					{
						if(i > center.r)
						{
							Point2d p;
							p.r = i;
							p.c = j;
							points.push_back(p);
						}
					}
				}
			}
			catch(...){}
		}
		if((points.size() == 2 || points.size() == 1) && i > center.r)
		{
			if(buff_dist_between == -1 && points.size() == 2)
			{
				buff_dist_between = find_distance_between_points(points[0], points[1]);
			}
			else if(buff_dist_between == -1 && points.size() == 1)
			{
				buff_dist_between == 0;
			}
			else if(buff_dist_between >= (points.size() == 1 ? 0 : find_distance_between_points(points[0], points[1])))
			{
				voltmeter_dots++;
			}
		}
		if(row_dots_count > (radius / 4 < 4 ? 4 : radius / 4) && i < center.r + radius / 4 && i > center.r - radius / 4)	//
		{																													// ampermeter check
			ampermeter = true;																								//
		}																													//
		// rect check
		if(e.v1.dir < 3)
		{
			if(row_dots_count == 2 || row_dots_count >= (radius * 3) / 2)				// there was just radius
			{
				if(row_dots_count >= (radius * 3) / 2)
				{
					parallel_lines_count++;
				}
				rect_prob += row_dots_count;
			}
			else if(row_dots_count < (radius * 3) / 2 && row_dots_count > radius / 2)	//
			{																			// branching check
				rect_prob += row_dots_count;											//
//				printf("Found ya! Here are sum dots\n");								// I can rewrite it later as finding holes in lines
				dots = true;
			}																			//
		}
	}
	if(voltmeter_dots >= radius / 5) // or 8, for more approx, but this way it can see less
	{
		voltmeter = true;
	}
	if(e.v1.dir >= 3)
	{
		for(int i = center.c - radius; i <= center.c + radius; i++)
		{
			int col_dots_count = 0;
			for(int j = center.r - radius; j <= center.r + radius; j++)
			{
				try
				{
					if(pic.at(j).at(i))
					{
						col_dots_count++;
					}
				}
				catch(...){}
			}
			if(col_dots_count == 2 || col_dots_count >= (radius * 3) / 2)				// there too
			{
				if(col_dots_count >= (radius * 3) / 2)
				{
					parallel_lines_count++;
				}
				rect_prob += col_dots_count;
			}
			else if(col_dots_count < (radius * 3) / 2 && col_dots_count > radius / 2)	//
			{																			// branching check
				rect_prob += col_dots_count;											//
//				printf("Found ya! Here are sum dots\n");								// I can rewrite it later as finding holes in lines
				dots = true;
			}																			//
		}
	}
	if(parallel_lines_count == 0) battery = true;
	else if(parallel_lines_count == 2) resistor = true;
	if(dots_count < MIN_ELEMENT_SQUARE) return 0;
	round_prob /= dots_count;
	rect_prob /= dots_count;
	if(rect_prob > round_prob)
	{
		if(dots) e.type = "dots";
		else
		{		
			if(battery) e.type = "battery";
			else if(resistor) e.type = "resistor";
//			else e.type = "dots";
		}
		cv::line(out, cv::Point(min_c, min_r), cv::Point(max_c, max_r), cv::Scalar(255, 255, 255));
	}
	else
	{
		if(ampermeter)
		{
			e.type = "ampermeter";
	//		cv::line(out, cv::Point(min_c, min_r), cv::Point(max_c, max_r), cv::Scalar(255, 255, 255));
		}
		if(voltmeter) e.type = "voltmeter";
		if(!ampermeter && !voltmeter) e.type = "lamp";										// ne kostyl
		cv::circle(out, cv::Point(center.c, center.r), radius, cv::Scalar(255, 255, 255));
	}
	if(e.type == "dots") return -1;
//	printf("dots_count: %i round prob: %f rect prob: %f\n", dots_count, round_prob, rect_prob);
	return 1;
}

void bfs(std::vector<Elem>& elements, vb2 pic, Vertex s, int id, vb2& used, bool dot)
{
	if(s.connections.size() != 0 && !dot) return;
	printf("Entering bfs\n");
	bool break_bfs = false;
	std::queue<Point2d> q;
	used[s.r][s.c] = true;
	Point2d sp;
	sp.r = s.r;
	sp.c = s.c;
	if(dot)
	{
		bool exit = false;
		for(int i = s.r - 1; i <= s.r + 1; i++)
		{
			for(int j = s.c - 1; j <= s.c + 1; j++)
			{
				if(!used[i][j] && pic[i][j])
				{
					sp.r = i;
					sp.c = j;
					exit = true;
					break;
				}
			}
			if(exit) break;
		}
		if(!exit) return;
	}
	used[sp.r][sp.c] = true;
	q.push(sp);
	if(!dot)
	{
		switch(s.dir)
		{
			case 1:
				try {used.at(s.r + 1).at(s.c + 1) = true;} catch(...) {}
				try {used.at(s.r).at(s.c + 1) = true;} catch(...) {}
				try {used.at(s.r - 1).at(s.c + 1) = true;} catch(...) {}
				break;
			case 2:
				try {used.at(s.r + 1).at(s.c - 1) = true;} catch(...) {}
				try {used.at(s.r).at(s.c - 1) = true;} catch(...) {}
				try {used.at(s.r - 1).at(s.c - 1) = true;} catch(...) {}
				break;
			case 3:
				try {used.at(s.r - 1).at(s.c - 1) = true;} catch(...) {}
				try {used.at(s.r - 1).at(s.c) = true;} catch(...) {}
				try {used.at(s.r - 1).at(s.c + 1) = true;} catch(...) {}
				break;
			case 4:
				try {used.at(s.r + 1).at(s.c - 1) = true;} catch(...) {}
				try {used.at(s.r + 1).at(s.c) = true;} catch(...) {}
				try {used.at(s.r + 1).at(s.c + 1) = true;} catch(...) {}
				break;
		}
	}
	while(!q.empty())
	{
		if(break_bfs) break;
		Point2d curr = q.front();
		q.pop();
		for(int i = curr.r - 1; i <= curr.r + 1; i++)
		{
			for(int j = curr.c - 1; j <= curr.c + 1; j++)
			{
				try
				{
					if(pic[i][j] && !used[i][j])
					{
						Point2d tmp;
						tmp.r = i;
						tmp.c = j;
						q.push(tmp);
						used[i][j] = true;
						for(int _i = 0; _i < elements.size(); _i++)
						{
							if(elements[_i].v1.r == i && elements[_i].v1.c == j 
									&& (elements[_i].v1.connections.size() == 0 || elements[_i].type == "dot"))
							{
								elements[_i].v1.connections.push_back(id);
								if (elements[id].v1.connections.size() == 0 || dot)
								{
									elements[id].v1.connections.push_back(_i);
								}
								else
								{
									elements[id].v2.connections.push_back(_i);
								}
								if (elements[_i].type == "dot") {
									used[i][j] = false;
								}
								break_bfs = true;
							}
							else if(elements[_i].v2.r == i && elements[_i].v2.c == j 
									&& elements[_i].v2.connections.size() == 0 && elements[_i].type != "dot")
							{
								elements[_i].v2.connections.push_back(id);
								if (elements[id].v1.connections.size() == 0 || dot)
								{
									elements[id].v1.connections.push_back(_i);
								}
								else
								{
									elements[id].v2.connections.push_back(_i);
								}
								break_bfs = true;
							}
						}
					}
				}
				catch(...){}
			}
		}
		cv::Mat matrix(used.size() + 1, used[0].size() + 1, CV_8UC1);
		for(int i = 0; i < used.size(); i++)
		{
			for(int j = 0; j < used[0].size(); j++)
			{
				if(used[i][j])
				{
					matrix.at<uint8_t>(i, j) = static_cast<uint8_t>(254);
				}
				else matrix.at<uint8_t>(i, j) = static_cast<uint8_t>(1);
			}
		}
		cv::imshow("bfs", matrix);
		cv::waitKey(0);
	}
	if(dot)
	{
		used[s.r][s.c] = false;
	}
}

void find_connections(std::vector<Elem>& elements, vb2 pic)
{
	vb2 used(pic.size(), std::vector<bool>(pic[0].size()));
	for(int i = 0; i < elements.size(); i++)
	{
		if(elements[i].type != "dot")
		{
			Vertex s = elements[i].v1;
			bfs(elements, pic, s, i, used, false);
			s = elements[i].v2;
			bfs(elements, pic, s, i, used, false);
		}
		else bfs(elements, pic, elements[i].v1, i, used, true);
	}
}

void validate_elements(std::vector<Elem>& elements, vb2& dest_array, cv::Mat& out)
{
	for(int i = 0; i < elements.size(); i++)
	{
		Elem e = elements[i];
		if(e.valid)
		{
//			printf("i: %i vc: %i v1r: %i v1c: %i v1dir: %i v2r: %i v2c: %i v2dir: %i\n", 
//					i, e.vc, e.v1.r, e.v1.c, e.v1.dir, e.v2.r, e.v2.c, e.v2.dir);
			int code = recognize(elements[i], dest_array, out);
			if(code == 0)
			{
				elements.erase(std::find(elements.begin(), elements.end(), elements[i]));
				--i;
			}
			else if(code == -1)
			{
				Elem dot;
				dot.valid = true;
				dot.vc = 1;
				dot.v1 = elements[i].v2;
				dot.type = "dot";
				elements.push_back(dot);
				elements[i].v2.r = -1;
				elements[i].v2.c = -1;
				elements[i].v2.dir = -1;
				elements[i].v2.connections.clear();
				elements[i].vc--;
				elements[i].type = "dot";
			}
		}
		else
		{
			elements.erase(std::find(elements.begin(), elements.end(), elements[i]));
			--i;
		}
	}
	for(int i = 0; i < elements.size(); i++)
	{
		elements[i].id = i;
	}
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
	validate_elements(elements, dest_array, out);
	find_connections(elements, dest_array);
	for(int i = 0; i < elements.size(); i++)
	{
		Elem e = elements[i];
		if(e.type != "dot")
		{
			printf("id: %i type: %s id1: %i id2: %i\n", e.id, e.type.c_str(), e.v1.connections[0], e.v2.connections[0]);
		}
		else
		{
			printf("id: %i type: %s ", e.id, e.type.c_str());
			for(int j = 0; j < e.v1.connections.size(); j++)
			{
				printf("id1_%i: %i ", j, e.v1.connections[j]);
			}
			printf("\n");
		}
	}
	cv::imshow("kk", out);
	cv::waitKey(0);
	return 0;
}
