

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
                        1: Testure Wrapping Mode -- REPEAT
                        2: Testure Wrapping Mode -- MIRRORED REPEAT
                        3: Testure Wrapping Mode -- CLAMP TO EDGE
                        4: Testure Wrapping Mode -- CLAMP TO BORDER
                        }: Increase the Scale
                        {: Decrease the Scale
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

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
using namespace std; 

/* Shader program Macro */
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    // Windows Name
    const char* const WINDOW_TITLE = "Pyramid Light"; 

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

    //Texture
    GLuint gTextureId;

    // Triangle Mesh Data
    GLMesh gMesh;

    // Shader Program
    GLuint gProgramId;
    GLuint gLampProgramId;
    GLuint gFillProgramId;

    glm::vec2 gUVScale(5.0f, 5.0f);
    GLint gTexWrapMode = GL_REPEAT;

    // Camera
    Camera gCamera(glm::vec3(-1.0f, 0.0f, 6.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // Timing
    float gDeltaTime = 0.0f; 
    float gLastFrame = 0.0f;

    // Ortho
    bool ortho = false;

    /* Scale & Position */

    // Pyramid Scale and Posiiton
    glm::vec3 gPositionPyramid(0.0f, 0.0f, 0.0f);
    glm::vec3 gScalePyramid(2.0f);

    /* Light & Object Color */
    glm::vec3 gObjectColor(1.f, 0.2f, 0.0f);
    glm::vec3 gLightColor(0.0f, 1.0f, 0.0f);

    // Light Position and Scale
    glm::vec3 gLightPosition(1.5f, 0.5f, 3.0f);
    glm::vec3 gLightScale(1.0f);

    // Key Color, Position & Scale
    glm::vec3 gFillColor(0.0f, 1.0f, 0.0f);
    glm::vec3 gFillPosition(-4.0f, 0.5f, -3.0f);
    glm::vec3 gFillScale(1.0f);

    // Lamp Animation
    bool gIsLampOrbiting = true;

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

// Create and Destroy Texture
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);

// Render Graphics
void URender();

// Shader Program
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);

/* Cube Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,

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
const GLchar* fragmentShaderSource = GLSL(440,

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
    uniform sampler2D uTexture; // Useful when working with multiple textures
    uniform vec2 uvScale;

// Main
void main()
{
    /* Phon Lighting Model */
    
    // Set Ambient/Global Lighting Strenght
    float ambientStrength = 1.0f;

    // Generate Ambient Light Color
    vec3 ambient = ambientStrength * lightColor;

    /* Calculate Diffuse lighting */

    vec3 norm = normalize(vertexNormal); 

    // Calculate Distance Between Light & Fragment on the Pyramid
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos);

    // Calculate Diffuse Impact
    float impact = max(dot(norm, lightDirection), 0.0);

    // Generate Diffuse Light Color
    vec3 diffuse = impact * lightColor;

    /* Calculate Specular Lighting */

    // Specular Light Strength
    float specularIntensity = 0.2f; 

    // Specular Highlight Size
    float highlightSize = 8.0f; 

    // Calculate View Direction
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); 

    // Calculate Reflection Vector
    vec3 reflectDir = reflect(-lightDirection, norm);

    //Calculate Specular Component
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    vec3 specular = specularIntensity * specularComponent * lightColor;


    /* Calculate Key Lighting */

    // Set Ambient/Global Lighting Strength
    float fillAmbientStrength = 0.01f; 

    // Generate Ambient Light Color
    vec3 fillAmbient = fillAmbientStrength * fillColor; 

    /* Calculate Diffuse Lighting */

    // Calculate Distance Between Light Source & Fragment on the Pyramid
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

/* Fragment Shader Source Code */
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

    // Create Shader Program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    // Create Shader Program
    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId))
        return EXIT_FAILURE;

    // Create Shader Program
    if (!UCreateShaderProgram(fillVertexShaderSource, fillFragmentShaderSource, gFillProgramId))
        return EXIT_FAILURE;


    // Load Texture
    const char* texFilename = "brick.png";
    if (!UCreateTexture(texFilename, gTextureId))
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


    // Release Shader Program
    UDestroyShaderProgram(gProgramId);
    UDestroyShaderProgram(gLampProgramId);
    UDestroyShaderProgram(gFillProgramId);

    //Release Texture
    UDestroyTexture(gTextureId);

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

    // P Key to Reset Ortho Projection
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
        gCamera.Position = glm::vec3(-2.5f, 5.2f, -1.5f);
        gCamera.Pitch = -100.0f;
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    // O Key to Reset Ortho Projection
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) { //resets the ortho projection
        gCamera.Position = glm::vec3(0.0f, 0.0f, 5.0f);
        gCamera.Pitch = 0.0f;
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    // ESC Key to Finalize
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

    // 1 Key Texture Wrapping Mode: REPEAT
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && gTexWrapMode != GL_REPEAT)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_REPEAT;

        cout << "Current Texture Wrapping Mode: REPEAT" << endl;
    }

    // 2 Key Texture Wrapping Mode: MIRRORED REPEAT
    else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && gTexWrapMode != GL_MIRRORED_REPEAT)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_MIRRORED_REPEAT;

        cout << "Current Texture Wrapping Mode: MIRRORED REPEAT" << endl;
    }

    // 3 Key Texture Wrapping Mode: CLAMP TO EDGE
    else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && gTexWrapMode != GL_CLAMP_TO_EDGE)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_CLAMP_TO_EDGE;

        cout << "Current Texture Wrapping Mode: CLAMP TO EDGE" << endl;
    }

    // 4 Key Texture Wrapping Mode: CLAMP TO BORDER
    else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && gTexWrapMode != GL_CLAMP_TO_BORDER)
    {
        float color[] = { 1.0f, 0.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_CLAMP_TO_BORDER;

        cout << "Current Texture Wrapping Mode: CLAMP TO BORDER" << endl;
    }

    // } Key Increase the Scale
    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
    {
        gUVScale += 0.1f;
        cout << "Current scale (" << gUVScale[0] << ", " << gUVScale[1] << ")" << endl;
    }

    // { Decrease the Scale
    else if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
    {
        gUVScale -= 0.1f;
        cout << "Current scale (" << gUVScale[0] << ", " << gUVScale[1] << ")" << endl;
    }

    // Pause & Resume Lamp Orbiting


    static bool isLKeyDown = false;

    // L Key to Stop Orbiting
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !gIsLampOrbiting)
        gIsLampOrbiting = true;

    // K Key to Resume Orbiting
    else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && gIsLampOrbiting)
        gIsLampOrbiting = false;

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
        cout << "Unhandled Mouse Button Event" << endl;
        break;
    }
}

// URender Frame Function
void URender()
{
    // Lamp Orbits Around the Origin
    const float angularVelocity = glm::radians(45.0f);
    if (gIsLampOrbiting)
    {
        glm::vec4 newPosition = glm::rotate(angularVelocity * gDeltaTime, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(gLightPosition, 1.0f);
        gLightPosition.x = newPosition.x;
        gLightPosition.y = newPosition.y;
        gLightPosition.z = newPosition.z;
    }

    // Enable Z-Depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // Set the shader to be used
    glUseProgram(gProgramId);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

    // Model matrix: transformations are applied right-to-left order
    glm::mat4 model = glm::translate(gPositionPyramid) * glm::scale(gScalePyramid); //* rotationPyramid;


    // Creates a perspective projection
    glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

    // camera/view transformation
    glm::mat4 view;// = gCamera.GetViewMatrix();


    // Setup views and projections
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


    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
    GLint objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    GLint lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    GLint lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    GLint fillColorLoc = glGetUniformLocation(gProgramId, "fillColor");
    GLint fillPositionLoc = glGetUniformLocation(gProgramId, "fillPos");
    GLint viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    glUniform3f(fillColorLoc, gFillColor.r, gFillColor.g, gFillColor.b);
    glUniform3f(fillPositionLoc, gFillPosition.x, gFillPosition.y, gFillPosition.z);

    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);


    GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices); // Draws the triangle

    // LAMP: draw lamp
    //----------------
    glUseProgram(gLampProgramId);

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gLightPosition) * glm::scale(gLightScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);


    //Draw fill
    //----------------
    glUseProgram(gFillProgramId);

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gFillPosition) * glm::scale(gFillScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gFillProgramId, "model");
    viewLoc = glGetUniformLocation(gFillProgramId, "view");
    projLoc = glGetUniformLocation(gFillProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);


    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
    glUseProgram(0);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}


// UCreateMesh Function
void UCreateMesh(GLMesh& mesh)
{
    // Vertex data
    GLfloat verts[] = {

    // Vertices                // Normals             // Texture 
    -0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,      0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,      1.0f, 0.0f,
     0.0f,  0.5f,  0.0f,     0.0f,  0.0f, -1.0f,      0.5f, 1.0f,

    -0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,      0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,      1.0f, 0.0f,
     0.0f,  0.5f,  0.0f,     0.0f,  0.0f,  1.0f,      0.5f, 1.0f,

    -0.5f, -0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,      0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,      0.0f, 0.0f,
     0.0f,  0.5f,  0.0f,    -1.0f,  0.0f,  0.0f,      0.5f, 1.0f,

     0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,      0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,     1.0f,  0.0f,  0.0f,      0.0f, 0.0f,
     0.0f,  0.5f,  0.0f,     1.0f,  0.0f,  0.0f,      0.5f, 1.0f,

     0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,      1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,      1.0f, 0.0f,
     0.0f,  0.5f,  0.0f,     0.0f, -1.0f,  0.0f,      0.5f, 1.0f,

    -0.5f, -0.5f,  0.5f,     0.0f,  1.0f,  0.0f,      0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,     0.0f,  1.0f,  0.0f,      0.0f, 1.0f,
     0.0f,  0.5f,  0.0f,     0.0f,  1.0f,  0.0f,      0.5f, 1.0f




    };
    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;


    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);

    // Activates the Buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

    // Send Vertex and Coordinations
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); 

    // Stride between Vertex and Coordinations
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

// Destroy Mesh Function
void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
}


/* Generate & Load Texture */

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

    // Link Shader Program
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