#pragma once
#include <vector>
#include <opencv2/opencv.hpp>

using std::vector;
using std::pair;

double point_dist(const cv::Point& p0, const cv::Point& p1);

vector<int> connect_points_tsp(const vector<cv::Point>& points);

vector<int> connect_points(const vector<cv::Point>& points);

void RamerDouglasPeucker(const vector<cv::Point>& pointList, double epsilon, vector<cv::Point>& out);