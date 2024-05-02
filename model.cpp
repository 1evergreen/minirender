#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), faces_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v.raw[i];
            verts_.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            Face f;
            Face uv;
            int itrash, idx, idx2;
            iss >> trash;
            for(int i=0; i<3; i++) {
                iss >> idx >> trash >> idx2 >> trash >> itrash;
                idx--; // in wavefront obj all indices start at 1, not zero
                idx2--;
                f.idx[i] = idx;
                uv.idx[i]= idx2;
            }
            faces_.push_back(f);
            uv_faces_.push_back(uv);
        }
        else if(!line.compare(0, 4, "vt  "))
        {
            iss >> trash >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v.raw[i];
            uv_.push_back(v);
        }
    }
    std::cerr << "# v# " << verts_.size() 
    << "# vt# " << uv_.size()
    << " f# "  << faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

const Face&  Model::face(int idx) const {
    return faces_[idx];
}

const Face&  Model::uv_face(int idx) const {
    return uv_faces_[idx];
}
Vec3f Model::vert(int i) {
    return verts_[i];
}
Vec3f Model::uv(int i) {
    return uv_[i];
}

void Model::load_texture(std::string filename){
    texture_.read_tga_file(filename.c_str());
    texture_.flip_vertically();
}
TGAColor Model::diffuse(int x, int y) 
{
    return texture_.get(x, y);
}