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

// Camera Header
#include "camera.h"

using namespace std; 

// Shader Program Macro
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    // Windows Name
    const char* const WINDOW_TITLE = "3D Scene"; 
    const int STRIDE = 7;

    // Window Width & Height
    const int WINDOW_WIDTH = 900;
    const int WINDOW_HEIGHT = 700;

    // GL Data Relative to Mesh
    struct GLMesh
    {
        // Vertex Array Object
        GLuint cylinderVao;   

        // Vertex Buffer Objects
        GLuint cylinderVbo[2];  

        // Number of Indices of Mesh
        GLuint cylinderVertices; 

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

    glm::vec3 gSodaPosition(-0.3f, -0.2f, 0.0f);
    glm::vec3 gSodaScale(2.0f);

}

// Initialize Program
bool UInitialize(int, char* [], GLFWwindow** window);

// Set Windows Size
void UResizeWindow(GLFWwindow* window, int width, int height);

// User Input Process
void UProcessInput(GLFWwindow* window);

// Mouse Functions
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

// Create Shapes
void UCreateCylinder(GLfloat verts[], GLushort indices[], int numSides, float radius, float halfLen);

// Create Mesh and Empty
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);

// Render Graphics
void URender();

// Shader Program
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Vertex Shader Source Code */
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position; 
    layout(location = 1) in vec4 color;  

    // Variables to Transfer Color
    out vec4 vertexColor; 

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

// Main
void main()
{
    // Convert Vertices to Clip Coordinates
    gl_Position = projection * view * model * vec4(position, 1.0f); 

    // Reference Incoming Color Data
    vertexColor = color; 
}
);


/* Fragment Shader Source Code */
const GLchar* fragmentShaderSource = GLSL(440,
    in vec4 vertexColor; 
    
    //Cube Color
    out vec4 fragmentColor;

// Main
void main()
{
    // Fragment Color
    fragmentColor = vec4(vertexColor);
}
);

// Main Function
int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create Mesh
    UCreateMesh(gMesh); 

    // Create Shader Program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    // Set Background Color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Render Loop
    while (!glfwWindowShouldClose(gWindow))
    {

        // Pre-Frame Timing
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // Input
        UProcessInput(gWindow);

        // Render
        URender();

        glfwPollEvents();
    }

    // Release Mesh Data
    UDestroyMesh(gMesh);

    // Release Shader Program
    UDestroyShaderProgram(gProgramId);

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

    /// GLFW

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

    /// GLEW
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


// GLFW: Keyboard Movement
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;

    // ESC Key to Exit
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
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}

/* ------------- Create the Cylinder ------------- */
void UCreateCylinder(GLfloat verts[], GLushort indices[], int numSides, float radius, float halfLen) {
    const float TWO_PI = 2.0f * 3.1415926f;
    const float radiansPerSide = TWO_PI / numSides;

    int currentVertex = 0;

    verts[0] = 0.0f;        //0 X
    verts[1] = halfLen;     //0 Y
    verts[2] = 0.0f;        //0 Z
    verts[3] = 0.0f;        //0 R
    verts[4] = 1.0f;        //0 G    
    verts[5] = 1.0f;        //0 B
    verts[6] = 1.0f;        //0 A
    currentVertex++;
    verts[7] = 0.0f;        //1 X
    verts[8] = -halfLen;    //1 Y
    verts[9] = 0.0f;        //1 Z
    verts[10] = 1.0f;       //1 R
    verts[11] = 0.0f;       //1 G
    verts[12] = 1.0f;       //1 B
    verts[13] = 1.0f;       //1 A
    currentVertex++;

    int currentTriangle = 0;
    for (int edge = 0; edge < numSides; edge++) {
        float theta = ((float)edge) * radiansPerSide;

        verts[currentVertex * STRIDE + 0] = radius * cos(theta);    // X
        verts[currentVertex * STRIDE + 1] = halfLen;                // Y
        verts[currentVertex * STRIDE + 2] = radius * sin(theta);    // Z
        verts[currentVertex * STRIDE + 3] = 1.0f;                   // R
        verts[currentVertex * STRIDE + 4] = 0.0f;                   // G
        verts[currentVertex * STRIDE + 5] = 1.0f;                   // B
        verts[currentVertex * STRIDE + 6] = 1.0f;                   // A
        currentVertex++;

        verts[currentVertex * STRIDE + 0] = radius * cos(theta);    // X
        verts[currentVertex * STRIDE + 1] = -halfLen;               // Y
        verts[currentVertex * STRIDE + 2] = radius * sin(theta);    // Z
        verts[currentVertex * STRIDE + 3] = 0.0f;                   // R
        verts[currentVertex * STRIDE + 4] = 1.0f;                   // G
        verts[currentVertex * STRIDE + 5] = 0.0f;                   // B
        verts[currentVertex * STRIDE + 6] = 1.0f;                   // A
        currentVertex++;

        //Indices for the Triangles
        if (edge > 0) {

            // Top Triangle
            indices[(3 * currentTriangle) + 0] = 0;                 // Center-Top Prisim
            indices[(3 * currentTriangle) + 1] = currentVertex - 4; // Upper-Left Side
            indices[(3 * currentTriangle) + 2] = currentVertex - 2; // Upper Right Side

            // Bottom Tringle
            indices[(3 * currentTriangle) + 0] = 1;                 // Center-Top Prisim
            indices[(3 * currentTriangle) + 1] = currentVertex - 3; // Upper-Left Side
            indices[(3 * currentTriangle) + 2] = currentVertex - 1; // Upper Right Side
            currentTriangle++;

            // Triangle for 1/2 Rectangular Side
            indices[(3 * currentTriangle) + 0] = currentVertex - 4; // Upper-Left Side
            indices[(3 * currentTriangle) + 1] = currentVertex - 3; // Bottom-Left Side
            indices[(3 * currentTriangle) + 2] = currentVertex - 1; // Bottom-Right Side
            currentTriangle++;

            // Traingle for Second 1/2 Rectangular Side
            indices[(3 * currentTriangle) + 0] = currentVertex - 1; // Bottom-Right Side
            indices[(3 * currentTriangle) + 1] = currentVertex - 2; // Upper-Right Side
            indices[(3 * currentTriangle) + 2] = currentVertex - 4; // Upper-Left Side
            currentTriangle++;
        }
    }

    // Indices for Triangles


    // Top Triangle
    indices[(3 * currentTriangle) + 0] = 0;                     // Center Top Prims
    indices[(3 * currentTriangle) + 1] = currentVertex - 2;     // Upper-Left Side
    indices[(3 * currentTriangle) + 2] = 2;                     // Fist Upper-Left (Change to Right)
    currentTriangle++;

    // Bottom Triangle
    indices[(3 * currentTriangle) + 0] = 1;                     //Center-Bottom Prism
    indices[(3 * currentTriangle) + 1] = currentVertex - 1;     //Bottom-Left Side
    indices[(3 * currentTriangle) + 2] = 3;                     // Fist Bottom-Left (Change to Right)
    currentTriangle++;

    // First Triangle 1/2 Rectangular Side
    indices[(3 * currentTriangle) + 0] = currentVertex - 2;     // Upper-Left Side
    indices[(3 * currentTriangle) + 1] = currentVertex - 1;     //Bottom-Left Side
    indices[(3 * currentTriangle) + 2] = 3;                     // Bottom-Right Side
    currentTriangle++;

    // Second Triangle 1/2 Rectangular Side
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

    // Object Location
    glm::mat4 translation = glm::translate(glm::vec3(0.0f, 0.0f, -8.0f));

    // Objcet Scale
    glm::mat4 scale = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));

    // Rotating the Shape on the X Axis
    glm::mat4 rotation = glm::rotate(45.0f, glm::vec3(-90.0, 1.0f, 1.0f));

    // Transformations Right-to-Left Order
    glm::mat4 model = translation * rotation * scale;

    // Camera/View
    glm::mat4 view = gCamera.GetViewMatrix();

    // Perspective Projection
    glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

    glUseProgram(gProgramId);

    // Retrieves and Passes Transform Matrices to the Shader Program
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");

    model = glm::translate(gSodaPosition) * glm::scale(gSodaScale);

    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");


    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(gMesh.cylinderVao);

    glDrawElements(GL_TRIANGLES, gMesh.cylinderVertices, GL_UNSIGNED_SHORT, NULL);


    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    glfwSwapBuffers(gWindow); 
}

// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh)
{
    // Number of Sides Create to the Cylinder
    const int NUM_SIDES = 100;

    // Number of Vertices (number of sides * 2)
    const int NUM_VERTICES = STRIDE * (2 + (2 * NUM_SIDES));

    // Number of Indices (3 * the number of triangles)
    const int NUM_INDICES = 12 * NUM_SIDES;

    // Position & Color Data
    GLfloat verts[NUM_VERTICES];

    // Index Data and Color Data
    GLushort indices[NUM_INDICES];

    // Fill Vertex and Indices for the UCreateCylinder 
    UCreateCylinder(verts, indices, NUM_SIDES, 0.20f, 0.10f);

    /* --------------- Draw the Cylinder --------------- */
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

    // Activates the Buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.cylinderVbo[1]);

    // Send Vertex and Coordinations
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Stride between Vertex and Coordinations
    GLint stride2 = sizeof(float) * (floatsPerVertex2 + floatsPerColor2);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex2, GL_FLOAT, GL_FALSE, stride2, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor2, GL_FLOAT, GL_FALSE, stride2, (char*)(sizeof(float) * floatsPerVertex2));
    glEnableVertexAttribArray(1);

};

// Destroy Mesh Function
void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.cylinderVao);
    glDeleteBuffers(2, mesh.cylinderVbo);

}

// UCreateShaders Function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and Linkage Error Reporting
    int success = 0;
    char infoLog[512];

    // Create Shader Program Object
    programId = glCreateProgram();

    // Create the Vertex and Fragment Shader Objects
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

// Destroy Shader Program Function
void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}