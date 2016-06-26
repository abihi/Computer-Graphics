#ifndef _MESH_H_
#define _MESH_H_

#include "algebra.h"

typedef struct _Triangle {
	int vInds[3]; //vertex indices
} Triangle;

typedef struct _Material {
	Vector diffuse;
	Vector ambient;
	Vector specular;
	float shininess;
} Material;

typedef struct _Mesh { 
	int nv;
	Vector *vertices;
	Vector *vnorms;
	int nt;
	Triangle *triangles;
	struct _Mesh *next;
	Vector *tNorms;
	unsigned int vbo, ibo, vao; // OpenGL handles for rendering
	HomVector T, R, S;
	
	Vector modelCenterPoint;
	float modelRadius;

	Material material;

	Vector SphereCenterpoint;
	float SphereRadius;
} Mesh;

typedef struct _Camera {
	Vector position;
	Vector rotation;
	double fov; 
	double nearPlane; 
	double farPlane; 
} Camera;

void insertModel(Mesh ** objlist, int nv, float * vArr, int nt, int * tArr, float scale, Material material);
Material createMaterial(Vector KD, Vector KA, Vector KS);
#endif
