/*
*
* Name: Sergio Mateos
* Institute: Souther New Hampshire University
* Class: CS 330 Comp Graphic and Visualization 22EW1
*
*/

#include <iostream>         
#include <cstdlib>          
#include <GLEW/glew.h>        
#include <GLFW/glfw3.h>     

using namespace std;

// Unnamed namespace
namespace
{
    //Tittle
    const char* const WINDOW_TITLE = "Triangles";

    // Window width & height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Mesh 
    struct GLMesh
    {
        //Vertex Array Object
        GLuint vao;

        //Vertex Buffer Object
        GLuint vbos[2];

        //Number of indices on the mesh
        GLuint nIndices;
    };

    //Main GLFW Window
    GLFWwindow* gWindow = nullptr;

    //Triangle Mesh Data
    GLMesh gMesh;

    //Shader
    GLuint gProgramId;
}
//Initialize Program
bool UInitialize(int, char* [], GLFWwindow** window);

//Set Windows Size
void UResizeWindow(GLFWwindow* window, int width, int height);

//Uer Input Process
void UProcessInput(GLFWwindow* window);

//Render Graphics
void UCreateMesh(GLMesh& mesh);
void URender();

//Free Memory on Close
void UDestroyMesh(GLMesh& mesh);
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


//Vertex Source Code
const char* vertexShaderSource = "#version 440 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec4 colorFromVBO;\n"
"out vec4 colorFromVS;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"   colorFromVS = colorFromVBO;\n"
"}\n\0";


//Shader Source Code
const char* fragmentShaderSource = "#version 440 core\n"
"in vec4 colorFromVS;\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = colorFromVS;\n"
"}\n\0";


//Main Function
int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    //Create the Mesh
    UCreateMesh(gMesh);

    //Create Shder Program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    //Background Color (Black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    while (!glfwWindowShouldClose(gWindow))
    {
        
        UProcessInput(gWindow);

        URender();

        glfwPollEvents();
    }

    //Release Mesh Data
    UDestroyMesh(gMesh);

    //Release Shader Program
    UDestroyShaderProgram(gProgramId);

    exit(EXIT_SUCCESS);
}


//Create the Window and Iniziate GLEW & GLFW
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    //GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    //Verify the GLFW 
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);

    //GLEW
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


//GLFW Process input
void UProcessInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}


//GLFW windows size
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


//Functioned for the Render a Frame
void URender()
{
    //Clear the background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    //Use the shaders
    glUseProgram(gProgramId);

    //Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao);

    //Draws the triangle
    glDrawElements(GL_TRIANGLES, gMesh.nIndices, GL_UNSIGNED_SHORT, NULL);

    //Deactivate the VAO
    glBindVertexArray(0);

    //GLFW: swap buffers 
    glfwSwapBuffers(gWindow);   
}


//UCreateMesh function
void UCreateMesh(GLMesh& mesh)
{
    //Location of the Vertex and Color
    GLfloat verts[] =
    {
        //Vertex Locations

        -1.0f, 1.0f, 0.0f,      //Location: Top-Left
        1.0f, 0.0f, 0.0f, 1.0f, //Color: Red

        -1.0f, 0.0f, 0.0f,      //Location: Left-Center
        0.0f, 0.0f, 1.0f, 1.0f, //Color: Blue

        -0.48f, 0.0f, 0.0f,      //Location: top-first third of the screen
        0.0f, 1.0f, 0.0f, 1.0f,  //Color: Green

        0.0f, 0.0f, 0.0f,       //Location: Center
        1.0f, 0.0f, 0.0f, 1.0f, //Color: Red

        0.0f, -1.0f, 0.0f,      //Location: Bottom-center
        0.0f, 1.0f, 0.0f, 1.0f  //Color: Green
    };

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    // Create 2 Buffers
    glGenBuffers(2, mesh.vbos);

    //Vertex Data
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]);

    //Indices
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); 

    //Creates a Buffer Object for the Indices
    GLushort indices[] = { 0, 1, 2, 3, 2, 4 };
    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    //Vertex Attribute Pointer for the Screen Location
    const GLuint floatsPerVertex = 3; 
    const GLuint floatsPerColor = 4; 

    // Strides between vertex coordinates is 6 (x, y, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);// The number of floats before each

    //Vertex Attribute Pointer
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
}


void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(2, mesh.vbos);
}


//UCreateShaders Function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    //Shader Report
    programId = glCreateProgram();

    //Vertex & Shader Objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    //Shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); 

    //Verify if there is Shader Compile Errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId);

    //Verify Shader Compile Errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    //Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    //Link Shader Program
    glLinkProgram(programId);

    //Verification for lincking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);

    return true;
}

void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}