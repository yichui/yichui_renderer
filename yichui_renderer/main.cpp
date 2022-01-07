// yichui_renderer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
#include <iostream>
#include <limits>
#include "Core/model.h"
#include "Core/geometry.h"
#include "Core/tgaimage.h"
#include <vector> 
#include <cmath>
#include <cstdlib>
#include <limits>
#include <algorithm>
using namespace std;

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

const int width = 800;
const int height = 800;

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) 
{
	//Bresenham's line algorithm

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



/// <summary>
/// 叉乘
/// </summary>
/// <param name="v1"></param>
/// <param name="v2"></param>
/// <returns></returns>
Vec3f cross(const Vec3f& v1, const Vec3f& v2) {
	return Vec3f{v1.y* v2.z - v1.z * v2.y, v1.z* v2.x - v1.x * v2.z, v1.x* v2.y - v1.y * v2.x};
}

//Vec3f barycentric(Vec2i pts[3], Vec2i P) {
//	Vec3f u = cross(Vec3f(pts[2].u - pts[0].u, pts[1].u - pts[0].u, pts[0].u - P.u), Vec3f(pts[2].v - pts[0].v, pts[1].v - pts[0].v, pts[0].v - P.v));
//	if (std::abs(u.z) < 1) return Vec3f(-1, 1, 1);
//	return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
//}
//
//Vec3f barycentric(Vec3i pts[3], Vec3i P) {
//	Vec3f u = cross(Vec3f(pts[2].u - pts[0].u, pts[1].u - pts[0].u, pts[0].u - P.u), Vec3f(pts[2].v - pts[0].v, pts[1].v - pts[0].v, pts[0].v - P.v));
//	if (std::abs(u.z) < 1) return Vec3f(-1, 1, 1);
//	return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
//}

//计算重心坐标
Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
	Vec3f s[2];
	//计算[AB,AC,PA]的x和y分量
	for (int i = 2; i--; ) {
		s[i][0] = C[i] - A[i];
		s[i][1] = B[i] - A[i];
		s[i][2] = A[i] - P[i];
	}
	//[u,v,1]和[AB,AC,PA]对应的x和y向量都垂直，所以叉乘
	Vec3f u = cross(s[0], s[1]);
	//三点共线时，会导致u[2]为0，此时返回(-1,1,1)
	if (std::abs(u[2]) > 1e-2)
		//若1-u-v，u，v全为大于0的数，表示点在三角形内部
		return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
	return Vec3f(-1, 1, 1);
}

//世界坐标转屏幕坐标
Vec3f world2screen(Vec3f v) {
	return Vec3f(int((v.x + 1.) * width / 2. + .5), int((v.y + 1.) * height / 2. + .5), v.z);
}

//绘制三角形(坐标数组，zbuffer指针，tga指针，颜色)
void triangle(Vec3f* pts, float* zbuffer, TGAImage& image, TGAColor color) {
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
	//确定三角形的边框
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
		}
	}
	Vec3f P;
	//遍历三角形内的每一个像素点
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			////计算重心
			//if (P.x > 600 && P.y > 500)
			//{
			//	P.x += 0.01;
			//}
			//bc_screen就是重心坐标
			//P点对于三角形ABC的质心坐标(1-u-v, u, v)
			Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);
			//质心坐标有一个负值，说明点在三角形外
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
			P.z = 0;
			//计算zbuffer
			//在遍历中比较每一个像素点的z轴坐标与该位置的深度缓存并更新
			for (int i = 0; i < 3; i++)
			{
				//每个顶点的z值乘上对应的重心坐标分量
				P.z += pts[i].z * bc_screen[i];
			}
			if (zbuffer[int(P.x + P.y * width)] < P.z) {
				zbuffer[int(P.x + P.y * width)] = P.z;
				image.set(P.x, P.y, color);
			}
		}
	}
}



int main(int argc, char** argv) {
	Model* model = NULL;
	if (2 == argc) {
		model = new Model(argv[1]);
	}
	else {
		model = new Model("Obj/african_head/african_head.obj");
	}
	TGAImage image(width, height, TGAImage::RGB);

	//创建zbuffer，大小为画布大小
	float* zbuffer = new float[width * height];
	//初始化zbuffer，设定一个很小的值
	for (int i = width * height; i >= 0; i--)
		zbuffer[i] = std::numeric_limits<float>::min();

	Vec3f light_dir(0, 0, -1);
	//遍历模型每个三角面
	for (int i = 0; i < model->nfaces(); i++) {
		std::vector<int> face = model->face(i);
		Vec3f screen_coords[3]; //屏幕坐标系
		Vec3f world_coords[3]; //世界坐标系
		for (int j = 0; j < 3; j++) 
		{
			
			Vec3f v = model->vert(face[j]);
			//将面所包含的三个顶点转成屏幕坐标
			screen_coords[j] = world2screen(model->vert(face[j]));
			//存储世界坐标方便后续使用
			world_coords[j] = v;
		}
		//用世界坐标计算法向量
		//cross叉乘
		Vec3f n = cross((world_coords[2] - world_coords[0]), (world_coords[1] - world_coords[0]));
		n.normalize();
		float intensity = n * light_dir;//光照强度=法向量*光照方向，即法向量和光照方向重合时，亮度最高
		//强度小于0，说明平面朝向为内  即背面裁剪
		if (intensity > 0) {
			triangle(screen_coords, zbuffer, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
		}
	}

	image.flip_vertically();
	image.write_tga_file("framebuffer5.tga");
	return 0;
}

/**
 * learn from:
 * 1.https://zhuanlan.zhihu.com/p/399056546
 * 2.https://github.com/ssloy/tinyrenderer/wiki
 */