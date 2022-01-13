﻿#include <vector>
#include <cmath>
#include <limits>

#include "Core/tgaimage.h"
#include "Core/model.h"

#include "Core/geometry.h"

#include <algorithm>
const int width = 800;
const int height = 800;
const int depth = 255;

Model* model = NULL;
int* zbuffer = NULL;
Vec3f light_dir(0.2, 0.15, -1);
Vec3f camera(0, 0, 3);
//摄像机位置
Vec3f eye(2, 1, 3);
//焦点位置
Vec3f center(0, 0, 1);
//4d-->3d
//除以最后一个分量。（当最后一个分量为0，表示向量）
//不为0，表示坐标
Vec3f m2v(Matrix m) {
	return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
}

//3d-->4d
//添加一个1表示坐标
Matrix v2m(Vec3f v) {
	Matrix m(4, 1);
	m[0][0] = v.x;
	m[1][0] = v.y;
	m[2][0] = v.z;
	m[3][0] = 1.f;
	return m;
}

//视角矩阵
//将物体x，y坐标(-1,1)转换到屏幕坐标(100,700)    1/8width~7/8width
//zbuffer(-1,1)转换到0~255
Matrix viewport(int x, int y, int w, int h) {
	Matrix m = Matrix::identity(4);
	//第4列表示平移信息
	m[0][3] = x + w / 2.f;
	m[1][3] = y + h / 2.f;
	m[2][3] = depth / 2.f;
	//对角线表示缩放信息
	m[0][0] = w / 2.f;
	m[1][1] = h / 2.f;
	m[2][2] = depth / 2.f;
	return m;
}

//绘制三角形(坐标1，坐标2，坐标3，顶点光照强度1，顶点光照强度2，顶点光照强度3，tga指针，zbuffer)
void triangle(Vec3i t0, Vec3i t1, Vec3i t2, float ity0, float ity1, float ity2, Vec2i uv0, Vec2i uv1, Vec2i uv2, float dis0, float dis1, float dis2, TGAImage& image, int* zbuffer) {
	//按照y分割为两个三角形
	if (t0.y == t1.y && t0.y == t2.y) return;
	if (t0.y > t1.y) { std::swap(t0, t1); std::swap(ity0, ity1); std::swap(uv0, uv1); }
	if (t0.y > t2.y) { std::swap(t0, t2); std::swap(ity0, ity2); std::swap(uv0, uv2); }
	if (t1.y > t2.y) { std::swap(t1, t2); std::swap(ity1, ity2); std::swap(uv1, uv2); }
	int total_height = t2.y - t0.y;
	for (int i = 0; i < total_height; i++) {
		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
		float alpha = (float)i / total_height;
		float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height;
		//计算A,B两点的坐标
		Vec3i A = t0 + Vec3f(t2 - t0) * alpha;
		Vec3i B = second_half ? t1 + Vec3f(t2 - t1) * beta : t0 + Vec3f(t1 - t0) * beta;
		//计算A,B两点的光照强度
		float ityA = ity0 + (ity2 - ity0) * alpha;
		float ityB = second_half ? ity1 + (ity2 - ity1) * beta : ity0 + (ity1 - ity0) * beta;
		//计算UV
		Vec2i uvA = uv0 + (uv2 - uv0) * alpha;
		Vec2i uvB = second_half ? uv1 + (uv2 - uv1) * beta : uv0 + (uv1 - uv0) * beta;
		//计算距离
		float disA = dis0 + (dis2 - dis0) * alpha;
		float disB = second_half ? dis1 + (dis2 - dis1) * beta : dis0 + (dis1 - dis0) * beta;
		if (A.x > B.x) { std::swap(A, B); std::swap(ityA, ityB); }
		//x坐标作为循环控制
		for (int j = A.x; j <= B.x; j++) {
			float phi = B.x == A.x ? 1. : (float)(j - A.x) / (B.x - A.x);
			//计算当前需要绘制点P的坐标，光照强度
			Vec3i    P = Vec3f(A) + Vec3f(B - A) * phi;
			float ityP = ityA + (ityB - ityA) * phi;
			ityP = std::min(1.f, std::abs(ityP) + 0.01f);
			Vec2i uvP = uvA + (uvB - uvA) * phi;
			float disP = disA + (disB - disA) * phi;
			int idx = P.x + P.y * width;
			//边界限制
			if (P.x >= width || P.y >= height || P.x < 0 || P.y < 0) continue;
			if (zbuffer[idx] < P.z) {
				zbuffer[idx] = P.z;
				TGAColor color = model->diffuse(uvP);
				image.set(P.x, P.y, TGAColor(color.bgra[2], color.bgra[1], color.bgra[0]) * ityP * (20.f / std::pow(disP, 2.f)));
				//image.set(P.x, P.y, TGAColor(255,255,255)* ityP);
			}
		}
	}
}

//朝向矩阵，变换矩阵
//更改摄像机视角=更改物体位置和角度，操作为互逆矩阵
//摄像机变换是先旋转再平移，所以物体需要先平移后旋转，且都是逆矩阵
Matrix lookat(Vec3f eye, Vec3f center, Vec3f up) {
    //计算出z，根据z和up算出x，再算出y
    Vec3f z = (eye - center).normalize();
    Vec3f x = (up ^ z).normalize();
    Vec3f y = (z ^ x).normalize();
    Matrix rotation = Matrix::identity(4);
    Matrix translation = Matrix::identity(4);
    //***矩阵的第四列是用于平移的。因为观察位置从原点变为了center，所以需要将物体平移-center***
    for (int i = 0; i < 3; i++) {
        rotation[i][3] = -center[i];
    }
    //正交矩阵的逆 = 正交矩阵的转置
    //矩阵的第一行即是现在的x
    //矩阵的第二行即是现在的y
    //矩阵的第三行即是现在的z
    //***矩阵的三阶子矩阵是当前视线旋转矩阵的逆矩阵***
    for (int i = 0; i < 3; i++) {
        rotation[0][i] = x[i];
        rotation[1][i] = y[i];
        rotation[2][i] = z[i];
    }
    //这样乘法的效果是先平移物体，再旋转
    Matrix res = rotation*translation;
    return res;
}


int main(int argc, char** argv) {
	//读取模型
	if (2 == argc) {
		model = new Model(argv[1]);
	}
	else {
		model = new Model("Obj/african_head/african_head.obj");
	}
	//构造zbuffer
	zbuffer = new int[width * height];
	for (int i = 0; i < width * height; i++) {
		//初始化zbuffer
		zbuffer[i] = std::numeric_limits<int>::min();
	}

	//绘制
	{
		//模型变换矩阵
		Matrix ModelView = lookat(eye, center, Vec3f(0, 1, 0));
		//初始化投影矩阵
		Matrix Projection = Matrix::identity(4);
		//初始化视角矩阵
		Matrix ViewPort = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
		//投影矩阵[3][2]=-1/c，c为相机z坐标
		//Projection[3][2] = -1.f / camera.z;
		Projection[3][2] = -1.f / (eye - center).norm();

		//构造tga
		//TGAImage image(width, height, TGAImage::RGB);
		////以模型面为循环控制变量
		//for (int i = 0; i < model->nfaces(); i++) {
		//	std::vector<int> face = model->face(i);
		//	Vec3i screen_coords[3];
		//	Vec3f world_coords[3];
		//	for (int j = 0; j < 3; j++) {
		//		Vec3f v = model->vert(face[j]);
		//		//视角矩阵*投影矩阵*坐标
		//		screen_coords[j] = m2v(ViewPort * Projection * v2m(v));
		//		world_coords[j] = v;
		//	}
		//	//计算法向量
		//	Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
		//	n.normalize();
		//	//计算光照
		//	float intensity = n * light_dir;
		//	intensity = std::min(std::abs(intensity), 1.f);
		//	if (intensity > 0) {
		//		Vec2i uv[3];
		//		for (int k = 0; k < 3; k++) {
		//			uv[k] = model->uv(i, k);
		//		}
		//		//绘制三角形
		//		triangle(screen_coords[0], screen_coords[1], screen_coords[2], uv[0], uv[1], uv[2], image, intensity, zbuffer);
		//	}
		//}
		////tga默认原点在左上，现改为左下
		//image.flip_vertically();
		//image.write_tga_file("output_4.tga");

		TGAImage image(width, height, TGAImage::RGB);
		for (int i = 0; i < model->nfaces(); i++) {
			std::vector<int> face = model->face(i);
			Vec3i screen_coords[3];
			float intensity[3];
			float distance[3];
			for (int j = 0; j < 3; j++) {
				Vec3f v = model->vert(face[j]);
				Matrix m_v = ModelView * Matrix(v);
				screen_coords[j] = Vec3f(ViewPort * Projection * m_v);
				intensity[j] = model->norm(i, j) * light_dir;
				Vec3f new_v = Vec3f(m_v);
				distance[j] = std::pow((std::pow(new_v.x - eye.x, 2.0f) + std::pow(new_v.y - eye.y, 2.0f) + std::pow(new_v.z - eye.z, 2.0f)), 0.5f);
			}
			Vec2i uv[3];
			for (int k = 0; k < 3; k++) {
				uv[k] = model->uv(i, k);
			}
			triangle(screen_coords[0], screen_coords[1], screen_coords[2], intensity[0], intensity[1], intensity[2], uv[0], uv[1], uv[2], distance[0], distance[1], distance[2], image, zbuffer);
		}
		image.flip_vertically();
		image.write_tga_file("output6.tga");
	}
	//输出zbuffer
	{
		TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				zbimage.set(i, j, TGAColor(zbuffer[i + j * width]));
			}
		}
		zbimage.flip_vertically();
		zbimage.write_tga_file("zbuffer_6.tga");
	}
	delete model;
	delete[] zbuffer;
	return 0;
}

