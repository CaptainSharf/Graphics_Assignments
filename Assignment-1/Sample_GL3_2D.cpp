#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}
/**************************
 * Customizable functions *
 **************************/
 float canon_rotation = 90;
glm::mat4 trnsfrm;
int amm_amount=4;
int num_appear=0;
int num_coins=4;
float time_travelled=0;
float flag=0;
float use=0;
float velocity_x=(float)7*cos(canon_rotation*M_PI/180.0f);
float velocity_y=(float)7*sin(canon_rotation*M_PI/180.0f);
float position_x;
float position_y;
int bounce=0;
 void initlz()
{
trnsfrm = glm::mat4(1.0f);
time_travelled=0;
flag=0;
velocity_x=(float)7*cos(canon_rotation*M_PI/180.0f);
velocity_y=(float)7*sin(canon_rotation*M_PI/180.0f);
position_x;
position_y;
bounce=0;
}

class coins{
    public:int pos_x;
    public:int pos_y;
    public:int appear;
public:
    void set_coins(int x,int y)
    {
        pos_x=x;
        pos_y=y;
        appear=1;
    }
};
coins coin[4];
void dist(coins coin[])
{
    for(int i=0;i<num_coins;i++)
    {
        if(abs(position_x-coin[i].pos_x)<=0.4f or abs(position_y-coin[i].pos_y)<=0.4f)
            coin[i].appear=0;
    }
}
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{


    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_C:
                
                break;
            case GLFW_KEY_P:
                
                break;
            case GLFW_KEY_X:
                // do something ..
                break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            case GLFW_KEY_U:
                if(canon_rotation<90)
                  canon_rotation+=10;
                break;
            case GLFW_KEY_D:
                if(canon_rotation>0)
                  canon_rotation-=10;
                break;
            case GLFW_KEY_N:
                if(amm_amount>0)
                {
                    use=1;
                    amm_amount--;
                    initlz();
                }


            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
              
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
               
            }
            break;
        default:
            break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

    Matrices.projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 500.0f);
}

VAO *rectangle,*canon,*gun,*bullet;
void createBullet()
{
  static const GLfloat vertex_buffer_data[]={
     0,0,0,
    cos(15*M_PI/180.0f),sin(15*M_PI/180.0f),0,
    cos(2*15*M_PI/180.0f),sin(2*15*M_PI/180.0f),0,
    0,0,0,
    cos(2*15*M_PI/180.0f),sin(2*15*M_PI/180.0f),0,
    cos(3*15*M_PI/180.0f),sin(3*15*M_PI/180.0f),0,
    0,0,0,
    cos(3*15*M_PI/180.0f),sin(3*15*M_PI/180.0f),0,
    cos(4*15*M_PI/180.0f),sin(4*15*M_PI/180.0f),0,
    0,0,0,
    cos(4*15*M_PI/180.0f),sin(4*15*M_PI/180.0f),0,
    cos(5*15*M_PI/180.0f),sin(5*15*M_PI/180.0f),0,
    0,0,0,
    cos(5*15*M_PI/180.0f),sin(5*15*M_PI/180.0f),0,
    cos(6*15*M_PI/180.0f),sin(6*15*M_PI/180.0f),0,
    0,0,0,
    cos(6*15*M_PI/180.0f),sin(6*15*M_PI/180.0f),0,
    cos(7*15*M_PI/180.0f),sin(7*15*M_PI/180.0f),0,
    0,0,0,
    cos(7*15*M_PI/180.0f),sin(7*15*M_PI/180.0f),0,
    cos(8*15*M_PI/180.0f),sin(8*15*M_PI/180.0f),0,
    0,0,0,
    cos(8*15*M_PI/180.0f),sin(8*15*M_PI/180.0f),0,
    cos(9*15*M_PI/180.0f),sin(9*15*M_PI/180.0f),0,
    0,0,0,
    cos(9*15*M_PI/180.0f),sin(9*15*M_PI/180.0f),0,
    cos(10*15*M_PI/180.0f),sin(10*15*M_PI/180.0f),0,
    0,0,0,
    cos(10*15*M_PI/180.0f),sin(10*15*M_PI/180.0f),0,
    cos(11*15*M_PI/180.0f),sin(11*15*M_PI/180.0f),0,
    0,0,0,
    cos(11*15*M_PI/180.0f),sin(11*15*M_PI/180.0f),0,
    cos(12*15*M_PI/180.0f),sin(12*15*M_PI/180.0f),0,
    0,0,0,
    cos(12*15*M_PI/180.0f),sin(12*15*M_PI/180.0f),0,
    cos(13*15*M_PI/180.0f),sin(13*15*M_PI/180.0f),0,
    0,0,0,
    cos(13*15*M_PI/180.0f),sin(13*15*M_PI/180.0f),0,
    cos(14*15*M_PI/180.0f),sin(14*15*M_PI/180.0f),0,
    0,0,0,
    cos(14*15*M_PI/180.0f),sin(14*15*M_PI/180.0f),0,
    cos(15*15*M_PI/180.0f),sin(15*15*M_PI/180.0f),0,
    0,0,0,
    cos(15*15*M_PI/180.0f),sin(15*15*M_PI/180.0f),0,
    cos(16*15*M_PI/180.0f),sin(16*15*M_PI/180.0f),0,
    0,0,0,
    cos(16*15*M_PI/180.0f),sin(16*15*M_PI/180.0f),0,
    cos(17*15*M_PI/180.0f),sin(17*15*M_PI/180.0f),0,
    0,0,0,
    cos(17*15*M_PI/180.0f),sin(17*15*M_PI/180.0f),0,
    cos(18*15*M_PI/180.0f),sin(18*15*M_PI/180.0f),0,
    0,0,0,
    cos(18*15*M_PI/180.0f),sin(18*15*M_PI/180.0f),0,
    cos(19*15*M_PI/180.0f),sin(19*15*M_PI/180.0f),0,
    0,0,0,
    cos(19*15*M_PI/180.0f),sin(19*15*M_PI/180.0f),0,
    cos(20*15*M_PI/180.0f),sin(20*15*M_PI/180.0f),0,
    0,0,0,
    cos(20*15*M_PI/180.0f),sin(20*15*M_PI/180.0f),0,
    cos(21*15*M_PI/180.0f),sin(21*15*M_PI/180.0f),0,
    0,0,0,
    cos(21*15*M_PI/180.0f),sin(21*15*M_PI/180.0f),0,
    cos(22*15*M_PI/180.0f),sin(22*15*M_PI/180.0f),0,
    0,0,0,
    cos(22*15*M_PI/180.0f),sin(22*15*M_PI/180.0f),0,
    cos(23*15*M_PI/180.0f),sin(23*15*M_PI/180.0f),0,
    0,0,0,
    cos(23*15*M_PI/180.0f),sin(23*15*M_PI/180.0f),0,
    cos(24*15*M_PI/180.0f),sin(24*15*M_PI/180.0f),0,
    0,0,0,
    cos(24*15*M_PI/180.0f),sin(24*15*M_PI/180.0f),0,
    cos(25*15*M_PI/180.0f),sin(25*15*M_PI/180.0f),0,
    0,0,0,
    cos(25*15*M_PI/180.0f),sin(25*15*M_PI/180.0f),0,
    cos(26*15*M_PI/180.0f),sin(26*15*M_PI/180.0f),0,
    0,0,0,
    cos(26*15*M_PI/180.0f),sin(26*15*M_PI/180.0f),0,
    cos(27*15*M_PI/180.0f),sin(27*15*M_PI/180.0f),0

  };
  static const GLfloat color_buffer_data []={
     0,0,0,
    cos(15*M_PI/180.0f),sin(15*M_PI/180.0f),0,
    cos(2*15*M_PI/180.0f),sin(2*15*M_PI/180.0f),0,
    0,0,0,
    cos(2*15*M_PI/180.0f),sin(2*15*M_PI/180.0f),0,
    cos(3*15*M_PI/180.0f),sin(3*15*M_PI/180.0f),0,
    0,0,0,
    cos(3*15*M_PI/180.0f),sin(3*15*M_PI/180.0f),0,
    cos(4*15*M_PI/180.0f),sin(4*15*M_PI/180.0f),0,
    0,0,0,
    cos(4*15*M_PI/180.0f),sin(4*15*M_PI/180.0f),0,
    cos(5*15*M_PI/180.0f),sin(5*15*M_PI/180.0f),0,
    0,0,0,
    cos(5*15*M_PI/180.0f),sin(5*15*M_PI/180.0f),0,
    cos(6*15*M_PI/180.0f),sin(6*15*M_PI/180.0f),0,
    0,0,0,
    cos(6*15*M_PI/180.0f),sin(6*15*M_PI/180.0f),0,
    cos(7*15*M_PI/180.0f),sin(7*15*M_PI/180.0f),0,
    0,0,0,
    cos(7*15*M_PI/180.0f),sin(7*15*M_PI/180.0f),0,
    cos(8*15*M_PI/180.0f),sin(8*15*M_PI/180.0f),0,
    0,0,0,
    cos(8*15*M_PI/180.0f),sin(8*15*M_PI/180.0f),0,
    cos(9*15*M_PI/180.0f),sin(9*15*M_PI/180.0f),0,
    0,0,0,
    cos(9*15*M_PI/180.0f),sin(9*15*M_PI/180.0f),0,
    cos(10*15*M_PI/180.0f),sin(10*15*M_PI/180.0f),0,
    0,0,0,
    cos(10*15*M_PI/180.0f),sin(10*15*M_PI/180.0f),0,
    cos(11*15*M_PI/180.0f),sin(11*15*M_PI/180.0f),0,
    0,0,0,
    cos(11*15*M_PI/180.0f),sin(11*15*M_PI/180.0f),0,
    cos(12*15*M_PI/180.0f),sin(12*15*M_PI/180.0f),0,
    0,0,0,
    cos(12*15*M_PI/180.0f),sin(12*15*M_PI/180.0f),0,
    cos(13*15*M_PI/180.0f),sin(13*15*M_PI/180.0f),0,
    0,0,0,
    cos(13*15*M_PI/180.0f),sin(13*15*M_PI/180.0f),0,
    cos(14*15*M_PI/180.0f),sin(14*15*M_PI/180.0f),0,
    0,0,0,
    cos(14*15*M_PI/180.0f),sin(14*15*M_PI/180.0f),0,
    cos(15*15*M_PI/180.0f),sin(15*15*M_PI/180.0f),0,
    0,0,0,
    cos(15*15*M_PI/180.0f),sin(15*15*M_PI/180.0f),0,
    cos(16*15*M_PI/180.0f),sin(16*15*M_PI/180.0f),0,
    0,0,0,
    cos(16*15*M_PI/180.0f),sin(16*15*M_PI/180.0f),0,
    cos(17*15*M_PI/180.0f),sin(17*15*M_PI/180.0f),0,
    0,0,0,
    cos(17*15*M_PI/180.0f),sin(17*15*M_PI/180.0f),0,
    cos(18*15*M_PI/180.0f),sin(18*15*M_PI/180.0f),0,
    0,0,0,
    cos(18*15*M_PI/180.0f),sin(18*15*M_PI/180.0f),0,
    cos(19*15*M_PI/180.0f),sin(19*15*M_PI/180.0f),0,
    0,0,0,
    cos(19*15*M_PI/180.0f),sin(19*15*M_PI/180.0f),0,
    cos(20*15*M_PI/180.0f),sin(20*15*M_PI/180.0f),0,
    0,0,0,
    cos(20*15*M_PI/180.0f),sin(20*15*M_PI/180.0f),0,
    cos(21*15*M_PI/180.0f),sin(21*15*M_PI/180.0f),0,
    0,0,0,
    cos(21*15*M_PI/180.0f),sin(21*15*M_PI/180.0f),0,
    cos(22*15*M_PI/180.0f),sin(22*15*M_PI/180.0f),0,
    0,0,0,
    cos(22*15*M_PI/180.0f),sin(22*15*M_PI/180.0f),0,
    cos(23*15*M_PI/180.0f),sin(23*15*M_PI/180.0f),0,
    0,0,0,
    cos(23*15*M_PI/180.0f),sin(23*15*M_PI/180.0f),0,
    cos(24*15*M_PI/180.0f),sin(24*15*M_PI/180.0f),0,
    0,0,0,
    cos(24*15*M_PI/180.0f),sin(24*15*M_PI/180.0f),0,
    cos(25*15*M_PI/180.0f),sin(25*15*M_PI/180.0f),0,
    0,0,0,
    cos(25*15*M_PI/180.0f),sin(25*15*M_PI/180.0f),0,
    cos(26*15*M_PI/180.0f),sin(26*15*M_PI/180.0f),0,
    0,0,0,
    cos(26*15*M_PI/180.0f),sin(26*15*M_PI/180.0f),0,
    cos(27*15*M_PI/180.0f),sin(27*15*M_PI/180.0f),0
  };
  bullet=create3DObject(GL_TRIANGLES,78,vertex_buffer_data,color_buffer_data,GL_FILL);
}
// Creates the triangle object used in this sample code
void createCanon()
{
    static const GLfloat vertex_buffer_data [] ={
    0,0,0,
    cos(15*M_PI/180.0f),sin(15*M_PI/180.0f),0,
    cos(2*15*M_PI/180.0f),sin(2*15*M_PI/180.0f),0,
    0,0,0,
    cos(2*15*M_PI/180.0f),sin(2*15*M_PI/180.0f),0,
    cos(3*15*M_PI/180.0f),sin(3*15*M_PI/180.0f),0,
    0,0,0,
    cos(3*15*M_PI/180.0f),sin(3*15*M_PI/180.0f),0,
    cos(4*15*M_PI/180.0f),sin(4*15*M_PI/180.0f),0,
    0,0,0,
    cos(4*15*M_PI/180.0f),sin(4*15*M_PI/180.0f),0,
    cos(5*15*M_PI/180.0f),sin(5*15*M_PI/180.0f),0,
    0,0,0,
    cos(5*15*M_PI/180.0f),sin(5*15*M_PI/180.0f),0,
    cos(6*15*M_PI/180.0f),sin(6*15*M_PI/180.0f),0,
    0,0,0,
    cos(6*15*M_PI/180.0f),sin(6*15*M_PI/180.0f),0,
    cos(7*15*M_PI/180.0f),sin(7*15*M_PI/180.0f),0,
    0,0,0,
    cos(7*15*M_PI/180.0f),sin(7*15*M_PI/180.0f),0,
    cos(8*15*M_PI/180.0f),sin(8*15*M_PI/180.0f),0,
    0,0,0,
    cos(8*15*M_PI/180.0f),sin(8*15*M_PI/180.0f),0,
    cos(9*15*M_PI/180.0f),sin(9*15*M_PI/180.0f),0,
    0,0,0,
    cos(9*15*M_PI/180.0f),sin(9*15*M_PI/180.0f),0,
    cos(10*15*M_PI/180.0f),sin(10*15*M_PI/180.0f),0,
    0,0,0,
    cos(10*15*M_PI/180.0f),sin(10*15*M_PI/180.0f),0,
    cos(11*15*M_PI/180.0f),sin(11*15*M_PI/180.0f),0,
    0,0,0,
    cos(11*15*M_PI/180.0f),sin(11*15*M_PI/180.0f),0,
    cos(12*15*M_PI/180.0f),sin(12*15*M_PI/180.0f),0,
    0,0,0,
    cos(12*15*M_PI/180.0f),sin(12*15*M_PI/180.0f),0,
    cos(13*15*M_PI/180.0f),sin(13*15*M_PI/180.0f),0,
    0,0,0,
    cos(13*15*M_PI/180.0f),sin(13*15*M_PI/180.0f),0,
    cos(14*15*M_PI/180.0f),sin(14*15*M_PI/180.0f),0,
    0,0,0,
    cos(14*15*M_PI/180.0f),sin(14*15*M_PI/180.0f),0,
    cos(15*15*M_PI/180.0f),sin(15*15*M_PI/180.0f),0,
    0,0,0,
    cos(15*15*M_PI/180.0f),sin(15*15*M_PI/180.0f),0,
    cos(16*15*M_PI/180.0f),sin(16*15*M_PI/180.0f),0,
    0,0,0,
    cos(16*15*M_PI/180.0f),sin(16*15*M_PI/180.0f),0,
    cos(17*15*M_PI/180.0f),sin(17*15*M_PI/180.0f),0,
    0,0,0,
    cos(17*15*M_PI/180.0f),sin(17*15*M_PI/180.0f),0,
    cos(18*15*M_PI/180.0f),sin(18*15*M_PI/180.0f),0,
    0,0,0,
    cos(18*15*M_PI/180.0f),sin(18*15*M_PI/180.0f),0,
    cos(19*15*M_PI/180.0f),sin(19*15*M_PI/180.0f),0,
    0,0,0,
    cos(19*15*M_PI/180.0f),sin(19*15*M_PI/180.0f),0,
    cos(20*15*M_PI/180.0f),sin(20*15*M_PI/180.0f),0,
    0,0,0,
    cos(20*15*M_PI/180.0f),sin(20*15*M_PI/180.0f),0,
    cos(21*15*M_PI/180.0f),sin(21*15*M_PI/180.0f),0,
    0,0,0,
    cos(21*15*M_PI/180.0f),sin(21*15*M_PI/180.0f),0,
    cos(22*15*M_PI/180.0f),sin(22*15*M_PI/180.0f),0,
    0,0,0,
    cos(22*15*M_PI/180.0f),sin(22*15*M_PI/180.0f),0,
    cos(23*15*M_PI/180.0f),sin(23*15*M_PI/180.0f),0,
    0,0,0,
    cos(23*15*M_PI/180.0f),sin(23*15*M_PI/180.0f),0,
    cos(24*15*M_PI/180.0f),sin(24*15*M_PI/180.0f),0,
    0,0,0,
    cos(24*15*M_PI/180.0f),sin(24*15*M_PI/180.0f),0,
    cos(25*15*M_PI/180.0f),sin(25*15*M_PI/180.0f),0,
    0,0,0,
    cos(25*15*M_PI/180.0f),sin(25*15*M_PI/180.0f),0,
    cos(26*15*M_PI/180.0f),sin(26*15*M_PI/180.0f),0,
    0,0,0,
    cos(26*15*M_PI/180.0f),sin(26*15*M_PI/180.0f),0,
    cos(27*15*M_PI/180.0f),sin(27*15*M_PI/180.0f),0,
    0,0.25f,0,
    2,0.25f,0,
    2,-0.25f,0,
    0,0.25f,0,
    0,-0.25f,0,
    2,-0.25f,0
  };
  static const GLfloat color_buffer_data []={
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1
  };
  canon=create3DObject(GL_TRIANGLES,84,vertex_buffer_data,color_buffer_data,GL_FILL);
}
void createRectangle()
{
    static const GLfloat vertex_buffer_data []=
    {
        -0.5,-0.5,0,
        0.5,-0.5,0,
        0.5,0.5,0,
        -0.5,-0.5,0,
        -0.5,0.5,0,
        0.5,0.5,0
    };
    static const GLfloat color_buffer_data []=
    {
        1,0,0,
        0,1,0,
        0,0,1,
        1,0,0,
        0,0,1
    };
    rectangle = create3DObject(GL_TRIANGLES,6,vertex_buffer_data,color_buffer_data,GL_FILL);
}

float camera_rotation_angle = 90;
/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0));// Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Load identity to model matrix
  Matrices.model =glm::mat4(1.0f);
  glm::mat4 translateCanon =glm::translate(glm::vec3(-4.0f,-4.0f,0.0f));
  glm::mat4 rotateCanon =glm::rotate((float)(canon_rotation*M_PI/180.0f),glm::vec3(0,0,1));
  glm::mat4 scaleCanon=glm::scale(glm::vec3(0.5f,0.5f,0.5f));
  glm::mat4 Canontransform = (translateCanon*rotateCanon*scaleCanon);
  Matrices.model*=Canontransform;
  MVP=VP*Matrices.model;

  glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
  draw3DObject(canon);


  if (num_appear==0)
  {
    coin[0].set_coins(3,3);
    coin[1].set_coins(4,1);
    coin[2].set_coins(2,4);
    coin[3].set_coins(2,2);
    num_appear++;
  }
  
  for(int i=0;i<num_coins;i++)
  {
    Matrices.model=glm::mat4(1.0f);
    if(coin[i].appear!=0)
    {
        Matrices.model*=(glm::translate(glm::vec3(coin[i].pos_x,coin[i].pos_y,0))*glm::scale(glm::vec3(0.2f,0.2f,0)));
        MVP=VP*Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
        draw3DObject(bullet);
    }
  }


  //glm::mat4 translateBullet = glm::translate(glm::vec3(1,0,0.0f));
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateBullet = glm::translate(glm::vec3(-4+1.2f*cos(canon_rotation*M_PI/180.0f),-4+1.2f*sin(canon_rotation*M_PI/180.0f),0));
  glm::mat4 scaleBullet = glm::scale(glm::vec3(0.2,0.2,0));
  glm::mat4 antirotateBullet =glm::rotate((float)((360-canon_rotation)*M_PI/180.0f),glm::vec3(0,0,1));
  if(use!=0)
  {
    if (flag==0)
     {
        initlz();
        trnsfrm=translateBullet;
        //trnsfrm=(translateCanon*rotateCanon*translateBullet*antirotateBullet);
        //trnsfrm=(translateCanon*rotateCanon*translateBullet*scaleBullet);
        Matrices.model*=(trnsfrm*scaleBullet);
        position_x=0;
        position_y=0;
        flag=1;
    }
    else
    {
        position_x+=0.01f*velocity_x;
        position_y+=0.01f*(velocity_y+5-(10*time_travelled));
    
        if(position_y<=-(0.3f+1.0f*sin(canon_rotation*M_PI/180.0f)))
        {
            if(bounce<5)
            {
                velocity_y=0.5f*velocity_y;
                bounce++;
            }
            time_travelled=0;
        if(bounce>=5)
        {
            use=0;
        }

        }
        else if(position_x>=(8-cos(canon_rotation*M_PI/180.0f)))
        {

            velocity_x=-0.5f*velocity_x;
        }
        glm::mat4 moveBullet = glm::translate(glm::vec3(position_x,position_y,0));
        Matrices.model*=(trnsfrm*moveBullet*scaleBullet);
    // if(position_x>=10-(0.5f+cos(canon_rotation*M_PI/180.0f)) or (position_x<=-(0.5+cos(canon_rotation*M_PI/180.0f))))
     //   velocity_x*=-0.5f;
        time_travelled+=0.01;
        dist(coin);
        MVP=VP*Matrices.model;
    //time_x+=0.5;
        glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
        draw3DObject(bullet);
    }

   } 
/***** WALLS******/

  Matrices.model = glm::mat4(1.0f);
  glm::mat4 scalerect1 = glm::scale(glm::vec3(10.0f,0.5f,0));
  glm::mat4 translaterect1 = glm::translate(glm::vec3(0,-4.75,0));
  Matrices.model*=translaterect1*scalerect1;
  MVP=VP*Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
  draw3DObject(rectangle);

  Matrices.model = glm::mat4(1.0f);
  glm::mat4 scalerect2 = glm::scale(glm::vec3(0.5f,10.0f,0));
  glm::mat4 translaterect2 = glm::translate(glm::vec3(-4.75,0,0));
  Matrices.model*=translaterect2*scalerect2;
  MVP=VP*Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
  draw3DObject(rectangle);

  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translaterect3 = glm::translate(glm::vec3(4.75,0,0));
  Matrices.model*=translaterect3*scalerect2;
  MVP=VP*Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
  draw3DObject(rectangle);

  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translaterect4 = glm::translate(glm::vec3(0,4.75f,0));
  Matrices.model*=translaterect4*scalerect1;
  MVP=VP*Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
  draw3DObject(rectangle);


  /***** WALLS ******/
}

GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; 

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
   // Generate the VAO, VBOs, vertices data & copy into the array buffer
  createCanon();
  createBullet();
  createRectangle();
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 800;
	int height = 800;

    GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        draw();

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5)
        { 
            last_update_time = current_time;
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
