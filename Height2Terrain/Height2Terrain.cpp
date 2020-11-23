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

cv::Vec3f cur_point_color{ 255, 255, 200 };
cv::Vec3f cur_seg_color{ 0,255,255 };
cv::Vec3f stored_point_color{ 100,100,50 };
cv::Vec3f stored_seg_color{ 0,100,100 };


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
	// draw current image base on gs
	switch (cv_data.drawMode) {
	case DrawMode::POINT:
		draw_points(cv_data.cur_raw_points, *cv_data.gs_points_graph_output, cur_point_color);
		break;
	case DrawMode::SEGMENT:
		draw_segs(cv_data.cur_result_points, *cv_data.gs_points_graph_output, false, cur_point_color, cur_seg_color);
	}

	// draw stored points
	for (int i = 0; i < 255; i++) {
		draw_segs(cv_data.recorded_points[i], *cv_data.gs_points_graph_output, false, stored_point_color, stored_seg_color);
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
			cv_data.recorded_points[cv_data.cur_gs] = cv_data.cur_result_points;
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

void auto_simplify(CV_Data &cv_data) {
	double i;
	for (i = 0; i < 100; i+=0.1) {
		if (cv_data.cur_result_points.size() != contour_division_num) {
			cv_data.RDP_epsilon = i;
			update_simplified_points(cv_data);
		}
		else {
			break;
		}
	}
	if (i >= 100) {
		cout << "Cannot find appropriaty parameter for simplify, please select another grayscale" << endl;
	}
}

void contour_finding_procedure(cv::Mat img) {
	if (img.empty())
	{
		cout << "Could not open or find the image" << endl;
		exit(-1);
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

	// user define edge number
	cout << "Please enter edge number" << endl;
	while (1) {
		string input;
		cin >> input;
		try {
			contour_division_num = std::stoi(input);
			break;
		}
		catch (std::invalid_argument) {
			cout << "Error, invalid segment number, please re-enter the contour division" << endl;
		}
	}

	cv::imshow("gray_img", img);
	cv::imshow("gs_distr_output", gs_dstr_graph_output);
	cv::imshow("result_output", gs_points_graph_output);

	cv::moveWindow("gray_img", 0, 0);
	cv::moveWindow("gs_distr_output", img.cols, 0);
	cv::moveWindow("result_output", 0, img.rows);


	int cur_key;

	int slider_val = 0;

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
			auto_simplify(cv_data);
			cv_data.updated_RDP_epsilon = true;
		}
		if (cur_key == 'r' || cur_key == 'R') {
			cv_data.updated_gs = true;
			cv_data.updated_RDP_epsilon = true;
		}
		// enter
		if (cur_key == 32) {
			cv_data.recorded_points[cv_data.cur_gs] = cv_data.cur_result_points;
		}

		bool updated = false;
		if (cv_data.updated_gs) {
			update_raw_points_contour(cv_data);
			update_gs_dstr_graph(cv_data);
			cv::imshow("gs_distr_output", gs_dstr_graph_output);
			cv::imshow("result_output", *cv_data.gs_points_graph_output);
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
	}

	cv::destroyWindow("gray_img");
	cv::destroyWindow("gs_distr_output");
	cv::destroyWindow("result_output");
}

int main(int argc, char** argv)
{
	cv::Mat img = cv::imread("hm2.jpg", cv::IMREAD_GRAYSCALE);
	if (USE_LEGACY) {
		legacy_procedure(img);
	}
	else {
		contour_finding_procedure(img);
	}
	return 0;
}
