#pragma once

#include "GLTools.h"
#ifdef __APPLE__
#include "glut/glut.h"
#else
#define FREEGLUT_STATIC
#include "GL/glut.h"
#endif

#include <stdio.h>

#define YV12_TEXTURE_PARAM_NUM  3

int player(int argc, char* argv[]);

class CShader
{
public:
	CShader();
	~CShader();

	bool InitShader(const char* v_shader, const char* s_shader);
	bool MakeShaderProgram(bool flag);

private:
	void ReleaseShader();

	GLuint vertex_handle_;
	GLuint fragment_handle_;
	GLint texture_param_[YV12_TEXTURE_PARAM_NUM];
	GLuint program_handle_;
};

class CPlayer
{
public:
	CPlayer();
	~CPlayer();

	bool Init(const char* file_path, int width, int height);
	bool Draw(bool flag);
	void UnInit();

protected:
	bool OpenFile(const char* file_path);
	bool LoadOneFrameData(void *buf, int size);
	void UpdateFrameYV12Data(unsigned char* frame_data
		, unsigned int width
		, unsigned int height);
	bool ShowNewFrame();
	bool Refresh();

private:
	FILE *fp_yuv_;
	int yuv_width_;
	int yuv_height_;
	unsigned char *buf_;
	CShader shader_;
};