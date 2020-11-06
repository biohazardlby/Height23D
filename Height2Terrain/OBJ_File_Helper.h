#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <tuple>
#include "Primitives.h"

using std::string;
using std::stringstream;
using std::tuple;

using std::endl;
using std::cout;

using std::snprintf;

namespace OBJ_File_Helper {
	string create_OBJ_string(Vertex** vertices, int vtx_size, vector<unsigned int>* faces, int face_size);
}