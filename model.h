#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
#include "tgaimage.h"

struct Face {
	int idx[3];
	int operator[](int i) const { return idx[i];}
};

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<Vec3f> uv_;
	std::vector<Face> faces_;
	std::vector<Face> uv_faces_;

	TGAImage texture_;

public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	Vec3f uv(int i);
	const Face& face(int idx) const;
	const Face& uv_face(int idx) const;
	TGAColor diffuse(int x, int y) ;	
	void load_texture(std::string filename);
	const TGAImage& texture() const {return texture_;}
};

#endif //__MODEL_H__
