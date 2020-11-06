#include "OBJ_File_Helper.h"

namespace OBJ_File_Helper {


	string point_OBJ_format(const Point& p) {
		char buffer[100];
		int char_size = snprintf(buffer, 100, "v %.6f %.6f %.6f", p.x, p.y, p.z);
		
		return string(buffer, char_size);
	}

	string face_OBJ_format(const vector<unsigned int>& face) {
		stringstream ss;
		ss << "f ";
		for (int i = 0; i < face.size(); i++) {
			ss << face[i] << "/" << face[i] << "/" << face[i] << ' ';
		}
		return ss.str();

	}
	string create_OBJ_string(Vertex** vertices, int vtx_size, vector<unsigned int>* faces, int face_size) {
		stringstream ss;
		ss << "# This file is created by Height2Terrain for class CSCI-716. " << endl;
		ss << "g default" << endl;

		// write vertices
		for (int i = 0; i < vtx_size; i++) {
			ss << point_OBJ_format(vertices[i]->location) << endl;
		}
		ss << endl;

		// no UV setup
		for (int i = 0; i < vtx_size; i++) {
			ss << "vt " << "0.000000 0.000000" << endl;
		}
		ss << endl;

		// for now all normal up
		for (int i = 0; i < vtx_size; i++) {
			ss << "vn " << "0.000000 1.000000 0.000000" << endl;
		}
		ss << endl;

		// export group
		ss << "g Height2Terrain" << endl;
		for (int i = 0; i < face_size; i++) {
			ss << face_OBJ_format(faces[i]) << endl;
		}

		return ss.str();
	}
}