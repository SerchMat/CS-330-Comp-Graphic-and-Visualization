/*
*
Student Name: Sergio Mateos
Institute: Southern New Hampshire University
Course: CS 330 Comp Graphic and Visualization

*/

/*
Note: The movements are define by the next input by the keyboard
                        W: Zoom In
                        S: Zoom Out
                        A: Left
                        D: Right
                        Q: Up

*/

// GLEW and GLFW Library
#include <iostream>        
#include <cstdlib>      
#include <GLEW/glew.h>     
#include <GLFW/glfw3.h>     

// GLM Math Header inclusions
#include <glm/glm/glm.hpp>
#include <glm/glm/gtx/transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

using namespace std; 

// camera.h 
#include "camera.h"

// Shader program Macro
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
using namespace std; 

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    // Windows Name
    const char* const WINDOW_TITLE = "Camera & Pyramid"; 

    // Window Width & Height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // GL Data Relative to Mesh
    struct GLMesh
    {
        GLuint vao;         
        GLuint vbo;     
        GLuint nVertices;    
    };

    // GLFW window
    GLFWwindow* gWindow = nullptr;

    // Triangle Mesh Data
    GLMesh gMesh;

    // Shader Program
    GLuint gProgramId;

    // Camera 
    Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // Timing
    float gDeltaTime = 0.0f; 
    float gLastFrame = 0.0f;

}


 // Initialize Program
bool UInitialize(int, char* [], GLFWwindow** window);

// Set Windows Size
void UResizeWindow(GLFWwindow* window, int width, int height);

//User Input Process
void UProcessInput(GLFWwindow* window);

// CallBack Functions
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

// Free Memory on Close
void UDestroyMesh(GLMesh& mesh);
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);

// Render Graphics
void URender();
void UCreateMesh(GLMesh& mesh);



// Vertex Shader Source Code
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position; 
layout(location = 1) in vec4 color;  

// Transfer Color Data to the Fragment
out vec4 vertexColor; 

// Global Variables for Transform Matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Main 
void main()
{
    // Transformation of Vertex to Coordinations
    gl_Position = projection * view * model * vec4(position, 1.0f);

    // Color Data Incoming
    vertexColor = color; 
}
);


// Fragment Shader Source Code
const GLchar* fragmentShaderSource = GLSL(440,
    in vec4 vertexColor;

out vec4 fragmentColor;

// Main
void main()
{
    // Vertex Color
    fragmentColor = vec4(vertexColor);
}
);

// Main Function
int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow)) {
        return EXIT_FAILURE;
    }

    // Create the mesh
    UCreateMesh(gMesh); 

    // Shader Program Creation
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId)) {
        return EXIT_FAILURE;
    }

    // Set Background Color (BLACK)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Render Loop
    while (!glfwWindowShouldClose(gWindow))
    {

        // Time Frame
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // Input
        UProcessInput(gWindow);

        // Render Frame
        URender();

        glfwPollEvents();
    }

    // Release Mesh Data
    UDestroyMesh(gMesh);

    // Release Shader Program
    UDestroyShaderProgram(gProgramId);

    // Terminate the Program
    exit(EXIT_SUCCESS); 
}


//  GLFW, GLEW & Create a Window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW
 
    // Initialize
    glfwInit(); 
    
    // Configuration
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW
    
    // Window Creation
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);;

    // GLFW: Capture Mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL Version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// GLFW Process Input  
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // W Key Zoom In
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);

    // S Key Zoom Out
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);

    // A Key Move Left
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);

    // D Key Move Right
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);



    // Upward/Downward Movement
    
    // Q Key Move Up
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) 
        gCamera.ProcessKeyboard(UP, gDeltaTime);

    // E Key Move Down
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);
}


// GLFW: If Window Chnage this Option will Execute
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// GLFW: Mouse Movement
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; 

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// GLFW: Scroll Wheel Action
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}

// GLFW: Mouse Bottoms
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
        // Left Buttom 
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left Mouse Buttom" << endl;
        else
            cout << "Left Mouse Buttom Released" << endl;
    }
    break;

    // Middle Buttom
    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle Mouse Buttom" << endl;
        else
            cout << "Middle Mouse Buttom Released" << endl;
    }
    break;

    // Right Buttom
    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right Mouse Buttom" << endl;
        else
            cout << "Right Mouse Buttom Released" << endl;
    }
    break;

    default:
        cout << "Unhandled Mouse Button" << endl;
        break;
    }
}


// Render Frame Function
void URender()
{

    // Enable Z-Depth
    glEnable(GL_DEPTH_TEST);

    // Clear the Frame and Z Buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Camera/View Transformation
    glm::mat4 view = gCamera.GetViewMatrix();

    // Scales the Object by 1
    glm::mat4 scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));

    // Rotates Shape by 15 degrees in the X Axis
    glm::mat4 rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));

    // Place Object at the Origin
    glm::mat4 translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));

    // Transformations Right-to-Left Order
    glm::mat4 model = translation * rotation * scale;


    // Creates a Orthographic Projection
    glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

    // Set Shader
    glUseProgram(gProgramId);

    // Retrieves and Passes Transform Matrices to the Shader Program
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Activate the VBOs 
    glBindVertexArray(gMesh.vao);

    // Draws the Triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    // GLFW: Swap Buffers 
    glfwSwapBuffers(gWindow);  
}


// UCreateMesh Function
void UCreateMesh(GLMesh& mesh)
{
    // Position and Color data
    GLfloat verts[] = {

        // Vertex Positions        // Colors (R ,G ,B ,A)
        -0.5f,  -0.5f, -0.5f,      1.0f, 1.0f, 0.0f, 1.0f,
         0.5f,  -0.5f, -0.5f,      0.0f, 1.0f, 1.0f, 1.0f,
         0.0f,   0.5f,  0.0f,      1.0f, 1.0f, 0.0f, 1.0f,


        -0.5f, -0.5f, 0.5f,        0.0f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.5f,        1.0f, 0.0f, 1.0f, 1.0f,
         0.0f,  0.5f, 0.0f,        0.0f, 1.0f, 1.0f, 1.0f,

         -0.5f,  -0.5f, -0.5f,     1.0f, 0.0f, 1.0f, 1.0f,
         -0.5f,  -0.5f,  0.5f,     1.0f, 0.0f, 0.0f, 1.0f,
          0.0f,  0.5f,   0.0f,     1.0f, 0.0f, 0.0f, 1.0f,


          0.5f, -0.5f, -0.5f,      1.0f, 0.0f, 1.0f, 1.0f,
          0.5f, -0.5f,  0.5f,      0.0f, 1.0f, 0.0f, 1.0f,
          0.0f,  0.5f,  0.0f,      0.0f, 0.0f, 1.0f, 1.0f,

           0.5f, -0.5f, -0.5f,     0.0f, 1.0f, 0.0f, 1.0f,
           0.5f, -0.5f,  0.5f,     1.0f, 1.0f, 0.0f, 1.0f,
           0.0f,  0.5f,  0.0f,     0.0f, 1.0f, 0.0f, 1.0f,

          -0.5f, -0.5f,  0.5f,     0.0f, 1.0f, 0.0f, 1.0f,
          -0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 0.0f, 1.0f,
           0.0f,  0.5f,  0.0f,     1.0f, 0.0f, 1.0f, 1.0f,
    };



    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerColor));

    glGenVertexArrays(1, &mesh.vao); 
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);

    // Activate the Buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); 

    // Send Vertex and Coordinations
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); 

    // Stride between Vertex and Coordinations
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);


    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
}

// Destroy Mesh Function
void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
}


// UCreateShaders Function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and Linkage Error Reporting
    int success = 0;
    char infoLog[512];

    // Create Shader program Object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the Shader Source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the Vertex Shader
    glCompileShader(vertexShaderId);

    // Check for Shader Compile Errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);

        // Print Error Message
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }
    // Compile Fragment Shader
    glCompileShader(fragmentShaderId);

    // Shader Compile Errors Check
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);

        // Print Error Message
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Compiled Shaders to the Shader Program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);

    // Linking Errors Check
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

// Dstroy Shader Program Function
void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}