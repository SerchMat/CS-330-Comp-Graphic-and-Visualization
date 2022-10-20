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
                        E: Down
                        P: Ortho Projection
                        O: Reset Ortho Projection
                        ESC: Close Windows
                        L: Resume Orbiting
                        K: Stop Orbiting
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

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    // Windows Name
    const char* const WINDOW_TITLE = "Light & Objects"; 
    const int STRIDE = 7;

    // Window Width & Height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // GL Data Relative to Mesh
    struct GLMesh
    {
        // Vertex Array Object
        GLuint ruleVao, planeVao, cylinderVao, cubeVao, logoVao;

        // Vertex Buffer Object
        GLuint ruleVbo, planeVbo, cubeVbo, logoVbo;

        // Indices of the Mesh
        GLuint ruleVertices, planeVertices, cylinderVertices, cubeVertices, logoVertices;
        GLuint cylinderVbos[2];
    };

    // GLFW window
    GLFWwindow* gWindow = nullptr;

    // Triangle Mesh Data
    GLMesh gMesh;

    // Texture
    GLuint gTextureId;          /* Rule */
    GLuint gPlanePattern;       /* Keyboard Mat */
    GLuint gSpeakerPattern;    /* Speaker */
    GLuint gPlugPattern;       /* Wall Plug */
    GLuint gLogoPattern;       /* Logo */



    // Shader programs
    GLuint gProgramId;          /* Rule */
    GLuint gLampProgramId;      /* Lamp */
    GLuint gFillProgramId;      /* Fill */
    GLuint gPlaneProgramId;     /* Keyboard Mat */
    GLuint gSpeakerProgramId;   /* Speaker */
    GLuint gPlugProgramId;      /* Wall Plug */
    GLuint gLogoProgramId;      /* Logo */

    glm::vec2 gUVScale(5.0f, 5.0f);
    GLint gTexWrapMode = GL_REPEAT;

    // Camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 8.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // Timing
    float gDeltaTime = 0.0f; 
    float gLastFrame = 0.0f;

    // Cube & Light Color
    glm::vec3 gObjectColor(1.0f, 0.2f, 0.0f);

    glm::vec3 gLightColor(1.0f, 0.9f, 1.2f);
    glm::vec3 gLightPosition(4.0f, 5.5f, 3.0f);
    glm::vec3 gLightScale(1.3f);

    // Wall Plug (3D RECTANGLE)
    glm::vec3 gPlugPosition(0.0f, -0.62f, -1.0f);
    glm::vec3 gPlugScale(0.6f);

    glm::vec3 gFillColor(1.0f, 1.0f, 1.2f);
    glm::vec3 gFillPosition(-8.0f, 11.5f, 7.0f);
    glm::vec3 gFillScale(1.3f);

    // Rule (RECTANGLE)
    glm::vec3 gRulePosition(0.0f, -1.0f, -0.3f);
    glm::vec3 gRuleScale(0.85f);

    // Keyword Mat (PLANE)
    glm::vec3 gPlanePosition(0.0f, -0.5f, 0.0f);
    glm::vec3 gPlaneScale(3.0f);

    // Speaker (CYLINDER)
    glm::vec3 gSpeakerPosition(0.0f, -0.8f, 1.0f);
    glm::vec3 gSpeakerScale(3.0f);

    // Logo (CUBE)
    glm::vec3 gLogoPosition(0.4f, -0.5f, 0.95f);
    glm::vec3 gLogoScale(0.3f);

    // Lamp animation
    bool gLampIsOrbiting = true;
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

// Create Speaker
void UCreateSpeaker(GLfloat verts[], GLushort indices[], int numSides, float radius, float halfLen);

// Create Mesh and Empty
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);

// Create and Destroy Texture
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);

// Render Graphics
void URender();

// Shader Program
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);

/* Cube Vertex Shader Source Code */
const GLchar* cubeVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; 
    layout(location = 1) in vec3 normal; 
    layout(location = 2) in vec2 textureCoordinate;

    // Variables to Transfer Color
    out vec3 vertexNormal; 
    out vec3 vertexFragmentPos; 
    out vec2 vertexTextureCoordinate;

    // Variables for the Transform Matrices
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
    vertexNormal = mat3(transpose(inverse(model))) * normal;
    vertexTextureCoordinate = textureCoordinate;
}
);

/* Cube Fragment Shader Source Code */
const GLchar* cubeFragmentShaderSource = GLSL(440,

    in vec3 vertexNormal; 
    in vec3 vertexFragmentPos; 
    in vec2 vertexTextureCoordinate;

    // Variables to Transfer Color
    out vec4 fragmentColor; 
    out vec4 fillFragmentColor;

    // Variables for Object Color, Light Color, Light Position, and Camera/View Position
    uniform vec3 objectColor;
    uniform vec3 lightColor;
    uniform vec3 lightPos;
    uniform vec3 fillColor;
    uniform vec3 fillPos;
    uniform vec3 viewPosition;
    uniform sampler2D uTexture; 
    uniform vec2 uvScale;

// Main
void main()
{
    /* Phon Lighting Model */

    // Ambient/Global Lighting Strength
    float ambientStrength = 0.5f; 

    // Ambient Light Color
    vec3 ambient = ambientStrength * lightColor; 

    /* Calculate Diffuse Lighting */

    vec3 norm = normalize(vertexNormal);

    // Calculate Distance Between Light & Fragment Objects
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos);

    // Calculate Diffuse Impact
    float impact = max(dot(norm, lightDirection), 0.0);

    // Generate Diffuse Light Color
    vec3 diffuse = impact * lightColor; 

    /* Calculate Specular lighting */

    // Specular Light Strength
    float specularIntensity = 0.3f;

    // Specular Highlight Size
    float highlightSize = 2.0f;

    // Calculate View Direction
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos);

    // Calculate Reflection Vector
    vec3 reflectDir = reflect(-lightDirection, norm);

    //Calculate Specular Component
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    vec3 specular = specularIntensity * specularComponent * lightColor;

    /* Calculate Key Lighting */

    // Set Ambient/Global Lighting Strength
    float fillAmbientStrength = 0.1f; 

    // Generate Ambient Light Color
    vec3 fillAmbient = fillAmbientStrength * fillColor; 

    /* Calculate Diffuse Lighting */

    // Calculate Distance Between Light Source & Fragment on the Objects
    vec3 fillDirection = normalize(fillPos - vertexFragmentPos);

    // Calculate Diffuse Impact
    float fillImpact = max(dot(norm, fillDirection), 0.0);

    // Generate Diffuse Light Color
    vec3 fillDiffuse = fillImpact * fillColor;

    /* Calculate Specular lighting */

    // Specular light strength
    float fillSpecularIntensity = 0.5f;

    // Specular Highlight Size
    float fillHighlightSize = 8.0f; 

    // Calculate View Direction
    vec3 fillViewDir = normalize(viewPosition - vertexFragmentPos);

    // Calculate Reflection Vector
    vec3 fillReflectDir = reflect(-fillDirection, norm);

    // Calculate Specular Component
    float fillSpecularComponent = pow(max(dot(fillViewDir, fillReflectDir), 0.0), fillHighlightSize);
    vec3 fillSpecular = fillSpecularIntensity * fillSpecularComponent * fillColor;

    // Calculate Phong Result
    vec3 objectColor = texture(uTexture, vertexTextureCoordinate).xyz;
    vec3 keyResult = (ambient + diffuse + specular);
    vec3 fillResult = (fillAmbient + fillDiffuse + fillSpecular);
    vec3 lightingResult = keyResult + fillResult;
    vec3 phong = (lightingResult)*objectColor;

    // Send Light Results
    fragmentColor = vec4(phong, 1.0f); 
}
);

/* Lamp Shader Source Code */
const GLchar* lampVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; 

    // Variables for the Transform Matrices
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

// Main
void main()
{
    // Transforms Vertices into Clip Coordinates
    gl_Position = projection * view * model * vec4(position, 1.0f); 
}
);

/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; 

// Main
void main()
{
    // Set Color
    fragmentColor = vec4(1.0f); 
}
);

/* Fill Vertex Shader Source */
const GLchar* fillVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; 

    // Variables for the Transform Matrices
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

// Main
void main()
{
    // Transforms Vertices into Clip Coordinates
    gl_Position = projection * view * model * vec4(position, 1.0f);
}
);


/* Fragment Shader Source Code */
const GLchar* fillFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; 

// Main
void main()
{
    // Set Color
    fragmentColor = vec4(1.0f);
}
);

// Position the Texture based on the X & Y Axis
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
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

    /* Rule */
    if (!UCreateShaderProgram(cubeVertexShaderSource, cubeFragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    /* Lamp */
    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId))
        return EXIT_FAILURE;

    /* Keyboard Mat */
    if (!UCreateShaderProgram(cubeVertexShaderSource, cubeFragmentShaderSource, gPlaneProgramId))
        return EXIT_FAILURE;

    /* Speaker */
    if (!UCreateShaderProgram(cubeVertexShaderSource, cubeFragmentShaderSource, gSpeakerProgramId))  
        return EXIT_FAILURE;

    /* Wall Plug */
    if (!UCreateShaderProgram(cubeVertexShaderSource, cubeFragmentShaderSource, gPlugProgramId))
        return EXIT_FAILURE;

    /* Logo */
    if (!UCreateShaderProgram(cubeVertexShaderSource, cubeFragmentShaderSource, gLogoProgramId))
        return EXIT_FAILURE;

    /* Fill */
    if (!UCreateShaderProgram(fillVertexShaderSource, fillFragmentShaderSource, gFillProgramId))
        return EXIT_FAILURE;

    /* ******************** LOAD TEXTURES ******************** */
    
    // RULE
    const char* texFilename = "rule.jpg";
    if (!UCreateTexture(texFilename, gTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    // KEYBOARD(PLANE)
    texFilename = "razenmat.png";              
    if (!UCreateTexture(texFilename, gPlanePattern))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    // SPEAKER
    texFilename = "speaker.png";         
    if (!UCreateTexture(texFilename, gSpeakerPattern))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    // WALL PLUG
    texFilename = "wplug.jpg";              
    if (!UCreateTexture(texFilename, gPlugPattern))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    // LOGO
    texFilename = "logo.png";               
    if (!UCreateTexture(texFilename, gLogoPattern))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    
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

    // Release Texture
    UDestroyTexture(gTextureId);

    // Release Shader Programs
    UDestroyShaderProgram(gProgramId);
    UDestroyShaderProgram(gLampProgramId);
    UDestroyShaderProgram(gPlaneProgramId);
    UDestroyShaderProgram(gSpeakerProgramId);
    UDestroyShaderProgram(gPlugProgramId);
    UDestroyShaderProgram(gFillProgramId);
    UDestroyShaderProgram(gLogoProgramId);
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

    // ESC Key to Finilize
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

    // Pause & Resume Lamp Orbiting
    static bool isLKeyDown = false;

    // L Key to the Stop Lamp Orbitate
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !gLampIsOrbiting)
        gLampIsOrbiting = true;

    // K Key to the Stop Lamp Orbitate
    else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && gLampIsOrbiting)
        gLampIsOrbiting = false;

}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// GLFW: If Window Chnage this Option will Execute
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

/* ------------- Create the Speaker ------------- */
void UCreateSpeaker(GLfloat verts[], GLushort indices[], int numSides, float radius, float halfLen) {
    const float TWO_PI = 2.0f * 3.1415926f;
    const float radiansPerSide = TWO_PI / numSides;

    int currentVertex = 0;

    verts[0] = 0.0f;        //0 X
    verts[1] = halfLen;     //0 Y
    verts[2] = 0.0f;        //0 Z
    verts[3] = 0.0f;        //0 R
    verts[4] = 0.0f;        //0 G    
    verts[5] = 0.0f;        //0 B
    verts[6] = 1.0f;        //0 A
    currentVertex++;
    verts[7] = 0.0f;        //1 X
    verts[8] = -halfLen;    //1 Y
    verts[9] = 0.0f;        //1 Z
    verts[10] = 0.0f;       //1 R
    verts[11] = 0.0f;       //1 G
    verts[12] = 0.0f;       //1 B
    verts[13] = 1.0f;       //1 A
    currentVertex++;

    // Track of Triangles Added
    int currentTriangle = 0;
    for (int edge = 0; edge < numSides; edge++) {

        // Calculate the Center Point to the Next Vertex
        float theta = ((float)edge) * radiansPerSide;

        verts[currentVertex * STRIDE + 0] = radius * cos(theta);    // X
        verts[currentVertex * STRIDE + 1] = halfLen;                // Y
        verts[currentVertex * STRIDE + 2] = radius * sin(theta);    // Z
        verts[currentVertex * STRIDE + 3] = 1.0f;                   // R
        verts[currentVertex * STRIDE + 4] = 0.0f;                   // G
        verts[currentVertex * STRIDE + 5] = 0.0f;                   // B
        verts[currentVertex * STRIDE + 6] = 1.0f;                   // A
        currentVertex++;

        verts[currentVertex * STRIDE + 0] = radius * cos(theta);    // X
        verts[currentVertex * STRIDE + 1] = -halfLen;               // Y
        verts[currentVertex * STRIDE + 2] = radius * sin(theta);    // Z
        verts[currentVertex * STRIDE + 3] = 1.0f;                   // R
        verts[currentVertex * STRIDE + 4] = 0.0f;                   // G
        verts[currentVertex * STRIDE + 5] = 0.0f;                   // B
        verts[currentVertex * STRIDE + 6] = 1.0f;                   // A
        currentVertex++;

        //Indices for the Triangles
        if (edge > 0) {

            // Top triangle
            indices[(3 * currentTriangle) + 0] = 0;                 // Center-Top Prisim
            indices[(3 * currentTriangle) + 1] = currentVertex - 4; // Upper-Left Side
            indices[(3 * currentTriangle) + 2] = currentVertex - 2; // Upper Right Side
            currentTriangle++;

            // Bottom Triangle
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
    indices[(3 * currentTriangle) + 0] = 0;                         // Center Top Prims
    indices[(3 * currentTriangle) + 1] = currentVertex - 2;         // Upper-Left Side
    indices[(3 * currentTriangle) + 2] = 2;                         // Fist Upper-Left (Change to Right)
    currentTriangle++;

    // Bottom Triangle
    indices[(3 * currentTriangle) + 0] = 1;                         //Center-Bottom Prism
    indices[(3 * currentTriangle) + 1] = currentVertex - 1;         //Bottom-Left Side
    indices[(3 * currentTriangle) + 2] = 3;                         // Fist Bottom-Left (Change to Right)
    currentTriangle++;

    // First Triangle 1/2 Rectangular Side
    indices[(3 * currentTriangle) + 0] = currentVertex - 2;         // Upper-Left Side
    indices[(3 * currentTriangle) + 1] = currentVertex - 1;         //Bottom-Left Side
    indices[(3 * currentTriangle) + 2] = 3;                         // Bottom-Right Side
    currentTriangle++;

    // Second Triangle 1/2 Rectangular Side
    indices[(3 * currentTriangle) + 0] = 3;                         // Bottom-Right Side
    indices[(3 * currentTriangle) + 1] = 2;                         // Upper-Right Side
    indices[(3 * currentTriangle) + 2] = currentVertex - 2;         // Upper-Left Side
    currentTriangle++;
}


// URender Frame Function
void URender()
{
    // Lamp Orbits (Around the Origin)
    const float angularVelocity = glm::radians(45.0f);
    if (!gLampIsOrbiting)
    {
        glm::vec4 newPosition = glm::rotate(angularVelocity * gDeltaTime, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(gLightPosition, 1.0f);
        gLightPosition.x = newPosition.x;
        gLightPosition.y = newPosition.y;
        gLightPosition.z = newPosition.z;
    }

    // Enable Z-Depth
    glEnable(GL_DEPTH_TEST);

    // Clear the Frame and Z Buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Activate the Rule VAO (used by Rule and Lamp)
    glBindVertexArray(gMesh.ruleVao);

    /* --------------------------- DRAW THE RULE --------------------------- */
    glUseProgram(gProgramId);

    // Model Matrix: Right-to-Left Order
    glm::mat4 model = glm::translate(gRulePosition) * glm::scale(gRuleScale);

    // Camera/View transformation
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

    // Reference Matrix Uniforms from the Rule Shader Program for the Cub Color, Light Color, Light Position, and Camera Position
    GLint objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    GLint lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    GLint lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    GLint fillColorLoc = glGetUniformLocation(gProgramId, "fillColor");
    GLint fillPositionLoc = glGetUniformLocation(gProgramId, "fillPos");
    GLint viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

    // Pass Color, Light, and Camera Data to the Cube Shader Program's Corresponding Uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    glUniform3f(fillColorLoc, gFillColor.r, gFillColor.g, gFillColor.b);
    glUniform3f(fillPositionLoc, gFillPosition.x, gFillPosition.y, gFillPosition.z);

    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // Texture Corresponing to Object
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

    // Draws the Rule
    glDrawArrays(GL_TRIANGLES, 0, gMesh.ruleVertices);


    /* --------------------------- DRAW THE KEYBOARD MAT --------------------------- */
    glUseProgram(gProgramId);

    glBindTexture(GL_TEXTURE_2D, gPlanePattern);
    glActiveTexture(GL_TEXTURE0);

    model = glm::translate(gPlanePosition) * glm::scale(gPlaneScale);

    modelLoc = glGetUniformLocation(gPlaneProgramId, "model");
    viewLoc = glGetUniformLocation(gPlaneProgramId, "view");
    projLoc = glGetUniformLocation(gPlaneProgramId, "projection");


    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glBindVertexArray(gMesh.planeVao);

    glDrawArrays(GL_TRIANGLES, 0, gMesh.planeVertices);


    /* --------------------------- DRAW THE SPEAKER --------------------------- */
    glUseProgram(gProgramId);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gSpeakerPattern);


    model = glm::translate(gSpeakerPosition) * glm::scale(gSpeakerScale);

    modelLoc = glGetUniformLocation(gSpeakerProgramId, "model");
    viewLoc = glGetUniformLocation(gSpeakerProgramId, "view");
    projLoc = glGetUniformLocation(gSpeakerProgramId, "projection");


    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(gMesh.cylinderVao);

    glDrawElements(GL_TRIANGLES, gMesh.cylinderVertices, GL_UNSIGNED_SHORT, NULL);


    /* --------------------------- DRAW THE WALL PLUG --------------------------- */
    glUseProgram(gProgramId);
    glBindTexture(GL_TEXTURE_2D, gPlugPattern);
    glActiveTexture(GL_TEXTURE0);

    model = glm::translate(gPlugPosition) * glm::scale(gPlugScale);

    modelLoc = glGetUniformLocation(gPlugProgramId, "model");
    viewLoc = glGetUniformLocation(gPlugProgramId, "view");
    projLoc = glGetUniformLocation(gPlugProgramId, "projection");


    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glBindVertexArray(gMesh.cubeVao);

    glDrawArrays(GL_TRIANGLES, 0, gMesh.cubeVertices);


    /* --------------------------- DRAW THE SPEAKER LOGO --------------------------- */
    glUseProgram(gProgramId);
    glBindTexture(GL_TEXTURE_2D, gLogoPattern);
    glActiveTexture(GL_TEXTURE0);

    model = glm::translate(gLogoPosition) * glm::scale(gLogoScale);

    modelLoc = glGetUniformLocation(gLogoProgramId, "model");
    viewLoc = glGetUniformLocation(gLogoProgramId, "view");
    projLoc = glGetUniformLocation(gLogoProgramId, "projection");


    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glBindVertexArray(gMesh.logoVao);

    glDrawArrays(GL_TRIANGLES, 0, gMesh.logoVertices);


    /* --------------------------- DRAW THE LAMP --------------------------- */
    glUseProgram(gLampProgramId);

    // Transform the Smaller Cube used as a Visual for the Light Source
    model = glm::translate(gLightPosition) * glm::scale(gLightScale);

    // Reference Matrix Uniforms from the Lamp Shader Program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    // Pass Matrix Data to the Lamp Shader program's Matrix Uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, gMesh.cylinderVertices);


    /* --------------------------- DRAW THE FILL --------------------------- */
    glUseProgram(gFillProgramId);

    // Transform the Smaller Cube used as a Visual for the Light Source
    model = glm::translate(gFillPosition) * glm::scale(gFillScale);

    // Reference Matrix Uniforms from the Lamp Shader Program
    modelLoc = glGetUniformLocation(gFillProgramId, "model");
    viewLoc = glGetUniformLocation(gFillProgramId, "view");
    projLoc = glGetUniformLocation(gFillProgramId, "projection");

    // Pass Matrix Data to the Lamp Shader program's Matrix Uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, gMesh.cylinderVertices);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
    glUseProgram(0);

    // GLFW: Swap Buffers 
    glfwSwapBuffers(gWindow); 
}

// Implements the UCreateMesh Function
void UCreateMesh(GLMesh& mesh)
{
    // Number of Sides Create to the Cylinder
    const int NUM_SIDES = 100;

    // Number of Vertices (number of sides * 2)
    const int NUM_VERTICES = STRIDE * (2 + (2 * NUM_SIDES));

    // Number of Indices (12 * the number of triangles)
    const int NUM_INDICES = 12 * NUM_SIDES;

    // Position & Color Data
    GLfloat verts[NUM_VERTICES];

    // Index Data and Color Data
    GLushort indices[NUM_INDICES];

    /* ======================== POSITION & COLOR DATA ======================== */

    // RULE
    GLfloat ruleVerts[] = {

    //          Positions                   Normals               Textures
         -1.5f, -0.10f, -0.20f,       0.0f,  0.0f, -1.0f,        0.0f, 0.0f,
          1.5f, -0.10f, -0.20f,       0.0f,  0.0f, -1.0f,        1.0f, 0.0f,
          1.5f,  0.10f, -0.20f,       0.0f,  0.0f, -1.0f,        1.0f, 1.0f,
          1.5f,  0.10f, -0.20f,       0.0f,  0.0f, -1.0f,        1.0f, 1.0f,
         -1.5f,  0.10f, -0.20f,       0.0f,  0.0f, -1.0f,        0.0f, 1.0f,
         -1.5f, -0.10f, -0.20f,       0.0f,  0.0f, -1.0f,        0.0f, 0.0f,


         -1.5f, -0.10f,  0.20f,       0.0f,  0.0f,  1.0f,        0.0f, 0.0f,
          1.5f, -0.10f,  0.20f,       0.0f,  0.0f,  1.0f,        1.0f, 0.0f,
          1.5f,  0.10f,  0.20f,       0.0f,  0.0f,  1.0f,        1.0f, 1.0f,
          1.5f,  0.10f,  0.20f,       0.0f,  0.0f,  1.0f,        1.0f, 1.0f,
         -1.5f,  0.10f,  0.20f,       0.0f,  0.0f,  1.0f,        0.0f, 1.0f,
         -1.5f, -0.10f,  0.20f,       0.0f,  0.0f,  1.0f,        0.0f, 0.0f,


         -1.5f,  0.10f,  0.20f,      -1.0f,  0.0f,  0.0f,        1.0f, 0.0f,
         -1.5f,  0.10f, -0.20f,      -1.0f,  0.0f,  0.0f,        1.0f, 1.0f,
         -1.5f, -0.10f, -0.20f,      -1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
         -1.5f, -0.10f, -0.20f,      -1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
         -1.5f, -0.10f,  0.20f,      -1.0f,  0.0f,  0.0f,        0.0f, 0.0f,
         -1.5f,  0.10f,  0.20f,      -1.0f,  0.0f,  0.0f,        1.0f, 0.0f,


          1.5f,  0.10f,  0.20f,       1.0f,  0.0f,  0.0f,        1.0f, 0.0f,
          1.5f,  0.10f, -0.20f,       1.0f,  0.0f,  0.0f,        1.0f, 1.0f,
          1.5f, -0.10f, -0.20f,       1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
          1.5f, -0.10f, -0.20f,       1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
          1.5f, -0.10f,  0.20f,       1.0f,  0.0f,  0.0f,        0.0f, 0.0f,
          1.5f,  0.10f,  0.20f,       1.0f,  0.0f,  0.0f,        1.0f, 0.0f,


         -1.5f, -0.10f, -0.20f,       0.0f, -1.0f,  0.0f,        0.0f, 1.0f,
          1.5f, -0.10f, -0.20f,       0.0f, -1.0f,  0.0f,        1.0f, 1.0f,
          1.5f, -0.10f,  0.20f,       0.0f, -1.0f,  0.0f,        1.0f, 0.0f,
          1.5f, -0.10f,  0.20f,       0.0f, -1.0f,  0.0f,        1.0f, 0.0f,
         -1.5f, -0.10f,  0.20f,       0.0f, -1.0f,  0.0f,        0.0f, 0.0f,
         -1.5f, -0.10f, -0.20f,       0.0f, -1.0f,  0.0f,        0.0f, 1.0f,


         -1.5f,  0.10f, -0.20f,       0.0f,  1.0f,  0.0f,        0.0f, 1.0f,
          1.5f,  0.10f, -0.20f,       0.0f,  1.0f,  0.0f,        1.0f, 1.0f,
          1.5f,  0.10f,  0.20f,       0.0f,  1.0f,  0.0f,        1.0f, 0.0f,
          1.5f,  0.10f,  0.20f,       0.0f,  1.0f,  0.0f,        1.0f, 0.0f,
         -1.5f,  0.10f,  0.20f,       0.0f,  1.0f,  0.0f,        0.0f, 0.0f,
         -1.5f,  0.10f, -0.20f,       0.0f,  1.0f,  0.0f,        0.0f, 1.0f

    };

    // KEYBOARD MAT
    GLfloat planeVerts[] = {
     //       Positions                   Normals               Textures
          2.0f, -0.2f,  1.0f,        0.0f,  0.0f, 1.0f,        1.0f, 0.0f,
         -2.0f, -0.2f, -1.0f,        0.0f,  0.0f, 1.0f,        0.0f, 0.0f,
          2.0f, -0.2f, -1.0f,        0.0f,  0.0f, 1.0f,        0.0f, 1.0f,

          2.0f, -0.2f,  1.0f,        0.0f,  0.0f, 1.0f,        1.0f, 0.0f,
         -2.0f, -0.2f,  1.0f,        0.0f,  0.0f, 1.0f,        1.0f, 1.0f,
         -2.0f, -0.2f, -1.0f,        0.0f,  0.0f, 1.0f,        0.0f, 0.0f,
    };

    // PLUG
    GLfloat plugVerts[] = {

             //Positions                  Normals                Textures
         -0.5f, -0.75f, -0.65f,       0.0f,  0.0f, -1.0f,        0.0f, 0.0f,
          0.5f, -0.75f, -0.65f,       0.0f,  0.0f, -1.0f,        1.0f, 0.0f,
          0.5f,  0.75f, -0.65f,       0.0f,  0.0f, -1.0f,        1.0f, 1.0f,
          0.5f,  0.75f, -0.65f,       0.0f,  0.0f, -1.0f,        1.0f, 1.0f,
         -0.5f,  0.75f, -0.65f,       0.0f,  0.0f, -1.0f,        0.0f, 1.0f,
         -0.5f, -0.75f, -0.65f,       0.0f,  0.0f, -1.0f,        0.0f, 0.0f,


         -0.5f, -0.75f,  0.65f,       0.0f,  0.0f,  1.0f,        0.0f, 0.0f,
          0.5f, -0.75f,  0.65f,       0.0f,  0.0f,  1.0f,        1.0f, 0.0f,
          0.5f,  0.75f,  0.65f,       0.0f,  0.0f,  1.0f,        1.0f, 1.0f,
          0.5f,  0.75f,  0.65f,       0.0f,  0.0f,  1.0f,        1.0f, 1.0f,
         -0.5f,  0.75f,  0.65f,       0.0f,  0.0f,  1.0f,        0.0f, 1.0f,
         -0.5f, -0.75f,  0.65f,       0.0f,  0.0f,  1.0f,        0.0f, 0.0f,


         -0.5f,  0.75f,  0.65f,      -1.0f,  0.0f,  0.0f,        1.0f, 0.0f,
         -0.5f,  0.75f, -0.65f,      -1.0f,  0.0f,  0.0f,        1.0f, 1.0f,
         -0.5f, -0.75f, -0.65f,      -1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
         -0.5f, -0.75f, -0.65f,      -1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
         -0.5f, -0.75f,  0.65f,      -1.0f,  0.0f,  0.0f,        0.0f, 0.0f,
         -0.5f,  0.75f,  0.65f,      -1.0f,  0.0f,  0.0f,        1.0f, 0.0f,


          0.5f,  0.75f,  0.65f,       1.0f,  0.0f,  0.0f,        1.0f, 0.0f,
          0.5f,  0.75f, -0.65f,       1.0f,  0.0f,  0.0f,        1.0f, 1.0f,
          0.5f, -0.75f, -0.65f,       1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
          0.5f, -0.75f, -0.65f,       1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
          0.5f, -0.75f,  0.65f,       1.0f,  0.0f,  0.0f,        0.0f, 0.0f,
          0.5f,  0.75f,  0.65f,       1.0f,  0.0f,  0.0f,        1.0f, 0.0f,


         -0.5f, -0.75f, -0.65f,       0.0f, -1.0f,  0.0f,        0.0f, 1.0f,
          0.5f, -0.75f, -0.65f,       0.0f, -1.0f,  0.0f,        1.0f, 1.0f,
          0.5f, -0.75f,  0.65f,       0.0f, -1.0f,  0.0f,        1.0f, 0.0f,
          0.5f, -0.75f,  0.65f,       0.0f, -1.0f,  0.0f,        1.0f, 0.0f,
         -0.5f, -0.75f,  0.65f,       0.0f, -1.0f,  0.0f,        0.0f, 0.0f,
         -0.5f, -0.75f, -0.65f,       0.0f, -1.0f,  0.0f,        0.0f, 1.0f,


         -0.5f,  0.75f, -0.65f,       0.0f,  1.0f,  0.0f,        0.0f, 1.0f,
          0.5f,  0.75f, -0.65f,       0.0f,  1.0f,  0.0f,        1.0f, 1.0f,
          0.5f,  0.75f,  0.65f,       0.0f,  1.0f,  0.0f,        1.0f, 0.0f,
          0.5f,  0.75f,  0.65f,       0.0f,  1.0f,  0.0f,        1.0f, 0.0f,
         -0.5f,  0.75f,  0.65f,       0.0f,  1.0f,  0.0f,        0.0f, 0.0f,
         -0.5f,  0.75f, -0.65f,       0.0f,  1.0f,  0.0f,        0.0f, 1.0f
    };

    // LOGO & LIGHT
    GLfloat logoVerts[] = {

         // Positions                     Normals                Textures
         -0.5f, -0.25f, -0.5f,       0.0f,  0.0f, -1.0f,        0.0f, 0.0f,
          0.5f, -0.25f, -0.5f,       0.0f,  0.0f, -1.0f,        1.0f, 0.0f,
          0.5f,  0.25f, -0.5f,       0.0f,  0.0f, -1.0f,        1.0f, 1.0f,
          0.5f,  0.25f, -0.5f,       0.0f,  0.0f, -1.0f,        1.0f, 1.0f,
         -0.5f,  0.25f, -0.5f,       0.0f,  0.0f, -1.0f,        0.0f, 1.0f,
         -0.5f, -0.25f, -0.5f,       0.0f,  0.0f, -1.0f,        0.0f, 0.0f,


         -0.5f, -0.25f,  0.5f,       0.0f,  0.0f,  1.0f,        0.0f, 0.0f,
          0.5f, -0.25f,  0.5f,       0.0f,  0.0f,  1.0f,        1.0f, 0.0f,
          0.5f,  0.25f,  0.5f,       0.0f,  0.0f,  1.0f,        1.0f, 1.0f,
          0.5f,  0.25f,  0.5f,       0.0f,  0.0f,  1.0f,        1.0f, 1.0f,
         -0.5f,  0.25f,  0.5f,       0.0f,  0.0f,  1.0f,        0.0f, 1.0f,
         -0.5f, -0.25f,  0.5f,       0.0f,  0.0f,  1.0f,        0.0f, 0.0f,


         -0.5f,  0.25f,  0.5f,      -1.0f,  0.0f,  0.0f,        1.0f, 0.0f,
         -0.5f,  0.25f, -0.5f,      -1.0f,  0.0f,  0.0f,        1.0f, 1.0f,
         -0.5f, -0.25f, -0.5f,      -1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
         -0.5f, -0.25f, -0.5f,      -1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
         -0.5f, -0.25f,  0.5f,      -1.0f,  0.0f,  0.0f,        0.0f, 0.0f,
         -0.5f,  0.25f,  0.5f,      -1.0f,  0.0f,  0.0f,        1.0f, 0.0f,


          0.5f,  0.25f,  0.5f,       1.0f,  0.0f,  0.0f,        1.0f, 0.0f,
          0.5f,  0.25f, -0.5f,       1.0f,  0.0f,  0.0f,        1.0f, 1.0f,
          0.5f, -0.25f, -0.5f,       1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
          0.5f, -0.25f, -0.5f,       1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
          0.5f, -0.25f,  0.5f,       1.0f,  0.0f,  0.0f,        0.0f, 0.0f,
          0.5f,  0.25f,  0.5f,       1.0f,  0.0f,  0.0f,        1.0f, 0.0f,


         -0.5f, -0.25f, -0.5f,       0.0f, -1.0f,  0.0f,        0.0f, 1.0f,
          0.5f, -0.25f, -0.5f,       0.0f, -1.0f,  0.0f,        1.0f, 1.0f,
          0.5f, -0.25f,  0.5f,       0.0f, -1.0f,  0.0f,        1.0f, 0.0f,
          0.5f, -0.25f,  0.5f,       0.0f, -1.0f,  0.0f,        1.0f, 0.0f,
         -0.5f, -0.25f,  0.5f,       0.0f, -1.0f,  0.0f,        0.0f, 0.0f,
         -0.5f, -0.25f, -0.5f,       0.0f, -1.0f,  0.0f,        0.0f, 1.0f,


         -0.5f,  0.25f, -0.5f,       0.0f,  1.0f,  0.0f,        0.0f, 1.0f,
          0.5f,  0.25f, -0.5f,       0.0f,  1.0f,  0.0f,        1.0f, 1.0f,
          0.5f,  0.25f,  0.5f,       0.0f,  1.0f,  0.0f,        1.0f, 0.0f,
          0.5f,  0.25f,  0.5f,       0.0f,  1.0f,  0.0f,        1.0f, 0.0f,
         -0.5f,  0.25f,  0.5f,       0.0f,  1.0f,  0.0f,        0.0f, 0.0f,
         -0.5f,  0.25f, -0.5f,       0.0f,  1.0f,  0.0f,        0.0f, 1.0f
    };

    // Activate and Fill the Speaker
    UCreateSpeaker(verts, indices, NUM_SIDES, 0.25f, 0.10f);

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.ruleVertices = sizeof(ruleVerts) / (sizeof(ruleVerts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    // Create Buffers
    glGenVertexArrays(1, &mesh.ruleVao); 
    glBindVertexArray(mesh.ruleVao);

    // Send Verter or Coordinations
    glGenBuffers(1, &mesh.ruleVbo);

    // Activates the Buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh.ruleVbo);

    // Send Vertex and Coordinations
    glBufferData(GL_ARRAY_BUFFER, sizeof(ruleVerts), ruleVerts, GL_STATIC_DRAW);

    // Stride between Vertex and Coordinations
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);


    /* ******************* DRAW THE PLANE ******************* */
    mesh.planeVertices = sizeof(planeVerts) / (sizeof(planeVerts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.planeVao); 
    glBindVertexArray(mesh.planeVao);

    // Create Buffers
    glGenBuffers(1, &mesh.planeVbo);

    // Activate the Buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh.planeVbo); 

    // Send Verter or Coordinations
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVerts), planeVerts, GL_STATIC_DRAW);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);


    /* ******************* DRAW THE SPEAKER ******************* */
    const GLuint floatsPerVertex2 = 3;
    const GLuint floatsPerColor2 = 4;
    const GLuint floatsPerUV2 = 2;

    glGenVertexArrays(1, &mesh.cylinderVao);
    glBindVertexArray(mesh.cylinderVao);

    // Create Buffers
    glGenBuffers(2, mesh.cylinderVbos);

    // Activate the Buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh.cylinderVbos[0]); 

    // Send Verter or Coordinations
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    mesh.cylinderVertices = sizeof(indices) / sizeof(indices[0]);

    // Activates the Buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.cylinderVbos[1]);

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


    /* ******************* DRAW THE PLUG ******************* */
    const GLuint cubeFloatsPerVertex = 3;
    const GLuint cubeFloatsPerNormal = 3;
    const GLuint cubeFloatsPerUV = 2;

    mesh.cubeVertices = sizeof(plugVerts) / (sizeof(plugVerts[0]) * (cubeFloatsPerVertex + cubeFloatsPerNormal + cubeFloatsPerUV));

    glGenVertexArrays(1, &mesh.cubeVao); 
    glBindVertexArray(mesh.cubeVao);

    // Create Buffers
    glGenBuffers(1, &mesh.cubeVbo);

    // Activate the Buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh.cubeVbo); 

    // Send Verter or Coordinations
    glBufferData(GL_ARRAY_BUFFER, sizeof(plugVerts), plugVerts, GL_STATIC_DRAW); 

    // Stride between Vertex and Coordinations
    GLint cubeStride = sizeof(float) * (cubeFloatsPerVertex + cubeFloatsPerNormal + cubeFloatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, cubeFloatsPerVertex, GL_FLOAT, GL_FALSE, cubeStride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, cubeFloatsPerNormal, GL_FLOAT, GL_FALSE, cubeStride, (void*)(sizeof(float) * cubeFloatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, cubeFloatsPerUV, GL_FLOAT, GL_FALSE, cubeStride, (void*)(sizeof(float) * (cubeFloatsPerVertex + cubeFloatsPerNormal)));
    glEnableVertexAttribArray(2);


    /* ******************* DRAW THE LOGO ******************* */
    const GLuint cube2FloatsPerVertex = 3;
    const GLuint cube2FloatsPerNormal = 3;
    const GLuint cube2FloatsPerUV = 2;

    mesh.logoVertices = sizeof(logoVerts) / (sizeof(logoVerts[0]) * (cube2FloatsPerVertex + cube2FloatsPerNormal + cube2FloatsPerUV));

    glGenVertexArrays(1, &mesh.logoVao);
    glBindVertexArray(mesh.logoVao);

    // Create Buffers
    glGenBuffers(1, &mesh.logoVbo);

    // Activate the Buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh.logoVbo);

    // Send Verter or Coordinations
    glBufferData(GL_ARRAY_BUFFER, sizeof(logoVerts), logoVerts, GL_STATIC_DRAW); 

    // Stride between Vertex and Coordinations
    GLint cube2Stride = sizeof(float) * (cube2FloatsPerVertex + cube2FloatsPerNormal + cube2FloatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, cube2FloatsPerVertex, GL_FLOAT, GL_FALSE, cube2Stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, cube2FloatsPerNormal, GL_FLOAT, GL_FALSE, cube2Stride, (void*)(sizeof(float) * cube2FloatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, cube2FloatsPerUV, GL_FLOAT, GL_FALSE, cube2Stride, (void*)(sizeof(float) * (cube2FloatsPerVertex + cube2FloatsPerNormal)));
    glEnableVertexAttribArray(2);

}

// Destroy Mesh Function
void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.ruleVao);
    glDeleteBuffers(1, &mesh.ruleVbo);
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

    // Error Loading the Image
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

    // Links the Shader Program
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