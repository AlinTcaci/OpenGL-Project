#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"

#include <iostream>

int glWindowWidth = 1100;
int glWindowHeight = 800;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 8 * 2048;
const unsigned int SHADOW_HEIGHT = 8 * 2048;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

gps::Camera myCamera(
				glm::vec3(22.24f, 12.36f, 11.72f),
				glm::vec3(-0.85f, -0.34f, -0.39f),
				glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.1f;

bool pressedKeys[1024];
GLfloat lightAngle;

gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::Model3D staticScene;
gps::Model3D walle;
gps::Model3D drone;
gps::Model3D bodyWindmill;
gps::Model3D headWindmill;

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

GLuint fogDensityLoc;
GLfloat fogDensity;

bool showDepthMap;

// point light
int pointinit = 0;
glm::vec3 lightPointPos; // pe casa + fan
GLuint lightPointPosLoc;

// scene presentation
bool startPresentation = false;
bool updateCameraAfterPresentation = false;
float presentationAngle;

//walle
float walleRotationAngle = 0.0f; // Angle in degrees
glm::vec3 wallePosition = glm::vec3(0.0f, 0.65f, 5.0f); // Initial position
float moveSpeed = 0.01f; // Speed of movement

//drone
float droneRotationAngle = 0.0f; // Angle in degrees for the drone's rotation
glm::vec3 droneRotationCenter = glm::vec3(0.0f, 9.5f, 3.0f); // Center of rotation
float droneOrbitRadius = 10.0f; // Radius of the orbit
float droneOrbitSpeed = 0.5f; // Speed of orbit rotation

//windmill
float headWindmillRotationAngle = 0.0f;

//nightMode
bool isNightMode = false;

GLenum glCheckError_(const char *file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
    fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);

    glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

    myCustomShader.useShaderProgram();

    // set projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    //send matrix data to shader
    GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    lightShader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // set Viewport transform
    glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;
    
    if (key == GLFW_KEY_O && action == GLFW_PRESS) {
        isNightMode = true;
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        isNightMode = false;
    }

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
    
    // Toggle mouse cursor when CTRL key is pressed
    if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL) {
        if (action == GLFW_PRESS) {
            int cursorMode = glfwGetInputMode(window, GLFW_CURSOR);
            if (cursorMode == GLFW_CURSOR_DISABLED) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
        }
    }
}

// mouseCallback variables
bool firstMouse = true;
double lastX = 0.0, lastY = 0.0;
float yaw = 0.0f, pitch = 0.0f;

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    double sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    myCamera.rotate(pitch, yaw);
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}

void processMovement()
{
    
    headWindmillRotationAngle += 0.5f; // Adjust the value to control speed
    if (headWindmillRotationAngle > 360.0f)
        headWindmillRotationAngle -= 360.0f;
    
    droneRotationAngle += droneOrbitSpeed;
    if (droneRotationAngle >= 360.0f) {
        droneRotationAngle -= 360.0f;
    }
    
    if (pressedKeys[GLFW_KEY_UP]) {
       wallePosition.z -= moveSpeed; // Move walle forward
    }
    
    if (pressedKeys[GLFW_KEY_DOWN]) {
        wallePosition.z += moveSpeed; // Move wallebackward
    }
    
    if (pressedKeys[GLFW_KEY_LEFT]) {
        wallePosition.x -= moveSpeed; // Move walle left
    }
    
    if (pressedKeys[GLFW_KEY_RIGHT]) {
        wallePosition.x += moveSpeed; // Move walle right
    }
    
	if (pressedKeys[GLFW_KEY_Q]) {
        walleRotationAngle += 1.0f; //rotate walle
	}

	if (pressedKeys[GLFW_KEY_E]) {
        walleRotationAngle -= 1.0f; //rotate walle
	}

	if (pressedKeys[GLFW_KEY_J]) { //rotate light cube
		lightAngle -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_L]) { //rotate light cube
		lightAngle += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_W]) { //move camera forward
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S]) { //move camera backward
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A]) { //move camera to left
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) { //move camera to right
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}
    
    if (pressedKeys[GLFW_KEY_Z]) { //move camera up
        myCamera.move(gps::MOVE_UP, cameraSpeed);
    }
    
    if (pressedKeys[GLFW_KEY_X]) { //move camera down
        myCamera.move(gps::MOVE_DOWN, cameraSpeed);

    }
    
    if(pressedKeys[GLFW_KEY_1]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    
    if(pressedKeys[GLFW_KEY_2]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }
    
    if(pressedKeys[GLFW_KEY_3]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    
    if(pressedKeys[GLFW_KEY_F]) { // increase fog density
        fogDensity += 0.002f;
        if(fogDensity >= 0.3f){
            fogDensity = 0.3f;
        }
        myCustomShader.useShaderProgram();
        glUniform1fv(fogDensityLoc, 1, &fogDensity);
    }
    
    if(pressedKeys[GLFW_KEY_G]) { // decrease fog density
        fogDensity -= 0.002f;
        if(fogDensity <= 0.0f){
            fogDensity = 0.0f;
        }
        myCustomShader.useShaderProgram();
        glUniform1fv(fogDensityLoc, 1, &fogDensity);
    }
    
    //start presentation
    if (pressedKeys[GLFW_KEY_9]) {
        startPresentation = true;
    }

    // stop preview
    if (pressedKeys[GLFW_KEY_0]) {
        startPresentation = false;
        updateCameraAfterPresentation = true;
    }
    
    // start pointlight
    if (pressedKeys[GLFW_KEY_5]) {
        myCustomShader.useShaderProgram();
        pointinit = 1;
        glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "pointinit"), pointinit);
    }

    // stop pointlight
    if (pressedKeys[GLFW_KEY_6]) {
        myCustomShader.useShaderProgram();
        pointinit = 0;
        glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "pointinit"), pointinit);
    }
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    //window scaling for HiDPI displays
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    //for sRBG framebuffer
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    //for antialising
    glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Project ", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	//glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

#if not defined (__APPLE__)
    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    glewInit();
#endif

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initSkyBox(){
    std::vector<const GLchar*> faces;
    faces.push_back("skybox/arrakisday_rt.tga");
    faces.push_back("skybox/arrakisday_lf.tga");
    faces.push_back("skybox/arrakisday_up.tga");
    faces.push_back("skybox/arrakisday_dn.tga");
    faces.push_back("skybox/arrakisday_bk.tga");
    faces.push_back("skybox/arrakisday_ft.tga");
    
    mySkyBox.Load(faces);
}

void initObjects() {
	lightCube.LoadModel("objects/cube/cube.obj");
    screenQuad.LoadModel("objects/quad/quad.obj");
    staticScene.LoadModel("objects/staticScene/staticSceneFinal.obj");
    walle.LoadModel("objects/walle/walle.obj");
    drone.LoadModel("objects/drone/drone.obj");
    bodyWindmill.LoadModel("objects/windmill/bodyWindmill.obj");
    headWindmill.LoadModel("objects/windmill/headWindmill2.obj");
}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	depthMapShader.loadShader("shaders/shadow.vert", "shaders/shadow.frag");
	depthMapShader.useShaderProgram();
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();
}

void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //set the light direction (direction towards the light)
    lightDir = glm::vec3(35.0f, 25.0f, 0.0f);
    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
    
    //point light
    lightPointPos = glm::vec3(-8.9984f, 3.0707f, -4.8560f);
    lightPointPosLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPointPos");
    glUniform3fv(lightPointPosLoc, 1, glm::value_ptr(lightPointPos));

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    
    //fog density
    fogDensityLoc = glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity");
    glUniform1fv(fogDensityLoc, 1, &fogDensity);
}

void initFBO() {
	glGenFramebuffers(1, &shadowMapFBO);
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT,0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 lightView = glm::lookAt(glm::vec3(lightRotation * glm::vec4(lightDir,1.0f)), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const GLfloat near_plane = 0.1f, far_plane = 300.0f;
    glm::mat4 lightProjection = glm::ortho(-35.0f, 35.0f, -35.0f, 35.0f, near_plane, far_plane);
    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
    return lightSpaceTrMatrix;
}

void drawObjects(gps::Shader shader, bool depthPass) {
		
	shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(16.44f, 5.4f,-6.0f));
    model = glm::scale(model, glm::vec3(0.6));
    model = glm::rotate(model, glm::radians(headWindmillRotationAngle), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around Z-axis
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    headWindmill.Draw(shader);
    
    model = glm::translate(glm::mat4(1.0f), glm::vec3(17.0f, 0.65f,-6.0f));
    model = glm::scale(model, glm::vec3(0.6));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    bodyWindmill.Draw(shader);
    
    model = glm::translate(glm::mat4(1.0f), wallePosition);
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(walleRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    walle.Draw(shader);
    
    // Calculate drone's position in its orbit
    glm::vec3 dronePosition;
    dronePosition.x = droneRotationCenter.x + droneOrbitRadius * cos(glm::radians(droneRotationAngle));
    dronePosition.z = droneRotationCenter.z + droneOrbitRadius * sin(glm::radians(droneRotationAngle));
    dronePosition.y = droneRotationCenter.y; // Adjust if you want vertical movement
    // Set drone's model matrix
    model = glm::translate(glm::mat4(1.0f), dronePosition);
    model = glm::scale(model, glm::vec3(0.5f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    drone.Draw(shader);

    
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    if(!depthPass)
        mySkyBox.Draw(skyboxShader, view, projection);
    
    
    staticScene.Draw(shader);
}

void renderScene() {
	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0,0,SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	drawObjects(depthMapShader, 1);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// render depth map on screen - toggled with the M key
	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {
		// final scene rendering pass (with shadows)
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();
        
        // Update the night mode uniform
        GLint nightModeLoc = glGetUniformLocation(myCustomShader.shaderProgram, "isNightMode");
        glUniform1i(nightModeLoc, isNightMode);

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
				
		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		drawObjects(myCustomShader, false);

		//draw a white cube around the light
		lightShader.useShaderProgram();

		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

        model = lightRotation;
        model = glm::translate(model, 1.0f * lightDir);
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        lightCube.Draw(lightShader);
	}
}

void cleanup() {
	glDeleteTextures(1,& depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

void scenePresentation() {
    if (startPresentation) {
        presentationAngle += 0.2f;
        myCamera.scenePresentation(presentationAngle);
    } else if (updateCameraAfterPresentation) {
        // Reset the initial yaw and pitch based on the current camera front direction
        glm::vec3 front = myCamera.cameraFrontDirection;
        yaw = glm::degrees(atan2(front.z, front.x));
        pitch = glm::degrees(asin(front.y));

        // Get the current mouse position
        double mouseX, mouseY;
        glfwGetCursorPos(glWindow, &mouseX, &mouseY);
        lastX = mouseX;
        lastY = mouseY;

        updateCameraAfterPresentation = false;
    }
}

int main(int argc, const char * argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();
    initSkyBox();
    
    //Calculate initial yaw and pitch based on the camera's front direction
    glm::vec3 front = myCamera.cameraFrontDirection;
    yaw = glm::degrees(atan2(front.z, front.x));
    pitch = glm::degrees(asin(front.y));

    // Get the current mouse position
    double mouseX, mouseY;
    glfwGetCursorPos(glWindow, &mouseX, &mouseY);
    lastX = mouseX;
    lastY = mouseY;

	glCheckError();

	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
        scenePresentation();
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}
