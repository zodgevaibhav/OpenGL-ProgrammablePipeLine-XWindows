//******** Standerd Library
#include<stdio.h>
#include<iostream>
#include<stdlib.h> //for exit(0) function
#include<memory.h> //for memset() function

#include "vmath.h"

//*******  X11 Library
#include<X11/Xlib.h> //this is main file of Xlib, majority of Xlib symbols are declared in this file.
#include<X11/Xutil.h>  //this library contain symbols required for inter-client communication (?)
#include<X11/XKBlib.h> //for keyboard functions
#include<X11/keysym.h> //for key code to symbol conversion functions


//#include<SOIL/SOIL.h> //for key code to symbol conversion functions

//*******  OpenGL libraries

#include<GL/glew.h>
#include<GL/gl.h>
#include<GL/glu.h>
#include<GL/glx.h>  //bridge library between opengl and X Window

//name space
using namespace std; //directives for all identifier in current name space. Directive means, it help to define the scope to identify the "functions, variables.." within current name space. It also organize the code logically"
using namespace vmath;

enum
{
	VDG_ATTRIBUTE_VERTEX = 0,
	VDG_ATTRIBUTE_COLOR=1,
	VDG_ATTRIBUTE_NORMAL=2,
	VDG_ATTRIBUTE_TEXTURE0=3,
};


bool bFullscreen=false;
Display *gpDisplay=NULL;  //This is used as Handle to XServer which can be used to request the server. All the member under this struct indicase the specifics of XServer that the client (program) connected to.

XVisualInfo *gpXVisualInfo=NULL;  //Single display can support multiple screens, each screen can have multiple visuals types supported at different depth.
Colormap gColormap;  //each xwindow is associated with color map which provides level of indirection between pixel value and color display on the screen.
Window gWindow;  // This struct holds all the information about the Window

int giWindowWidth=800;
int giWindowHeight=600;

float angle = 0.0f;

GLXContext gGLXContext; //Created rendering context

GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;


GLuint gVao_triangle;
GLuint gVbo_triangle;
GLuint gVbo_normal_triangle;



GLuint gMVPUniform;

	
    GLuint  gModelViewMatrixUniform, gProjectionMatrixUniform;
    GLuint  gLdUniform_red, gKdUniform_red, gLightPositionUniform_red;
    GLuint  gLdUniform_blue, gKdUniform_blue, gLightPositionUniform_blue;
	
GLuint gLKeyPressedUniform;
	
float angleRotate=0.0f;
	
bool gbAnimate;
bool gbLight;

mat4 gPerspectiveProjectionMatrix;

int main(void)
{
	void CreateWindow(void);
	void ToggleFullscreen(void);
	void initialize(void);
	void display(void);
	void resize(int, int);
	void uninitialize();
	void update(void);

	int winWidth=giWindowWidth;
	int winHeight = giWindowHeight;
	
	bool bDone=false;

	CreateWindow();

	initialize();

	XEvent event; //Every event in XWindow has its own struct (actually struct XAnyEvent) with type {GraphicExpose etc..}, XEvent is union of all the events.
	KeySym keysym;  //KeySym is the key symbol mapped with key code, when any key press event occur XWindow send KeyPress event alomg with KeyCode of the key pressed. KeyCode and KeySym are mapped together. KeyCode is Physycal Key lies between range of 8 to 255, in geometry fashion, so that it can be interpreted server-dependent fashion and desierd KeySym can be mapped. Mapping between Key and KeyCode can not be changed

	while(bDone==false)
	{
		while(XPending(gpDisplay))  //XPending return the event count (int) that are received from XServer for the display but not yet been removed from queue. If no event in list then it returns 0.
		{
			XNextEvent(gpDisplay,&event);  //get next event from the event queue and then flush it from the queue for the display. If no event received then it flush the output buffer and block until an event is reveived.
			switch(event.type)
				{
					case MapNotify:
						break;
					case KeyPress:
						keysym=XkbKeycodeToKeysym(gpDisplay,event.xkey.keycode,0,0);
						switch(keysym)
							{
								case XK_Escape:
									bDone=true;
									break;

								case XK_F:
								case XK_f:
									if(bFullscreen==false)
										{
											ToggleFullscreen();
											bFullscreen=true;
										}
									else
										{
											ToggleFullscreen();
											bFullscreen=false;	
										}
									break;
								case XK_L:
								case XK_l:
				
					                 	if(gbLight==false)
									gbLight=true;
								else
									gbLight=false;
								default:
									break;
							}
						break;
					
					case ButtonPress:
						switch(event.xbutton.button)
						{
							case 1:
								break;
							case 2:
								break;
							case 3:
								break;
							default:
								break;
						}
						break;
					case MotionNotify:
						break;
					case ConfigureNotify:
						winWidth=event.xconfigure.width;
						winHeight=event.xconfigure.height;
						resize(winWidth,winHeight);
						break;
					case Expose:
						break;
					case DestroyNotify:
						break;
					case 33:
						bDone=true;
						break;
					default:
						break;
				}
		}
		update();
		display();
	}
	return(0);
}

void CreateWindow(void)
{
	void uninitialize(void);
	
	XSetWindowAttributes winAttribs;

	int defaultScreen;
	int defaultDepth;
	int styleMask;
	
	static int frameBufferAttributes[]=
		{
			GLX_RGBA, //Rendering type
			GLX_RED_SIZE,8,    //for double buffer it is set to 8 lease it is 1 for single buffer
			GLX_GREEN_SIZE,8,
			GLX_BLUE_SIZE,8,
			GLX_ALPHA_SIZE,8,
			GLX_DEPTH_SIZE,24,   //for double buffer it is set to 24
			GLX_DOUBLEBUFFER,True,
			None  //macro is used to mention array is getting stopped
		};
	gpDisplay=XOpenDisplay(NULL); //before program start use of display, we must create connection to XServer. To create connection to XServer we use XOpenDisplay function/macro. 
				// parameter to this function is char* displayName. if we have multiple display available then we can provide which display to be used. If we want to use default screen connected, then we can send NULL, it is POSICS standerd.

	if(gpDisplay==NULL)
	{
		printf("ERROR: Unable tp create connection to XServer. \n Exitting now... \n");
		uninitialize();
		exit(1);	
	}

	defaultScreen = XDefaultScreen(gpDisplay);

	gpXVisualInfo=glXChooseVisual(gpDisplay,defaultScreen,frameBufferAttributes);

	winAttribs.border_pixel=0;
	winAttribs.background_pixmap=0;
	winAttribs.colormap=XCreateColormap(gpDisplay,
						RootWindow(gpDisplay,gpXVisualInfo->screen),
						gpXVisualInfo->visual,
						AllocNone);
	gColormap=winAttribs.colormap;
	
	winAttribs.background_pixel=BlackPixel(gpDisplay,defaultScreen);
	
	winAttribs.event_mask=ExposureMask|VisibilityChangeMask|ButtonPressMask|KeyPressMask|PointerMotionMask|StructureNotifyMask;

	styleMask=CWBorderPixel|CWBackPixel|CWEventMask|CWColormap;

	gWindow=XCreateWindow(gpDisplay,
				RootWindow(gpDisplay,gpXVisualInfo->screen),
				0,
				0,
				giWindowWidth,
				giWindowHeight,
				0,
				gpXVisualInfo->depth,
				InputOutput,
				gpXVisualInfo->visual,
				styleMask,
				&winAttribs);

	if(!gWindow)
	{
		printf("ERROR: Unable to create window.\n Exiting now... \n");
		uninitialize();
		exit(1);
	}

	XStoreName(gpDisplay,gWindow, "Two Phong Light on rotating Pyramid");

	Atom windowManagerDelete=XInternAtom(gpDisplay, "WM_DELETE_WINDOW",True);

	XSetWMProtocols(gpDisplay,gWindow,&windowManagerDelete,1);

	XMapWindow(gpDisplay,gWindow);
	
}

void uninitialize(void)
{
	GLXContext currentGLXContext;
	currentGLXContext = glXGetCurrentContext();

	if(currentGLXContext!=NULL && currentGLXContext==gGLXContext)
	{
		glXMakeCurrent(gpDisplay,0,0);
	}

	if(gGLXContext)
	{
		glXDestroyContext(gpDisplay, gGLXContext);
	}
	if(gWindow)
	{
		XDestroyWindow(gpDisplay,gWindow);
	}
	if(gColormap)
	{
		XFreeColormap(gpDisplay,gColormap);
	}
	if(gpXVisualInfo)
	{
		free(gpXVisualInfo);
		gpXVisualInfo=NULL;
	}
	if(gpDisplay)
	{
		XCloseDisplay(gpDisplay);
		gpDisplay=NULL;	
	}
}


void initialize(void)
{
	void resize(int,int);
	
	gGLXContext=glXCreateContext(gpDisplay, gpXVisualInfo,NULL,GL_TRUE);

	glXMakeCurrent(gpDisplay,gWindow,gGLXContext);

	glewInit();

//**************************************** Vertex shader **********************************************
	gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	const GLchar *vertexShaderSourceCode =
		"#version 430 core"\
		"\n"\
			"in vec4 vPosition;" \
						"in vec3 vNormal;" \
						"uniform int u_LKeyPressed;" \
						"uniform mat4 u_model_view_matrix;" \
						"uniform mat4 u_projection_matrix;" \
		/*Light intensity*/		"uniform vec3 u_Ld_red;" \
		/*Light material*/		"uniform vec3 u_Kd_red;" \
						"uniform vec4 u_light_position_red;" \
		/*Light intensity*/		"uniform vec3 u_Ld_blue;" \
		/*Light material*/		"uniform vec3 u_Kd_blue;" \
						"uniform vec4 u_light_position_blue;" \
						"out vec3 diffuse_light;" \
						"void main(void)" \
						"{" \
							"if (u_LKeyPressed == 1)" \
							"{" \
								"vec4 eyeCoordinates = u_model_view_matrix * vPosition;" \
								"vec3 tnorm = normalize(mat3(u_model_view_matrix) * vNormal);" \
								"vec3 s = normalize(vec3(u_light_position_red - eyeCoordinates));" \
								"diffuse_light = u_Ld_red * u_Kd_red * max(dot(s, tnorm), 0.0);" \
								" s = normalize(vec3(u_light_position_blue - eyeCoordinates));" \
							"diffuse_light =diffuse_light+( u_Ld_blue * u_Kd_blue * max(dot(s, tnorm), 0.0));" \
							"}" \
						"gl_Position = u_projection_matrix * u_model_view_matrix * vPosition;" \
						"}";

	glShaderSource(gVertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);

	//******************* Compile Vertex shader 
	glCompileShader(gVertexShaderObject);

	GLint iInfoLogLength = 0;
	GLint iShaderCompiledStatus = 0;
	char *szInfoLog = NULL;
	glGetShaderiv(gVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if (iShaderCompiledStatus == GL_FALSE)
	{
		glGetShaderiv(gVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gVertexShaderObject, iInfoLogLength, &written, szInfoLog);
				printf("***** Vertex Shader Compilation Log : %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}

	//**************************************** Fragment shader **********************************************
	gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar *fragmentShaderSourceCode =
	"#version 430 core"\
		"\n"\
		"in vec3 diffuse_light;" \
		"out vec4 FragColor;" \
		"uniform int u_LKeyPressed;" \
		"void main(void)" \
			"{" \
			"vec4 color;" \
			"if (u_LKeyPressed == 1)" \
				"{" \
					"color = vec4(diffuse_light,1.0);" \
				"}" \
				"else" \
				"{" \
					"color = vec4(1.0, 1.0, 1.0, 1.0);" \
				"}" \
			"FragColor = color;" \
		"}";

	glShaderSource(gFragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);

	//******************* Compile fragment shader 

	glCompileShader(gFragmentShaderObject);
	glGetShaderiv(gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if (iShaderCompiledStatus == GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gFragmentShaderObject, iInfoLogLength, &written, szInfoLog);
				printf("***** Fragment Shader Compilation Log : %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}

	//**************************************** Shader program attachment **********************************************
	// Code from sir

	gShaderProgramObject = glCreateProgram();

	// attach vertex shader to shader program
	glAttachShader(gShaderProgramObject, gVertexShaderObject);

	// attach fragment shader to shader program
	glAttachShader(gShaderProgramObject, gFragmentShaderObject);

	//**************************************** Link Shader program **********************************************
	glLinkProgram(gShaderProgramObject);
	GLint iShaderProgramLinkStatus = 0;
	glGetProgramiv(gShaderProgramObject, GL_LINK_STATUS, &iShaderProgramLinkStatus);
	if (iShaderProgramLinkStatus == GL_FALSE)
	{
		glGetProgramiv(gShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength>0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(gShaderProgramObject, iInfoLogLength, &written, szInfoLog);
				printf("Shader Program Link Log : %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}

	//**************************************** END Link Shader program **********************************************

	gModelViewMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_model_view_matrix");
	gProjectionMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");

	gLKeyPressedUniform = glGetUniformLocation(gShaderProgramObject, "u_LKeyPressed");

	gLdUniform_red = glGetUniformLocation(gShaderProgramObject, "u_Ld_red");
	gKdUniform_red = glGetUniformLocation(gShaderProgramObject, "u_Kd_red");
	gLightPositionUniform_red = glGetUniformLocation(gShaderProgramObject, "u_light_position_red");;

	gLdUniform_blue = glGetUniformLocation(gShaderProgramObject, "u_Ld_blue");
	gKdUniform_blue = glGetUniformLocation(gShaderProgramObject, "u_Kd_blue");
	gLightPositionUniform_blue = glGetUniformLocation(gShaderProgramObject, "u_light_position_blue");;

	const GLfloat triangleVertices[] =
	{ 
		0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,

		0.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,

		0.0f, 1.0f, 0.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,

		0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f
	};


	glGenVertexArrays(1, &gVbo_triangle);
	glBindVertexArray(gVao_triangle);

	glGenBuffers(1, &gVbo_triangle);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_triangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//************************************************* Triangle Color
	const GLfloat triangleNormals[] =
	{ 
		0.0f, 0.447214f, 0.894427f, 
		0.894427f, 0.447214f, 0.0f,
		0.0f, 0.447214f, -0.894427f,
		-0.894427f, 0.447214f, 0.0f
	};

	glGenVertexArrays(1, &gVbo_normal_triangle);
	glBindVertexArray(gVao_triangle);

	glGenBuffers(1, &gVbo_normal_triangle);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_normal_triangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleNormals), triangleNormals, GL_STATIC_DRAW);

	glVertexAttribPointer(VDG_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(VDG_ATTRIBUTE_NORMAL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	//*************************************************
	

//************************************************************************************************************************************


	glClearColor(0.0f,0.0f,0.0f,0.0f);
	
	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

}

void display(void)
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(gShaderProgramObject);

	if (gbLight == true)
	{
		glUniform1i(gLKeyPressedUniform, 1);

		glUniform3f(gLdUniform_red, 1.0f, 0.0f, 0.0f);
		glUniform3f(gKdUniform_red, 0.5f, 0.5f, 0.5f);

		float lightPosition[] = {-1.0f, 0.5f, 2.0f, 1.0f };
		glUniform4fv(gLightPositionUniform_red, 1, (GLfloat *)lightPosition);

		glUniform3f(gLdUniform_blue, 0.0f, 0.0f, 1.0f);
		glUniform3f(gKdUniform_blue, 0.5f, 0.5f, 0.5f);

		float lightPosition_blue[] = {1.0f, 0.5f, 2.0f, 1.0f };
		glUniform4fv(gLightPositionUniform_blue, 1, (GLfloat *)lightPosition_blue);

	}
	else
	{
		glUniform1i(gLKeyPressedUniform, 0);
	}


	//******************************* Triangle block **********************************
	mat4 modelMatrix = mat4::identity();
	mat4 modelViewMatrix = mat4::identity();
	mat4 rotationMatrix = mat4::identity();

	// apply z axis translation to go deep into the screen by -5.0,
	// so that triangle with same fullscreen co-ordinates, but due to above translation will look small
	modelMatrix = translate(0.0f, 0.0f, -5.0f);

	// all axes rotation by gAngle angle
	rotationMatrix = rotate(0.0f,angleRotate,0.0f);

	// multiply rotation matrix and model matrix to get modelView matrix
	modelViewMatrix = modelMatrix * rotationMatrix; // ORDER IS IMPORTANT

													// pass modelview matrix to the vertex shader in 'u_model_view_matrix' shader variable
													// whose position value we already calculated in initialize() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelViewMatrixUniform, 1, GL_FALSE, modelViewMatrix);

	// pass projection matrix to the vertex shader in 'u_projection_matrix' shader variable
	// whose position value we already calculated in initialize() by using glGetUniformLocation()
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);

	glBindVertexArray(gVao_triangle);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 12);

	glBindVertexArray(0);



	glXSwapBuffers(gpDisplay,gWindow);
}

void resize(int width, int height)
{
	
	if (height == 0)
		height = 1;

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

		gPerspectiveProjectionMatrix = perspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);
}

void ToggleFullscreen(void)
{
	Atom wm_state;
	Atom fullscreen;
	XEvent xev={0};
	
	wm_state=XInternAtom(gpDisplay,"_NET_WM_STATE",False);
	memset(&xev,0,sizeof(xev));

	xev.type=ClientMessage;
	xev.xclient.window=gWindow;
	xev.xclient.message_type=wm_state;
	xev.xclient.format=32;
	xev.xclient.data.l[0]=bFullscreen ? 0 : 1;

	fullscreen=XInternAtom(gpDisplay,"_NET_WM_STATE_FULLSCREEN",False);

	xev.xclient.data.l[1]=fullscreen;

	XSendEvent(gpDisplay,
		RootWindow(gpDisplay,gpXVisualInfo->screen),
		False,
		StructureNotifyMask,
		&xev);
}

void update(void)
{
	//code
	angleRotate = angleRotate + 0.05f;
	if (angleRotate >= 360.0f)
		angleRotate = 0.0f;

}
