//#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <GL\glew.h>
#include <GL\freeglut.h>
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <time.h>
#include <cmath>
#include <glm/glm.hpp>

#include "algebra.h"
#include "shaders.h"
#include "mesh.h"
#include "frustum.h"

int OP_PP = 0, mesh_chooser = 0;
int screen_width = 1024;
int screen_height = 768;

double w = 1024, h = 768, ar = w / h;
static float zoomFactor = 1.0; /* Global, if you want. Modified by user input. Initially 1.0 */
float a = 0.0f, b = 0.0f, l = 0.0f;

bool frustumToggle = true;
int ObjectsRendered = 0;
Mesh *meshList = NULL, *activeMesh = NULL; // Pointer to linked list of triangle meshes
Mesh *boundingSphere = NULL;

Shader *shaderList = NULL,*activeShader = NULL;

const int numOfLights = 3;
Lights *lights[numOfLights];

Vector c = { 0, 0, 60 };
Camera cam = {c, {a,b,l}, 60, 1, 100000}; // Setup the camera parameters

bool normalize = true;
bool checkPlane = true;
int changeShader = 0;

GLuint shprg; // Shader program id

// Transform matrices
// V is view transform
// P is the projection transform
// PV = P * V
Matrix V, P, PP, PV, S, RX, RY, RZ, RXY, Rxyz, W;

char * loadShaderSource(const char* shaderFile)
{
	long length;
	FILE* fp;
	fopen_s(&fp, shaderFile, "r");
	if (!fp) {
		fprintf(stderr, "Error!\n");
		return 0;
	}
	char* buf;
	fseek(fp, 0, SEEK_END);
	length = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	buf = (char*)malloc(length * sizeof(char));
	length = fread(buf, 1, (long)length, fp);
	buf[length] = '\0';
	fclose(fp);
	return buf;
}

void prepareShaderProgram() {
	GLint success = GL_FALSE;

	shprg = glCreateProgram();
	
	const GLchar *source_vs = (const GLchar*) loadShaderSource(activeShader->vs);
	const GLchar *source_fs = (const GLchar*) loadShaderSource(activeShader->fs);;
	
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &source_vs, NULL);
	glCompileShader(vs);
	glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLint maxLength = 0;
		glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &maxLength);
		printf("Error in vertex shader!\n");
		std::vector<GLchar> infoLog(maxLength);
		glGetShaderInfoLog(vs, maxLength, &maxLength, &infoLog[0]);
		for (GLchar c : infoLog)
			std::cout << c;
	}
	else printf("Vertex shader compiled successfully!\n");

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &source_fs, NULL);
	glCompileShader(fs);
	glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLint maxLength = 0;
		glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &maxLength);
		printf("Error in fragment shader!\n");
		std::vector<GLchar> infoLog(maxLength);
		glGetShaderInfoLog(fs, maxLength, &maxLength, &infoLog[0]);
		for (GLchar c : infoLog)
			std::cout << c;
	}
	else printf("Fragment shader compiled successfully!\n");

	glAttachShader(shprg, vs);
	glAttachShader(shprg, fs);
	glLinkProgram(shprg);
	GLint isLinked = GL_FALSE;
	glGetProgramiv(shprg, GL_LINK_STATUS, &isLinked);
	if (!isLinked){
		GLint maxLength = 0;
		glGetProgramiv(shprg, GL_INFO_LOG_LENGTH, &maxLength);
		printf("Linker error in shader program!\n");
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(shprg, maxLength, &maxLength, &infoLog[0]);
		for (GLchar c : infoLog)
			std::cout << c;
	}
	else printf("Shader program linked successfully!\n");
	printf("%s shading activated!\n", activeShader->name);
}

void prepareMesh(Mesh *mesh) {
	int sizeVerts = mesh->nv * 3 * sizeof(float);
	int sizeCols  = mesh->nv * 3 * sizeof(float);
	int sizeTris = mesh->nt * 3 * sizeof(int);

	// Allocate GPU buffer and load mesh data
	glGenBuffers(1, &mesh->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeVerts + sizeCols, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeVerts, (void *)mesh->vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeVerts, sizeCols, (void *)mesh->vnorms);

	// For specification of the data stored in the vbo
	glGenVertexArrays(1, &mesh->vao);
	glBindVertexArray(mesh->vao);

	// Allocate GPU index buffer and load mesh indices
	glGenBuffers(1, &mesh->ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeTris, (void *)mesh->triangles, GL_STATIC_DRAW);
}

void renderBoundingSphere(Mesh *mesh) {
	Vector S = ScalarVecMul(mesh->modelRadius, { mesh->S.x, mesh->S.y, mesh->S.z });
	Matrix MM = Scale(S.x, S.y, S.z);
	MM = MatMatMul(Translation(-mesh->T.x, -mesh->T.y, -mesh->T.z), MM);
	MM = MatMatMul(PV, MM);

	if (!checkPlane && frustumToggle) return;

	// Pass the viewing transform to the shader
	GLint loc_PV = glGetUniformLocation(shprg, "PV");
	glUniformMatrix4fv(loc_PV, 1, GL_FALSE, MM.e);

	// Select current resources 
	glBindBuffer(GL_ARRAY_BUFFER, boundingSphere->vbo);
	glBindBuffer(GL_VERTEX_ARRAY, boundingSphere->vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boundingSphere->ibo);

	// Set up vertex array
	GLint vPos = glGetAttribLocation(shprg, "vPos");
	glEnableVertexAttribArray(vPos);
	glVertexAttribPointer(vPos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	// Set up normal array
	GLint vNorm = glGetAttribLocation(shprg, "vNorm");
	glEnableVertexAttribArray(vNorm);
	glVertexAttribPointer(vNorm, 3, GL_FLOAT, GL_FALSE, 0, (void *)(boundingSphere->nv * 3 * sizeof(float)));

	// To accomplish wireframe rendering (can be removed to get filled triangles)
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Draw all triangles
	glDrawElements(GL_TRIANGLES, boundingSphere->nt * 3, GL_UNSIGNED_INT, NULL);
	
}

Matrix CalcObjectTransform(Mesh *mesh)
{
	Matrix MM = Translation(-mesh->modelCenterPoint.x, -mesh->modelCenterPoint.y, -mesh->modelCenterPoint.z);
	MM = MatMatMul(Scale(mesh->S.x, mesh->S.y, mesh->S.z), MM);
	MM = MatMatMul(RotateZ(mesh->R.z), MM);
	MM = MatMatMul(RotateY(mesh->R.y), MM);
	MM = MatMatMul(RotateX(mesh->R.x), MM);
	MM = MatMatMul(Translation(-mesh->T.x, -mesh->T.y, -mesh->T.z), MM);
	return MatMatMul(V, MM);
}

bool renderMesh(Mesh *mesh) {
	// Assignment 1: Apply the transforms from local mesh coordinates to world coordinates here
	// Combine it with the viewing transform that is pass to the shader below
	Matrix MM = CalcObjectTransform(mesh);

	checkPlane = createFrustumPlane(mesh, MatMatMul(P, MM));
	if (!checkPlane) {
		return false;
	}

	// Pass the viewing transform to the shader
	GLint loc_PV = glGetUniformLocation(shprg, "PV");
	glUniformMatrix4fv(loc_PV, 1, GL_FALSE, PV.e);

	GLint loc_V = glGetUniformLocation(shprg, "V");
	glUniformMatrix4fv(loc_V, 1, GL_FALSE, V.e);

	GLint loc_P = glGetUniformLocation(shprg, "P");
	glUniformMatrix4fv(loc_P, 1, GL_FALSE, P.e);

	GLint loc_MV = glGetUniformLocation(shprg, "MV");
	glUniformMatrix4fv(loc_MV, 1, GL_FALSE, MM.e);

	GLint loc_ambient = glGetUniformLocation(shprg, "material.ambient");
	glUniform3f(loc_ambient, mesh->material.ambient.x, mesh->material.ambient.y, mesh->material.ambient.z);

	GLint loc_diffuse = glGetUniformLocation(shprg, "material.diffuse");
	glUniform3f(loc_diffuse, mesh->material.diffuse.x, mesh->material.diffuse.y, mesh->material.diffuse.z);

	GLint loc_specular = glGetUniformLocation(shprg, "material.specular");
	glUniform3f(loc_specular, mesh->material.specular.x, mesh->material.specular.y, mesh->material.specular.z);

	GLint loc_shininess = glGetUniformLocation(shprg, "material.shininess");
	glUniform1f(loc_shininess, mesh->material.shininess);

	GLint loc_numberOfLights = glGetUniformLocation(shprg, "numberOfLights");
	glUniform1i(loc_numberOfLights, numOfLights);
	
	for (int i = 0; i < numOfLights; ++i)
	{
		std::string index("lights[" + std::to_string(i) + "]");
		GLint loc_lightsPos = glGetUniformLocation(shprg, (index + ".position").c_str());
		glUniform3f(loc_lightsPos, lights[i]->position.x, lights[i]->position.y, lights[i]->position.z);

		GLint loc_lightsDiffuse = glGetUniformLocation(shprg, (index + ".diffuse").c_str());
		glUniform3f(loc_lightsDiffuse, lights[i]->diffuse.x, lights[i]->diffuse.y, lights[i]->diffuse.z);

		GLint loc_lightsSpecular = glGetUniformLocation(shprg, (index + ".specular").c_str());
		glUniform3f(loc_lightsSpecular, lights[i]->specular.x, lights[i]->specular.y, lights[i]->specular.z);
	}

	// Select current resources 
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	glBindBuffer(GL_VERTEX_ARRAY, mesh->vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo); //1437 was here :^)

	// Set up vertex array
	GLint vPos = glGetAttribLocation(shprg, "vPos");
	glEnableVertexAttribArray(vPos);
	glVertexAttribPointer(vPos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	// Set up normal array
	GLint vNorm = glGetAttribLocation(shprg, "vNorm");
	glEnableVertexAttribArray(vNorm);
	glVertexAttribPointer(vNorm, 3, GL_FLOAT, GL_FALSE, 0, (void *)(mesh->nv * 3 * sizeof(float)));

	// To accomplish wireframe rendering (can be removed to get filled triangles)
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Draw all triangles
	glDrawElements(GL_TRIANGLES, mesh->nt * 3, GL_UNSIGNED_INT, NULL);
	return true;
}

void display(void) {
	Mesh *mesh;
	LARGE_INTEGER frequency; // ticks per second
	LARGE_INTEGER t1, t2;	// ticks

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glMatrixMode(GL_PROJECTION);

	// Assignment1: Calculate the transform to view coordinates yourself 
	V = Translation(-cam.position.x, -cam.position.y, -cam.position.z);
	S = Scale(1.0f, 1.0f, 1.0f);
	Vector rotate = { a, b, l };
	Rxyz = Rotate(rotate);
	V = MatMatMul(Rxyz, V);


	
	// Assignment1: Calculate the projection transform yourself
	if (OP_PP == 1)
		P = OrthoProjection(-18 * ar, 18 * ar, -16, 16, cam.nearPlane, cam.farPlane);
	else
		P = PerspectiveProjection(cam.fov, ar, cam.nearPlane, cam.farPlane);

	PV = MatMatMul(P, V);
	glMatrixMode(GL_MODELVIEW);
	glUseProgram(shprg);

	// render all meshes in the scene	
	mesh = meshList;
	
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&t1);
	int activeObjects = ObjectsRendered;
	while (mesh != NULL) {
		if (!renderMesh(mesh)) activeObjects--;
		//renderBoundingSphere(mesh);
		mesh = mesh->next;
	}
	QueryPerformanceCounter(&t2);

	printf("Active objects: %d, Total objects: %d\n", activeObjects, ObjectsRendered);
	printf("Mesh render time: %fms\n", (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart );
	glFlush();
}

void changeSize(int w, int h) {
	screen_width = w;
	screen_height = h;
	glViewport(0, 0, screen_width, screen_height);
}

void keypress(unsigned char key, int x, int y) {
	Mesh *mesh;
	mesh = meshList;
	
	switch(key) {
	case 'e':
		OP_PP = !OP_PP;
		break;
	case 'i':
		if (activeMesh->R.x >= 6.0f){
			activeMesh->R.x = 0.0f;
		} else {
			activeMesh->R.x += 0.5f;
		}
		break;
	case 'I':
		if (a >= 6.0f){
			a = 0.0f;
		} else {
			a -= 0.5f;
		}
		break;
	case 'j':
		if (activeMesh->R.y >= 6.0f){
			activeMesh->R.y = 0.0f;
		} else {
			activeMesh->R.y += 0.5f;
		}
		break;
	case 'J':
		if (b >= 6.0f){
			b = 0.0f;
		} else {
			b -= 0.5f;
		}
		break;
	case 'k':
		if (activeMesh->R.z >= 6.0f){
			activeMesh->R.z = 0.0f;
		} else {
			activeMesh->R.z += 0.5f;
		}
		break;
	case 'K':
		if (l >= 6.0f){
			l = 0.0f;
		} else {
			l -= 0.5f;
		}
		break;
	case 'f':
		frustumToggle = !frustumToggle;
		break;

	// Move active mesh
	case 'w':
		activeMesh->T.y -= 1.0f;
		break;
	case 's':
		activeMesh->T.y += 1.0f;
		break;
	case 'd':
		activeMesh->T.x -= 1.0f;
		break;
	case 'a':
		activeMesh->T.x += 1.0f;
		break;
	case 'R':
		activeMesh->T.z -= 1.0f;
		break;
	case 'r':
		activeMesh->T.z += 1.0f;
		break;
	case 'g':
		if (activeMesh->S.x > 0.1f)
		{
			activeMesh->S.x += 0.1f;
			activeMesh->S.y += 0.1f;
			activeMesh->S.z += 0.1f;
		}
		break;
	case 'G':
		activeMesh->S.x -= 0.1f;
		activeMesh->S.y -= 0.1f;
		activeMesh->S.z -= 0.1f;
		break;

	// Cam positions
	case 'A':
		cam.position.x -= 1.5f;
		break;
	case 'D':
		cam.position.x += 1.5f;
		break;
	case 'S':
		cam.position.y -= 1.5f;
		break;
	case 'W':
		cam.position.y += 1.5f;
		break;
	case 'z':
		cam.position.z -= 1.5f;
		break;
	case 'Z':
		cam.position.z += 1.5f;
		break;
	case 'Q':
		glLoadIdentity();
		break;
	case 'q':
		exit(0);
		break;
	case 'c':
		if (activeMesh->next != NULL)
			activeMesh = activeMesh->next;
		else
			activeMesh = meshList;
		break;
	case 'n':
		if (activeShader->next != NULL)
			activeShader = activeShader->next;
		else
			activeShader = shaderList;
		prepareShaderProgram();
		break;
	}
	glutPostRedisplay();
}

void init(void) {
	// Compile and link shader program
	prepareShaderProgram();

	// Setup OpenGL buffers for rendering of the meshes
	Mesh * mesh = meshList;
	activeMesh = meshList;
	while (mesh != NULL) {
		ObjectsRendered++;
		prepareMesh(mesh);
		mesh = mesh->next;
	}
	prepareMesh(boundingSphere);
}

void cleanUp(void) {	
	// Free openGL resources
	// ...

	// Free meshes
	// ...
}

Shader *insert_Shader(char* name, char *vs, char *fs)
{
	Shader *shader = (Shader*) malloc(sizeof(Shader));
	shader->name = name;
	shader->vs = vs;
	shader->fs = fs;
	shader->next = NULL;
	return shader;
}

Lights *insert_Light(Vector position, Vector diffuse, Vector specular)
{
	Lights *light = (Lights*)malloc(sizeof(Lights));
	light->position = position;
	light->diffuse = diffuse;
	light->specular = specular;
	return light;
}

// Include data for some triangle meshes (hard coded in struct variables)
#include "./models/mesh_bunny.h"
#include "./models/mesh_cow.h"
#include "./models/mesh_cube.h"
//#include "./models/mesh_frog.h"
//#include "./models/mesh_knot.h"
#include "./models/mesh_sphere.h"
#include "./models/mesh_teapot.h"
#include "./models/mesh_triceratops.h"


int main(int argc, char **argv) {
	// Setup freeGLUT	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(screen_width, screen_height);
	glutCreateWindow("DVA338 Assignments");
	glutDisplayFunc(display);
	glutReshapeFunc(changeSize);
	glutKeyboardFunc(keypress);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	// Enables z-buffer
	glEnable(GL_DEPTH_TEST); 
	glDepthFunc(GL_LESS);

	// Specify your preferred OpenGL version and profile
	glutInitContextVersion(4, 5);
	//glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);	
	glutInitContextProfile(GLUT_CORE_PROFILE);

	// Use an OpenGL Loading Library to get access to modern core features as well as extensions
	GLenum err = glewInit(); 
	if (GLEW_OK != err) { fprintf(stdout, "Error: %s\n", glewGetErrorString(err)); return 1; }

	// Output OpenGL version info
	fprintf(stdout, "GLEW version: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, "OpenGL version: %s\n", (const char *)glGetString(GL_VERSION));
	fprintf(stdout, "OpenGL vendor: %s\n\n", glGetString(GL_VENDOR));

	// Insert the 3D models you want in your scene here in a linked list of meshes
	// Note that "meshList" is a pointer to the first mesh and new meshes are added to the front of the list
	//insertModel(&meshList, cube.nov, cube.verts, cube.nof, cube.faces, 5.0, { { 1.0, 1.0, 1.0 },{ 1.0, 1.0, 1.0 },{ 1.0, 1.0, 1.0 }, 4.0f });
	//insertModel(&meshList, frog.nov, frog.verts, frog.nof, frog.faces, 2.5, { { 1.0, 1.0, 1.0 },{ 1.0, 1.0, 1.0 },{ 1.0, 1.0, 1.0 }, 4.0f });
	//insertModel(&meshList, knot.nov, knot.verts, knot.nof, knot.faces, 1.0, { { 1.0, 1.0, 1.0 },{ 1.0, 1.0, 1.0 },{ 1.0, 1.0, 1.0 }, 4.0f });
	//insertModel(&meshList, sphere.nov, sphere.verts, sphere.nof, sphere.faces, 12.0, { { 1.0, 1.0, 1.0 },{ 1.0, 1.0, 1.0 },{ 1.0, 1.0, 1.0 }, 4.0f });
	insertModel(&meshList, teapot.nov, teapot.verts, teapot.nof, teapot.faces, 3.0, { { 1.0, 1.0, 1.0 }, { 1.0, 1.0, 1.0 }, { 1.0, 1.0, 1.0 }, 200.0f });
	insertModel(&meshList, cow.nov, cow.verts, cow.nof, cow.faces, 20.0, { { 1.0, 1.0, 0.5 },{ 1.0, 1.0, 1.0 },{ 1.0, 1.0, 1.0 }, 140.0f });
	//insertModel(&meshList, bunny.nov, bunny.verts, bunny.nof, bunny.faces, 60.0, { { 1.0, 1.0, 1.0 },{ 1.0, 1.0, 1.0 },{ 1.0, 1.0, 1.0 }, 20.0f });
	insertModel(&meshList, triceratops.nov, triceratops.verts, triceratops.nof, triceratops.faces, 3.0, { { 1.0, 1.0, 1.0 }, { 1.0, 1.0, 1.0 }, { 1.0, 1.0, 1.0 }, 80.0f });
	insertModel(&boundingSphere, sphere.nov, sphere.verts, sphere.nof, sphere.faces, 1.1, { { 1.0, 1.0, 1.0 },{ 1.0, 1.0, 1.0 },{ 1.0, 1.0, 1.0 }, 4.0f });
	
	// Insert shaders to shader list
	shaderList = insert_Shader("Phong", "vs_phong.glsl", "fs_phong.glsl");
	shaderList->next = insert_Shader("Gouraud", "vs_gouraud.glsl", "fs_gouraud.glsl");
	shaderList->next->next = insert_Shader("Cartoon", "vs_cartoon.glsl", "fs_cartoon.glsl");
	activeShader = shaderList;

	// Insert lights to light list
	lights[0] = insert_Light({ 30.0f, 0.0f, 60.0f }, { 1.0f, .0f, .0f }, { 1.0f, 1.0f, 1.0f });
	lights[1] = insert_Light({ 0.0f, 0.0f, 60.0f }, { .0f, 1.0f, .0f }, { 1.0f, 1.0f, 1.0f });
	lights[2] = insert_Light({ -30.0f, 0.0f, 60.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f });

	init();
	glutMainLoop();

	cleanUp();	
	return 0;
}