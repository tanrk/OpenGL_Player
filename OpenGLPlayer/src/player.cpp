

#include "player.h"

#pragma comment(lib, "gltools.lib")

CPlayer player_;

#define ATTRIB_POINTER_SIZE     (2)             // 使用二维坐标表示
#define TRIANGLE_POINTER_NUM    (4)             // 画三角形的点个数

///<Attribute index.
enum
{
	ATTRIB_VERTEX, // 顶点坐标属性索引
	ATTRIB_COLOR,  // 颜色值属性索引
	ATTRIB_TEXCOORD // 纹理坐标属性索引
};

const static char SHADER_VSH[] =
{
	"attribute vec4 position;\n"
	"attribute vec4 texCoords; \n"
	"varying vec4 pp;\n"

	"void main()\n"
	"{\n"
	"	gl_Position = position;\n"
	"	pp = texCoords;\n"
	"}\n"
};


const unsigned int YV12_PARAM_NUM = YV12_TEXTURE_PARAM_NUM;
const static char  SHADER_SFPROGRAMYV12[] =
{
	"varying lowp vec4 pp;"
	"uniform sampler2D Ytexture; "
	"uniform sampler2D Utexture; "
	"uniform sampler2D Vtexture; "
	"void main(void) {	 "
	"  mediump float r,g,b,y,u,v; "

	"  y=texture2D(Ytexture, pp.st).r ; "
	"  u=texture2D(Utexture, pp.st).r ; "
	"  v=texture2D(Vtexture, pp.st).r ; "

	"  y=1.1643*(y-0.0625);	"
	"  u=u-0.5;	"
	"  v=v-0.5;	 "

	"  r=y+1.5958*v ; "
	"  g=y-0.39173*u-0.81290*v;	 "
	"  b=y+2.017*u;	 "
	"  gl_FragColor=vec4(r,g,b,1.0); "
	"} "
};

static const char * const SSHADER_INDEX[YV12_PARAM_NUM] =
{
	"Ytexture",     // Y纹理
	"Utexture",     // U纹理
	"Vtexture",     // V纹理
};

CShader::CShader()
{

}

CShader::~CShader()
{

}

bool CompileShade(GLuint *shader_handle, GLenum type, const char* shader)
{
	if(shader_handle == NULL || shader == NULL)
	{
		return false;
	}

	// 创建着色器program索引
	*shader_handle = glCreateShader(type);
	if(*shader_handle == 0)
	{
		return false;
	}

	// 加载着色器程序
	glShaderSource(*shader_handle, 1, &shader, NULL);

	// 编译着色器程序
	glCompileShader(*shader_handle);

#if defined(DEBUG) || defined(_DEBUG)
	// 检查着色器编译日志信息
	GLint log_length = 0;
	glGetShaderiv(*shader_handle, GL_INFO_LOG_LENGTH, &log_length);
	if(log_length > 0)
	{
		GLchar *pLog = (GLchar*)malloc(log_length);
		if(pLog == NULL)
		{
			return false;
		}
		glGetShaderInfoLog(*shader_handle, log_length, &log_length, pLog);
		printf("Shader Compile Log:%s\n", pLog);
		free(pLog);
	}
#endif

	// 检查着色器状态
	GLint nStatus = 0;
	glGetShaderiv(*shader_handle, GL_COMPILE_STATUS, &nStatus);
	if(nStatus == 0)
	{
		glDeleteShader(*shader_handle);
		*shader_handle = 0;
		return false;
	}

	return true;
}

int LinkShaderProgram(GLuint program_handle)
{
	// 连接指定的着色器program
	glLinkProgram(program_handle);

#if defined(DEBUG) || defined(_DEBUG)
	// 着色器连接日志信息
	GLint log_length = 0;
	glGetProgramiv(program_handle, GL_INFO_LOG_LENGTH, &log_length);
	if(log_length > 0)
	{
		GLchar *log = (GLchar*)malloc(log_length);
		if(log == NULL)
		{
			return false;
		}
		glGetProgramInfoLog(program_handle, log_length, &log_length, log);
		printf("Shader Compile Log:%s\n", log);
		free(log);
	}
#endif

	// 着色器状态检查
	GLint nStatus = 0;
	glGetProgramiv(program_handle, GL_LINK_STATUS, &nStatus);
	if(nStatus == 0)
	{
		return false;
	}

	// 测试着色器program是否有效
	glValidateProgram(program_handle);

#if defined(DEBUG) || defined(_DEBUG)
	// 获取使用着色器出现的警告错误等信息
	glGetProgramiv(program_handle, GL_INFO_LOG_LENGTH, &log_length);
	if(log_length > 0)
	{
		GLchar *log = (GLchar*)malloc(log_length);
		if(log == NULL)
		{
			return false;
		}
		glGetProgramInfoLog(program_handle, log_length, &log_length, log);
		printf("Shader Compile Log:%s\n", log);
		free(log);
	}
#endif

	// 检查着色器的状态
	glGetProgramiv(program_handle, GL_VALIDATE_STATUS, &nStatus);
	if(nStatus == 0)
	{
		return false;
	}

	return true;
}

bool CShader::InitShader(const char* v_shader, const char* s_shader)
{
	bool ret = false;
	do
	{
		program_handle_ = glCreateProgram();
		if (program_handle_ == 0)
		{
			break;
		}

		if (!CompileShade(&vertex_handle_, GL_VERTEX_SHADER, v_shader))
		{
			break;
		}

		if (!CompileShade(&fragment_handle_, GL_FRAGMENT_SHADER, s_shader))
		{
			break;
		}

		glAttachShader(program_handle_, vertex_handle_);
		glAttachShader(program_handle_, fragment_handle_);

		glBindAttribLocation(program_handle_, ATTRIB_VERTEX, "position");
		glBindAttribLocation(program_handle_, ATTRIB_TEXCOORD, "texCoords");
		if (!LinkShaderProgram(program_handle_))
		{
			printf("Failed to link program: %d\n", program_handle_);
			ReleaseShader();
			break;
		}

		glUseProgram(program_handle_);
		for (int i = 0; i < YV12_PARAM_NUM; ++i)
		{
			texture_param_[i] = glGetUniformLocation(program_handle_, SSHADER_INDEX[i]);
		}
		glUseProgram(0);

		ret = true;
	} while (false);

	return ret;
}

void CShader::ReleaseShader()
{
	if (program_handle_ != 0)
	{
		glDetachShader(program_handle_, fragment_handle_);
		glDetachShader(program_handle_, vertex_handle_);
		glDeleteProgram(program_handle_);
		program_handle_ = 0;
	}

	if (vertex_handle_ != 0)
	{
		glDeleteShader(vertex_handle_);
		vertex_handle_ = 0;
	}

	if (fragment_handle_ != 0)
	{
		glDeleteShader(fragment_handle_);
		fragment_handle_ = 0;
	}
}

bool CShader::MakeShaderProgram(bool flag)
{
	if (flag)
	{
		glUseProgram(program_handle_);
		for (int i = 0; i < YV12_PARAM_NUM; ++i)
		{
			glUniform1i(texture_param_[i], i);
		}
	}
	else
	{
		glUseProgram(0);
	}
	return true;
}

CPlayer::CPlayer()
{

}

CPlayer::~CPlayer()
{

}

bool CPlayer::Init(const char* file_path, int width, int height)
{
	bool ret = false;
	if (width > 0 && height > 0 && file_path)
	{
		yuv_width_ = width;
		yuv_height_ = height;
		ret = OpenFile(file_path);
		if (ret)
		{
			if (buf_)
			{
				free(buf_);
				buf_ = NULL;
			}

			buf_ = (unsigned char*)malloc(yuv_height_ * yuv_width_ * 3 / 2);
		}
	}
	return ret && NULL != buf_ && shader_.InitShader(SHADER_VSH, SHADER_SFPROGRAMYV12);
}

bool CPlayer::Refresh()
{
	shader_.MakeShaderProgram(true);
	UpdateFrameYV12Data(buf_, yuv_width_, yuv_height_);

	float texcoords[] = {0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f};

	float square_verties[] =
	{-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};


	// 设置显示矩形坐标
	glVertexAttribPointer(ATTRIB_VERTEX, ATTRIB_POINTER_SIZE, GL_FLOAT, 0, 0, square_verties);
	glEnableVertexAttribArray(ATTRIB_VERTEX);

	// 设置纹理坐标
	glVertexAttribPointer(ATTRIB_TEXCOORD, ATTRIB_POINTER_SIZE, GL_FLOAT, 0, 0, texcoords);
	glEnableVertexAttribArray(ATTRIB_TEXCOORD);

	// 四个点画两个三角形
	glDrawArrays(GL_TRIANGLE_STRIP, 0, TRIANGLE_POINTER_NUM);
	shader_.MakeShaderProgram(false);
	return true;
}

bool CPlayer::ShowNewFrame()
{
	const int size = yuv_width_ * yuv_height_ * 3 / 2;
	if (LoadOneFrameData(buf_, size))
	{
		Refresh();
	}
	return true;
}

void UpdateTexture(unsigned char* frame_data,
				   unsigned int   width,
				   unsigned int   height,
				   unsigned int   pixel_format,
				   unsigned int   texture_name,
				   unsigned int   texture_index,
				   unsigned int   pixel_type)
{
	// 启动指定并绑定纹理资源
	glActiveTexture(texture_name);
	glBindTexture(GL_TEXTURE_2D, texture_index);

	// 生成纹理
	glTexImage2D(GL_TEXTURE_2D,
		0,
		pixel_format,
		width,
		height,
		0,
		pixel_format,
		pixel_type,
		frame_data);

	// 设置纹理拉伸方式-线性滤波
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void CPlayer::UpdateFrameYV12Data(unsigned char* frame_data , unsigned int width , unsigned int height)
{
	///<上传Y分量纹理
	UpdateTexture(frame_data,
		width,
		height,
		GL_LUMINANCE,
		GL_TEXTURE0,
		0,
		GL_UNSIGNED_BYTE);

	///<上传U分量纹理
	UpdateTexture(frame_data + width * height,
		width / 2,
		height / 2,
		GL_LUMINANCE,
		GL_TEXTURE1,
		1,
		GL_UNSIGNED_BYTE);

	///<上传V分量纹理
	UpdateTexture(frame_data + width * height * 5 / 4,
		width / 2,
		height / 2,
		GL_LUMINANCE,
		GL_TEXTURE2,
		2,
		GL_UNSIGNED_BYTE);
}

void CPlayer::UnInit()
{
	if (fp_yuv_)
	{
		fclose(fp_yuv_);
		fp_yuv_ = NULL;
	}
}

bool CPlayer::OpenFile(const char* file_path)
{
	if (fp_yuv_)
	{
		fclose(fp_yuv_);
		fp_yuv_ = NULL;
	}

	fp_yuv_ = fopen(file_path, "rb");
	return NULL != fp_yuv_;
}

bool CPlayer::LoadOneFrameData(void *buf, int size)
{
	const int frame_size = yuv_width_ * yuv_height_ * 3 /2;
	if (size > 0 && size < frame_size && buf && fp_yuv_)
	{
		return false;
	}

	size_t read_length = fread(buf, 1, frame_size, fp_yuv_);
	if (read_length != frame_size)
	{
		fseek(fp_yuv_, 0, SEEK_SET);
	}
	return read_length == frame_size;
}

bool CPlayer::Draw(bool flag)
{
	if (flag)
	{
		return ShowNewFrame();
	}
	else
	{
		return Refresh();
	}
}


///////////////////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering context. 
// This is the first opportunity to do any OpenGL related tasks.
void SetupRC()
{
	printf("SetupRC\n");
	// Black background
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f );
	player_.Draw(true);
}

// Respond to arrow keys by moving the camera frame of reference
void SpecialKeys(int key, int x, int y)
{
// 	GLfloat stepSize = 0.025f;
// 
// 	GLfloat blockX = vVerts[0];   // Upper left X
// 	GLfloat blockY = vVerts[7];  // Upper left Y
// 
// 	if(key == GLUT_KEY_UP)
// 		blockY += stepSize;
// 
// 	if(key == GLUT_KEY_DOWN)
// 		blockY -= stepSize;
// 
// 	if(key == GLUT_KEY_LEFT)
// 		blockX -= stepSize;
// 
// 	if(key == GLUT_KEY_RIGHT)
// 		blockX += stepSize;
// 
// 	// Collision detection
// 	if(blockX < -1.0f) blockX = -1.0f;
// 	if(blockX > (1.0f - blockSize * 2)) blockX = 1.0f - blockSize * 2;
// 	if(blockY < -1.0f + blockSize * 2)  blockY = -1.0f + blockSize * 2;
// 	if(blockY > 1.0f) blockY = 1.0f;
// 
// 	// Recalculate vertex positions
// 	vVerts[0] = blockX;
// 	vVerts[1] = blockY - blockSize*2;
// 
// 	vVerts[3] = blockX + blockSize*2;
// 	vVerts[4] = blockY - blockSize*2;
// 
// 	vVerts[6] = blockX + blockSize*2;
// 	vVerts[7] = blockY;
// 
// 	vVerts[9] = blockX;
// 	vVerts[10] = blockY;
// 
// 	glutPostRedisplay();
}



///////////////////////////////////////////////////////////////////////////////
// Called to draw scene
void RenderScene(void)
{
	printf("RenderScene draw\n");
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);

	player_.Draw(false);

	// Flush drawing commands
	glutSwapBuffers();
}


void TimeEvent(int value)
{
	printf("TimeEvent draw\n");
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);

	player_.Draw(true);

	// Flush drawing commands
	glutSwapBuffers();
	glutTimerFunc(50, TimeEvent, 0);
}

///////////////////////////////////////////////////////////////////////////////
// Window has changed size, or has just been created. In either case, we need
// to use the window dimensions to set the viewport and the projection matrix.
void ChangeSize(int w, int h)
{
	glViewport(0, 0, w, h);
}


///////////////////////////////////////////////////////////////////////////////
// Main entry point for GLUT based programs
int player(int argc, char* argv[])
{
	if (argc >= 4)
	{
		gltSetWorkingDirectory(argv[0]);
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
		glutInitWindowSize(800, 600);
		glutCreateWindow("yuv player");

		GLenum err = glewInit();
		if (GLEW_OK != err)
		{
			// Problem: glewInit failed, something is seriously wrong.
			fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
			return 1;
		}

		player_.Init(argv[1], atoi(argv[2]), atoi(argv[3]));
		glutReshapeFunc(ChangeSize);
		glutDisplayFunc(RenderScene);
		glutSpecialFunc(SpecialKeys);
		glutTimerFunc(50, TimeEvent, 0);
		SetupRC();
		glutMainLoop();
		player_.UnInit();
	}
	else
	{
		printf("cmd param error. enter like that:\n");
		printf("OpenGLPlayer.exe paris.yuv 176 144\n");
		getchar();
	}
	
	return 0;
}
