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

//绘制三角形(坐标1，坐标2，坐标3，tga指针，颜色)
void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, TGAColor color) 
{
	//三角形面积为0
	if (t0.y == t1.y && t0.y == t2.y) return;

	//按照y轴进行排序
	if (t0.y > t1.y) std::swap(t0, t1);
	if (t0.y > t2.y) std::swap(t0, t2);
	if (t1.y > t2.y) std::swap(t1, t2);

	int total_height = t2.y - t0.y;

	//以高度差作为循环控制变量，此时不需要考虑斜率，因为着色完后每行都会被填充
	//绘制三角形上半部分
	//for (int y = t0.y; y <= t1.y; y++) {
	//	int segment_height = t1.y - t0.y + 1;
	//	float alpha = (float)(y - t0.y) / total_height;
	//	float beta = (float)(y - t0.y) / segment_height; // be careful with divisions by zero 
	//	Vec2i A = t0 + (t2 - t0) * alpha;
	//	Vec2i B = t0 + (t1 - t0) * beta;
	//	if (A.x > B.x) std::swap(A, B);
	//	for (int j = A.x; j <= B.x; j++) {
	//		image.set(j, y, color); // attention, due to int casts t0.y+i != A.y 
	//	}
	//}
	////绘制三角形下半部分
	//for (int y = t1.y; y <= t2.y; y++) {
	//	int segment_height = t2.y - t1.y + 1;
	//	float alpha = (float)(y - t0.y) / total_height;
	//	float beta = (float)(y - t1.y) / segment_height; // be careful with divisions by zero 
	//	Vec2i A = t0 + (t2 - t0) * alpha;
	//	Vec2i B = t1 + (t2 - t1) * beta;
	//	if (A.x > B.x) std::swap(A, B);
	//	for (int j = A.x; j <= B.x; j++) {
	//		image.set(j, y, color); // attention, due to int casts t0.y+i != A.y 
	//	}
	//}
	for (int i = 0; i <= total_height; i++) {
		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
		float alpha = (float)i / total_height;
		float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height; // be careful: with above conditions no division by zero here 
		Vec2i A = t0 + (t2 - t0) * alpha;
		Vec2i B = second_half ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;
		if (A.x > B.x) std::swap(A, B);
		for (int j = A.x; j <= B.x; j++) {
			image.set(j, t0.y + i, color); // attention, due to int casts t0.y+i != A.y 
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


//test Model wireframe
//int main(int argc, char** argv) {
//	Model* model = NULL;
//	if (2 == argc) {
//		model = new Model(argv[1]);
//	}
//	else {
//		//model = new Model("Obj/african_head/african_head.obj");
//		model = new Model("Obj/cube.obj");
//	}
//
//	TGAImage image(width, height, TGAImage::RGB);
//	for (int i = 0; i < model->nfaces(); i++) {
//		std::vector<int> face = model->face(i);
//		for (int j = 0; j < 3; j++) {
//			Vec3f v0 = model->vert(face[j]);
//			Vec3f v1 = model->vert(face[(j + 1) % 3]);
//			int x0 = (v0.x + 1.) / 2. * width ;
//			int y0 = (v0.y + 1.) / 2. * height;
//			int x1 = (v1.x + 1.) / 2. * width ;
//			int y1 = (v1.y + 1.) / 2. * height ;
//		
//			line(x0, y0, x1, y1, image, blue);
//		}
//	}
//	image.flip_vertically(); //设置图片从左下角开始绘制
//	image.write_tga_file("outputcube.tga");
//	delete model;
//	return 0;
//}

//draw triangle
int main(int argc, char** argv) {
	TGAImage image(200, 200, TGAImage::RGB);
	Vec2i t0[3] = { Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80) };
	Vec2i t1[3] = { Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180) };
	Vec2i t2[3] = { Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180) };
	triangle(t0[0], t0[1], t0[2], image, red);
	triangle(t1[0], t1[1], t1[2], image, white);
	triangle(t2[0], t2[1], t2[2], image, blue);
	image.flip_vertically();
	image.write_tga_file("output_triangle1.tga");
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
