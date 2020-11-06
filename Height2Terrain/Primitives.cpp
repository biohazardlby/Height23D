#include "Primitives.h"

Point::Point() : x{ 0 }, y{ 0 }, z{ 0 }
{
}

Point::Point(float x, float y, float z) : x{ x }, y{ y }, z{ z }
{
}

bool Point::operator==(const Point& p)
{
	return x == p.x && y == p.y && z == p.z;
}

std::ostream& operator<<(std::ostream& os, const Point& p)
{
	os << "(" << p.x << ", " << p.y << ", " << p.z << ")";
	return os;
}

unsigned int Vertex::cur_id = 1;

Vertex::Vertex(Point p) : location{ p }, id{ cur_id++ }
{
}

Vertex::Vertex(float x, float y, float z) : location{ x,y,z }, id{ cur_id++ }
{
}

bool Vertex::operator==(const Vertex& v)
{
	return id == v.id;
}

std::ostream& operator<<(std::ostream& os, const Vertex& v)
{
	os << '[' << v.id << " | " << v.location.x << ", " << v.location.y << ", " << v.location.z << "]";
	return os;
}

std::ostream& operator<<(std::ostream& os, const Edge& e)
{
	os << "{" <<e.v0 << "-"<< e.v1 << "}";
	return os;
}

Edge::Edge(Vertex v0, Vertex v1) : v0{ v0 }, v1{ v1 }
{
}

bool Edge::operator==(const Edge& e)
{
	return v0 == e.v0 && v1 == e.v1;
}
