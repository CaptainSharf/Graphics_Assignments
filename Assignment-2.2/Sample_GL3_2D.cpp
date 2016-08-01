#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <time.h>

#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
int arr[1000][3],obs[100][3];
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

/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
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
void missing()
{
  for(int i=0;i<4;i++)
  {
    arr[i][0]=-2*i;
    arr[i][1]=-2*i;
  }
  for(int i=0;i<4;i++)
  {
    arr[i+4][0]=-2*i;
    arr[i+4][1]=2*i;
  }
  arr[8][0] = 2;
  arr[8][1] = -2;
}

int search(int x,int z)
{
  for(int i=0;i<9;i++)
  {
    if(arr[i][0]==x and arr[i][1]==z)
      return 1;
  }
  return 0;
}
int pos_x;
int pos_z;
int appear=0;
int lives=4;

/* Executed when a regular key is pressed */
void keyboardDown (unsigned char key, int x, int y)
{
    switch (key) {
        case 'Q':
        case 'q':
        case 27: //ESC
            exit (0);
        default:
            break;
    }
}
int flag=0;
/* Executed when a regular key is released */
void keyboardUp (unsigned char key, int x, int y)
{
    switch (key) {
        case 'n':
        {
          if(appear == 0)
          {
            appear=1;
            pos_x = -10;
            pos_z = -10;
          }
          break;
        }
        case 'd':
        {
          if(pos_z>10)
          {
            appear=0;
            lives--;
          }
          else 
            pos_z+=2;
          break;
        }
        case 'a':
        {
          if(pos_z<-10)
          {
            appear=0;
            lives--;
          }
          else 
            pos_z-=2;
          break;
        }
        case 'w':
        {
          if(pos_x>10)
          {
            appear=0;
            lives--;
          }
          else 
            pos_x+=2;
          break;
        }
        case 's':
        {
          if(pos_z<-10)
          {
            appear=0;
            lives--;
          }
          else 
            pos_x-=2;
          break;
        }
        case 't':
        {
          flag=1;
          break;
        }
    }
}

/* Executed when a special key is pressed */
void keyboardSpecialDown (int key, int x, int y)
{
}

/* Executed when a special key is released */
void keyboardSpecialUp (int key, int x, int y)
{
}

/* Executed when a mouse button 'button' is put into state 'state'
 at screen position ('x', 'y')
 */
void mouseClick (int button, int state, int x, int y)
{
    switch (button) {
        case GLUT_LEFT_BUTTON:
            if (state == GLUT_UP)
                
            break;
        case GLUT_RIGHT_BUTTON:
            if (state == GLUT_UP) {
                
            }
            break;
        default:
            break;
    }
}


/* Executed when the mouse moves to position ('x', 'y') */
void mouseMotion (int x, int y)
{
}

void obstacle()
{
  obs[0][0] = 2;
  obs[0][1] = 2;
  obs[1][0] = 4;
  obs[1][0] = 6;
  obs[2][0] = 2;
  obs[2][1] = -4;
  obs[3][0] = -2;
  obs[3][1] = -4;
  obs[4][0] = -6;
  obs[4][1] = 8;
  obs[5][0] = 8;
  obs[5][1] = 10;
  obs[6][0] = 6;
  obs[6][1] = -8;
}

int fallorcollide()
{
  for(int i=0;i<9;i++)
  {
    if(pos_x==arr[i][0] and pos_z==arr[i][1])
      return 1;
  }
  for(int i=0;i<6;i++)
  {
    if(pos_x==obs[i][0] and pos_z==obs[i][1])
      return 1;
  }
  return 0;
}

/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (int width, int height)
{
	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) width, (GLsizei) height);

	// set the projection matrix as perspective/ortho
	// Store the projection matrix in a variable for future use

    // Perspective projection for 3D views
    Matrices.projection = glm::perspective (fov, (GLfloat) width / (GLfloat) height, 0.1f, 500.0f);

    // Ortho projection for 2D views
    //Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *triangle, *rectangle;

// Creates the triangle object used in this sample code
void createTriangle ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
    -1.0f,-1.0f,-1.0f, // triangle 1 : begin

     -1.0f,-1.0f, 1.0f,

     -1.0f, 1.0f, 1.0f, // triangle 1 : end

     1.0f, 1.0f,-1.0f, // triangle 2 : begin

     -1.0f,-1.0f,-1.0f,

     -1.0f, 1.0f,-1.0f, // triangle 2 : end

     1.0f,-1.0f, 1.0f,

     -1.0f,-1.0f,-1.0f,

     1.0f,-1.0f,-1.0f,

     1.0f, 1.0f,-1.0f,

     1.0f,-1.0f,-1.0f,

     -1.0f,-1.0f,-1.0f,

     -1.0f,-1.0f,-1.0f,

     -1.0f, 1.0f, 1.0f,

     -1.0f, 1.0f,-1.0f,

     1.0f,-1.0f, 1.0f,

     -1.0f,-1.0f, 1.0f,

     -1.0f,-1.0f,-1.0f,

     -1.0f, 1.0f, 1.0f,

     -1.0f,-1.0f, 1.0f,

     1.0f,-1.0f, 1.0f,

     1.0f, 1.0f, 1.0f,

     1.0f,-1.0f,-1.0f,

     1.0f, 1.0f,-1.0f,

     1.0f,-1.0f,-1.0f,

     1.0f, 1.0f, 1.0f,

     1.0f,-1.0f, 1.0f,

     1.0f, 1.0f, 1.0f,

     1.0f, 1.0f,-1.0f,

     -1.0f, 1.0f,-1.0f,

     1.0f, 1.0f, 1.0f,

     -1.0f, 1.0f,-1.0f,

     -1.0f, 1.0f, 1.0f,

     1.0f, 1.0f, 1.0f,

     -1.0f, 1.0f, 1.0f,

     1.0f,-1.0f, 1.0f

  };

  static const GLfloat color_buffer_data [] = {
     0.583f,  0.771f,  0.014f,

     0.609f,  0.115f,  0.436f,

     0.327f,  0.483f,  0.844f,

     0.822f,  0.569f,  0.201f,

     0.435f,  0.602f,  0.223f,

     0.310f,  0.747f,  0.185f,

     0.597f,  0.770f,  0.761f,

     0.559f,  0.436f,  0.730f,

     0.359f,  0.583f,  0.152f,

     0.483f,  0.596f,  0.789f,

     0.559f,  0.861f,  0.639f,

     0.195f,  0.548f,  0.859f,

     0.014f,  0.184f,  0.576f,

     0.771f,  0.328f,  0.970f,

     0.406f,  0.615f,  0.116f,

     0.676f,  0.977f,  0.133f,

     0.971f,  0.572f,  0.833f,

     0.140f,  0.616f,  0.489f,

     0.997f,  0.513f,  0.064f,

     0.945f,  0.719f,  0.592f,

     0.543f,  0.021f,  0.978f,

     0.279f,  0.317f,  0.505f,

     0.167f,  0.620f,  0.077f,

     0.347f,  0.857f,  0.137f,

     0.055f,  0.953f,  0.042f,

     0.714f,  0.505f,  0.345f,

     0.783f,  0.290f,  0.734f,

     0.722f,  0.645f,  0.174f,

     0.302f,  0.455f,  0.848f,

     0.225f,  0.587f,  0.040f,

     0.517f,  0.713f,  0.338f,

     0.053f,  0.959f,  0.120f,

     0.393f,  0.621f,  0.362f,

     0.673f,  0.211f,  0.457f,

     0.820f,  0.883f,  0.371f,

     0.982f,  0.099f,  0.879f
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  triangle = create3DObject(GL_TRIANGLES,36, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createRectangle ()
{
  // GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    -1.2,-1,0, // vertex 1
    1.2,-1,0, // vertex 2
    1.2, 1,0, // vertex 3

    1.2, 1,0, // vertex 3
    -1.2, 1,0, // vertex 4
    -1.2,-1,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0.3,0.3,0.3, // color 4
    1,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}



float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;

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
  glm::vec3 eye (-15,15,0);

  if(flag==1)
  {
    eye=glm::vec3(-5,25,0);
  }

  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (1,0,0);

  // Compute Camera matrix (view)
   Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  //Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Load identity to model matrix
  for(int k=0;k<6;k++)
  {
    for(int i=0;i<6;i++)
    {
      for(int j=0;j<6;j++)
      {
        //glm::mat4 scalecube = glm::scale(glm::vec3(0.25f,0.25f,0.25f));
        Matrices.model = glm::mat4(1.0f);
        //glm::mat4 translatecube3 = glm::translate(glm::vec3(0.5f*i,0.5f*j,0.5f*k));
        glm::mat4 translatecube3 = glm::translate(glm::vec3(2*i,2*j,2*k));
        //Matrices.model*=(translatecube3*scalecube);
        Matrices.model*=(translatecube3);
        MVP = VP*Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
        if(search(2*i,2*k)==1 and j==5)
        {
        }
        else
          draw3DObject(triangle);

        Matrices.model = glm::mat4(1.0f);
        //Matrices.model*=(translatecube3*(glm::translate(glm::vec3(0,-j,0)))*scalecube);
        //Matrices.model*=(translatecube3*(glm::translate(glm::vec3(0,-4*j,0)))*scalecube);
        Matrices.model*=(translatecube3*(glm::translate(glm::vec3(0,-4*j,0))));
        MVP = VP*Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
        draw3DObject(triangle);

        Matrices.model = glm::mat4(1.0f);
        //Matrices.model*=(translatecube3*(glm::translate(glm::vec3(0,-j,-k)))*scalecube);
        //Matrices.model*=(translatecube3*(glm::translate(glm::vec3(0,-4*j,-4*k)))*scalecube);
        Matrices.model*=(translatecube3*(glm::translate(glm::vec3(0,-4*j,-4*k))));
        MVP = VP*Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
        draw3DObject(triangle);


        Matrices.model = glm::mat4(1.0f);
        //Matrices.model*=(translatecube3*(glm::translate(glm::vec3(0,0,-k)))*scalecube);
        Matrices.model*=(translatecube3*(glm::translate(glm::vec3(0,0,-4*k))));
        MVP = VP*Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
        if(search(2*i,-2*k) and j==5)
        {

        }
        else
          draw3DObject(triangle);

        Matrices.model = glm::mat4(1.0f);
        //glm::mat4 translatecube4 = glm::translate(glm::vec3(-0.5f*i,0.5f*j,-0.5f*k));
        glm::mat4 translatecube4 = glm::translate(glm::vec3(-2*i,2*j,-2*k));
        //Matrices.model*=(translatecube4*scalecube);
        Matrices.model*=(translatecube4);
        MVP = VP*Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
        if(search(-2*i,-2*k) and j==5)
        {
        }
        else
          draw3DObject(triangle);

        Matrices.model = glm::mat4(1.0f);
        //Matrices.model*=(translatecube4*(glm::translate(glm::vec3(0,-j,0)))*scalecube);
        Matrices.model*=(translatecube4*(glm::translate(glm::vec3(0,-4*j,0))));
        MVP = VP*Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
        draw3DObject(triangle);

        Matrices.model = glm::mat4(1.0f);
        //Matrices.model*=(translatecube4*(glm::translate(glm::vec3(0,-j,k)))*scalecube);
        Matrices.model*=(translatecube4*(glm::translate(glm::vec3(0,-4*j,4*k))));
        MVP = VP*Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
        draw3DObject(triangle);

        
        Matrices.model = glm::mat4(1.0f);
        //Matrices.model*=(translatecube4*(glm::translate(glm::vec3(0,0,k)))*scalecube);
        Matrices.model*=(translatecube4*(glm::translate(glm::vec3(0,0,4*k))));
        MVP = VP*Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
        if(search(-2*i,2*k) and j==5)
        {
        }
        else
          draw3DObject(triangle);
      }
    }
  }
  for(int i=0;i<7;i++)
  {
    Matrices.model = glm::mat4(1.0f);
    Matrices.model*=(glm::translate(glm::vec3(obs[i][0],12,obs[i][1])));
    MVP = VP*Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
    draw3DObject(triangle);
  }
  if(appear ==1)
  {
    Matrices.model = glm::mat4(1.0f);
    Matrices.model*=(glm::translate(glm::vec3(pos_x,12,pos_z)));
    MVP = VP*Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
    draw3DObject(triangle);
  }
  if(fallorcollide())
  {
    pos_x=-10;
    pos_z=-10;
    lives--;
  }
  glutSwapBuffers ();

}

/* Executed when the program is idle (no I/O activity) */
void idle () {
    // OpenGL should never stop drawing
    // can draw the same scene or a modified scene
    draw (); // drawing same scene
}


/* Initialise glut window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
void initGLUT (int& argc, char** argv, int width, int height)
{
    // Init glut
    glutInit (&argc, argv);

    // Init glut window
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitContextVersion (3, 3); // Init GL 3.3
    glutInitContextFlags (GLUT_CORE_PROFILE); // Use Core profile - older functions are deprecated
    glutInitWindowSize (width, height);
    glutCreateWindow ("Sample OpenGL3.3 Application");

    // Initialize GLEW, Needed in Core profile
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        cout << "Error: Failed to initialise GLEW : "<< glewGetErrorString(err) << endl;
        exit (1);
    }

    // register glut callbacks
    glutKeyboardFunc (keyboardDown);
    glutKeyboardUpFunc (keyboardUp);

    glutSpecialFunc (keyboardSpecialDown);
    glutSpecialUpFunc (keyboardSpecialUp);

    glutMouseFunc (mouseClick);
    glutMotionFunc (mouseMotion);

    glutReshapeFunc (reshapeWindow);

    glutDisplayFunc (draw); // function to draw when active
    glutIdleFunc (idle); // function to draw when idle (no I/O activity)
    
    glutIgnoreKeyRepeat (true); // Ignore keys held down
}

/* Process menu option 'op' */
void menu(int op)
{
    switch(op)
    {
        case 'Q':
        case 'q':
            exit(0);
    }
}

void addGLUTMenus ()
{
    // create sub menus
    int subMenu = glutCreateMenu (menu);
    glutAddMenuEntry ("Do Nothing", 0);
    glutAddMenuEntry ("Really Quit", 'q');

    // create main "middle click" menu
    glutCreateMenu (menu);
    glutAddSubMenu ("Sub Menu", subMenu);
    glutAddMenuEntry ("Quit", 'q');
    glutAttachMenu (GLUT_MIDDLE_BUTTON);
}


/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (int width, int height)
{
	// Create the models
	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (width, height);

	// Background color of the scene
	glClearColor ((float)235/255,(float)95/255,(float)250/255,0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	createRectangle ();

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 1366;
	int height = 768;
  missing();
  obstacle();
  if(lives>=0)
  {
    initGLUT (argc, argv, width, height);

    addGLUTMenus ();

	initGL (width, height);

    glutMainLoop ();
  }

    return 0;
}
