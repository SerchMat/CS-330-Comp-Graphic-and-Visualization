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

using namespace std;

// camera.h 
#include "camera.h"

// Shader Program Macro
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
using namespace std;

// Shader program Macro
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    // Windows Name
    const char* const WINDOW_TITLE = "Texture Pyramid";

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

    // Texture
    GLuint gTextureId;

     // Shader Program
    GLuint gProgramId;

    glm::vec2 gUVScale(5.0f, 5.0f);
    GLint gTexWrapMode = GL_REPEAT;


    // Camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // Timing
    float gDeltaTime = 0.0f; 
    float gLastFrame = 0.0f;

    // Change the Initial Position on the Pyramid
    glm::vec3 gPositionPyramid(-2.0f, 0.0f, -9.0f);
    glm::vec3 gScalePyramid(2.0f);

    // Allow Rotations
    glm::mat4 rotationPyramidX = glm::rotate(-1.57f, glm::vec3(1.0, 0.0f, 0.0f));
    glm::mat4 rotationPyramidY = glm::rotate(0.1f, glm::vec3(0.0, 1.0f, 0.0f));
    glm::mat4 rotationPyramidZ = glm::rotate(-0.5f, glm::vec3(0.0, 0.0f, 1.0f));

    glm::mat4 rotationPyramid = rotationPyramidX * rotationPyramidY * rotationPyramidZ;


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

// Create Mesh and Empty
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);

// Create Te3xture and Empty
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);

// Render Graphics
void URender();

// Shader Program
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);



// Vertex Shader Source
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position;
    layout(location = 1) in vec4 color;
    layout(location = 2) in vec2 textureCoordinate;

// Transfer Color and Texture
out vec4 vertexColor; 
out vec3 vertexFragmentPos;   
out vec2 vertexTextureCoordinate;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Main
void main()
{
    // Transforms Vertices Coordinates
    gl_Position = projection * view * model * vec4(position, 1.0f); 
    vertexFragmentPos = vec3(model * vec4(position, 1.0f));

    vertexColor = color;
    vertexTextureCoordinate = textureCoordinate;
}
);



// Fragment Shader Source Code
const GLchar* fragmentShaderSource = GLSL(440,
    in vec2 vertexTextureCoordinate;
    
    // Fragment Posiiton
    in vec3 vertexFragmentPos; 

    // Color Pyramid
    out vec4 fragmentColor;


uniform sampler2D uTexture;

// Main 
void main()
{
    // Vertex Texture 
    fragmentColor = texture(uTexture, vertexTextureCoordinate); 
    
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

    // Shader Program Creation
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;


    // Load Texture
    const char* texFilename = "brick.png"; // image name on file
    if (!UCreateTexture(texFilename, gTextureId))
    {
        cout << "Failed to Load Texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    
    glUseProgram(gProgramId);

    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);


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

    // Release Texture
    UDestroyTexture(gTextureId);

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

    // GLFW

    // Window Creation
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to Create GLFW Window" << std::endl;
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

    // Transformations Right-to-Left Order
    glm::mat4 modelPyramid = glm::translate(gPositionPyramid) * glm::scale(gScalePyramid) * rotationPyramid;

    // Creates a Orthographic Projection
    glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

    // Set Shader
    glUseProgram(gProgramId);

    // Retrieves and Passes Transform Matrices to the Shader Program
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPyramid));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


    const glm::vec3 cameraPosition = gCamera.Position;


    GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // Activate the VBOs
    glBindVertexArray(gMesh.vao);

    // Textures on Corresponding Texture Units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

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

     // Vertex Position     // Colors (r, g, b, a)       // Texture 
     0.5f,  0.5f, 0.0f,   0.0f,  1.0f,  0.0f,  1.0f,    1.0f, 0.0f,
    -0.5f,  0.5f, 0.0f,   0.0f,  1.0f,  0.0f,  1.0f,    1.0f, 1.0f,
     0.0f,  0.0f, 1.0f,   0.0f,  1.0f,  0.0f,  1.0f,    0.0f, 0.0f,

     0.0f,  0.0f, 1.0f,   1.0f,  0.0f,  0.0f,  1.0f,    1.0f, 1.0f,
     0.5f,  0.5f, 0.0f,   1.0f,  0.0f,  0.0f,  1.0f,    0.0f, 1.0f,
     0.5f, -0.5f, 0.0f,   1.0f,  0.0f,  0.0f,  1.0f,    0.0f, 0.0f,

     0.0f,  0.0f, 1.0f,   1.0f,  0.0f,  0.0f,  1.0f,    0.0f, 0.0f,
    -0.5f, -0.5f, 0.0f,   1.0f,  0.0f,  0.0f,  1.0f,    1.0f, 0.0f,
    -0.5f,  0.5f, 0.0f,   1.0f,  0.0f,  0.0f,  1.0f,    1.0f, 1.0f,

     0.0f,  0.0f, 1.0f,   0.0f,  1.0f,  0.0f,  1.0f,    1.0f, 1.0f,
    -0.5f, -0.5f, 0.0f,   0.0f,  1.0f,  0.0f,  1.0f,    0.0f, 1.0f,
     0.5f, -0.5f, 0.0f,   0.0f,  1.0f,  0.0f,  1.0f,    0.0f, 0.0f,

     0.5f,  0.5f, 0.0f,   0.0f,  0.0f,  1.0f,  1.0f,    1.0f, 1.0f,
    -0.5f, -0.5f, 0.0f,   0.0f,  0.0f,  1.0f,  1.0f,    0.0f, 0.0f,
    -0.5f,  0.5f, 0.0f,   0.0f,  0.0f,  1.0f,  1.0f,    1.0f, 0.0f,

     0.5f,  0.5f, 0.0f,   0.0f,  0.0f,  1.0f,  1.0f,    1.0f, 1.0f,
    -0.5f, -0.5f, 0.0f,   0.0f,  0.0f,  1.0f,  1.0f,    0.0f, 0.0f,
     0.5f, -0.5f, 0.0f,   0.0f,  0.0f,  1.0f,   1.0f,   0.0f, 1.0f
    };
    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;
    const GLuint floatsPerUV = 2;


    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerColor + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); 
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);

    // Activate the Buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); 

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
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
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
            cout << "Not Implemented to Handle image with " << channels << " Channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); 

        return true;
    }

    // Error Loading the image
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
