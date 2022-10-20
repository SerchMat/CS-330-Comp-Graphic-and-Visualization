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
                        P: Ortho
                        O: Reset Ortho

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

// Camera.h
#include "camera.h"

using namespace std;

// Shader Program Macro
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    // Window Title 
    const char* const WINDOW_TITLE = "Shape and Plane"; 
    const int STRIDE = 7;

    // Width & Height of the Window
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // GL Data Relative to Mesh
    struct GLMesh
    {
        GLuint planeVao, cylinderVao;         
        GLuint planeVbo, cylinderVbo[2];           
        GLuint planeVertices, cylinderVertices; 

    };

    // GLFW Window
    GLFWwindow* gWindow = nullptr;

    // Triangle Mesh Data
    GLMesh gMesh;
    GLMesh gMeshPlane;

    // Shader program
    GLuint gProgramId;
    GLuint gProgramIdPlane;

    // Camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 10.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // Timing
    float gDeltaTime = 0.0f; 
    float gLastFrame = 0.0f;

    // Bool keys[1024];
    bool ortho = false;

    // Position and Scale for the Speaker
    glm::vec3 gPositionSpeaker(-2.0f, -0.25f, 0.0f);
    glm::vec3 gScaleSpeaker(2.0f);

    // Position and Scale for the Plane
    glm::vec3 gPositionPlane(0.0f, 0.0f, 0.0f);
    glm::vec3 gScalePlane(2.0f);
}

// Initialize Program
bool UInitialize(int, char* [], GLFWwindow** window);

// Set Windows Size
void UResizeWindow(GLFWwindow* window, int width, int height);

// User Input Process
void UProcessInput(GLFWwindow* window);

// CallBack Functions
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

// Create Speaker, Plane & Mesh
void UCreateSpeaker(GLfloat verts[], GLushort indices[], int numSides, float radius, float halfLen);
void UCreateMesh(GLMesh& mesh);
void UCreateMeshPlane(GLMesh& mesh);

// Graphics Render
void URender();

// Free Memory on Close
void UDestroyMesh(GLMesh& mesh);
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);

// Plane Vertex Shader Source Code
const GLchar* vertexShaderSourcePlane = GLSL(440,
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
const GLchar* fragmentShaderSourcePlane = GLSL(440,
    in vec4 vertexColor; 

// Transfer Color Data to the Fragment
out vec4 fragmentColor;

// Main
void main()
{
    fragmentColor = vec4(vertexColor);
}
);

// Vertex Shader Source Code 
const GLchar* vertexShaderSourceSpeaker = GLSL(440,
    layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

// Transfer Color Data to Fragment Shaders
out vec4 vertexColor;

// Global Variables for Transform Matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Main
void main()
{
    // Transform Vertices to Clip Coordinates
    gl_Position = projection * view * model * vec4(position, 1.0f);

    // Incoming Vertex Data
    vertexColor = color; 
}
);


// Vertex Shader Source Code
const GLchar* fragmentShaderSourceSpeaker = GLSL(440,
    in vec4 vertexColor;

// Transfer Color Data
out vec4 fragmentColor;

// Main
void main()
{
    fragmentColor = vec4(vertexColor);
}
);

// Main Function
int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the Mesh
    UCreateMesh(gMesh);

    // Shader Program Creation for the Speaker
    if (!UCreateShaderProgram(vertexShaderSourceSpeaker, fragmentShaderSourceSpeaker, gProgramId))
        return EXIT_FAILURE;

    // Create the Mesh
    UCreateMeshPlane(gMeshPlane); 

    // Shader Program Creation for the Plane
    if (!UCreateShaderProgram(vertexShaderSourcePlane, fragmentShaderSourcePlane, gProgramIdPlane))
        return EXIT_FAILURE;




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
    UDestroyMesh(gMeshPlane);

    // Release Shader Program
    UDestroyShaderProgram(gProgramId);
    UDestroyShaderProgram(gProgramIdPlane);

    // Terminate Program
    exit(EXIT_SUCCESS); 
}


// GLFW, GLEW & Create a Window
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
    
    // Wndow Creation
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        // Print Message if Failed
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    // GLFW Funtions
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);;

    // GLFW Mouse Capture
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW   
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// Process Inputs
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;

    // Ortho Projection Creation

    // P Key to Create Ortho
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
        gCamera.Position = glm::vec3(-2.5f, 5.2f, -1.5f);
        gCamera.Pitch = -100.0f;
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Reset Ortho Projection

    // O Key to Reset Ortho
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) 
    { 
        gCamera.Position = glm::vec3(0.0f, 0.0f, 5.0f);
        gCamera.Pitch = 0.0f;
        glfwSwapBuffers(window);
        glfwPollEvents();
    }


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

// UCreateSpeaker Funtion to Create the Speaker Shape
void UCreateSpeaker(GLfloat verts[], GLushort indices[], int numSides, float radius, float halfLen) {

    // Contant Value for Calculations
    const float TWO_PI = 2.0f * 3.1415926f;
    const float radiansPerSide = TWO_PI / numSides;

    // Value Incremented
    int currentVertex = 0;

    // Vertex Step of Size
    verts[0] = 0.0f;        //0 x
    verts[1] = halfLen;     //0 y
    verts[2] = 0.0f;        //0 z
    verts[3] = 0.0f;        //0 r
    verts[4] = 0.0f;        //0 g    
    verts[5] = 1.0f;        //0 b
    verts[6] = 1.0f;        //0 a
    currentVertex++;
    verts[7] = 0.0f;        //1 x
    verts[8] = -halfLen;    //1 y
    verts[9] = 0.0f;        //1 z
    verts[10] = 0.0f;       //1 r
    verts[11] = 0.0f;       //1 g
    verts[12] = 1.0f;       //1 b
    verts[13] = 1.0f;       //1 a
    currentVertex++;

    // Track of Triangles Added
    int currentTriangle = 0;

    for (int edge = 0; edge < numSides; edge++) {

        // Calculate the Center Point to the Next Vertex
        float theta = ((float)edge) * radiansPerSide;

        // Top Triangle
        verts[currentVertex * STRIDE + 0] = radius * cos(theta);    // X
        verts[currentVertex * STRIDE + 1] = halfLen;                // Y
        verts[currentVertex * STRIDE + 2] = radius * sin(theta);    // Z
        verts[currentVertex * STRIDE + 3] = 0.0f;                   // R
        verts[currentVertex * STRIDE + 4] = 1.0f;                   // G
        verts[currentVertex * STRIDE + 5] = 1.0f;                   // B
        verts[currentVertex * STRIDE + 6] = 1.0f;                   // A
        currentVertex++;

        // Bottom Tringle 
        verts[currentVertex * STRIDE + 0] = radius * cos(theta);    // X
        verts[currentVertex * STRIDE + 1] = -halfLen;               // Y
        verts[currentVertex * STRIDE + 2] = radius * sin(theta);    // Z
        verts[currentVertex * STRIDE + 3] = 0.0f;                   // R
        verts[currentVertex * STRIDE + 4] = 1.0f;                   // G
        verts[currentVertex * STRIDE + 5] = 1.0f;                   // B
        verts[currentVertex * STRIDE + 6] = 1.0f;                   // A
        currentVertex++;

        //Indices for the Triangles
        if (edge > 0) {

            // Top triangle
            indices[(3 * currentTriangle) + 0] = 0;                 // Center-Top Prisim
            indices[(3 * currentTriangle) + 1] = currentVertex - 4; // Upper-Left Side
            indices[(3 * currentTriangle) + 2] = currentVertex - 2; // Upper Right Side
            currentTriangle++;

            // Bottom triangle
            indices[(3 * currentTriangle) + 0] = 1;                 // Center-Bottom Prisim
            indices[(3 * currentTriangle) + 1] = currentVertex - 3; // Bottom-Left Side
            indices[(3 * currentTriangle) + 2] = currentVertex - 1; // Bottom-Right Side
            currentTriangle++;

            // Triangle for 1/2 rectangular side
            indices[(3 * currentTriangle) + 0] = currentVertex - 4; // Upper-Left Side
            indices[(3 * currentTriangle) + 1] = currentVertex - 3; // Bottom-Left Side
            indices[(3 * currentTriangle) + 2] = currentVertex - 1; // Bottom-Right Side
            currentTriangle++;

            // Traingle for second 1/2 rectangular side
            indices[(3 * currentTriangle) + 0] = currentVertex - 1; // Bottom-Right Side
            indices[(3 * currentTriangle) + 1] = currentVertex - 2; // Upper-Right Side
            indices[(3 * currentTriangle) + 2] = currentVertex - 4; // Upper-Left Side
            currentTriangle++;
        }
    }

    // Indices for Triangles
    
    //Top Triangle
    indices[(3 * currentTriangle) + 0] = 0;                     // Center Top Prims
    indices[(3 * currentTriangle) + 1] = currentVertex - 2;     // Upper-Left Side
    indices[(3 * currentTriangle) + 2] = 2;                     // Fist Upper-Left (Change to Right)
    currentTriangle++;

    // Bottom Triangle
    indices[(3 * currentTriangle) + 0] = 1;                     //Center-Bottom Prism
    indices[(3 * currentTriangle) + 1] = currentVertex - 1;     //Bottom-Left Side
    indices[(3 * currentTriangle) + 2] = 3;                     // Fist Bottom-Left (Change to Right)
    currentTriangle++;

    // First Triangle 1/2 rectangular Side
    indices[(3 * currentTriangle) + 0] = currentVertex - 2;     // Upper-Left Side
    indices[(3 * currentTriangle) + 1] = currentVertex - 1;     //Bottom-Left Side
    indices[(3 * currentTriangle) + 2] = 3;                     // Bottom-Right Side
    currentTriangle++;

    // Second Triangle 1/2 rectangular Side
    indices[(3 * currentTriangle) + 0] = 3;                     // Bottom-Right Side
    indices[(3 * currentTriangle) + 1] = 2;                     // Upper-Right Side
    indices[(3 * currentTriangle) + 2] = currentVertex - 2;     // Upper-Left Side
    currentTriangle++;
}

// URender Frame Function
void URender()
{

    // Enable Z-Depth
    glEnable(GL_DEPTH_TEST);

    // Clear the Frame and Z Buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glUseProgram(gProgramId);

    glm::mat4 model = glm::translate(gPositionSpeaker) * glm::scale(gScaleSpeaker);

    // Camera Transformation
    glm::mat4 projection;

    // View Transformation
    glm::mat4 view;

    // Views and Projections
    if (ortho) {
        GLfloat oWidth = (GLfloat)WINDOW_WIDTH * 0.01f; // 10% of width
        GLfloat oHeight = (GLfloat)WINDOW_HEIGHT * 0.01f; // 10% of height

        view = gCamera.GetViewMatrix();
        projection = glm::ortho(-oWidth, oWidth, oHeight, -oHeight, 0.1f, 100.0f);
    }
    else {
        view = gCamera.GetViewMatrix();
        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    }


    // Retrieves and Passes Transform matrices to the Shader Program
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");



    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


    glBindVertexArray(gMesh.cylinderVao);

    glDrawElements(GL_TRIANGLES, gMesh.cylinderVertices, GL_UNSIGNED_SHORT, NULL);

    // DRAW: Plane

    // Use Shader 
    glUseProgram(gProgramIdPlane);

    model = glm::translate(gPositionPlane) * glm::scale(gScalePlane);


    // Views & Projections
    if (ortho) {
        GLfloat oWidth = (GLfloat)WINDOW_WIDTH * 0.01f; // 10% of width
        GLfloat oHeight = (GLfloat)WINDOW_HEIGHT * 0.01f; // 10% of height

        view = gCamera.GetViewMatrix();
        projection = glm::ortho(-oWidth, oWidth, oHeight, -oHeight, 0.1f, 100.0f);
    }
    else {
        view = gCamera.GetViewMatrix();
        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    }


    // Retrieves and Passes Transform Matrices to the Shader Program
    GLint modelLocPlane = glGetUniformLocation(gProgramIdPlane, "model");
    GLint viewLocPlane = glGetUniformLocation(gProgramIdPlane, "view");
    GLint projLocPlane = glGetUniformLocation(gProgramIdPlane, "projection");

    glUniformMatrix4fv(modelLocPlane, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLocPlane, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLocPlane, 1, GL_FALSE, glm::value_ptr(projection));



    // Activate the Plane VAO 
    glBindVertexArray(gMeshPlane.planeVao);

    // Draws the Triangles
    glDrawArrays(GL_TRIANGLES, 0, gMeshPlane.planeVertices);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    // GLFW: Swap Buffers
    glfwSwapBuffers(gWindow);    
}

// UCreateMesh Function
void UCreateMesh(GLMesh& mesh)
{
    // Number of Side in the Cylinder
    const int NUM_SIDES = 100;

    // Number of Vertices (number of sides * 2)
    const int NUM_VERTICES = STRIDE * (2 + (2 * NUM_SIDES));

    // Number of Indices (3 * the number of triangles)
    const int NUM_INDICES = 12 * NUM_SIDES;

    // Position & Color Data
    GLfloat verts[NUM_VERTICES];

    // Index Data and Color Data
    GLushort indices[NUM_INDICES];

    // Fill Vertex and Indices for the UCreateSpeaker
    UCreateSpeaker(verts, indices, NUM_SIDES, 0.18f, 0.08f);

    // Draw Cylinder - Speaker
    const GLuint floatsPerVertex2 = 3;
    const GLuint floatsPerColor2 = 4;

    glGenVertexArrays(1, &mesh.cylinderVao); 
    glBindVertexArray(mesh.cylinderVao);

    // Create Buffers
    glGenBuffers(2, mesh.cylinderVbo);

    // Activate the Buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh.cylinderVbo[0]);

    // Send Verter or Coordinations
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); 

    mesh.cylinderVertices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.cylinderVbo[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides Between Vertex Coordinates
    GLint stride2 = sizeof(float) * (floatsPerVertex2 + floatsPerColor2);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex2, GL_FLOAT, GL_FALSE, stride2, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor2, GL_FLOAT, GL_FALSE, stride2, (char*)(sizeof(float) * floatsPerVertex2));
    glEnableVertexAttribArray(1);

};

// UCreatePlane Function (Create the Plane)
void UCreateMeshPlane(GLMesh& mesh)
{
    // Vertex Data
    GLfloat verts[] = {

       // Triangle 1
       // Vertex              // Color (R, G, B, A)
       2.0f, -0.26f,  2.0f,   1.0f,  0.0f,  1.0f,  1.0f, // Top-Right Vertex
      -2.0f, -0.26f, -2.0f,   0.0f,  1.0f,  1.0f,  1.0f, // Bottom-Left Vertex
       2.0f, -0.26f, -2.0f,   1.0f,  0.0f,  1.0f,  1.0f, // Top-Left Vertex

       // Triangle 2
       // Vertex              // Color (R, G, B, A)
       2.0f, -0.26f,  2.0f,   1.0f,  0.0f,  1.0f,  1.0f, // Top-Right Vertex
      -2.0f, -0.26f,  2.0f,   0.0f,  1.0f,  1.0f,  1.0f, // Bottom-Left Vertex
      -2.0f, -0.26f, -2.0f,   0.0f,  0.0f,  1.0f,  1.0f // Top-Left Vertex
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;

    mesh.planeVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerColor));

    glGenVertexArrays(1, &mesh.planeVao); 
    glBindVertexArray(mesh.planeVao);

    // Create Buffers
    glGenBuffers(1, &mesh.planeVbo);

    // Activate Buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh.planeVbo); 

    // Send Verter or Coordinations
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); 

    // Vertex Coordination Strides
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

}

// Destroy Mesh Function
void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.cylinderVao);
    glDeleteBuffers(2, mesh.cylinderVbo);
    glDeleteVertexArrays(1, &mesh.planeVao);
    glDeleteBuffers(1, &mesh.planeVbo);

}

// UCreateShaders Function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and Linkage Error Reporting
    int success = 0;
    char infoLog[512];

    // Shader Program Object Creation
    programId = glCreateProgram();

    // Vertex and Fragment Shader Objects Creation
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the Shader Source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the Vertex Shader
    glCompileShader(vertexShaderId);

    // Print Message if there is any Error

    // Shader Compile Errors Check
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Compile Fragment of Shader
    glCompileShader(fragmentShaderId);

    // Shader Compile Errors Check
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);

    // Linking Error Check
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Shader Program Use
    glUseProgram(programId);   

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}