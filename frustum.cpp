#include "algebra.h"
#include "frustum.h"
#include <math.h> 
#include <stdio.h>

bool checkIfInside(Vector Center, float radius, Vector plane, float w)
{
	float len, distance;

	len = Length(plane);
	plane = Normalize(plane);
	w = w / len;

	distance = DotProduct(Center, plane);
	distance = distance + w;
	//printf("dot %f rad %f\n", distance, radius);

	if (distance > -radius)	
		return true;

	return false;
}

bool createFrustumPlane(Mesh *mesh, Matrix MM)
{
	//PrintMatrix("Frustrum Matrix", MM);
	HomVector plane0, plane1, plane2, plane3, plane4, plane5;

	// Left
	plane0.x = MM.e[3] - MM.e[0];
	plane0.y = MM.e[7] - MM.e[4];
	plane0.z = MM.e[11] - MM.e[8];
	plane0.w = MM.e[15] - MM.e[12];
	if (!checkIfInside(mesh->modelCenterPoint, mesh->modelRadius, {plane0.x, plane0.y, plane0.z}, plane0.w))
		return false;

	// Right
	plane1.x = MM.e[3] + MM.e[0];
	plane1.y = MM.e[7] + MM.e[4];
	plane1.z = MM.e[11] + MM.e[8];
	plane1.w = MM.e[15] + MM.e[12];
	if (!checkIfInside(mesh->modelCenterPoint, mesh->modelRadius, { plane1.x, plane1.y, plane1.z }, plane1.w))
		return false;

	// Bottom
	plane2.x = MM.e[3] - MM.e[1];
	plane2.y = MM.e[7] - MM.e[5];
	plane2.z = MM.e[11] - MM.e[9];
	plane2.w = MM.e[15] - MM.e[13];
	if (!checkIfInside(mesh->modelCenterPoint, mesh->modelRadius, { plane2.x, plane2.y, plane2.z }, plane2.w))
		return false;

	// Top
	plane3.x = MM.e[3] + MM.e[1];
	plane3.y = MM.e[7] + MM.e[5];
	plane3.z = MM.e[11] + MM.e[9];
	plane3.w = MM.e[15] + MM.e[13];
	if (!checkIfInside(mesh->modelCenterPoint, mesh->modelRadius, { plane3.x, plane3.y, plane3.z }, plane3.w))
		return false;

	// Near
	plane4.x = MM.e[3] - MM.e[2];
	plane4.y = MM.e[7] - MM.e[6];
	plane4.z = MM.e[11] - MM.e[10];
	plane4.w = MM.e[15] - MM.e[14];
	if (!checkIfInside(mesh->modelCenterPoint, mesh->modelRadius, { plane4.x, plane4.y, plane4.z }, plane4.w))
		return false;

	// Far
	plane5.x = MM.e[3] + MM.e[2];
	plane5.y = MM.e[7] + MM.e[6];
	plane5.z = MM.e[11] + MM.e[10];
	plane5.w = MM.e[15] + MM.e[14];
	if (!checkIfInside(mesh->modelCenterPoint, mesh->modelRadius, { plane5.x, plane5.y, plane5.z }, plane5.w))
		return false;

	return true;
}