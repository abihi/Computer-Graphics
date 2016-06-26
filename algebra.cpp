#define _USE_MATH_DEFINES // To get M_PI defined
#include <math.h>
#include <stdio.h>
#include <GL\glew.h>
#include <GL\freeglut.h>
#include "algebra.h"
#include "mesh.h"

float Distance(Vector a, Vector b)
{
	return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z) * (a.z - b.z));
}

Vector CrossProduct(Vector a, Vector b) {
	Vector v = { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
	return v;
}

float DotProduct(Vector a, Vector b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

Vector Subtract(Vector a, Vector b) {
	Vector v = { a.x - b.x, a.y - b.y, a.z - b.z };
	return v;
}

Vector Add(Vector a, Vector b) {
	Vector v = { a.x + b.x, a.y + b.y, a.z + b.z };
	return v;
}

float Length(Vector a) {
	return sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
}

float HomLength(HomVector a) {
	return sqrt(a.x*a.x + a.y*a.y + a.z*a.z + a.w*a.w);
}

Vector Normalize(Vector a) {
	float len = Length(a);
	Vector v = { a.x/len, a.y/len, a.z/len };
	return v;
}

HomVector HomNormalize(HomVector a) {
	float len = HomLength(a);
	HomVector v = { a.x / len, a.y / len, a.z / len, a.w / len };
	return v;
}

Vector ScalarVecMul(float t, Vector a) {
	Vector b = { t*a.x, t*a.y, t*a.z };
	return b;
}

HomVector MatVecMul(Matrix a, Vector b) {
	HomVector h;
	h.x = b.x*a.e[0] + b.y*a.e[4] + b.z*a.e[8] + a.e[12];
	h.y = b.x*a.e[1] + b.y*a.e[5] + b.z*a.e[9] + a.e[13];
	h.z = b.x*a.e[2] + b.y*a.e[6] + b.z*a.e[10] + a.e[14];
	h.w = b.x*a.e[3] + b.y*a.e[7] + b.z*a.e[11] + a.e[15];
	return h;
}

Vector Homogenize(HomVector h) {
	Vector a;
	if (h.w == 0.0) {
		fprintf(stderr, "Homogenize: w = 0\n");
		a.x = a.y = a.z = 9999999;
		return a;
	}
	a.x = h.x / h.w;
	a.y = h.y / h.w;
	a.z = h.z / h.w;
	return a;
}

Matrix MatMatMul(Matrix a, Matrix b) {
	Matrix c;
	int i, j, k;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			c.e[j*4+i] = 0.0;
			for (k = 0; k < 4; k++)
				c.e[j*4+i] += a.e[k*4+i] * b.e[j*4+k];
		}
	}
	return c;
}


Matrix Translation(float Tx, float Ty, float Tz)
{
	Matrix V;
	V.e[0] = 1.0f; V.e[4] = 0.0f; V.e[8] = 0.0f; V.e[12] = Tx;
	V.e[1] = 0.0f; V.e[5] = 1.0f; V.e[9] = 0.0f; V.e[13] = Ty;
	V.e[2] = 0.0f; V.e[6] = 0.0f; V.e[10] = 1.0f; V.e[14] = Tz;
	V.e[3] = 0.0f; V.e[7] = 0.0f; V.e[11] = 0.0f; V.e[15] = 1.0f;
	return V;
}

Matrix Scale(float sx, float sy, float sz){
	Matrix V;
	V.e[0] = sx; V.e[4] = 0.0f; V.e[8] = 0.0f; V.e[12] = 1.0f;
	V.e[1] = 0.0f; V.e[5] = sy; V.e[9] = 0.0f; V.e[13] = 1.0f;
	V.e[2] = 0.0f; V.e[6] = 0.0f; V.e[10] = sz; V.e[14] = 1.0f;
	V.e[3] = 0.0f; V.e[7] = 0.0f; V.e[11] = 0.0f; V.e[15] = 1.0f;
	return V;
}

Matrix RotateX(float angle){
	float cosx = cos(angle);
	float sinx = sin(angle);
	Matrix V;
	V.e[0] = 1.0f; V.e[4] = 0.0f; V.e[8] = 0.0f; V.e[12] = 0.0f;
	V.e[1] = 0.0f; V.e[5] = cosx; V.e[9] = sinx; V.e[13] = 0.0f;
	V.e[2] = 0.0f; V.e[6] = -sinx; V.e[10] = cosx; V.e[14] = 0.0f;
	V.e[3] = 0.0f; V.e[7] = 0.0f; V.e[11] = 0.0f; V.e[15] = 1.0f;
	return V;
}

Matrix RotateY(float angle){
	float cosy = cos(angle);
	float siny = sin(angle);
	Matrix V;
	V.e[0] = cosy; V.e[4] = 0.0f; V.e[8] = -siny; V.e[12] = 0.0f;
	V.e[1] = 0.0f; V.e[5] = 1.0f; V.e[9] = 0.0f; V.e[13] = 0.0f;
	V.e[2] = siny; V.e[6] = 0.0f; V.e[10] = cosy; V.e[14] = 0.0f;
	V.e[3] = 0.0f; V.e[7] = 0.0f; V.e[11] = 0.0f; V.e[15] = 1.0f;
	return V;
}

Matrix RotateZ(float angle){
	float cosz = cos(angle);
	float sinz = sin(angle);
	Matrix V;
	V.e[0] = cosz; V.e[4] = sinz; V.e[8] = 0.0f; V.e[12] = 0.0f;
	V.e[1] = -sinz; V.e[5] = cosz; V.e[9] = 0.0f; V.e[13] = 0.0f;
	V.e[2] = 0.0f; V.e[6] = 0.0f; V.e[10] = 1.0f; V.e[14] = 0.0f;
	V.e[3] = 0.0f; V.e[7] = 0.0f; V.e[11] = 0.0f; V.e[15] = 1.0f;
	return V;
}

Matrix Rotate(Vector a)
{
	Matrix XRot = { 1, 0, 0, 0, 0, cos(a.x), sin(a.x), 0, 0, -sin(a.x), cos(a.x), 0, 0, 0, 0, 1 };
	Matrix YRot = { cos(a.y), 0, -sin(a.y), 0, 0, 1, 0, 0, sin(a.y), 0, cos(a.y), 0, 0, 0, 0, 1 };
	Matrix ZRot = { cos(a.z), sin(a.z), 0, 0, -sin(a.z), cos(a.z), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	return MatMatMul(XRot, MatMatMul(YRot, ZRot));
}

Matrix ModelCompositeW(HomVector T, HomVector R, HomVector S){
	Matrix Trans, Sc, TR, Rx, Ry, Rxy, Rz, Rxyz, W;
	Trans = Translation(T.x, T.y, T.z);
	Rx = RotateX(R.x);
	Ry = RotateY(R.y);
	Rz = RotateZ(R.z);
	Sc = Scale(S.x, S.y, S.z);
	Rxy = MatMatMul(Rx, Ry);
	Rxyz = MatMatMul(Rxy, Rz);
	TR = MatMatMul(Trans, Rxyz);
	W = MatMatMul(TR, Sc);
	return W;
}

Matrix ModelOrigin(){
	Vector T = {0.0f, 0.0f, 0.0f}, S = {1.0f, 1.0f, 1.0f}, R = {0.0f, 0.0f, 0.0f};
	Matrix Trans, Sc, TR, Rx, Ry, Rxy, Rz, Rxyz, W;
	Trans = Translation(T.x, T.y, T.z);
	Rx = RotateX(R.x);
	Ry = RotateY(R.y);
	Rz = RotateZ(R.z);
	Sc = Scale(S.x, S.y, S.z);
	Rxy = MatMatMul(Rx, Ry);
	Rxyz = MatMatMul(Rxy, Rz);
	TR = MatMatMul(Trans, Rxyz);
	W = MatMatMul(TR, Sc);
	return W;
}

Matrix PerspectiveProjection(double fov, double aspect, double nearPlane, double farPlane){
	double NplusF = nearPlane + farPlane;
	double FtimesN = farPlane*nearPlane;
	double FminusN = farPlane - nearPlane;
	Matrix P;
	P.e[0] = (float) atan(fov/2) / aspect; P.e[4] = 0.000000f; P.e[8] = 0.000000f; P.e[12] = 0.0f;
	P.e[1] = 0.000000f; P.e[5] = (float) atan(fov/2); P.e[9] = 0.000000f; P.e[13] = 0.0f;
	P.e[2] = 0.000000f; P.e[6] = 0.000000f; P.e[10] = (float) -(NplusF / FminusN); P.e[14] = (float) -(FtimesN / FminusN);
	P.e[3] = 0.000000f; P.e[7] = 0.000000f; P.e[11] = -1.000000f; P.e[15] = 0.0f;
	return P;
}

Matrix OrthoProjection(double left, double right, double bottom, double top, double nearPlane, double farPlane){
	double FplusN = farPlane + nearPlane;
	double FminN = farPlane - nearPlane;
	Matrix P;
	P.e[0] = (float) 2/(right-left); P.e[4] = 0.00000f; P.e[8] = 0.00f; P.e[12] = (float) -((right + left)/(right - left));
	P.e[1] = 0.0000f; P.e[5] = (float) 2/(top-bottom); P.e[9] = 0.000f; P.e[13] =(float) -((top+bottom)/(top-bottom));
	P.e[2] = 0.0000f; P.e[6] = 0.0000f; P.e[10] = (float) 2 / (nearPlane - farPlane); P.e[14] = (float) -(FplusN/FminN);
	P.e[3] = 0.0000f; P.e[7] = 0.0000f; P.e[11] = 0.0000f; P.e[15] = 1.00f;
	PrintMatrix("Matrix P: ", P);
	return P;
}

Matrix InverseProjection(double fov, double nearPlane, double farPlane){
	double NplusF = nearPlane + farPlane;
	double FtimesN = farPlane*nearPlane;
	double FminusN = farPlane - nearPlane;
	Matrix P;
	P.e[0] = (float) tan(fov / 2); P.e[4] = 0.00000f; P.e[8] = 0.00000f; P.e[12] = 0.0f;
	P.e[1] = 0.00000f; P.e[5] = (float) tan(fov / 2); P.e[9] = 0.00000f; P.e[13] = 0.0f;
	P.e[2] = 0.00000f; P.e[6] = 0.00000f; P.e[10] = (float) NplusF / FminusN; P.e[14] = (float) FtimesN / FminusN;
	P.e[3] = 0.00000f; P.e[7] = 0.00000f; P.e[11] = 1.000f; P.e[15] = 0.00f;
	return P;
}

Matrix IdentityMatrix(){
	Matrix V;
	V.e[0] = 1.0f; V.e[4] = 0.0f; V.e[8] = 0.0f; V.e[12] = 0.0f;
	V.e[1] = 0.0f; V.e[5] = 1.0f; V.e[9] = 0.0f; V.e[13] = 0.0f;
	V.e[2] = 0.0f; V.e[6] = 0.0f; V.e[10] = 1.0f; V.e[14] = 0.0f;
	V.e[3] = 0.0f; V.e[7] = 0.0f; V.e[11] = 0.0f; V.e[15] = 1.0f;
	return V;
}

void PrintVector(char *name, Vector a) {
	printf("%s: %6.5lf %6.5lf %6.5lf\n", name, a.x, a.y, a.z);
}

void PrintHomVector(char *name, HomVector a) {
	printf("%s: %6.5lf %6.5lf %6.5lf %6.5lf\n", name, a.x, a.y, a.z, a.w);
}

void PrintMatrix(char *name, Matrix a) { 
	int i,j;

	printf("%s:\n", name);
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			printf("%6.5lf ", a.e[j*4+i]);
		}
		printf("\n");
	}
}




