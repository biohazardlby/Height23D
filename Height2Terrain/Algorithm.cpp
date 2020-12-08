#include "Algorithm.h"


#include <iostream>
#include <algorithm>
#include <math.h>
#include <set>

using std::cout;
using std::endl;
using std::set;

double point_dist(int v0x, int v0y, int v1x, int v1y) {
	return sqrt(pow(v0x - v1x, 2) + pow(v0y - v1y, 2));
}

double point_dist(const cv::Point& p0, const cv::Point& p1) {
	return point_dist(p0.x, p0.y, p1.x, p1.y);
}



struct DLP {
	DLP* prev = nullptr;
	DLP* next = nullptr;
	int idx = -1;
};
inline
bool operator == (const DLP& d0, const DLP& d1) {
	return d0.idx == d1.idx;
}
bool operator != (const DLP& d0, const DLP& d1) {
	return d0.idx != d1.idx;
}


vector<int> connect_points_tsp(const vector<cv::Point>& points)
{
	if (points.size() < 3) {
		return vector<int>();
	}

	// create distance lookup graph
	double** distance_mat = new double* [points.size()];
	for (int i = 0; i < points.size(); i++) {
		distance_mat[i] = new double[points.size()];
	}
	// get all distance
	for (int i = 0; i < points.size(); i++) {
		for (int j = 0; j < points.size(); j++) {
			if (i == j) {
				distance_mat[i][j] = 0;
			}
			else {
				distance_mat[i][j] = point_dist(
					points[i].x, points[i].y, points[j].x, points[j].y);
			}
		}
	}
}


double PerpendicularDistance(const cv::Point& pt, const cv::Point& lineStart, const cv::Point& lineEnd)
{
	double dx = lineEnd.x - lineStart.x;
	double dy = lineEnd.y - lineStart.y;

	//Normalise
	double mag = pow(pow(dx, 2.0) + pow(dy, 2.0), 0.5);
	if (mag > 0.0)
	{
		dx /= mag; dy /= mag;
	}

	double pvx = pt.x - lineStart.x;
	double pvy = pt.y - lineStart.y;

	//Get dot product (project pv onto normalized direction)
	double pvdot = dx * pvx + dy * pvy;

	//Scale line direction vector
	double dsx = pvdot * dx;
	double dsy = pvdot * dy;

	//Subtract this from pv
	double ax = pvx - dsx;
	double ay = pvy - dsy;

	return pow(pow(ax, 2.0) + pow(ay, 2.0), 0.5);
}

void RamerDouglasPeucker(const vector<cv::Point>& pointList, double epsilon, vector<cv::Point>& out)
{
	if (pointList.size() < 2) return;

	// Find the Point2D with the maximum distance from line between start and end
	double dmax = 0.0;
	size_t index = 0;
	size_t end = pointList.size() - 1;
	for (size_t i = 1; i < end; i++)
	{
		double d = PerpendicularDistance(pointList[i], pointList[0], pointList[end]);
		if (d > dmax)
		{
			index = i;
			dmax = d;
		}
	}

	// If max distance is greater than epsilon, recursively simplify
	if (dmax > epsilon)
	{
		// Recursive call
		vector<cv::Point> recResults1;
		vector<cv::Point> recResults2;
		vector<cv::Point> firstLine(pointList.begin(), pointList.begin() + index + 1);
		vector<cv::Point> lastLine(pointList.begin() + index, pointList.end());
		RamerDouglasPeucker(firstLine, epsilon, recResults1);
		RamerDouglasPeucker(lastLine, epsilon, recResults2);

		// Build the result list
		out.assign(recResults1.begin(), recResults1.end() - 1);
		out.insert(out.end(), recResults2.begin(), recResults2.end());
		if (out.size() < 2)
			return;
	}
	else
	{
		//Just return start and end points
		out.clear();
		out.push_back(pointList[0]);
		out.push_back(pointList[end]);
	}
}


vector<int> connect_points(const vector<cv::Point> &points)
{
	if (points.size() < 3) {
		return vector<int>();
	}

	// create distance lookup graph
	double** distance_mat = new double* [points.size()];
	for (int i = 0; i < points.size(); i++) {
		distance_mat[i] = new double[points.size()];
	}
	// get all distance
	for (int i = 0; i < points.size(); i++) {
		for (int j = 0; j < points.size(); j++) {
			if (i == j) {
				distance_mat[i][j] = DBL_MAX;
			}
			else {
				distance_mat[i][j] = point_dist(
					points[i].x, points[i].y, points[j].x, points[j].y);
			}
		}
	}
	// create head and the second connected point
	DLP* head = new DLP();
	head->idx = 0;
	DLP* current = new DLP();
	current->idx = 1;

	head->next = current;
	current->prev = head;

	for (int i = 1; i < points.size(); i++) {
		if (distance_mat[0][i] < distance_mat[0][current->idx]) {
			current->idx = i;
		}
	}

	set<int> connected_idx;
	connected_idx.insert(current->idx);
	connected_idx.insert(head->idx);
	// main loop
	while (true) {
		DLP* next = new DLP();
		next->idx = current->idx;
		for (int i = 0; i < points.size(); i++) {
			if (connected_idx.find(i) != connected_idx.end()) continue;
			if (distance_mat[i][current->idx] < distance_mat[next->idx][current->idx]) {
				next->idx = i;
			}
		}
		if (current->idx == next->idx) break;
		connected_idx.insert(next->idx);
		current->next = next;
		next->prev = current;
		current = current->next;
	}

	vector<int> point_sequence;
	current = head;
	while(current != nullptr){
		point_sequence.push_back(current->idx);
		current = current->next;
	}

	for (int i = 0; i < points.size(); i++) {
		delete[] distance_mat[i];
	}
	delete[] distance_mat;

	
	return point_sequence;
}
