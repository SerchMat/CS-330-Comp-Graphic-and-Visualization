

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
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
    const char* const WINDOW_TITLE = "Object Texturing";
    const int STRIDE = 7;

    // Window Width & Height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // GL Data Relative to Mesh
    struct GLMesh
    {
        // Vertex Array Object
        GLuint cylinderVao, cubeVao, planeVao;

        // Vertex Buffer Objects
        GLuint cylinderVbo[2], cubeVbo, planeVbo;

        // Number of Indices of Mesh
        GLuint cylinderVertices, cubeVertices, planeVertices;

    };

    // GLFW window
    GLFWwindow* gWindow = nullptr;

    // Triangle Mesh Data
    GLMesh gMesh;
    GLMesh gMeshCube;
    GLMesh gMeshPlane;

    //Texture
    GLuint gTextureId;
    GLuint gTextureIdPlane;
    GLuint gTextureIdCube;


    // Shader Program
    GLuint gProgramId;
    GLuint gProgramIdCube;
    GLuint gProgramIdPlane;


    glm::vec2 gUVScale(5.0f, 5.0f);
    GLint gTexWrapMode = GL_CLAMP_TO_EDGE;

    // Camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // Timing
    float gDeltaTime = 0.0f;
    float gLastFrame = 0.0f;

    /* Scale & Position */

    // Speaker 
    glm::vec3 gPositionSpeaker(0.0f, 0.0f, 0.0f);
    glm::vec3 gScaleSpeaker(3.0f);

    // Cube(Logo)
    glm::vec3 gPositionCube(0.4f, 0.3f, 0.0f);
    glm::vec3 gScaleCube(0.30f);

    //PLane(Keyboard Mat)
    glm::vec3 gPositionPlane(0.0f, 0.5f, 0.0f);
    glm::vec3 gScalePlane(3.0f);


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
void UCreateSpeaker(GLfloat verts[], GLushort indices[], int numSides, float radius, float halfLen);
void UCreateMesh(GLMesh& mesh);
void UCreateMeshCube(GLMesh& mesh);

// Create Mesh and Empty 
void UCreateMeshPlane(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);

// Create and Destroy Texture
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);

// Render Graphics 
void URender();

// Shader Program
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Vertex Shader Source Code */
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position;
    layout(location = 1) in vec4 color;
    layout(location = 2) in vec2 textureCoordinate;

    // Variables to Transfer Color
    out vec4 vertexColor;
    out vec3 vertexFragmentPos;
    out vec2 vertexTextureCoordinate;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

// Main
void main()
{
    // Convert Vertices to Clip Coordinates
    gl_Position = projection * view * model * vec4(position, 1.0f);
    vertexFragmentPos = vec3(model * vec4(position, 1.0f));

    // Reference incoming Color Data
    vertexColor = color;
    vertexTextureCoordinate = textureCoordinate;
}
);

/* Fragment Shader Source Code */
const GLchar* fragmentShaderSource = GLSL(440,

    // Incoming Fragment Position
    in vec2 vertexTextureCoordinate;
    in vec3 vertexFragmentPos;

    //Cube Color
    out vec4 fragmentColor;
    uniform sampler2D uTexture;

// Main
void main()
{
    // Texture to the GPU Rendering
    fragmentColor = texture(uTexture, vertexTextureCoordinate);
}
);

/* --------------------------- VERTEX CODE CUBE(LOGO) --------------------------- */
const GLchar* vertexShaderSourceCube = GLSL(440,

    // Vertex & Color Data Attrib
    layout(location = 0) in vec3 position;
    layout(location = 1) in vec4 color;
    layout(location = 2) in vec2 textureCoordinate;

    // Transfer Color Data to Fragment Shader
    out vec4 vertexColor;

    // Color and Pixel Fragment
    out vec3 vertexFragmentPos;
    out vec2 vertexTextureCoordinate;

    // Global Variables
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

//Main
void main()
{
    // Transform Vertices to Clip Coordinates
    gl_Position = projection * view * model * vec4(position, 1.0f);
    vertexFragmentPos = vec3(model * vec4(position, 1.0f));

    // References Incoming Color Data
    vertexColor = color;
    vertexTextureCoordinate = textureCoordinate;
}
);

/* Fragment Shader Source Code */
const GLchar* fragmentShaderSourceCube = GLSL(440,
    // Incoming Fragment Position
    in vec2 vertexTextureCoordinate;
    in vec3 vertexFragmentPos; 

    //Cube Color
    out vec4 fragmentColor; 
    uniform sampler2D uTexture;

// Main
void main()
{
    // Texture to the GPU Rendering
    fragmentColor = texture(uTexture, vertexTextureCoordinate);
}
);

/* --------------------------- VERTEX CODE PLANE --------------------------- */

const GLchar* vertexShaderSourcePlane = GLSL(440,

 // Vertex & Color Data Attrib
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 textureCoordinate;

// Transfer Color Data to Fragment Shader
out vec4 vertexColor;

// Color and Pixel Fragment
out vec3 vertexFragmentPos;
out vec2 vertexTextureCoordinate;

// Global Variables
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Main
void main()
{
    // Transform Vertices to Clip Coordinates
    gl_Position = projection * view * model * vec4(position, 1.0f);
    vertexFragmentPos = vec3(model * vec4(position, 1.0f));

    // References Incoming Color Data
    vertexColor = color;
    vertexTextureCoordinate = textureCoordinate;
}
);


/* Fragment Shader Source Code */
const GLchar* fragmentShaderSourcePlane = GLSL(440,
    // Vertex & Color Data Attrib
    in vec2 vertexTextureCoordinate;
    in vec3 vertexFragmentPos; 

    // Transfer Color Data to Fragment Shader
    out vec4 fragmentColor; 


    uniform sampler2D uTexture;

// Main
void main()
{
    fragmentColor = texture(uTexture, vertexTextureCoordinate);
}
);

// Position the Texture based on the X & Y Axis
void flipImageVertically(unsigned char* image, int width, int height, int channels) {
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}

// Main Function
int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create Mesh
    UCreateMesh(gMesh);

    // Create Mesh
    UCreateMeshCube(gMeshCube);

    // Create Mesh
    UCreateMeshPlane(gMeshPlane);

    // Create Shader Program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    // Create Shader Programs
    if (!UCreateShaderProgram(vertexShaderSourceCube, fragmentShaderSourceCube, gProgramIdCube))
        return EXIT_FAILURE;

    // Create Shader Programs
    if (!UCreateShaderProgram(vertexShaderSourcePlane, fragmentShaderSourcePlane, gProgramIdPlane))
        return EXIT_FAILURE;


    /* ----- Load Texture ----- */

    // Texture for the speaker
    const char* texFilename = "logo.png";
    if (!UCreateTexture(texFilename, gTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    // Texture for the keyboard mat(plane)
    texFilename = "razenmat.png";
    if (!UCreateTexture(texFilename, gTextureIdPlane))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    texFilename = "speaker.png";
    if (!UCreateTexture(texFilename, gTextureIdCube))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    glUseProgram(gProgramIdPlane);

    glUseProgram(gProgramId);

    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);


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
    UDestroyMesh(gMeshCube);
    UDestroyMesh(gMeshPlane);

    // Release Texture
    UDestroyTexture(gTextureId);
    UDestroyTexture(gTextureIdPlane);
    UDestroyTexture(gTextureIdCube);

    // Release Shader Program
    UDestroyShaderProgram(gProgramId);
    UDestroyShaderProgram(gProgramIdCube);
    UDestroyShaderProgram(gProgramIdPlane);

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

// GLFW: Keyboard Movement
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
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}
/* ------------- Create the Speaker ------------- */
void UCreateSpeaker(GLfloat verts[], GLushort indices[], int numSides, float radius, float halfLen) {
    const float TWO_PI = 2.0f * 3.1415926f;
    const float radiansPerSide = TWO_PI / numSides;

    int currentVertex = 0;

    verts[0] = 0.0f;        //0 x
    verts[1] = halfLen;     //0 y
    verts[2] = 0.0f;        //0 z
    verts[3] = 0.0f;        //0 r
    verts[4] = 1.0f;        //0 g    
    verts[5] = 1.0f;        //0 b
    verts[6] = 1.0f;        //0 a
    currentVertex++;
    verts[7] = 0.0f;        //1 x
    verts[8] = -halfLen;    //1 y
    verts[9] = 0.0f;        //1 z
    verts[10] = 1.0f;       //1 r
    verts[11] = 0.0f;       //1 g
    verts[12] = 1.0f;       //1 b
    verts[13] = 1.0f;       //1 a
    currentVertex++;

    // Track of Triangles Added
    int currentTriangle = 0;
    for (int edge = 0; edge < numSides; edge++) {

        // Calculate the Center Point to the Next Vertex
        float theta = ((float)edge) * radiansPerSide;

        verts[currentVertex * STRIDE + 0] = radius * cos(theta);    // X
        verts[currentVertex * STRIDE + 1] = halfLen;                // Y
        verts[currentVertex * STRIDE + 2] = radius * sin(theta);    // Z
        verts[currentVertex * STRIDE + 3] = 0.0f;                   // R
        verts[currentVertex * STRIDE + 4] = 1.0f;                   // G
        verts[currentVertex * STRIDE + 5] = 1.0f;                   // B
        verts[currentVertex * STRIDE + 6] = 1.0f;                   // A
        currentVertex++;

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

            // Top Triangle
            indices[(3 * currentTriangle) + 0] = 0;                 // Center-Top Prisim
            indices[(3 * currentTriangle) + 1] = currentVertex - 4; // Upper-Left Side
            indices[(3 * currentTriangle) + 2] = currentVertex - 2; // Upper Right Side
            currentTriangle++;

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

            // Traingle for Second 1/2 rectangular Side
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

    glUseProgram(gProgramId);

    // Activate the Textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

    glm::mat4 model = glm::translate(gPositionSpeaker) * glm::scale(gScaleSpeaker);

    // Camera Transformation
    glm::mat4 view = gCamera.GetViewMatrix();

    // Creates a Perspective Projection
    glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

    // Retrieves and Passes Transform Matrices to the Shader Program
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    glBindVertexArray(gMesh.cylinderVao);

    // Texture Corresponing to Object
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

    glDrawElements(GL_TRIANGLES, gMesh.cylinderVertices, GL_UNSIGNED_SHORT, NULL);


    /* ----------------------- Draw Cube(LOGO) ----------------------- */

    glUseProgram(gProgramIdCube);

    model = glm::translate(gPositionCube) * glm::scale(gScaleCube);

    // Retrieves and Passes Transform Matrices to the Shader Program
    GLint modelLocCube = glGetUniformLocation(gProgramIdCube, "model");
    GLint viewLocCube = glGetUniformLocation(gProgramIdCube, "view");
    GLint projLocCube = glGetUniformLocation(gProgramIdCube, "projection");

    glUniformMatrix4fv(modelLocCube, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLocCube, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLocCube, 1, GL_FALSE, glm::value_ptr(projection));


    // Activate the plane VAO
    glBindVertexArray(gMeshCube.cubeVao);


    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMeshCube.cubeVertices);



    /* ----------------------- Draw Plane(KEYBOARD MAT) ----------------------- */
    glUseProgram(gProgramIdPlane);

    model = glm::translate(gPositionPlane) * glm::scale(gScalePlane);


    // Retrieves and Passes Transform Matrices to the Shader Program
    GLint modelLocPlane = glGetUniformLocation(gProgramIdPlane, "model");
    GLint viewLocPlane = glGetUniformLocation(gProgramIdPlane, "view");
    GLint projLocPlane = glGetUniformLocation(gProgramIdPlane, "projection");

    glUniformMatrix4fv(modelLocPlane, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLocPlane, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLocPlane, 1, GL_FALSE, glm::value_ptr(projection));

    GLint UVScaleLocPlane = glGetUniformLocation(gProgramIdPlane, "uvScale");
    glUniform2fv(UVScaleLocPlane, 1, glm::value_ptr(gUVScale));


    // Activate the plane VAO 
    glBindVertexArray(gMeshPlane.planeVao);

    // Texture Corresponing to Object
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdPlane);


    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMeshPlane.planeVertices);



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

    // Fill Vertex and Indices for the UCreateSpeaker
    UCreateSpeaker(verts, indices, NUM_SIDES, 0.20f, 0.09f);

    /* --------------- Draw the Spaker --------------- */

    const GLuint floatsPerVertex2 = 3;
    const GLuint floatsPerColor2 = 4;
    const GLuint floatsPerUV2 = 2;

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

    glVertexAttribPointer(2, floatsPerUV2, GL_FLOAT, GL_FALSE, stride2, (char*)(sizeof(float) * (floatsPerVertex2 + floatsPerColor2)));
    glEnableVertexAttribArray(2);

};

void UCreateMeshCube(GLMesh& mesh)

{
    // Vertex data
    GLfloat verts[] = {

        -0.5f, -0.5f, -0.5f,       0.0f,  0.0f, -1.0f,        0.0f, 0.0f,
          0.5f, -0.5f, -0.5f,       0.0f,  0.0f, -1.0f,        1.0f, 0.0f,
          0.5f,  0.5f, -0.5f,       0.0f,  0.0f, -1.0f,        1.0f, 1.0f,
          0.5f,  0.5f, -0.5f,       0.0f,  0.0f, -1.0f,        1.0f, 1.0f,
         -0.5f,  0.5f, -0.5f,       0.0f,  0.0f, -1.0f,        0.0f, 1.0f,
         -0.5f, -0.5f, -0.5f,       0.0f,  0.0f, -1.0f,        0.0f, 0.0f,


         -0.5f, -0.5f,  0.5f,       0.0f,  0.0f,  1.0f,        0.0f, 0.0f,
          0.5f, -0.5f,  0.5f,       0.0f,  0.0f,  1.0f,        1.0f, 0.0f,
          0.5f,  0.5f,  0.5f,       0.0f,  0.0f,  1.0f,        1.0f, 1.0f,
          0.5f,  0.5f,  0.5f,       0.0f,  0.0f,  1.0f,        1.0f, 1.0f,
         -0.5f,  0.5f,  0.5f,       0.0f,  0.0f,  1.0f,        0.0f, 1.0f,
         -0.5f, -0.5f,  0.5f,       0.0f,  0.0f,  1.0f,        0.0f, 0.0f,


         -0.5f,  0.5f,  0.5f,      -1.0f,  0.0f,  0.0f,        1.0f, 0.0f,
         -0.5f,  0.5f, -0.5f,      -1.0f,  0.0f,  0.0f,        1.0f, 1.0f,
         -0.5f, -0.5f, -0.5f,      -1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
         -0.5f, -0.5f, -0.5f,      -1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
         -0.5f, -0.5f,  0.5f,      -1.0f,  0.0f,  0.0f,        0.0f, 0.0f,
         -0.5f,  0.5f,  0.5f,      -1.0f,  0.0f,  0.0f,        1.0f, 0.0f,


          0.5f,  0.5f,  0.5f,       1.0f,  0.0f,  0.0f,        1.0f, 0.0f,
          0.5f,  0.5f, -0.5f,       1.0f,  0.0f,  0.0f,        1.0f, 1.0f,
          0.5f, -0.5f, -0.5f,       1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
          0.5f, -0.5f, -0.5f,       1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
          0.5f, -0.5f,  0.5f,       1.0f,  0.0f,  0.0f,        0.0f, 0.0f,
          0.5f,  0.5f,  0.5f,       1.0f,  0.0f,  0.0f,        1.0f, 0.0f,


         -0.5f, -0.5f, -0.5f,       0.0f, -1.0f,  0.0f,        0.0f, 1.0f,
          0.5f, -0.5f, -0.5f,       0.0f, -1.0f,  0.0f,        1.0f, 1.0f,
          0.5f, -0.5f,  0.5f,       0.0f, -1.0f,  0.0f,        1.0f, 0.0f,
          0.5f, -0.5f,  0.5f,       0.0f, -1.0f,  0.0f,        1.0f, 0.0f,
         -0.5f, -0.5f,  0.5f,       0.0f, -1.0f,  0.0f,        0.0f, 0.0f,
         -0.5f, -0.5f, -0.5f,       0.0f, -1.0f,  0.0f,        0.0f, 1.0f,


         -0.5f,  0.5f, -0.5f,       0.0f,  1.0f,  0.0f,        0.0f, 1.0f,
          0.5f,  0.5f, -0.5f,       0.0f,  1.0f,  0.0f,        1.0f, 1.0f,
          0.5f,  0.5f,  0.5f,       0.0f,  1.0f,  0.0f,        1.0f, 0.0f,
          0.5f,  0.5f,  0.5f,       0.0f,  1.0f,  0.0f,        1.0f, 0.0f,
         -0.5f,  0.5f,  0.5f,       0.0f,  1.0f,  0.0f,        0.0f, 0.0f,
         -0.5f,  0.5f, -0.5f,       0.0f,  1.0f,  0.0f,        0.0f, 1.0f


    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 3;
    const GLuint floatsPerUV = 2;


    mesh.cubeVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerColor + floatsPerUV));

    glGenVertexArrays(1, &mesh.cubeVao);
    glBindVertexArray(mesh.cubeVao);

    // Create VBO
    glGenBuffers(1, &mesh.cubeVbo);

    // Activates the Buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh.cubeVbo);

    // Send Vertex and Coordinations
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    // Stride between Vertex and Coordinations
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerColor)));
    glEnableVertexAttribArray(2);

}

void UCreateMeshPlane(GLMesh& mesh)
{
    // Vertex data
    GLfloat verts[] = {

        // Positions             // Colors (r, g, b, a)          // Textures
        2.0f, -0.26f,  1.0f,   1.0f,  0.0f,  1.0f,  1.0f,        1.0f, 1.0f,
       -2.0f, -0.26f, -1.0f,   0.0f,  1.0f,  1.0f,  1.0f,        1.0f, 0.0f,
        2.0f, -0.26f, -1.0f,   1.0f,  0.0f,  1.0f,  1.0f,        0.0f, 1.0f,

        2.0f, -0.26f,  1.0f,   1.0f,  0.0f,  1.0f,  1.0f,        0.0f, 1.0f,
       -2.0f, -0.26f,  1.0f,   0.0f,  1.0f,  1.0f,  1.0f,        0.0f, 0.0f,
       -2.0f, -0.26f, -1.0f,   0.0f,  0.0f,  1.0f,  1.0f,        1.0f, 0.0f
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;
    const GLuint floatsPerUV = 2;


    mesh.planeVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerColor + floatsPerUV));

    glGenVertexArrays(1, &mesh.planeVao);
    glBindVertexArray(mesh.planeVao);

    // Create VBO
    glGenBuffers(1, &mesh.planeVbo);

    // Activates the Buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh.planeVbo);

    // Send Vertex and Coordinations
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    // Stride between Vertex and Coordinations
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerColor)));
    glEnableVertexAttribArray(2);

}

// Destroy Mesh Function
void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.cylinderVao);
    glDeleteBuffers(2, mesh.cylinderVbo);
    glDeleteVertexArrays(1, &mesh.planeVao);
    glDeleteBuffers(1, &mesh.planeVbo);

}

// UCreateTexture Function
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // Texture Paramater
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // Filtering Texture Parameter
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0);

        return true;
    }
    // Error loading the image
    return false;
}

// UDestroyTexture Function
void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
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

// Destroy Shader Program Function
void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}