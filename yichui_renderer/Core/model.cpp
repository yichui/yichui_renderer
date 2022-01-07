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
        ////ʹ����������������ƶ�ȡ��c_str��ȡ���ַ���ַ��
        std::istringstream iss(line.c_str());
        char trash;
        //compare�����Ƚϴ�0��ʼ��2���ַ���������֮���ַ����������򷵻�0���ʱ��ʽ��ʾ�����ַ����ԡ�v ����ͷ��Ϊtrue��
        if (!line.compare(0, 2, "v ")) {
            //>>��ȡ�����ַ����˴�����v ��������������ȡ������Ϣ���Բ�ͬ���������в�ͬ������
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
			//�˴���ѭ����Ҫ��ȡ����x/x/x�е�һ�����֣�������ά���������ݣ�����ͨ������while��������ÿ�λ�ȡidx���ݡ�
            //ͨ���ı�һ����˳�򼴿ɻ�ȡ����λ�õ����֣�����ڶ���x����������Ҫ�Ĳ����ļ����ص�vt���ꡣ
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
