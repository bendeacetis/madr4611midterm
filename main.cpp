#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <fstream>
#include <sstream>

//#define GLT_IMPLEMENTATION
//#include "gltext.h"

#define M_PI 3.1415926535897932384626433832795

//window dimensions
const GLint WIDTH = 800, HEIGHT = 600;

float currentScore; //score user gets displayed after each run
float highScore;    //score displayed for the highscores during each run

//shaders
GLuint m_vertexShaderID = 0;
GLuint m_fragmentShaderID = 0;
GLuint m_programID = 0;

//variables Uniform
GLint m_uniformTransparencyID = -1;
GLint m_uniformProyectionID = -1;
GLint m_uniformViewID = -1;
GLint m_uniformModelID = -1;

//attributes
GLint m_inColorID = -1;
GLint m_inVertexID = -1;

void DebugLog(const char* _log)
{
    std::cout << _log << std::endl;
}

void DebugLog(std::string _log)
{
    std::cout << _log << std::endl;
}

/// <summary>
/// Create the window and the context for OpenGL
/// </summary>
/// <param name="_title"></param>
/// <param name="_width"></param>
/// <param name="_height"></param>
/// <returns></returns>

GLFWwindow * InitWindowContext(const char * _title, int _width, int _height)
{
    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow * _window = glfwCreateWindow(_width, _height, _title, NULL, NULL);
    if (!_window)
    {
        return NULL;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(_window);

    return _window;
}

// structure to represent a fish
struct Fish {
    float x, y; //position
    float speed; //horizontal speed
    bool visible; //visibility
};

std::vector<Fish> fishes; //vector to store fishes
int score = 0; //score of fish clicked
int numFishLeft = 0;

//function to initialize fishes
void initFishes() {
    srand(time(nullptr)); //set up randomization
    
    int numFishes = 5; //number of fishes
    numFishLeft = numFishes;
    float minY = -0.8f, maxY = 0.8f; //range for fish's y position
    
    for (int i = 0; i < numFishes; ++i) {
        Fish fish;
        fish.x = -1.0f + static_cast<float>(rand()) / (RAND_MAX / 2.0f); //random x position
        fish.y = minY + static_cast<float>(rand()) / (RAND_MAX / (maxY - minY)); //random y position
        fish.speed = 0.000005f + static_cast<float>(rand()) / (RAND_MAX / 0.01f); //random speed
        fish.visible = true;
        fishes.push_back(fish);
    }
}

void drawFish(float x, float y) {
    //body
    glBegin(GL_TRIANGLES);
    glColor3f(0.8f, 0.33f, 0.0f); // orange
    glVertex3f(x, y, 0.0f);
    glVertex3f(x + 0.1f, y - 0.05f, 0.0f);
    glVertex3f(x + 0.1f, y + 0.05f, 0.0f);
    glEnd();
    
    //head
    glBegin(GL_POLYGON);                        //middle circle
    double radius = 0.055;
    double ori_x = x+0.1f;                         // the origin or center of circle
    double ori_y = y;
    for (int i = 0; i <= 300; i++) {
        double angle = 2 * M_PI * i / 300;
        double x = cos(angle) * radius;
        double y = sin(angle) * radius;
        glVertex2d(ori_x + x, ori_y + y);
    }
    glEnd();
    
    //tail
    glColor3f(1.0f, 0.65f, 0.0f); // Orange color
    glBegin(GL_TRIANGLES);
    glVertex3f(x - 0.0f, y, 0.0f); // Head top
    glVertex3f(x - 0.1f, y + 0.05f, 0.0f); // Head left
    glVertex3f(x - 0.1f, y - 0.05f, 0.0f); // Head right
    glEnd();
}

//function that draws the fishing line following the cursor
void drawFishingLine(float x, float y){
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 0.0f); //black
    glVertex2f(x,HEIGHT);  //starts at top of frame
    glVertex2d(x,y);    //ends at cursor position
    glEnd();
}

//function that draws the sand at the base.
void drawSand(){
    glBegin(GL_QUADS);
    glColor3f(0.96f, 0.87f, 0.7f); // Sand color
    glVertex2f(-1.0f, -1.0f); // Bottom-left corner
    glVertex2f(1.0f, -1.0f);  // Bottom-right corner
    glVertex2f(1.0f, -0.8f);  // Top-right corner
    glVertex2f(-1.0f, -0.8f); // Top-left corner
    glEnd();
}

// function to handle mouse clicks
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        
        //convert screen coordinates to OpenGL coordinates
        float x = (float)xpos / WIDTH * 2.0f - 1.0f;
        float y = 1.0f - (float)ypos / HEIGHT * 2.0f;
        
        //check if clicked on any fish
        for (Fish& fish : fishes) {
            if (fish.visible) {
                float centerX = fish.x + 0.05f; // X coordinate of the circle center (adjusted based on fish width)
                float centerY = fish.y; // Y coordinate of the circle center
                float radius = 0.05f; //radius of the circle
                
                //calculate the distance between the click point and the circle center
                float distance = sqrt((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY));
                
                //if the distance is less than or equal to the radius, the click is inside the circle
                if (distance <= radius) {
                    fish.visible = false; //make fish disappear
                    numFishLeft--;
                    score++; //increment score
                    break;
                }
            }
        }
    }
}



/// <summary>
/// Load the shader from file. We could add the string directly but w/e
/// </summary>
/// <param name="_fileName"></param>
/// <param name="_type"></param>
/// <returns></returns>
GLuint LoadShader(const char * _fileName, GLenum _type) {
    //open the shader file
    std::ifstream file(_fileName);
    
    if (!file) {
        DebugLog("Shader file not found " + std::string(_fileName));
        return 0;
    }

    //read the shader source code
    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    const char* sourcePtr = source.c_str();
    GLint sourceLen = static_cast<GLint>(source.length());

    //create and compile the shader
    GLuint shader = glCreateShader(_type);
    glShaderSource(shader, 1, &sourcePtr, &sourceLen);
    glCompileShader(shader);

    //check for compilation errors
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
        GLint logLen;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> logString(logLen);
        glGetShaderInfoLog(shader, logLen, NULL, logString.data());
        std::cout << "Shader compilation error: " << logString.data() << std::endl;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}




/// <summary>
/// Initialization of the shaders
/// </summary>
/// <returns></returns>
bool InitializeShaders() {
    //compile vertex and fragment shaders
    m_vertexShaderID = LoadShader("/Users/bendeacetis/Desktop/Graphics/Test1 copy/Test1/vshader.glsl", GL_VERTEX_SHADER);
    m_fragmentShaderID = LoadShader("/Users/bendeacetis/Desktop/Graphics/Test1 copy/Test1/fshader.glsl", GL_FRAGMENT_SHADER);
    
    if (m_vertexShaderID == 0 || m_fragmentShaderID == 0)
        return false;

    //link shaders to the program
    m_programID = glCreateProgram();
    glAttachShader(m_programID, m_vertexShaderID);
    glAttachShader(m_programID, m_fragmentShaderID);

    glBindAttribLocation(m_programID, 0, "inVertex");
    glBindAttribLocation(m_programID, 1, "inColor");

    glLinkProgram(m_programID);

    //check for linking errors
    GLint linked;
    glGetProgramiv(m_programID, GL_LINK_STATUS, &linked);
    
    if (!linked) {
        GLint logLen;
        glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> logString(logLen);
        glGetProgramInfoLog(m_programID, logLen, NULL, logString.data());
        std::cout << "Shader program linking error: " << logString.data() << std::endl;
        glDeleteProgram(m_programID);
        m_programID = 0;
        return false;
    }

    //get uniform and attribute locations
    m_uniformTransparencyID = glGetUniformLocation(m_programID, "transparency");
    m_uniformProyectionID = glGetUniformLocation(m_programID, "proy");
    m_uniformViewID = glGetUniformLocation(m_programID, "view");
    m_uniformModelID = glGetUniformLocation(m_programID, "rot");
    
    m_inColorID = glGetAttribLocation(m_programID, "inColor");
    m_inVertexID = glGetAttribLocation(m_programID, "inVertex");

    return true;
}



/// <summary>
/// Free OpenGL libraries
/// </summary>
void FreeLibraries()
{
    glfwTerminate();
}

/// <summary>
/// Free buffers, shaders, program and ofc, OpenGL context
/// </summary>
/// <param name="_loadedShaders"></param>
void FreeResources(bool _loadedShaders)
{
    if (_loadedShaders)
    {
        glDetachShader(m_programID, m_vertexShaderID);
        glDetachShader(m_programID, m_fragmentShaderID);
        glDeleteShader(m_vertexShaderID);
        glDeleteShader(m_fragmentShaderID);
        glDeleteProgram(m_programID);
    }

    FreeLibraries();
}

//function that draws a button
void drawButton(){
    glBegin(GL_QUADS);
    glColor3f(0.43f, 0.49f, 0.96f); // Red color
    glVertex2f(-0.25f, -0.5f); // Bottom-left corner
    glVertex2f(0.25f, -0.5f); // Bottom-right corner
    glVertex2f(0.25f, -0.4f); // Top-right corner
    glVertex2f(-0.25f, -0.4f); // Top-left corner
    glEnd();
}
//function that draws a button
void drawButton2(){
    glBegin(GL_QUADS);
    glColor3f(0.96f, 0.43f, 0.47f); // Red color
    glVertex2f(-0.25f, -0.7f); // Bottom-left corner
    glVertex2f(0.25f, -0.7f); // Bottom-right corner
    glVertex2f(0.25f, -0.6f); // Top-right corner
    glVertex2f(-0.25f, -0.6f); // Top-left corner
    glEnd();
}

bool mouseOnButton(double x, double y) {
    //coordinates of the button
    float buttonLeft = -0.25f;
    float buttonRight = 0.25f;
    float buttonTop = -0.4f;
    float buttonBottom = -0.5f;

    //checks if mouse coordinates are within the button area
    if (x >= buttonLeft && x <= buttonRight && y >= buttonBottom && y <= buttonTop) {
        return true;
    }
    return false;
}

bool mouseOnButton2(double x, double y) {
    //coordinates of the button
    float buttonLeft = -0.25f;
    float buttonRight = 0.25f;
    float buttonTop = -0.6f;
    float buttonBottom = -0.7f;

    //check if mouse coordinates are within the button area
    if (x >= buttonLeft && x <= buttonRight && y >= buttonBottom && y <= buttonTop) {
        return true;
    }
    return false;
}

void startGame(GLFWwindow* window){
    while(!glfwWindowShouldClose(window)){
        
        //clear the screen
        glClearColor(0.84f, 0.99f, 0.99f, 0.0f); //color of background
        glClear(GL_COLOR_BUFFER_BIT);
        
        drawButton();
        
        //swap buffers
        glfwSwapBuffers(window);

        //poll for events
        glfwPollEvents();
        
        //check for mouse button click
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            float x = 2.0f * xpos / WIDTH - 1.0f;
            float y = 1.0f - (float)ypos / HEIGHT * 2.0f;

            //check if the click is on the button
            if (mouseOnButton(x, y)) {
                break;
            }
        }
    }
}

void runGame(GLFWwindow* window){
    while(!glfwWindowShouldClose(window)){
        
        //clear the screen
        glClearColor(0.0f, 0.65f, 0.8f, 0.0f); //color of background
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Render the sand with shader
        glUseProgram(m_programID);

        // Draw the sand
        drawSand();

        // Deactivate the shader program
        glUseProgram(0);

        //draw fishes
        for (const Fish& fish : fishes) {
            if (fish.visible) {
                drawFish(fish.x, fish.y);
            }
        }
        
        //draw fishing line
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        float x = 2.0f * xpos / WIDTH - 1.0f;
        float y = 1.0f - (float)ypos / HEIGHT * 2.0f;
        drawFishingLine(x,y);
        
        // move fishes
        for (Fish& fish : fishes) {
            if (fish.visible) {
                fish.x += fish.speed;
                if (fish.x > 1.0f) {
                    fish.x = -1.0f;
                }
            }
        }
        
        if(numFishLeft == 0){
            //clear the screen
            glClearColor(0.84f, 0.99f, 0.99f, 0.0f); //color of background
            glClear(GL_COLOR_BUFFER_BIT);
            
            //draw the button options
            drawButton();
            drawButton2();
            
            //print the score and highscore
            
            //check for mouse button click
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                float x = 2.0f * xpos / WIDTH - 1.0f;
                float y = 1.0f - (float)ypos / HEIGHT * 2.0f;
                
                //check if clicking on restart
                if (mouseOnButton(x,y)) {
                    for (Fish& fish : fishes) {
                        fish.visible = true;
                    }
                    //reset game
                    numFishLeft = 5;
                    
                    // swap buffers
                    glfwSwapBuffers(window);

                    // poll for events
                    glfwPollEvents();
                    
                    continue;
                }
                //check clicking on end button
                else if (mouseOnButton2(x,y)){
                    break;
                }
            }
            
        }

        // swap buffers
        glfwSwapBuffers(window);

        // poll for events
        glfwPollEvents();
    }
}


int main() {
    //initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    //create GLFW windoqw
    GLFWwindow * window = InitWindowContext("Fish Game", WIDTH, HEIGHT);
    
    //check if window is null
    if (window == NULL)
    {
        glfwTerminate();
        return -1;
    }
    
    //load shaders
    bool loadedShaders = InitializeShaders();
    
    //check if shaders were loaded successfully
    if (!loadedShaders) {
        std::cerr << "Failed to load shaders" << std::endl;
        return -1;
    }

    //make the OpenGL context current
    glfwMakeContextCurrent(window);

    //set up the viewport
    glViewport(0, 0, WIDTH, HEIGHT);
    
    //initialize fishes
    initFishes();
    
    //set mouse button callback function
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    //starting screen
    startGame(window);
    
    //main game loop
    runGame(window);

    FreeResources(loadedShaders);

    return 0;
}
