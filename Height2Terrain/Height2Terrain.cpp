#include <fstream>
#include "Primitives.h"
#include "OBJ_File_Helper.h"

#include <opencv2/opencv.hpp>

using std::cout;
using std::endl;

#define LEFT_KEYCODE 2424832 
#define UP_KEYCODE 2490368 
#define RIGHT_KEYCODE 2555904 
#define DOWN_KEYCODE 2621440


void test_OBJ() {
	Vertex** vertices = new Vertex * [4];
	vertices[0] = new Vertex{ -1,0,1 };
	vertices[1] = new Vertex{ 1,1,1 };
	vertices[2] = new Vertex{ 1,0,-1 };
	vertices[3] = new Vertex{ -1,0,-1 };

	std::ofstream ofs{ "output.obj" };

	vector<unsigned int> face{ vertices[0]->id, vertices[1]->id, vertices[2]->id, vertices[3]->id };
	ofs << OBJ_File_Helper::create_OBJ_string(vertices, 4, &face, 1);

	ofs.close();

	delete[] vertices;
}

cv::Scalar get_color_pixel(const cv::Mat img, int x, int y) {
	int cn = img.channels();
	uint8_t* pixel_ptr = (uint8_t*)img.data;
	int idx = x * img.cols * cn + y * cn;
	return (pixel_ptr[idx], pixel_ptr[idx + 1], pixel_ptr[idx + 2] );
}

cv::Mat get_blue_cn(const cv::Mat& origin_img) {

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
			gs_pixel_ptr[idx+1] = origin_pixel_ptr[idx];
			gs_pixel_ptr[idx+2] = origin_pixel_ptr[idx];

		}
	}
	return grayscale_img;
}

void slice_img(const cv::Mat& origin_img, cv::Mat& output, int gs) {

	int cn = origin_img.channels();

	for (int x = 0; x < origin_img.rows; x++) {
		for (int y = 0; y < origin_img.cols; y++) {
			if (get_color_pixel(origin_img, x, y)[0] == gs) {
				cv::circle(output, { y,x }, 1, { 200,200,200 }, 1);
			}
		}
	}
	
}

cv::Mat get_ct_img(unsigned int* gs_weight, int gs_weight_size, int ct_count, const cv::Mat& img) {
	if (ct_count > gs_weight_size) {
		cout << "contour line count exceed limit" << endl;
		exit(-1);
	}
	cv::Mat ct_img(img.rows, img.cols, CV_8UC3, cv::Scalar(10, 10, 10));

	int cn = img.channels();

	for (int i = 0; i < ct_count; i++) {
		for (int x = 0; x < img.rows; x++) {
			for (int y = 0; y < img.cols; y++) {
				
			}
		}
	}

	return ct_img;
}

std::pair<cv::Mat, cv::Mat> get_gs_distr(const cv::Mat& img) {

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
			cur_val = origin_pixel_ptr[idx];//get blue channel value
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
	return std::make_pair(gs_distr_img, get_ct_img(gs_weight, 256, 10, img));
}
struct CV_Mouse_Callback {
	int x;
	void CallBackFunc(int event, int x, int y, int flags, void* userdata)
	{
		if (event == cv::EVENT_LBUTTONDOWN)
		{
			cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
		}
	}
};

struct CV_Runtim_Data {
	int x = 0, y = 0;
	void CallBackFunc(int event, int x, int y, int flags, void* userdata)
	{
		if (event == cv::EVENT_LBUTTONDOWN)
		{
			cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
		}
	}
};

void test_gs() {

	cv::Mat img = cv::imread("heightmap1.png");
	if (img.empty())
	{
		cout << "Could not open or find the image" << endl;
		exit(-1);
	}

	cv::namedWindow("Original_img");	
	cv::namedWindow("gs_distr_output");
	cv::namedWindow("gs_slice_output");

	std::pair<cv::Mat, cv::Mat> gs_img_pair = get_gs_distr(img);
	cv::Mat gs_dstr_img;
	gs_img_pair.first.copyTo(gs_dstr_img);
	cv::Mat sliced_img(img.rows, img.cols, CV_8UC3, cv::Scalar(0, 0, 0));
	int cur_gs = 0;

	slice_img(img, sliced_img, cur_gs);

	cv::imshow("Original_img", img);
	cv::imshow("gs_distr_output", gs_dstr_img);
	cv::imshow("gs_slice_output", sliced_img);

	cv::moveWindow("Original_img", 0, 0);
	cv::moveWindow("gs_distr_output", img.cols, 0);
	cv::moveWindow("gs_slice_output", 0, img.rows);
	int cur_key;
	while (1) {
		cur_key = cv::waitKeyEx(0);
		if (cur_key == UP_KEYCODE) {
			cur_gs += 1;
			cur_gs = cur_gs < 255 ? cur_gs : 255;
			sliced_img.setTo(cv::Scalar(0, 0, 0));
			slice_img(img, sliced_img, cur_gs);
		}
		if (cur_key == 'w') {
			cur_gs += 10;
			cur_gs = cur_gs < 255 ? cur_gs : 255;
			sliced_img.setTo(cv::Scalar(0, 0, 0));
			slice_img(img, sliced_img, cur_gs);
		}
		if (cur_key == DOWN_KEYCODE) {
			cur_gs -= 1;
			cur_gs = cur_gs > 0 ? cur_gs : 0;
			sliced_img.setTo(cv::Scalar(0, 0, 0));
			slice_img(img, sliced_img, cur_gs);
		}
		if (cur_key == 's') {
			cur_gs -= 10;
			cur_gs = cur_gs > 0 ? cur_gs : 0;
			sliced_img.setTo(cv::Scalar(0, 0, 0));
			slice_img(img, sliced_img, cur_gs);
		}
		cv::imshow("gs_slice_output", sliced_img);
		gs_img_pair.first.copyTo(gs_dstr_img);
		cv::line(gs_dstr_img, { cur_gs*4+1,0 }, { cur_gs*4+1, gs_dstr_img.rows }, { 255,0,0 }, 1);
		cv::imshow("gs_distr_output", gs_dstr_img);

		if (cur_key == 27) break;
	}

	cv::destroyWindow("Original_img");
	cv::destroyWindow("gs_distr_output");
	cv::destroyWindow("gs_slice_output");


}

int main()
{
	test_gs();
	return 0;
}