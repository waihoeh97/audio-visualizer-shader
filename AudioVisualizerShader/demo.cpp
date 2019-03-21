#define GLFW_INCLUDE_ES2 1
#define GLFW_DLL 1
//#define GLFW_EXPOSE_NATIVE_WIN32 1
//#define GLFW_EXPOSE_NATIVE_EGL 1

#include <GLES2/gl2.h>
#include <EGL/egl.h>

#include <GLFW/glfw3.h>
//#include <GLFW/glfw3native.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <fstream> 
#include "bitmap.h"
#include <fmod.hpp>
#include <fmod_errors.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define TEXTURE_COUNT 2
#define PI 3.14159265359

#define SPECTRUM_SIZE 512

FMOD::System* m_fmodSystem;
FMOD::Sound* m_music;
FMOD::Channel* m_musicChannel;
float m_spectrumLeft[SPECTRUM_SIZE];
float m_spectrumRight[SPECTRUM_SIZE];
float spectrumAverage;

GLint GprogramID = -1;
GLuint GtextureID[TEXTURE_COUNT];

GLFWwindow* window;

void ERRCHECK(FMOD_RESULT result)
{
	if (result != FMOD_OK)
	{
		//printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		std::cout << "FMOD ERROR" << FMOD_ErrorString(result) << std::endl;
	}
}

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

GLuint LoadShader ( GLenum type, const char *shaderSrc )
{
	GLuint shader;
	GLint compiled;
   
	// Create the shader object
	shader = glCreateShader ( type );

	if ( shader == 0 )
	return 0;

	// Load the shader source
	glShaderSource ( shader, 1, &shaderSrc, NULL );
   
	// Compile the shader
	glCompileShader ( shader );

	// Check the compile status
	glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

	if ( !compiled ) 
	{
		GLint infoLen = 0;

		glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );
      
		if ( infoLen > 1 )
		{
		// char* infoLog = malloc (sizeof(char) * infoLen );
			char infoLog[512];
			glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
			printf ( "Error compiling shader:\n%s\n", infoLog );
         
		// free ( infoLog );
		}

		glDeleteShader ( shader );
		return 0;
	}

	return shader;
}

GLuint LoadShaderFromFile(GLenum shaderType, std::string path)
{
    GLuint shaderID = 0;
    std::string shaderString;
    std::ifstream sourceFile( path.c_str() );

    if( sourceFile )
    {
        shaderString.assign( ( std::istreambuf_iterator< char >( sourceFile ) ), 
								std::istreambuf_iterator< char >() );
        const GLchar* shaderSource = shaderString.c_str();

		return LoadShader(shaderType, shaderSource);
    }
    else
        printf( "Unable to open file %s\n", path.c_str() );

    return shaderID;
}

void loadTexture(const char* path, GLuint TextureID)
{
	CBitmap bitmap(path);

	//create lineear filtered Texture
	glBindTexture(GL_TEXTURE_2D, TextureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//nearest (minecraft) filtering
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//bilinear filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap.GetWidth(), bitmap.GetHeight(),
	0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.GetBits());

}

int Init ( void )
{
	//=============init fmod==========================================================================
	FMOD_RESULT  result;
	unsigned int version;

	result = FMOD::System_Create(&m_fmodSystem);
	ERRCHECK(result);

	/*result = m_fmodSystem->getVersion(&version);
	ERRCHECK(result);

	if (version < FMOD_VERSION) {
	printf("FMOD Error! You are using and old version of FMOD.", version, FMOD_VERSION);
	}*/

	// Initialize fmod system
	result = m_fmodSystem->init(32, FMOD_INIT_NORMAL, 0);
	ERRCHECK(result);

	// load and set up music
	result = m_fmodSystem->createStream("../media/M.mp3", FMOD_SOFTWARE, 0, &m_music);
	ERRCHECK(result);

	// play loaded mp3 music
	result = m_fmodSystem->playSound(FMOD_CHANNEL_FREE, m_music, false, &m_musicChannel);
	ERRCHECK(result);
	//=================================================================================================

	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint programObject;
	GLint linked;

	// Load the textures
	glGenTextures(TEXTURE_COUNT, GtextureID);
	loadTexture("../media/bokeh.bmp", GtextureID[0]);

	// Load the vertex/fragment shaders
	//vertexShader = LoadShader ( GL_VERTEX_SHADER, vShaderStr );
	//fragmentShader = LoadShader ( GL_FRAGMENT_SHADER, fShaderStr );

	vertexShader = LoadShaderFromFile(GL_VERTEX_SHADER, "../vertexShader1.vert" );
	fragmentShader = LoadShaderFromFile(GL_FRAGMENT_SHADER, "../fragmentShader1.frag" );


	// Create the program object
	programObject = glCreateProgram ( );
   
	if ( programObject == 0 )
		return 0;

	glAttachShader ( programObject, vertexShader );
	glAttachShader ( programObject, fragmentShader );

	// Bind vPosition to attribute 0   
	glBindAttribLocation ( programObject, 0, "vPosition" );
	glBindAttribLocation(programObject, 1, "vColor");
	glBindAttribLocation(programObject, 2, "vTexCoord");

	// Link the program
	glLinkProgram ( programObject );

	// Check the link status
	glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );

	if ( !linked ) 
	{
		GLint infoLen = 0;

		glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );
      
		if ( infoLen > 1 )
		{
			//char* infoLog = malloc (sizeof(char) * infoLen );
			char infoLog[512];
			glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
			printf ( "Error linking program:\n%s\n", infoLog );
         
			//free ( infoLog );
		}

		glDeleteProgram ( programObject );
		return 0;
	}

	// Store the program object
	GprogramID = programObject;

	glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
	return 1;
}

void updateFmod()
{
	m_fmodSystem->update();

	//set spectrum for left and right stereo channel
	m_musicChannel->getSpectrum(m_spectrumLeft, SPECTRUM_SIZE, 0, FMOD_DSP_FFT_WINDOW_RECT);
	m_musicChannel->getSpectrum(m_spectrumRight, SPECTRUM_SIZE, 0, FMOD_DSP_FFT_WINDOW_RECT);

	spectrumAverage = (m_spectrumLeft[0] + m_spectrumRight[0]) / 2.0f;

	//point the first audio spectrum for both left and right channels
	//std::cout << m_spectrumLeft[0] << ", " << m_spectrumRight[0] << std::endl;
}

void Draw(void)
{
	glUniform1i(glGetUniformLocation(GprogramID, "sampler2D"), 0);

	static float wave1 = 0.0f;
	static float wave2 = 0.0f;
	static float wave3 = 0.0f;
	static float wave4 = 0.0f;
	static float wave5 = 0.0f;
	static float wave6 = 1.0f;

	float move1 = 0.0f;

	move1 += 0.01f + (spectrumAverage * 0.05f);


	wave1 = 0.5f + (spectrumAverage * 5.0f);
	wave2 += spectrumAverage + 0.1f;
	wave3 -= 0.01f;
	wave4 = 2.0 + sinf(2.0f / 10.0f);
	wave5 += 0.01f;

	if (wave3 < -10.0f)
	{
		wave3 += 10.0f;
	}

	if (wave5 > 10.0f)
	{
		wave5 -= 10.0f;
	}

	if (wave6 > 1.2f)
	{
		wave6 -= spectrumAverage;
	}
	else
	{
		wave6 += spectrumAverage;
	}

	GLint factor1Loc = glGetUniformLocation(GprogramID, "Wave1");
	GLint factor2Loc = glGetUniformLocation(GprogramID, "Wave2");
	GLint factor3Loc = glGetUniformLocation(GprogramID, "Wave3");
	GLint factor4Loc = glGetUniformLocation(GprogramID, "Wave4");
	GLint factor5Loc = glGetUniformLocation(GprogramID, "Wave5");
	GLint factor6Loc = glGetUniformLocation(GprogramID, "Wave6");

	if (wave1 != 1)
	{
		glUniform1f(factor1Loc, wave1);
	}
	if (wave2 != 1)
	{
		glUniform1f(factor2Loc, wave2);
	}
	if (wave3 != 1)
	{
		glUniform1f(factor3Loc, wave3);
	}
	if (wave4 != 1)
	{
		glUniform1f(factor4Loc, wave4);
	}
	if (wave5 != 1)
	{
		glUniform1f(factor5Loc, wave5);
	}
	if (wave6 != 1)
	{
		glUniform1f(factor6Loc, wave6);
	}

	/*GLfloat vVertices[] = {
		0.0f,  0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f,  0.0f
	};*/

	GLfloat vVertices[] = {
		-wave6, wave6, 0.0f,
		-wave6, -wave6, 0.0f,
		wave6, -wave6, 0.0f,

		wave6, -wave6, 0.0f,
		wave6, wave6, 0.0f,
		-wave6, wave6, 0.0f,
	};

	GLfloat vColors[] = { 
		1.0f,  0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f,  0.0f, 1.0f, 1.0f,
		0.0f,  0.0f, 1.0f, 1.0f,
		1.0f,  1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 1.0f,
	};

	GLfloat vTexCoords[] = {
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};

	glBindTexture(GL_TEXTURE_2D, GtextureID[0]);

	/*	Full screen triangle
	GLfloat vVertices[] = {0.0f,  1.0f, 0.0f,
							-1.0f, -1.0f, 0.0f,
							1.0f, -1.0f,  0.0f};*/

	// Set the viewport
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	// Clear the color buffer
	glClear(GL_COLOR_BUFFER_BIT);

	// Use the program object
	glUseProgram(GprogramID);

	// Load the vertex data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, vColors);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, vTexCoords);
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	updateFmod();
}

int main(void)
{
	glfwSetErrorCallback(error_callback);

	// Initialize GLFW library
	if (!glfwInit())
	return -1;

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	// Create and open a window
	window = glfwCreateWindow(WINDOW_WIDTH,
							WINDOW_HEIGHT,
							"Audio Visualizer",
							NULL,
							NULL);

	if (!window)
	{
	glfwTerminate();
	printf("glfwCreateWindow Error\n");
	exit(1);
	}

	glfwMakeContextCurrent(window);

	Init();

	// Repeat
	while (!glfwWindowShouldClose(window)) {


	Draw();
	glfwSwapBuffers(window);
	glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
