#pragma once
#include <opencv2/opencv.hpp>

using std::cout;
using std::endl;

enum DrawMode {
	POINT, SEGMENT
};

inline
cv::Vec2d operator-(cv::Point p0, cv::Point p1) {
	return cv::Vec2d(p0.x - p1.x, p0.y - p1.y);
}
cv::Point find_center(const vector<cv::Point> points) {

	double center_x = 0, center_y = 0;
	for (cv::Point p : points) {
		center_x += p.x;
		center_y += p.y;
	}
	center_x /= points.size();
	center_y /= points.size();

	return cv::Point(center_x, center_y);

}
struct CV_Data {
	cv::Mat* gray_img;
	cv::Mat* gs_dstr_graph;
	cv::Mat* gs_dstr_graph_output;
	cv::Mat* gs_points_graph_output;

	int cur_gs;
	DrawMode drawMode;

	vector<cv::Point> cur_raw_points;
	vector<cv::Point> cur_result_points;
	vector<cv::Point>* contours = new vector<cv::Point>[255];
	vector<vector<cv::Point>> contour_connections;

	vector<int> insert_sequence;

	double RDP_epsilon = 2;

	bool updated_gs = false;
	bool updated_RDP_epsilon = false;

	vector<cv::Point> guide_points;
};



cv::Scalar get_pixel_color(const cv::Mat img, int x, int y) {
	int cn = img.channels();
	uint8_t* pixel_ptr = (uint8_t*)img.data;
	int idx = x * img.cols * cn + y * cn;
	return (pixel_ptr[idx], pixel_ptr[idx + 1], pixel_ptr[idx + 2]);
}

cv::Mat get_blue_cn_img(const cv::Mat& origin_img) {

	cv::Mat grayscale_img(origin_img.rows, origin_img.cols, CV_8UC3, cv::Scalar(0, 0, 0));
	uint8_t* gs_pixel_ptr = (uint8_t*)grayscale_img.data;
	uint8_t* origin_pixel_ptr = (uint8_t*)origin_img.data;
	int cn = origin_img.channels();

	for (int i = 0; i < origin_img.rows; i++)
	{
		for (int j = 0; j < origin_img.cols; j++)
		{
			int idx = i * origin_img.cols * cn + j * cn;
			gs_pixel_ptr[idx] = origin_pixel_ptr[idx];
			gs_pixel_ptr[idx + 1] = origin_pixel_ptr[idx];
			gs_pixel_ptr[idx + 2] = origin_pixel_ptr[idx];

		}
	}
	return grayscale_img;
}

void get_gs_points(const cv::Mat& origin_img, int gs, vector<cv::Point>& points) {

	int cn = origin_img.channels();

	points.clear();

	for (int x = 0; x < origin_img.rows; x++) {
		for (int y = 0; y < origin_img.cols; y++) {
			if (get_pixel_color(origin_img, x, y)[0] == gs) {
				points.push_back({ y,x });
			}
		}
	}

}
void draw_points(const vector<cv::Point>& points, cv::Mat& output, cv::Vec3f color) {
	for (auto point : points) {
		cv::circle(output, { point.x, point.y }, 1, color , 1);
	}
}

void draw_contour(const vector<int>& seq, const vector<cv::Point> points, cv::Mat& output, bool printIdx, cv::Vec3f color) {
	if (seq.size() != points.size()) {
		cout << "error: sequence array size and points size does not match" << endl;
		return;
	}
	if (seq.size() < 3) return;

	for (int i = 0; i < seq.size() - 1; i++) {
		if (printIdx) {
			cv::putText(output, std::to_string(seq[i]), points[seq[i]], cv::FONT_HERSHEY_COMPLEX, .5, { 255,255,255 }, 1);
			cout << seq[i] << points[seq[i]] << "-" << seq[i + 1] << points[seq[i + 1]] << endl;
		}
		cv::line(output, points[seq[i]], points[seq[i + 1]], color , 1);
	}
	int last = seq.size() - 1;
	if (printIdx) {
		cv::putText(output, std::to_string(seq[last]), points[seq[last]], cv::FONT_HERSHEY_COMPLEX, .5, { 255,255,255 }, 1);
		cout << seq[seq.size() - 1] << points[seq[seq.size() - 1]] << "-" << seq[0] << points[0] << endl;
	}
	cv::line(output, points[seq[last]], points[seq[0]], color, 1);
}

void draw_contour(const vector<cv::Point> points, cv::Mat& output, bool printIdx, cv::Vec3f pointColor, cv::Vec3f segColor) {
	vector<int> seq;
	for (int i = 0; i < points.size(); i++) {
		seq.push_back(i);
	}
	draw_contour(seq, points, output, printIdx, segColor);
	draw_points(points, output, pointColor);
}

void draw_seg(vector<cv::Point> points , cv::Mat& output, bool printIdx, cv::Vec3f pointColor, cv::Vec3f segColor) {

	if (points.size() < 3) return;

	for (int i = 0; i < points.size() - 1; i++) {
		if (printIdx) {
			cv::putText(output, std::to_string(i), points[i], cv::FONT_HERSHEY_COMPLEX, .5, { 255,255,255 }, 1);
			cout << i << points[i] << "-" << i+1 << points[i+1] << endl;
		}
		cv::line(output, points[i], points[i+1], segColor, 1);
	}
	draw_points(points, output, pointColor);
}
