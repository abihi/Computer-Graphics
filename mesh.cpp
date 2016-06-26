#include <stdlib.h>
#include <stdio.h>
#include "mesh.h"

void insertModel(Mesh **list, int nv, float * vArr, int nt, int * tArr, float scale, Material material) {
	Mesh * mesh = (Mesh *) malloc(sizeof(Mesh));
	Vector CenterPoint = { 0, 0, 0 };
	Vector meshradius;
	Vector c = { 0, 0, 0 };
	Vector BoxMax = { 0, 0, 0 };
	Vector BoxMin = { 0, 0, 0 };
	float MaxDistance = 0;

	mesh->T = { 0.0f, 0.0f, 0.0f, 1.0f };
	mesh->R = { 0.0f, 0.0f, 0.0f, 1.0f };
	mesh->S = { 1.0f, 1.0f, 1.0f, 1.0f };

	// set mesh material
	mesh->material = material;

	mesh->nv = nv;
	mesh->nt = nt;
	mesh->vertices = (Vector *)malloc(nv * sizeof(Vector));
	mesh->vnorms = (Vector *)malloc(nv * sizeof(Vector));
	mesh->triangles = (Triangle *)malloc(nt * sizeof(Triangle));

	// set mesh vertices
	for (int i = 0; i < nv; i++) {
		mesh->vertices[i].x = vArr[i * 3] * scale;
		mesh->vertices[i].y = vArr[i * 3 + 1] * scale;
		mesh->vertices[i].z = vArr[i * 3 + 2] * scale;
	}

	// set mesh triangles
	for (int i = 0; i < nt; i++) {
		mesh->triangles[i].vInds[0] = tArr[i * 3];
		mesh->triangles[i].vInds[1] = tArr[i * 3 + 1];
		mesh->triangles[i].vInds[2] = tArr[i * 3 + 2];
	}

	// Assignment 1: 
	// Calculate and store suitable vertex normals for the mesh here.
	// Replace the code below that simply sets some arbitrary normal values	
	for (int i = 0; i < nv; i++) {
		Vector temp = { 0, 0, 0 };
		for (int j = 0; j < nt; j++){
			for (int k = 0; k < 3; k++)
			{
				if (mesh->triangles[j].vInds[k] == i)
				{
					// Get 2 vectors between a triangles vertices
					Vector v1 = Subtract(mesh->vertices[mesh->triangles[j].vInds[1]], mesh->vertices[mesh->triangles[j].vInds[0]]);
					Vector v2 = Subtract(mesh->vertices[mesh->triangles[j].vInds[2]], mesh->vertices[mesh->triangles[j].vInds[0]]);

					// calculate triangle normal
					Vector normal = CrossProduct(v1, v2);
					normal = Normalize(normal);

					// Store all the triangle normals
					temp = Add(temp, normal);
					break;
				}
			}
		}
		mesh->vnorms[i] = Normalize(temp);
	}

	// Find the greatest distance
	for (int i = 0; i < mesh->nv; i++) {

		c.x += mesh->vertices[i].x;
		c.y += mesh->vertices[i].y;
		c.z += mesh->vertices[i].z;

	}
	c.x /= mesh->nv;
	c.y /= mesh->nv;
	c.z /= mesh->nv;

	mesh->modelCenterPoint = { c.x, c.y, c.z };

	float distance = 0.0f;
	float dtmp = 0.0f;
	for (int i = 0; i < mesh->nv; i++) {
		Vector tmp = mesh->vertices[i];
		tmp = Subtract(tmp, mesh->modelCenterPoint);
		dtmp = Length(tmp);
		if (dtmp >= distance){
			distance = dtmp;
		}
	}
	mesh->modelRadius = distance;

	mesh->next = *list;
	*list = mesh;	
}

Material createMaterial(Vector KD, Vector KA, Vector KS)
{
	Material material;
	material.diffuse = KD;
	material.ambient = KA;
	material.specular = KS;
	return material;
}