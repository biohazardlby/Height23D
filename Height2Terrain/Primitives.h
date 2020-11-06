#pragma once
#include <vector>
#include <iostream>

#define EPSILON 

using std::vector;

class Edge;

class Point {

public:	//stats
	float x, y, z;
	
public:	//ctor
	Point();
	Point(float x, float y, float z);

public:	//operator
	bool operator==(const Point& p);
};
std::ostream& operator<<(std::ostream& os, const Point& p);
class Vertex {

private://static stats
	static unsigned int cur_id;

public:	//stats
	unsigned int id;
	vector<Edge*> edges;
	Point location;
public:	//ctor
	Vertex(Point p);
	Vertex(float x, float y, float z);
	
public:	//operator
	bool operator==(const Vertex& v);
};
std::ostream& operator<<(std::ostream& os, const Vertex& v);

class Edge {
	
public:	//stats
	Vertex v0;
	Vertex v1;

public:	//ctor
	Edge(Vertex v0, Vertex v1);
public:
	bool operator==(const Edge &e);
};
std::ostream& operator<<(std::ostream& os, const Edge &e);
