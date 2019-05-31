#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

void Model::load_texture(std::string filename, const char *suffix, TGAImage &img)
{
	std::string texfile(filename);
	size_t dot = texfile.find_last_of(".");
	if (dot != std::string::npos) {
		texfile = texfile.substr(0, dot) + std::string(suffix);
		std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
		img.flip_vertically();
	}
}

void Model::load_texture(std::string filename, TGAImage &img)
{
	std::cerr << "texture file " << filename << " loading " << (img.read_tga_file(filename.c_str()) ? "ok" : "failed") << std::endl;
	img.flip_vertically();
}

Model::Model(const char *filename) : verts_(), faces_() {
	std::ifstream in;
	in.open(filename, std::ifstream::in);
	if (in.fail()) return;
	std::string line;
	while (!in.eof()) {
		std::getline(in, line);
		std::istringstream iss(line.c_str());
		char trash;
		if (!line.compare(0, 2, "v ")) {
			iss >> trash;
			Vec3f v;
			for (int i = 0; i < 3; i++) iss >> v.raw[i];
			verts_.push_back(v);
		}
		else if (!line.compare(0, 3, "vt ")) {
			iss >> trash >> trash;
			Vec2f uv;
			for (int i = 0; i < 2; i++) iss >> uv.raw[i];
			uv_.push_back(uv);
		}
		else if (!line.compare(0, 3, "vn ")) {
			iss >> trash >> trash;
			Vec3f n;
			for (int i = 0; i < 3; i++) iss >> n.raw[i];
			norms_.push_back(n);
		}
		else if (!line.compare(0, 2, "f ")) {
			std::vector<Vec3i> f;
			Vec3i tmp;
			iss >> trash;
			while (iss >> tmp.raw[0] >> trash >> tmp.raw[1] >> trash >> tmp.raw[2]) {
				for (int i = 0; i<3; i++) tmp.raw[i]--; // in wavefront obj all indices start at 1, not zero
				f.push_back(tmp);
			}
			faces_.push_back(f);
		}
	}
	std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << std::endl;

	load_texture(filename, "_diffuse.tga", diffusemap_);

	// using grid texture.
	//load_texture("F:\\workdir\\personal\\Rasterizer\\Resource\\grid.tga", diffusemap_);
	load_texture(filename, "_nm.tga", normalmap_);
	//load_texture(filename, "_nm_tangent.tga", normalmap_);
	load_texture(filename, "_spec.tga", specularmap_);
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
	for (int i = 0; i < (int)faces_[idx].size(); i++) face.push_back(faces_[idx][i].raw[0]);
	return face;
}

Vec3f Model::vert(int i) {
	return verts_[i];
}

Vec3f Model::vert(int iface, int nthvert)
{
	return verts_[faces_[iface][nthvert].raw[0]];
}

Vec2f Model::uv(int iface, int nthvert)
{
	return uv_[faces_[iface][nthvert].raw[1]];
}

Vec3f Model::norm(int iface, int nthvert)
{
	return norms_[faces_[iface][nthvert].raw[2]].normalize();
}

Vec3f Model::normal(Vec2f uvf) 
{
	Vec2i uv(uvf.x * normalmap_.get_width(), uvf.y * normalmap_.get_height());
	TGAColor c = normalmap_.get(uv.x, uv.y);
	Vec3f res;
	for (int i = 0; i < 3; i++)
	{
		res.raw[2 - i] = (float)c.bgra[i] / 255.f*2.f - 1.f;
	}

	return res;
}

TGAColor Model::diffuse(Vec2f uvf)
{
	Vec2i uv(uvf.raw[0] * diffusemap_.get_width(), uvf.raw[1] * diffusemap_.get_height());
	return diffusemap_.get(uv.raw[0], uv.raw[1]);
}

float Model::specular(Vec2f uvf) {
	Vec2i uv(uvf.raw[0] * specularmap_.get_width(), uvf.raw[1] * specularmap_.get_height());
	return specularmap_.get(uv.raw[0], uv.raw[1]).bgra[0] / 1.f;
}
