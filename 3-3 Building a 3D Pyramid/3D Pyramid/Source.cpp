/*

Student Name: Sergio Mateos
Institution: Southern New Hampshire University
Class: CS 330 Comp Graphic and Visualization

*/

#include <iostream>        
#include <cstdlib>      
#include <GLEW/glew.h>     
#include <GLFW/glfw3.h>     

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

/* Shader program Macro */
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

namespace
{
    // Window Title
    const char* const WINDOW_TITLE = "3D Pyramid";

    // Width & Height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // GLMesh Data
    struct GLMesh
    {
        // Vertex Array Object
        GLuint vao;

        // Vertex Buffer Objects
        GLuint vbos[2];  

        // Indices on the Mesh
        GLuint nIndices;    
    };

    // GLFW Window
    GLFWwindow* gWindow = nullptr;

    // Triangle Mesh Data
    GLMesh gMesh;

    // Shader program
    GLuint gProgramId;
}

 // Initialize program
bool UInitialize(int, char* [], GLFWwindow** window);

// Windows Size
void UResizeWindow(GLFWwindow* window, int width, int height);

// User Input
void UProcessInput(GLFWwindow* window);

// Copiler Shader
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UCreateMesh(GLMesh& mesh);

// Render Graphics
void URender();


// Free Memory on Close
void UDestroyMesh(GLMesh& mesh);
void UDestroyShaderProgram(GLuint programId);


/* Vertex Shader Source Code */
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0

// Color Data from VAP 1
layout(location = 1) in vec4 color;

// Transfer Color Data to the Shader
out vec4 vertexColor;

// Global variables 
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Transforms Vertices to Clip Coordinates
    gl_Position = projection * view * model * vec4(position, 1.0f);

    // Color Data
    vertexColor = color;
}
);


// Shader Source Code
const GLchar* fragmentShaderSource = GLSL(440,
    in vec4 vertexColor;

out vec4 fragmentColor;

void main()
{
    fragmentColor = vec4(vertexColor);
}
);


int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the Mesh
    UCreateMesh(gMesh);

    // Create the Shader Program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    // Set the color of the Background (Black)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);


    while (!glfwWindowShouldClose(gWindow))
    {

        UProcessInput(gWindow);

        // Render Frame
        URender();

        glfwPollEvents();
    }

    // Release Mesh Data
    UDestroyMesh(gMesh);

    // Release Shader Program
    UDestroyShaderProgram(gProgramId);

    exit(EXIT_SUCCESS);
}


// GLFW, GLEW, and Create a Window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Window Creation
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "GLFW window FAIL" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);

    // GLEW
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // GPU OpenGL Version
    cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// GLFW Input Processor
void UProcessInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}


// GLFW Resize
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// Render Function
void URender()
{
    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the Frame and Z Buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // GLM: Scale (Size 4)
    glm::mat4 scale = glm::scale(glm::vec3(4.0f, 4.0f, 4.0f));

    // GLM: Rotate 15 Degrees in the X Axis
    glm::mat4 rotation = glm::rotate(45.0f, glm::vec3(1.0, 1.0f, 1.0f));

    // GLM: Place the Pyramind in the Origin
    glm::mat4 translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));

    // GLM: Transformations (Right-to-Lef)
    glm::mat4 model = translation * rotation * scale;

    // GLM: View Move Back
    glm::mat4 view = glm::translate(glm::vec3(0.0f, 0.0f, -5.0f));

    // GLM: Create Projection
    glm::mat4 projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);

    // Shader can be Use
    glUseProgram(gProgramId);

    // Retrieves and Passes Shader Program
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //  VBOs Activation with the Mesh
    glBindVertexArray(gMesh.vao);

    // Draws the Triangles
    glDrawElements(GL_TRIANGLES, gMesh.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle

    // Vertex Array Object (VAO) Desactivation
    glBindVertexArray(0);

    // GLFW: Swap Buffer
    glfwSwapBuffers(gWindow);
}


// UCreateMesh Function
void UCreateMesh(GLMesh& mesh)
{
    // Position & Color 
    GLfloat verts[] = {

                 // Vertex Positions    // Colors (R,G,B,A)
 /* Vertex 0 */   0.3f,  1.0f, 0.0f,   0.0f, 0.0f, 1.0f, 1.0f, // Top 
 /* Vertex 1 */   0.5f, -1.0f, 0.0f,   0.0f, 1.0f, 1.0f, 1.0f, // Bottom 
 /* Vertex 2 */  -0.6f, -0.3f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f, // Bottom Left 
 /* Vertex 3 */  -0.5f, -0.7f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, // BR left 
 /* Vertex 4 */   0.5f, -1.5f, -1.0f,  1.0f, 1.0f, 0.0f, 1.0f // BR right 


    };

    // Share Position Data
    GLushort indices[] = {
         0, 1, 4, // Triangle 1
         0, 1, 2, // Triangle 2
         0, 2, 3, // Triangle 3
         0, 3, 4, // Triangle 4  
         2, 4, 3, // Triangle 5
         1, 2, 4  // Triangle 6
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;

    glGenVertexArrays(1, &mesh.vao); 
    glBindVertexArray(mesh.vao);

    // Vertex Data 
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); 

    // Indices
    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Vertex coordinates 
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
}

// UDestroyMesh Fucntion
void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(2, mesh.vbos);
}


// UCreateShaders Function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and Linkage Error Reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader Program 
    programId = glCreateProgram();

    // Vertex and Fragment Shader Objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the Shader Source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Verify Compilier Error and Send Notification
    glCompileShader(vertexShaderId); 

    // Verify Shader Compile Errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId);

    // Check for Shader Compile Errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached Compiled Shaders
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId); 

    // Verify Linking Errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}