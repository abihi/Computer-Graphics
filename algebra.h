#ifndef _ALGEBRA_H_
#define _ALGEBRA_H_

typedef struct { float x, y, z; } Vector;
typedef struct { float x, y, z, w; } HomVector;

/* Column-major order are used for the matrices here to be compatible with OpenGL.
** The indices used to access elements in the matrices are shown below.
**  _                _
** |                  |
** |   0   4   8  12  |
** |                  |
** |   1   5   9  13  |
** |                  |
** |   2   6  10  14  |
** |                  |
** |   3   7  11  15  |
** |_                _|
*/
typedef struct matrix { float e[16]; } Matrix;

float Distance(Vector a, Vector b);
Vector Add(Vector a, Vector b);
Vector Subtract(Vector a, Vector b);
Vector CrossProduct(Vector a, Vector b);
float DotProduct(Vector a, Vector b);
float Length(Vector a);
float HomLength(HomVector a);
Vector Normalize(Vector a);
HomVector HomNormalize(HomVector a);
Vector ScalarVecMul(float t, Vector a);
HomVector MatVecMul(Matrix a, Vector b);
Vector Homogenize(HomVector a);
Matrix MatMatMul(Matrix a, Matrix b);
void PrintMatrix(char *name, Matrix m);
void PrintVector(char *name, Vector v);
void PrintHomVector(char *name, HomVector h);
Matrix Translation(float Tx, float Ty, float Tz);
Matrix Scale(float sx, float sy, float sz);
Matrix RotateX(float angle);
Matrix RotateY(float angle);
Matrix RotateZ(float angle);
Matrix Rotate(Vector a);
Matrix PerspectiveProjection(double fov, double aspect, double nearPlane, double farPlane);
Matrix InverseProjection(double fov, double nearPlane, double farPlane);
Matrix OrthoProjection(double left, double right, double bottom, double top, double nearPlane, double farPlane);
Matrix ModelCompositeW(HomVector T, HomVector R, HomVector S);
Matrix ModelOrigin();
Matrix IdentityMatrix();
#endif

