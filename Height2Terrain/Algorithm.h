#pragma once
#include <vector>
#include <opencv2/opencv.hpp>

using std::vector;
using std::pair;

vector<int> connect_points_tsp(const vector<cv::Point>& points);

vector<int> connect_points(const vector<cv::Point>& points);

void RamerDouglasPeucker(const vector<cv::Point>& pointList, double epsilon, vector<cv::Point>& out);