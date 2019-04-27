#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <cstdio>
#include <cassert>
#include "GL_framework.h"
#include <vector>
#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <iostream>

#define NUM_OBJECTS 10000

///////// fw decl
namespace ImGui {
	void Render();
}

namespace Sphere {
	void setupSphere(glm::vec3 pos, float radius);
	void cleanupSphere();
	void updateSphere(glm::vec3 pos, float radius);
	void drawSphere(glm::vec3 mycolor, bool isLluna);
}

namespace Cube {
	void setupCube();
	void cleanupCube();
	void updateCube(const glm::mat4& transform);
	void drawCube(double currentTime, glm::vec4 myColor);
}

namespace MyLoadedModel {
	void setupModel();
	void cleanupModel();
	void updateModel(const glm::mat4& transform);
	void drawModel();

	void setupModel2();
	void cleanupModel2();
	void updateModel2(const glm::mat4& transform);
	void drawModel2();
}

// Variables
bool end = true, activateTS = true, show_test_window = false, light_moves = true;
bool key_a, key_b = true, key_c, key_c2, key_d, key_m, key_p, key_s, key_t, key_z;
int exercise = 0;
glm::vec3 lightPos, lightPos2, lightPos3, myColor3;
std::vector <glm::vec3> vertices, vertices2;
std::vector <glm::vec2> uvs, uvs2;
std::vector <glm::vec3> normals, normals2;

// Exercises
void Exercise1();
void Exercise2();
void Exercise3();

// Utils
void GUI();
bool CheckClickOption();
glm::mat4 Transform(glm::vec3 translate, float rotate, int rotAxis, float scale);
extern bool loadOBJ(const char * path, std::vector <glm::vec3> & out_vertices,
	std::vector <glm::vec2> & out_uvs, std::vector <glm::vec3> & out_normals);

namespace RenderVars {
	const float FOV = glm::radians(65.f);
	const float zNear = 1.f;
	const float zFar = 50000.f;

	glm::mat4 _projection;
	glm::mat4 _modelView;
	glm::mat4 _MVP;
	glm::mat4 _inv_modelview;
	glm::vec4 _cameraPoint;

	struct prevMouse {
		float lastx, lasty;
		MouseEvent::Button button = MouseEvent::Button::None;
		bool waspressed = false;
	} prevMouse;

	float panv[3] = { 0.f, -5.f, -3000.f };
	float rota[2] = { 0.f, 0.f };
}
namespace RV = RenderVars;

void GLResize(int width, int height) {
	glViewport(0, 0, width, height);
	if (height != 0) RV::_projection = glm::perspective(RV::FOV, (float)width / (float)height, RV::zNear, RV::zFar);
	else RV::_projection = glm::perspective(RV::FOV, 0.f, RV::zNear, RV::zFar);
}

void GLmousecb(MouseEvent ev) {
	if (RV::prevMouse.waspressed && RV::prevMouse.button == ev.button) {
		float diffx = ev.posx - RV::prevMouse.lastx;
		float diffy = ev.posy - RV::prevMouse.lasty;
		switch (ev.button) {
		case MouseEvent::Button::Left: // ROTATE
			RV::rota[0] += diffx * 0.005f;
			RV::rota[1] += diffy * 0.005f;
			break;
		case MouseEvent::Button::Right: // MOVE XY
			RV::panv[0] += diffx * 0.03f;
			RV::panv[1] -= diffy * 0.03f;
			break;
		case MouseEvent::Button::Middle: // MOVE Z
			RV::panv[2] += diffy * 0.05f;
			break;
		default: break;
		}
	}
	else {
		RV::prevMouse.button = ev.button;
		RV::prevMouse.waspressed = true;
	}
	RV::prevMouse.lastx = ev.posx;
	RV::prevMouse.lasty = ev.posy;
}

void GLinit(int width, int height) {
	glViewport(0, 0, width, height);
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glClearDepth(1.f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	RV::_projection = glm::perspective(RV::FOV, (float)width / (float)height, RV::zNear, RV::zFar);

	// LightPos
	lightPos = glm::vec3(0.f, 0.f, 100.f);


	// Setup models
	bool res = loadOBJ("wolf.obj", vertices, uvs, normals);
	MyLoadedModel::setupModel();

	bool res2 = loadOBJ("deer.obj", vertices2, uvs2, normals2);
	MyLoadedModel::setupModel2();

	//TODO ficar-ho a cada exercici
	RV::panv[0] = -2000.f; // X
	RV::panv[1] = -1600.f;   // Y
	RV::panv[2] = -3000.f; // Z
}

void GLrender(float currentTime) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	RV::_modelView = glm::mat4(1.f);
	RV::_modelView = glm::translate(RV::_modelView, glm::vec3(RV::panv[0], RV::panv[1], RV::panv[2]));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[1], glm::vec3(1.f, 0.f, 0.f));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[0], glm::vec3(0.f, 1.f, 0.f));

	// Render code
	if (CheckClickOption) {

		switch (exercise) {
			case 1:  Exercise1();  break;
			case 2:  Exercise2();  break;
			case 3:  Exercise3();  break;
			default: break;
		}
	}

	RV::_MVP = RV::_projection * RV::_modelView;

	ImGui::Render();
}

void GLcleanup() {
	Cube::cleanupCube();
	Sphere::cleanupSphere();

	MyLoadedModel::cleanupModel();
	MyLoadedModel::cleanupModel2();
}

//////////////////////////////////// COMPILE AND LINK
GLuint compileShader(const char* shaderStr, GLenum shaderType, const char* name = "") {
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderStr, NULL);
	glCompileShader(shader);
	GLint res;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &res);
	if (res == GL_FALSE) {
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &res);
		char *buff = new char[res];
		glGetShaderInfoLog(shader, res, &res, buff);
		fprintf(stderr, "Error Shader %s: %s", name, buff);
		delete[] buff;
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}
void linkProgram(GLuint program) {
	glLinkProgram(program);
	GLint res;
	glGetProgramiv(program, GL_LINK_STATUS, &res);
	if (res == GL_FALSE) {
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &res);
		char *buff = new char[res];
		glGetProgramInfoLog(program, res, &res, buff);
		fprintf(stderr, "Error Link: %s", buff);
		delete[] buff;
	}
}

////////////////////////////////////////////////// SPHERE
namespace Sphere {
	GLuint sphereVao;
	GLuint sphereVbo;
	GLuint sphereShaders[3];
	GLuint sphereProgram;
	float radius;

	glm::mat4 objMat = glm::mat4(1.f);

	const char* sphere_vertShader =
		"#version 330\n\
in vec3 in_Position;\n\
uniform mat4 mv_Mat;\n\
void main() {\n\
	gl_Position = mv_Mat * vec4(in_Position, 1.0);\n\
}";
	const char* sphere_geomShader =
		"#version 330\n\
layout(points) in;\n\
layout(triangle_strip, max_vertices = 4) out;\n\
out vec4 eyePos;\n\
out vec4 centerEyePos;\n\
uniform mat4 projMat;\n\
uniform float radius;\n\
vec4 nu_verts[4];\n\
void main() {\n\
	vec3 n = normalize(-gl_in[0].gl_Position.xyz);\n\
	vec3 up = vec3(0.0, 1.0, 0.0);\n\
	vec3 u = normalize(cross(up, n));\n\
	vec3 v = normalize(cross(n, u));\n\
	nu_verts[0] = vec4(-radius*u - radius*v, 0.0); \n\
	nu_verts[1] = vec4( radius*u - radius*v, 0.0); \n\
	nu_verts[2] = vec4(-radius*u + radius*v, 0.0); \n\
	nu_verts[3] = vec4( radius*u + radius*v, 0.0); \n\
	centerEyePos = gl_in[0].gl_Position;\n\
	for (int i = 0; i < 4; ++i) {\n\
		eyePos = (gl_in[0].gl_Position + nu_verts[i]);\n\
		gl_Position = projMat * eyePos;\n\
		EmitVertex();\n\
	}\n\
	EndPrimitive();\n\
}";
	const char* sphere_fragShader_flatColor =
		"#version 330\n\
in vec4 eyePos;\n\
in vec4 centerEyePos;\n\
out vec4 out_Color;\n\
uniform mat4 projMat;\n\
uniform mat4 mv_Mat;\n\
uniform vec4 color;\n\
uniform float radius;\n\
void main() {\n\
	vec4 diff = eyePos - centerEyePos;\n\
	float distSq2C = dot(diff, diff);\n\
	if (distSq2C > (radius*radius)) discard;\n\
	float h = sqrt(radius*radius - distSq2C);\n\
	vec4 nuEyePos = vec4(eyePos.xy, eyePos.z + h, 1.0);\n\
	vec4 nuPos = projMat * nuEyePos;\n\
	gl_FragDepth = ((nuPos.z / nuPos.w) + 1) * 0.5;\n\
	vec3 normal = normalize(nuEyePos - centerEyePos).xyz;\n\
	out_Color = vec4(color.xyz * dot(normal, (mv_Mat*vec4(0.0, 1.0, 0.0, 0.0)).xyz) + color.xyz * 0.3, 1.0 );\n\
}";

	bool shadersCreated = false;
	void createSphereShaderAndProgram() {
		if (shadersCreated) return;

		sphereShaders[0] = compileShader(sphere_vertShader, GL_VERTEX_SHADER, "sphereVert");
		sphereShaders[1] = compileShader(sphere_geomShader, GL_GEOMETRY_SHADER, "sphereGeom");
		sphereShaders[2] = compileShader(sphere_fragShader_flatColor, GL_FRAGMENT_SHADER, "sphereFrag");

		sphereProgram = glCreateProgram();
		glAttachShader(sphereProgram, sphereShaders[0]);
		glAttachShader(sphereProgram, sphereShaders[1]);
		glAttachShader(sphereProgram, sphereShaders[2]);
		glBindAttribLocation(sphereProgram, 0, "in_Position");
		linkProgram(sphereProgram);

		shadersCreated = true;
	}
	void cleanupSphereShaderAndProgram() {
		if (!shadersCreated) return;
		glDeleteProgram(sphereProgram);
		glDeleteShader(sphereShaders[0]);
		glDeleteShader(sphereShaders[1]);
		glDeleteShader(sphereShaders[2]);
		shadersCreated = false;
	}

	void setupSphere(glm::vec3 pos, float radius) {
		Sphere::radius = radius;
		glGenVertexArrays(1, &sphereVao);
		glBindVertexArray(sphereVao);
		glGenBuffers(1, &sphereVbo);

		glBindBuffer(GL_ARRAY_BUFFER, sphereVbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3, &pos, GL_DYNAMIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		createSphereShaderAndProgram();
	}
	void cleanupSphere() {
		glDeleteBuffers(1, &sphereVbo);
		glDeleteVertexArrays(1, &sphereVao);

		cleanupSphereShaderAndProgram();
	}
	void updateSphere(glm::vec3 pos, float radius) {
		glBindBuffer(GL_ARRAY_BUFFER, sphereVbo);
		float* buff = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		buff[0] = pos.x;
		buff[1] = pos.y;
		buff[2] = pos.z;
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		Sphere::radius = radius;
	}
	void drawSphere(glm::vec3 mycolor, bool isLluna) {
		glBindVertexArray(sphereVao);
		glUseProgram(sphereProgram);
		glUniformMatrix4fv(glGetUniformLocation(sphereProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(sphereProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RV::_MVP));
		glUniformMatrix4fv(glGetUniformLocation(sphereProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RV::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(sphereProgram, "projMat"), 1, GL_FALSE, glm::value_ptr(RV::_projection));

		// Sphere Rotation
		if (isLluna && exercise >= 5) {
			glm::mat4 rot = glm::rotate(glm::mat4(), glm::radians(135.0f), glm::vec3(0.f, 1.f, 0.f));
			objMat = rot;
		}

		glUniform4f(glGetUniformLocation(sphereProgram, "color"), mycolor.x, mycolor.y, mycolor.z, 1.f);
		glUniform1f(glGetUniformLocation(sphereProgram, "radius"), Sphere::radius);
		glDrawArrays(GL_POINTS, 0, 1);

		glUseProgram(0);
		glBindVertexArray(0);
	}
}

////////////////////////////////////////////////// CUBE
namespace Cube {
	GLuint cubeVao;
	GLuint cubeVbo[3];
	GLuint cubeShaders[2];
	GLuint cubeProgram;
	glm::mat4 objMat = glm::mat4(1.f);

	extern const float halfW = 0.5f;
	int numVerts = 24 + 6; // 4 vertex/face * 6 faces + 6 PRIMITIVE RESTART

						   //   4---------7
						   //  /|        /|
						   // / |       / |
						   //5---------6  |
						   //|  0------|--3
						   //| /       | /
						   //|/        |/
						   //1---------2
	glm::vec3 verts[] = {
		glm::vec3(-halfW, -halfW, -halfW),
		glm::vec3(-halfW, -halfW,  halfW),
		glm::vec3(halfW, -halfW,  halfW),
		glm::vec3(halfW, -halfW, -halfW),
		glm::vec3(-halfW,  halfW, -halfW),
		glm::vec3(-halfW,  halfW,  halfW),
		glm::vec3(halfW,  halfW,  halfW),
		glm::vec3(halfW,  halfW, -halfW)
	};

	glm::vec3 norms[] = {
		glm::vec3(0.f, -1.f,  0.f),
		glm::vec3(0.f,  1.f,  0.f),
		glm::vec3(-1.f,  0.f,  0.f),
		glm::vec3(1.f,  0.f,  0.f),
		glm::vec3(0.f,  0.f, -1.f),
		glm::vec3(0.f,  0.f,  1.f)
	};

	glm::vec3 cubeVerts[] = {
		verts[1], verts[0], verts[2], verts[3],
		verts[5], verts[6], verts[4], verts[7],
		verts[1], verts[5], verts[0], verts[4],
		verts[2], verts[3], verts[6], verts[7],
		verts[0], verts[4], verts[3], verts[7],
		verts[1], verts[2], verts[5], verts[6]
	};

	glm::vec3 cubeNorms[] = {
		norms[0], norms[0], norms[0], norms[0],
		norms[1], norms[1], norms[1], norms[1],
		norms[2], norms[2], norms[2], norms[2],
		norms[3], norms[3], norms[3], norms[3],
		norms[4], norms[4], norms[4], norms[4],
		norms[5], norms[5], norms[5], norms[5]
	};

	GLubyte cubeIdx[] = {
		0, 1, 2, 3, UCHAR_MAX,
		4, 5, 6, 7, UCHAR_MAX,
		8, 9, 10, 11, UCHAR_MAX,
		12, 13, 14, 15, UCHAR_MAX,
		16, 17, 18, 19, UCHAR_MAX,
		20, 21, 22, 23, UCHAR_MAX
	};

	const char* cube_vertShader =
		"#version 330\n\
	in vec3 in_Position;\n\
	in vec3 in_Normal;\n\
	out vec4 vert_Normal;\n\
	uniform mat4 objMat;\n\
	uniform mat4 mv_Mat;\n\
	uniform mat4 mvpMat;\n\
	void main() {\n\
		gl_Position = mvpMat * objMat * vec4(in_Position, 1.0);\n\
		vert_Normal = mv_Mat * objMat * vec4(in_Normal, 0.0);\n\
	}";

	const char* cube_fragShader =
		"#version 330\n\
	in vec4 vert_Normal;\n\
	out vec4 out_Color;\n\
	uniform mat4 mv_Mat;\n\
	uniform vec4 color;\n\
	void main() {\n\
		out_Color = vec4(color.xyz * dot(vert_Normal, mv_Mat*vec4(0.0, 1.0, 0.0, 0.0)) + color.xyz * 0.3, 1.0 );\n\
	}";

	void setupCube() {
		glGenVertexArrays(1, &cubeVao);
		glBindVertexArray(cubeVao);
		glGenBuffers(3, cubeVbo);

		glBindBuffer(GL_ARRAY_BUFFER, cubeVbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, cubeVbo[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeNorms), cubeNorms, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glPrimitiveRestartIndex(UCHAR_MAX);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeVbo[2]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIdx), cubeIdx, GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		cubeShaders[0] = compileShader(cube_vertShader, GL_VERTEX_SHADER, "cubeVert");
		cubeShaders[1] = compileShader(cube_fragShader, GL_FRAGMENT_SHADER, "cubeFrag");

		cubeProgram = glCreateProgram();
		glAttachShader(cubeProgram, cubeShaders[0]);
		glAttachShader(cubeProgram, cubeShaders[1]);
		glBindAttribLocation(cubeProgram, 0, "in_Position");
		glBindAttribLocation(cubeProgram, 1, "in_Normal");
		linkProgram(cubeProgram);
	}

	void cleanupCube() {
		glDeleteBuffers(3, cubeVbo);
		glDeleteVertexArrays(1, &cubeVao);

		glDeleteProgram(cubeProgram);
		glDeleteShader(cubeShaders[0]);
		glDeleteShader(cubeShaders[1]);
	}

	void updateCube(const glm::mat4& transform) {
		objMat = transform;
	}

	void drawCube(double currentTime, glm::vec4 myColor) {
		/*glm::mat4 trans = glm::translate(glm::mat4(), glm::vec3(15.f * cos((float)currentTime), 15.f * sin((float)currentTime), 1.f));
		objMat = trans;*/

		glEnable(GL_PRIMITIVE_RESTART);
		glBindVertexArray(cubeVao);
		glUseProgram(cubeProgram);
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myColor.x, myColor.y, myColor.z, myColor.a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		glUseProgram(0);
		glBindVertexArray(0);
		glDisable(GL_PRIMITIVE_RESTART);
	}
}

////////////////////////////////////////////////// MyModel
namespace MyLoadedModel {

	GLuint modelVao, modelVao2, modelVao3, modelVao4, modelVao5;
	GLuint modelVbo[3], modelVbo2[3], modelVbo3[3], modelVbo4[3], modelVbo5[3];
	GLuint modelShaders[2], modelShaders2[2], modelShaders3[2], modelShaders4[2], modelShaders5[2];
	GLuint modelProgram, modelProgram2, modelProgram3, modelProgram4, modelProgram5;
	glm::mat4 objMat = glm::mat4(1.f), objMat2 = glm::mat4(1.f), objMat3 = glm::mat4(1.f), objMat4 = glm::mat4(1.f), objMat5 = glm::mat4(1.f);

	const char* model_vertShader =
		"#version 330\n\
	in vec3 in_Position;\n\
	in vec3 in_Normal;\n\
	uniform vec3 lPos;\n\
	out vec3 lDir;\n\
	out vec4 vert_Normal;\n\
	uniform mat4 objMat;\n\
	uniform mat4 mv_Mat;\n\
	uniform mat4 mvpMat;\n\
	void main() {\n\
		gl_Position = mvpMat * objMat * vec4(in_Position, 1.0);\n\
		vert_Normal = mv_Mat * objMat * vec4(in_Normal, 0.0);\n\
		lDir = normalize(lPos - (objMat * vec4(in_Position, 1.0)).xyz);\n\
	}";


	const char* model_fragShader =
		"#version 330\n\
in vec4 vert_Normal;\n\
in vec3 lDir;\n\
out vec4 out_Color;\n\
uniform mat4 mv_Mat;\n\
uniform vec4 color;\n\
void main() {\n\
	float u = dot(normalize(vert_Normal), mv_Mat*vec4(lDir.x, lDir.y, lDir.z, 0.0));\n\
	out_Color = vec4(color.xyz * u, 1.0 );\n\
}";

	////////////////////////////////////////////////// 1º Model Wolf
#pragma region
	void setupModel() {
		glGenVertexArrays(1, &modelVao);
		glBindVertexArray(modelVao);
		glGenBuffers(3, modelVbo);

		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[0]);

		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[1]);

		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		modelShaders[0] = compileShader(model_vertShader, GL_VERTEX_SHADER, "cubeVert");
		modelShaders[1] = compileShader(model_fragShader, GL_FRAGMENT_SHADER, "cubeFrag");

		modelProgram = glCreateProgram();
		glAttachShader(modelProgram, modelShaders[0]);
		glAttachShader(modelProgram, modelShaders[1]);
		glBindAttribLocation(modelProgram, 0, "in_Position");
		glBindAttribLocation(modelProgram, 1, "in_Normal");
		linkProgram(modelProgram);
	}

	void cleanupModel() {
		glDeleteBuffers(2, modelVbo);
		glDeleteVertexArrays(1, &modelVao);

		glDeleteProgram(modelProgram);
		glDeleteShader(modelShaders[0]);
		glDeleteShader(modelShaders[1]);
	}

	void updateModel(const glm::mat4& transform) {
		objMat = transform;
	}

	void drawModel() {
		glBindVertexArray(modelVao);
		glUseProgram(modelProgram);
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform3f(glGetUniformLocation(modelProgram, "lPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform4f(glGetUniformLocation(modelProgram, "color"), 1.0f, 0.0f, 0.0f, 0.f);
		

		glDrawArrays(GL_TRIANGLES, 0, vertices.size());

		glUseProgram(0);
		glBindVertexArray(0);
	}
	void drawModelInst() {
		glBindVertexArray(modelVao);
		glUseProgram(modelProgram);
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform3f(glGetUniformLocation(modelProgram, "lPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform4f(glGetUniformLocation(modelProgram, "color"), 1.0f, 0.0f, 0.0f, 0.f);


		glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, vertices.size(), 5000, 0);
		//glVertexAttribDivisor de 1 per canviar la posició o el color
		//Crear buffers pel color i per les posicions

		glUseProgram(0);
		glBindVertexArray(0);
	}
#pragma endregion

	////////////////////////////////////////////////// 2º Model Deer
#pragma region
	void setupModel2() {
		glGenVertexArrays(1, &modelVao2);
		glBindVertexArray(modelVao2);
		glGenBuffers(3, modelVbo2);

		glBindBuffer(GL_ARRAY_BUFFER, modelVbo2[0]);

		glBufferData(GL_ARRAY_BUFFER, vertices2.size() * sizeof(glm::vec3), &vertices2[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, modelVbo2[1]);

		glBufferData(GL_ARRAY_BUFFER, normals2.size() * sizeof(glm::vec3), &normals2[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		modelShaders2[0] = compileShader(model_vertShader, GL_VERTEX_SHADER, "cubeVert");
		modelShaders2[1] = compileShader(model_fragShader, GL_FRAGMENT_SHADER, "cubeFrag");

		modelProgram2 = glCreateProgram();
		glAttachShader(modelProgram2, modelShaders2[0]);
		glAttachShader(modelProgram2, modelShaders2[1]);
		glBindAttribLocation(modelProgram2, 0, "in_Position");
		glBindAttribLocation(modelProgram2, 1, "in_Normal");
		linkProgram(modelProgram2);
	}

	void cleanupModel2() {
		glDeleteBuffers(2, modelVbo2);
		glDeleteVertexArrays(1, &modelVao2);

		glDeleteProgram(modelProgram2);
		glDeleteShader(modelShaders2[0]);
		glDeleteShader(modelShaders2[1]);
	}

	void updateModel2(const glm::mat4& transform) {
		objMat2 = transform;
	}

	void drawModel2() {
		glBindVertexArray(modelVao2);
		glUseProgram(modelProgram2);
		glUniformMatrix4fv(glGetUniformLocation(modelProgram2, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat2));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram2, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram2, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform3f(glGetUniformLocation(modelProgram2, "lPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform4f(glGetUniformLocation(modelProgram2, "color"), 1.f, 1.f, 0.f, 0.f);

		glDrawArrays(GL_TRIANGLES, 0, vertices2.size());

		glUseProgram(0);
		glBindVertexArray(0);
	}

		void drawModelInst2() {
			glBindVertexArray(modelVao2);
			glUseProgram(modelProgram2);
			glUniformMatrix4fv(glGetUniformLocation(modelProgram2, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat2));
			glUniformMatrix4fv(glGetUniformLocation(modelProgram2, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
			glUniformMatrix4fv(glGetUniformLocation(modelProgram2, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
			glUniform3f(glGetUniformLocation(modelProgram2, "lPos"), lightPos.x, lightPos.y, lightPos.z);
			glUniform4f(glGetUniformLocation(modelProgram2, "color"), 1.f, 1.f, 0.f, 0.f);

			glDrawArraysInstancedBaseInstance(GL_TRIANGLES,0, 5000, 100, 100);

			glUseProgram(0);
			glBindVertexArray(0);
	}
#pragma endregion



}


////////////////////////////////////////////////// Exercises
void Exercise1() {
	
	
	float xPos = 0, yPos = 0, prevPos = 0;
	int arrX[NUM_OBJECTS], arrY[NUM_OBJECTS];

	// Guardem totes les posicions
	for (int i = 0; i < NUM_OBJECTS; i++) {

		xPos++;

		if (xPos >= 137.f + prevPos) {
			prevPos = xPos;
		}

		if (xPos - prevPos == 0) {
			xPos = 0;
			yPos += 45.f;
		}
		
		arrX[i] = 30.f*xPos;
		arrY[i] = yPos;
		
	}

	// Pintem tots els objectes
	bool drawModelType = true;
	for (int i = 0; i < NUM_OBJECTS; i++) {
		if (drawModelType) {
			MyLoadedModel::updateModel(Transform(glm::vec3(arrX[i], arrY[i], 0.f), glm::radians(-90.f), 1.f, 0.07f));
			MyLoadedModel::drawModel();
		}
		else {
			MyLoadedModel::updateModel2(Transform(glm::vec3(arrX[i], arrY[i], 0.f), glm::radians(-90.f), 1.f, 0.04f));
			MyLoadedModel::drawModel2();
		}
		drawModelType = !drawModelType;
	}

	/*MyLoadedModel::updateModel(Transform(glm::vec3(0.f, 0.f, 0.f), glm::radians(-90.f), 1.f, 0.07f));
	MyLoadedModel::drawModel();*/

	MyLoadedModel::updateModel2(Transform(glm::vec3(0.f, 0.f, 0.f), glm::radians(-90.f), 1.f, 0.04f));
	MyLoadedModel::drawModel2();
}

void Exercise2() {
	/*
	RV::panv[0] = -20.f; // X
	RV::panv[1] = -16.f;   // Y
	RV::panv[2] = -100.f; // Z*/

	MyLoadedModel::updateModel(Transform(glm::vec3(0.f, 0.f, 0.f), glm::radians(-90.f), 1.f, 0.07f));
	MyLoadedModel::drawModelInst();

}

void Exercise3() {

}

////////////////////////////////////////////////// Utils
bool active = true;
#pragma region
void GUI() {
	bool show = true;

	if (exercise == 1)	   ImGui::Begin("Exercise 1", &show, 0);
	else if (exercise == 2)  ImGui::Begin("Exercise 2", &show, 0);
	else if (exercise == 3)  ImGui::Begin("Exercise 3", &show, 0);
	else ImGui::Begin("Welcome!", &show, 0);

	// Do your GUI code here....
	{
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate); // FrameRate

		// Selecció d'exercici
		const char* listbox_items[] = { "Looped", "Instanced", "MultiDrawIndirect" };
		static int listbox_item_current = -1, listbox_item_current2 = -1;
		ImGui::ListBox("", &listbox_item_current, listbox_items, IM_ARRAYSIZE(listbox_items), 3);
		ImGui::PushItemWidth(-1);
		ImGui::PopItemWidth();

		exercise = listbox_item_current + 1;


	}
	// .........................

	ImGui::End();

	// Example code -- ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if (show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 60), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}

bool CheckClickOption() {
	if (exercise == 0) return false;
	else return false;
}

glm::mat4 Transform(glm::vec3 translate, float rotate, int rotAxis, float scale) {
	glm::mat4 t = glm::translate(glm::mat4(), translate);

	glm::mat4 r;
	if (rotAxis == 0)	   r = glm::rotate(glm::mat4(), rotate, glm::vec3(1.f, 0.f, 0.f));
	else if (rotAxis == 1) r = glm::rotate(glm::mat4(), rotate, glm::vec3(0.f, 1.f, 0.f));
	else if (rotAxis == 2) r = glm::rotate(glm::mat4(), rotate, glm::vec3(0.f, 0.f, 1.f));

	glm::mat4 s = glm::scale(glm::mat4(), glm::vec3(scale, scale, scale));

	glm::mat4 myObjMat = t * r * s;
	return myObjMat;
}
#pragma endregion