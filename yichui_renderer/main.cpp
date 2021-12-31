// yichui_renderer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
#include <iostream>
#include <limits>
#include "Core/model.h"
#include "Core/geometry.h"
#include "Core/tgaimage.h"


const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

const int width = 800;
const int height = 800;

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) 
{
	//Bresenham's line algorithm

	//1.first attempt
	/*for (float t = 0.0; t < 1.0; t += 0.01) {
		int x = x0 + (x1 - x0) * t;
		int y = y0 + (y1 - y0) * t;
		image.set(x, y, color);
	}*/

	//2.second attempt
	/*for (int x = x0; x <= x1; x++) {
		float t = (x - x0) / (float)(x1 - x0);
		int y = y0 * (1. - t) + y1 * t;
		image.set(x, y, color);
	}*/

	//3.third attempt
	//bool steep = false;
	//if (std::abs(x0 - x1) < std::abs(y0 - y1)) { // if the line is steep, we transpose the image 
	//	std::swap(x0, y0);
	//	std::swap(x1, y1);
	//	steep = true;
	//}
	//if (x0 > x1) { // make it left−to−right 
	//	std::swap(x0, x1);
	//	std::swap(y0, y1);
	//}
	//for (int x = x0; x <= x1; x++) {
	//	float t = (x - x0) / (float)(x1 - x0);
	//	int y = y0 * (1. - t) + y1 * t;
	//	if (steep) {
	//		image.set(y, x, color); // if transposed, de−transpose 
	//	}
	//	else {
	//		image.set(x, y, color);
	//	}
	//}

	//4.four attempt
	/*bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	int dx = x1 - x0;
	int dy = y1 - y0;
	float derror = std::abs(dy / float(dx));
	float error = 0;
	int y = y0;
	for (int x = x0; x <= x1; x++) {
		if (steep) {
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
		error += derror;
		if (error > .5) {
			y += (y1 > y0 ? 1 : -1);
			error -= 1.;
		}
	}*/


	//5. fifth and final attempt
	bool steep = false; 
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	int dx = x1 - x0;
	int dy = y1 - y0;
	int derror2 = std::abs(dy) * 2;
	int error2 = 0;
	int y = y0;
	for (int x = x0; x <= x1; x++) {
		if (steep) {
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
		error2 += derror2;
		if (error2 > dx) {
			y += (y1 > y0 ? 1 : -1);
			error2 -= dx * 2;
		}
	}
}

//test draw line
//int main(int argc, char** argv) 
//{
//	TGAImage image(width, height, TGAImage::RGB);
//	//image.set(52, 41, red);
//
//	//设置图片从左下角开始绘制
//	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
//
//	// 斜率大于 1 
//	line(0, 0, 10, 80, image, white);
//	// 斜率小于 1 大于 0
//	line(0, 0, 50, 20, image, red);
//	// 斜率小于 0
//	line(0, 50, 40, 0, image, blue);
//	image.write_tga_file("output5.tga");
//
//	return 0;
//}

int main(int argc, char** argv) {
	Model* model = NULL;
	if (2 == argc) {
		model = new Model(argv[1]);
	}
	else {
		model = new Model("Obj/african_head/african_head.obj");
		//model = new Model("Obj/floor.obj");
	}

	TGAImage image(width, height, TGAImage::RGB);
	for (int i = 0; i < model->nfaces(); i++) {
		std::vector<int> face = model->face(i);
		for (int j = 0; j < 3; j++) {
			Vec3f v0 = model->vert(face[j]);
			Vec3f v1 = model->vert(face[(j + 1) % 3]);
			int x0 = (v0.x + 1.) / 2. * width ;
			int y0 = (v0.y + 1.) / 2. * height;
			int x1 = (v1.x + 1.) / 2. * width ;
			int y1 = (v1.y + 1.) / 2. * height ;
		
			line(x0, y0, x1, y1, image, blue);
		}
	}
	image.flip_vertically(); //设置图片从左下角开始绘制
	image.write_tga_file("outputMODEL.tga");
	delete model;
	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
