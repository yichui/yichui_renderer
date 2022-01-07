#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), faces_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail())
    {
        std::cerr << "open file failed"  << std::endl;
        return;
    }
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        ////使用输入控制流来控制读取，c_str提取首字符地址。
        std::istringstream iss(line.c_str());
        char trash;
        //compare函数比较从0开始的2个字符，若等于之后字符串的内容则返回0，故表达式表示该行字符串以“v ”开头则为true。
        if (!line.compare(0, 2, "v ")) {
            //>>读取的是字符，此处将“v ”内容跳过，读取后续信息，对不同类型数据有不同的重载
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v[i];
            verts_.push_back(v);
		}
		else if (!line.compare(0, 3, "vn ")) {
			iss >> trash >> trash;
			Vec3f n;
			for (int i = 0; i < 3; i++) iss >> n[i];
			norms_.push_back(n);
		}
		else if (!line.compare(0, 3, "vt ")) {
			iss >> trash >> trash;
			Vec2f uv;
			for (int i = 0; i < 2; i++) iss >> uv[i];
			uv_.push_back(uv);
		}
		else if (!line.compare(0, 2, "f ")) {
            std::vector<Vec3i> f;
            Vec3i tmp;
            int itrash, idx;
            iss >> trash;
			//此处的循环需要读取的是x/x/x中第一个数字，代表三维坐标轴数据，所以通过下述while条件即可每次获取idx内容。
            //通过改变一定的顺序即可获取其他位置的数字，比如第二个x就是我们需要的材质文件像素点vt坐标。
			while (iss >> tmp[0] >> trash >> tmp[1] >> trash >> tmp[2]) {
				for (int i = 0; i < 3; i++) tmp[i]--; // in wavefront obj all indices start at 1, not zero
				f.push_back(tmp);
			}
			faces_.push_back(f);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# "  << faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

std::vector<int> Model::face(int idx) {
	std::vector<int> face;
	for (int i = 0; i < (int)faces_[idx].size(); i++) face.push_back(faces_[idx][i][0]);
	return face;
}

Vec3f Model::vert(int i) {
    return verts_[i];
}
