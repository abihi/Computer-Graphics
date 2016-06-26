#ifndef _SHADERS_H_
#define _SHADERS_H_

//// Simple vertex shader treating vertex normals as RGB colors
//static const char * vs_n2c_src[] = {
//	"#version 420 core                                                 \n"
//	"                                                                  \n"
//	" in vec3 vPos;                                                    \n"
//	" in vec3 vNorm;                                                   \n"
//	" out vec4 color;                                                  \n"
//	" uniform mat4 PV;                                                 \n"
//	"                                                                  \n"
//	"void main(void)                                                   \n"
//	"{                                                                 \n"
//	"    color = abs(vec4(vNorm, 1.0));                                \n"
//	"    gl_Position = PV * vec4(vPos, 1.0f);                          \n"
//	"}                                                                 \n"
//};
//
//// Simple fragment shader for color interpolation
//static const char * fs_ci_src[] = {
//	"#version 420 core                                                 \n"
//	"                                                                  \n"
//	"in vec4 color;                                                    \n"
//	"out vec4 fColor;                                                  \n"
//	"                                                                  \n"
//	"void main(void)                                                   \n"
//	"{                                                                 \n"
//	"    fColor = color;                                               \n"
//	"}                                                                 \n"
//};

// The source code for additional shaders can be added here
// ...

typedef struct _Shader{
	char* name;
	const char* vs;
	const char* fs;
	struct _Shader *next;
} Shader;

typedef struct _Lights{
	Vector position;
	Vector diffuse;
	Vector specular;
} Lights;

#endif