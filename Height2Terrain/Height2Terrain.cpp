#include "OBJ_File_Helper.h"
#include "Algorithm.h"
#include "CVUtility.h"

#include <opencv2/opencv.hpp>
#include <fstream>
#include <time.h>
using std::cout;
using std::endl;
using std::cin;

#define LEFT_KEYCODE 2424832 
#define UP_KEYCODE 2490368 
#define RIGHT_KEYCODE 2555904 
#define DOWN_KEYCODE 2621440

#define USE_LEGACY false

int contour_division_num = 12;

cv::Vec3f CUR_POINT_COLOR{ 255, 255, 200 };
cv::Vec3f CUR_SEG_COLOR{ 0,255,255 };
cv::Vec3f STORED_POINT_COLOR{ 100,100,50 };
cv::Vec3f STORED_SEG_COLOR{ 0,100,100 };


CV_Data cv_data;

// Create grayscale distribution graph
cv::Mat create_gs_dstr_graph(const cv::Mat& img) {

	unsigned int* gs_weight = new unsigned int[256] {0};

	uint8_t* origin_pixel_ptr = (uint8_t*)img.data;
	int cn = img.channels();
	uint8_t cur_val;
	unsigned int max = 0;
	for (int i = 0; i < img.rows; i++)
	{
		for (int j = 0; j < img.cols; j++)
		{
			int idx = i * img.cols * cn + j * cn;
			cur_val = origin_pixel_ptr[idx];
			gs_weight[cur_val]++;
			max = max > gs_weight[cur_val] ? max : gs_weight[cur_val];
		}
	}
	int x_gap = 4;
	int height = 600, width = x_gap *(256+1);
	cv::Point last_pt(0,600);
	cv::Point cur_pt(0,0);
	cv::Scalar line_color{ 0,255,0 };
	cv::Mat gs_distr_img(height, width, CV_8UC3, cv::Scalar(0, 0, 0));
	for (int i = 0; i < 256; i++) {
		cur_pt.x =(x_gap + x_gap * i ) ;
		cur_pt.y = 600 - ((float)gs_weight[i] / (float)max * (float)height);
		cv::line(gs_distr_img, last_pt, cur_pt , line_color, 1);
		last_pt = cur_pt;
	}
	return gs_distr_img;
}

void update_point_output_graph(CV_Data& cv_data) {
	cv_data.gs_points_graph_output->setTo(cv::Scalar(0, 0, 0));
	// draw point output image
	switch (cv_data.drawMode) {
	case DrawMode::POINT:
		draw_points(cv_data.cur_raw_points, *cv_data.gs_points_graph_output, CUR_POINT_COLOR);
		break;
	case DrawMode::SEGMENT:
		draw_contour(cv_data.cur_result_points, *cv_data.gs_points_graph_output, false, CUR_POINT_COLOR, CUR_SEG_COLOR);
	}

	// draw stored simplified contours
	for (int i = 0; i < 255; i++) {
		draw_contour(cv_data.contours[i], *cv_data.gs_points_graph_output, false, STORED_POINT_COLOR, STORED_SEG_COLOR);
	}
	// draw connections
	for (int i = 0; i < cv_data.contour_connections.size(); i++) {
		draw_seg(cv_data.contour_connections[i], *cv_data.gs_points_graph_output, false, CUR_POINT_COLOR, CUR_SEG_COLOR);
	}
	cv::imshow("result_output", *cv_data.gs_points_graph_output);
}

void update_gs_dstr_graph(CV_Data& cv_data) {
	cv_data.gs_dstr_graph->copyTo(*cv_data.gs_dstr_graph_output);
	cv::line(*cv_data.gs_dstr_graph_output, { cv_data.cur_gs * 4 + 5, 0 }, { cv_data.cur_gs * 4 + 5, cv_data.gs_dstr_graph_output->rows }, { 255,0,0 }, 1);
}

void update_raw_points_contour(CV_Data& cv_data) {
	int rows = cv_data.gray_img->rows;
	int cols = cv_data.gray_img->cols;

	cv::Mat bw;
	cv::threshold(*cv_data.gray_img, bw, cv_data.cur_gs, 255, cv::THRESH_BINARY);

	vector<vector<cv::Point> > contours;
	vector<cv::Vec4i> hierarchy;

	cv::findContours(bw, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

	vector<cv::Point> result;
	for (vector<cv::Point> points : contours) {
		if (points.size() > result.size()) {
			result = points;
		}
	}
	cv_data.cur_raw_points = result;
	cv_data.cur_result_points = result;
}

/// <summary>
/// update raw points by getting all points in original image with corresponding grayscale
/// </summary>
/// <param name="cv_data"></param>
void update_raw_points(CV_Data& cv_data) {
	vector<cv::Point> all_points;
	get_gs_points(*cv_data.gray_img, cv_data.cur_gs, all_points);
	// shuffle to get different first head
	std::random_shuffle(all_points.begin(), all_points.end());
	cv_data.cur_raw_points = all_points;
}

/// <summary>
/// update the simplified points with current raw points
/// </summary>
/// <param name="cv_data"></param>
void update_simplified_points(CV_Data& cv_data) {

	if (USE_LEGACY) {
		vector<int> seq = connect_points(cv_data.cur_raw_points);
		vector<cv::Point> all_points_vec;
		for (int i = 0; i < seq.size(); i++) {
			all_points_vec.push_back(cv_data.cur_raw_points[seq[i]]);
		}

		vector<cv::Point> simplified_points;
		RamerDouglasPeucker(all_points_vec, cv_data.RDP_epsilon, simplified_points);
		cv_data.cur_result_points = simplified_points;
	}
	else {
		vector<cv::Point> simplified_points;
		RamerDouglasPeucker(cv_data.cur_raw_points, cv_data.RDP_epsilon, simplified_points);
		cv_data.cur_result_points = simplified_points;
	}
}

void auto_simplify_polygon(CV_Data& cv_data) {
	// pick a mid value
	double left = 0;
	double right = 100;
	double i = (left + right) / 2;
	cv_data.RDP_epsilon = i;
	update_simplified_points(cv_data);
	int count = 0;
	while (cv_data.cur_result_points.size() != contour_division_num+1 && count < 200) {
		if (cv_data.cur_result_points.size() > contour_division_num+1) {
			left = i;
			i = (right + i) / 2;
		}
		else {
			right = i;
			i = (left + i) / 2;
		}
		cv_data.RDP_epsilon = i;
		update_simplified_points(cv_data);
		count++;
	}
	if (count == 200) {
		cout << "Cannot simplify points to target division with 200 iterations, please pick another gray scale" << endl;
		cv_data.cur_result_points.clear();
	}
	else {
		cv_data.cur_result_points.erase(cv_data.cur_result_points.end()-1);
	}
}

/// <summary>
/// Callback Function for Opencv mouse event. Update the selected grayscale value and set the updated flag to true
/// </summary>
/// <param name="event"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="flags"></param>
/// <param name="userdata"></param>
void Mouse_Callback (int event, int x, int y, int flags, void* userdata) {
	CV_Data& data = *(CV_Data*)userdata;
	if (event == cv::EVENT_LBUTTONDOWN) {
		data.cur_gs = (int)get_pixel_color(*data.gray_img, y, x)[0];
		data.updated_gs = true;
		data.drawMode = DrawMode::POINT;
	}
}
void TrackBar_Callback(int, void*) {
	cv_data.updated_RDP_epsilon = true;
}

/// <summary>
/// Legacy procedure using closest point algorithm on all matching raw points,
/// possibly produce self intersecting polygon.
/// </summary>
/// <param name="img"></param>
void legacy_procedure(cv::Mat img) {

	if (img.empty())
	{
		cout << "Could not open or find the image" << endl;
		exit(-1);
	}

	cv::namedWindow("gray_img");	
	cv::namedWindow("gs_distr_output");
	cv::namedWindow("result_output");


	cv::Mat gs_dstr_graph = create_gs_dstr_graph(img);
	cv::Mat gs_dstr_graph_output;
	gs_dstr_graph.copyTo(gs_dstr_graph_output);
	cv::Mat gs_points_graph_output(img.rows, img.cols, CV_8UC3, cv::Scalar(0, 0, 0));


	cv_data = { &img, &gs_dstr_graph, &gs_dstr_graph_output, &gs_points_graph_output, 0, DrawMode::POINT};

	cv::setMouseCallback("gray_img", Mouse_Callback, &cv_data);

	update_raw_points(cv_data);

	cv::imshow("gray_img", img);
	cv::imshow("gs_distr_output", gs_dstr_graph_output);
	cv::imshow("result_output", gs_points_graph_output);

	cv::moveWindow("gray_img", 0, 0);
	cv::moveWindow("gs_distr_output", img.cols, 0);
	cv::moveWindow("result_output", 0, img.rows);
	int cur_key;

	int slider_val = 0;

	cv::createTrackbar("SimplifyEpsilon", "result_output", &slider_val, 100, TrackBar_Callback);
	while (1) {
		cv_data.RDP_epsilon = (double)slider_val / 10;
		cur_key = cv::waitKeyEx(50);
		if (cur_key == RIGHT_KEYCODE) {
			cv_data.cur_gs += 1;
			cv_data.cur_gs = cv_data.cur_gs < 255 ? cv_data.cur_gs : 255;
			cv_data.updated_gs = true;
		}
		if (cur_key == LEFT_KEYCODE) {
			cv_data.cur_gs -= 1;
			cv_data.cur_gs = cv_data.cur_gs > 0 ? cv_data.cur_gs : 0;
			cv_data.updated_gs = true;

		}
		if (cur_key == 'p' || cur_key == 'P') {
			cv_data.drawMode = DrawMode::POINT;
			cv_data.updated_gs = true;

		}
		if (cur_key == 's' || cur_key == 'S') {
			cv_data.drawMode = DrawMode::SEGMENT;
			cv_data.updated_RDP_epsilon = true;
		}
		if (cur_key == 'r' || cur_key == 'R') {
			cv_data.updated_gs = true;
			cv_data.updated_RDP_epsilon = true;
		}
		// enter
		if (cur_key == 32) {
			cv_data.contours[cv_data.cur_gs] = cv_data.cur_result_points;
		}

		bool updated = false;
		if (cv_data.updated_gs) {
			update_raw_points(cv_data);
			update_gs_dstr_graph(cv_data);
			cv::imshow("gs_distr_output", gs_dstr_graph_output);
			cv_data.updated_gs = false;
			updated = true;
		}
		if (cv_data.updated_RDP_epsilon) {
			update_simplified_points(cv_data);
			cv_data.updated_RDP_epsilon = false;
			updated = true;
		}
		if (updated) {
			update_point_output_graph(cv_data);
			updated = false;
		}
		if (cur_key == 27) break;

		cout << cur_key << endl;
	}

	cv::destroyWindow("gray_img");
	cv::destroyWindow("gs_distr_output");
	cv::destroyWindow("result_output");

}


void generate_mesh(CV_Data& cv_data) {
	cv_data.contour_connections.clear();
	vector<cv::Point>* contours = cv_data.contours;

	vector<vector<cv::Point>> min_dist_connections;


	cv::Point** connections = new cv::Point * [contour_division_num];
	for (int i = 0; i < contour_division_num; i++) {
		connections[i] = new cv::Point[cv_data.insert_sequence.size()];
	}

	for (int i = 0; i < contour_division_num; i++){
		connections[i][0] = contours[cv_data.insert_sequence[0]][i];
	}

	int prev_offset = 0;
	for (int i = 1; i < cv_data.insert_sequence.size(); i++) {
		int contour_idx = cv_data.insert_sequence[i];
		int prev_contour_idx = cv_data.insert_sequence[i - 1];

		int min_dist_offset;
		double min_dist_sum = DBL_MAX;
		for (int offset = 0; offset < contour_division_num; offset++) {
			double dist_sum = 0;
			for (int j = 0; j < contour_division_num; j++) {
				dist_sum += point_dist(contours[contour_idx][(j + offset) % contour_division_num], contours[prev_contour_idx][j]);
			}
			if (dist_sum < min_dist_sum) {
				min_dist_sum = dist_sum;
				min_dist_offset = offset;
			}
		}
		int actual_offset = (contour_division_num + min_dist_offset - prev_offset) % contour_division_num;
		for (int j = 0; j < contour_division_num; j++) {
			connections[j][i] = contours[contour_idx][(j + actual_offset) % contour_division_num];
		}

		prev_offset = actual_offset;
	}
	
	for (size_t i = 0; i < contour_division_num; i++) {
		vector<cv::Point> connection;
		for (size_t j = 0; j < cv_data.insert_sequence.size(); j++) {
			connection.push_back(connections[i][j]);
		}
		cv_data.contour_connections.push_back(connection);
	}

	/*
	* 	cv::Point center = find_center(contours[cv_data.insert_sequence[0]]);

	vector<cv::Point> prev_contour;
	for (int i = 0; i < contour_division_num; i++) {
		prev_contour.push_back(center);
	}


	cv::Point** connections = new cv::Point* [contour_division_num];
	for (int i = 0; i < contour_division_num; i++) {
		connections[i] = new cv::Point[cv_data.insert_sequence.size()];
	}

	int insert_offset = 0;

	for (int seq_idx = 0; seq_idx < cv_data.insert_sequence.size(); seq_idx++){
		vector<cv::Point> cur_contour = contours[cv_data.insert_sequence[seq_idx]];

		double max_dot = -1;
		int max_dot_prev_idx = -1;

		// pick one random point in current contour
		size_t rand_idx = rand() % (contour_division_num - 2) + 1;
		cv::Vec2d v0 = cv::normalize(cur_contour[rand_idx - 1] - cur_contour[rand_idx]);
		cv::Vec2d v1 = cv::normalize(cur_contour[rand_idx + 1] - cur_contour[rand_idx]);
		cv::Vec2d vtx_normal = cv::normalize(v0 + v1);

		cv::Point vtx = cur_contour[rand_idx];
		cv::Vec2d to_center_vec = find_center(cur_contour) - vtx;
		if (vtx_normal.dot(to_center_vec) < 0) {
			vtx_normal = -vtx_normal;
		}
		// find point in previous contour that fit with the mid vector of 
		// random picked point in current contour
		for (int i = 0; i < cur_contour.size(); i++) {
			cv::Vec2d to_prev_vec = cv::normalize(prev_contour[i] - vtx);
			double dot = to_prev_vec.dot(vtx_normal);
			if (dot > max_dot) {
				max_dot = dot;
				max_dot_prev_idx = i;
			}
		}
		cv_data.guide_points.push_back(prev_contour[max_dot_prev_idx]);
		cv_data.guide_points.push_back(cur_contour[rand_idx]);
		// locate 

		// insert current contour points to connections accordingly
		// with offset by the index of point found in previous contour
		for (int i = 0; i < contour_division_num; i++) {
			int output_idx = (contour_division_num + insert_offset + max_dot_prev_idx + i) % contour_division_num;
			int pt_idx = (rand_idx + i) % contour_division_num;
			connections[output_idx][seq_idx] = cur_contour[pt_idx];
		}

		insert_offset = (contour_division_num + insert_offset + max_dot_prev_idx - rand_idx) % contour_division_num;
		prev_contour = cur_contour;
	}

	for (size_t i = 0; i < contour_division_num; i++) {
		vector<cv::Point> connection;
		for (size_t j = 0; j < cv_data.insert_sequence.size(); j++) {
			connection.push_back(connections[i][j]);
		}
		cv_data.contour_connections.push_back(connection);
	}
	*/


}

void contour_finding_procedure(cv::Mat img) {
	if (img.empty())
	{
		cout << "Could not open or find the image" << endl;
		exit(-1);
	}


	// user define edge number
	cout << "Please contour division count" << endl;
	while (1) {
		string input;
		cin >> input;
		try {
			contour_division_num = std::stoi(input);
			break;
		}
		catch (std::invalid_argument) {
			cout << "Error, invalid number, please re-enter the contour division count" << endl;
		}
	}


	// create windows
	cv::namedWindow("gray_img");
	cv::namedWindow("gs_distr_output");
	cv::namedWindow("result_output");

	// setup cv matrix graph
	cv::Mat gs_dstr_graph = create_gs_dstr_graph(img);
	cv::Mat gs_dstr_graph_output;
	gs_dstr_graph.copyTo(gs_dstr_graph_output);
	cv::Mat gs_points_graph_output(img.rows, img.cols, CV_8UC3, cv::Scalar(0, 0, 0));

	// setup cv_data for data communication
	cv_data = CV_Data{ &img, &gs_dstr_graph, &gs_dstr_graph_output, &gs_points_graph_output, 0, DrawMode::POINT };

	// mouse input callback
	cv::setMouseCallback("gray_img", Mouse_Callback, &cv_data);

	update_raw_points(cv_data);


	cv::imshow("gray_img", img);
	cv::imshow("gs_distr_output", gs_dstr_graph_output);
	cv::imshow("result_output", gs_points_graph_output);

	cv::moveWindow("gray_img", 0, 0);
	cv::moveWindow("gs_distr_output", img.cols, 0);
	cv::moveWindow("result_output", 0, img.rows);


	int cur_key;

	int slider_val = 0;


	bool updated_points_output_graph = false;
	// mainloop
	cv::createTrackbar("SimplifyEpsilon", "result_output", &slider_val, 100, TrackBar_Callback);
	while (1) {

		cv_data.RDP_epsilon = (double)slider_val / 10;
		cur_key = cv::waitKeyEx(50);
		if (cur_key == RIGHT_KEYCODE) {
			cv_data.cur_gs += 1;
			cv_data.cur_gs = cv_data.cur_gs < 255 ? cv_data.cur_gs : 255;
			cv_data.drawMode = DrawMode::POINT;
			cv_data.updated_gs = true;
		}
		if (cur_key == LEFT_KEYCODE) {
			cv_data.cur_gs -= 1;
			cv_data.cur_gs = cv_data.cur_gs > 0 ? cv_data.cur_gs : 0;
			cv_data.drawMode = DrawMode::POINT;
			cv_data.updated_gs = true;
		}
		if (cur_key == 'p' || cur_key == 'P') {
			cv_data.drawMode = DrawMode::POINT;
			cv_data.updated_gs = true;
		}
		if (cur_key == 's' || cur_key == 'S') {
			cv_data.drawMode = DrawMode::SEGMENT;
			auto_simplify_polygon(cv_data);
			cv_data.updated_RDP_epsilon = true;
		}
		if (cur_key == 'r' || cur_key == 'R') {
			cv_data.updated_gs = true;
			cv_data.updated_RDP_epsilon = true;
		}
		// spacebar
		// record current simplified contour
		if (cur_key == 32) {
			cv_data.contours[cv_data.cur_gs] = cv_data.cur_result_points;
			cv_data.insert_sequence.push_back(cv_data.cur_gs);
		}
		//generate mesh
		if (cur_key == 13) {
			cout << "Generate mesh" << endl;
			generate_mesh(cv_data);
			updated_points_output_graph = true;
		}

		if (cv_data.updated_gs) {
			update_raw_points_contour(cv_data);
			update_gs_dstr_graph(cv_data);
			cv::imshow("gs_distr_output", gs_dstr_graph_output);
			cv::imshow("result_output", *cv_data.gs_points_graph_output);
			cv_data.updated_gs = false;
			updated_points_output_graph = true;
		}
		if (cv_data.updated_RDP_epsilon) {
			if (USE_LEGACY) {
				update_simplified_points(cv_data);
			}
			cv_data.updated_RDP_epsilon = false;
			updated_points_output_graph = true;
		}
		if (updated_points_output_graph) {
			update_point_output_graph(cv_data);
			updated_points_output_graph = false;
		}
		if (cur_key == 27) break;
	}

	cv::destroyWindow("gray_img");
	cv::destroyWindow("gs_distr_output");
	cv::destroyWindow("result_output");
}

int main(int argc, char** argv)
{
	cv::Mat img = cv::imread("hm2.png", cv::IMREAD_GRAYSCALE);
	if (USE_LEGACY) {
		legacy_procedure(img);
	}
	else {
		contour_finding_procedure(img);
	}
	return 0;
}
