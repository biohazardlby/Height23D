#pragma once
#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>

using std::vector;

//
//struct Seg2D {
//	cv::Point p0, p1;
//};
//inline
//bool operator==(const Seg2D& seg0, const Seg2D& seg1) {
//	return (seg0.p0 == seg1.p0 && seg0.p1 == seg1.p1)
//		|| (seg0.p1 == seg1.p0 && seg0.p0 == seg1.p1);
//}
//inline
//bool operator<(const Seg2D& seg0, const Seg2D& seg1) {
//	return seg0.p0.x < seg1.p0.x;
//}
//
//class Segment;
//
//class Point {
//
//public:	//stats
//	float x, y, z;
//	
//public:	//ctor
//	Point();
//	Point(float x, float y, float z);
//
//public:	//operator
//	bool operator==(const Point& p);
//};
//std::ostream& operator<<(std::ostream& os, const Point& p);
//class Vertex {
//
//private://static stats
//	static unsigned int cur_id;
//
//public:	//stats
//	unsigned int id;
//	vector<Segment*> edges;
//	Point location;
//public:	//ctor
//	Vertex(Point p);
//	Vertex(float x, float y, float z);
//	
//public:	//operator
//	bool operator==(const Vertex& v);
//};
//std::ostream& operator<<(std::ostream& os, const Vertex& v);
//
//class Segment {
//	
//public:	//stats
//	Vertex v0;
//	Vertex v1;
//
//public:	//ctor
//	Segment(Vertex v0, Vertex v1);
//public:
//	bool operator==(const Segment &e);
//};
//std::ostream& operator<<(std::ostream& os, const Segment &e);
