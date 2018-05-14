//Use core profile. This forces modern mode. Immediate mode is the other one that doesn't save the set of vertex and attribute data in the GPU.
//For use of extensions, want to use if(GL_ARB_EXTENSION NAME) to check if the extension is included. The else does it the old way. Will have to write twice, then, so be sure the extension is actually THAT cool and THAT big of an upgrade in performance or graphical quality.
//Remember GL is a "state machine," and once you set a context variable to change the way something's done, it stays that way until you say otherwise.
//OpenGL is in C, not C++, which is what you're using for the program's body. Objects are done with structs. Creation is done with a gen object function, where you're binding it to an int objectID. Then you bind the object to the target location. Then you set the options and set the ID in the target location to 0.
//Remember to always change the project setup to link glew, glfw, and glm's compiled .lib files. Their folders are in the repos directory here.
#pragma once
#define GLEW_STATIC //define that we're using the static version of GLEW

/*Important steps: Include GLEW, GLM, and GLFW. Includes must be in this order.*/
#include <glew.h>					//Extension wrangler library. Saves me the trouble of registering every function.
#include <gl/gl.h>					//GL functions.
#include <glfw3.h>					//GL UI functions.
#include <glm.hpp>					//For mat4.
#include <gtc/matrix_transform.hpp>	//Transformations on model, perspective and viewport.
#include <gtc/type_ptr.hpp>
#include <iostream>					//Outputting debug info.
#include <vector>					//Conversion function
#include <fstream>					//Reading 
#include <iomanip>
#include "cimg.h"					//Image processing library. See http://cimg.eu/ for details.
#include "file_read_error.h"		//An exception I defined. Thrown when shader source info can't be read.
#include <cassert>
//Additional dependencies:
//LibPNG (to add PNG support to Cimg) (requires LibPNG.dll in same directory as program, too)
//Zlib (via LibPNG)

namespace proj1 { //My namespace for globals
	GLenum GL_DRAW_STATE = GL_TRIANGLE_STRIP;
	GLbyte ROTATE_X = 0;
	GLbyte ROTATE_Y = 0;
	GLbyte ZOOM = 0;
	GLdouble LAST_CURSOR_Y = -1;//Only care about cursor Ypos for zoom
	GLbyte ROTATE_CAM_Y = 0;
	GLbyte ROTATE_CAM_X = 0;
}

//Function that resizes when user attempts to do so. Registered in main method.
void framebuffer_size_callback(GLFWwindow* window, int width, int height)//Adjust width and height of framebuffer size.
{
	glViewport(0, 0, width, height);
}



void mousemove_callback(GLFWwindow * window, double xpos, double ypos) {
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)== GLFW_PRESS) {
		if (proj1::LAST_CURSOR_Y > -1) {
			if (proj1::LAST_CURSOR_Y < ypos){//user dragged mouse up
				proj1::ZOOM = 1;
			}
			else if (proj1::LAST_CURSOR_Y > ypos) {//user dragged mouse down
				proj1::ZOOM = -1;
			}
		}
		proj1::LAST_CURSOR_Y = ypos;
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
		proj1::ZOOM = 0;
		proj1::LAST_CURSOR_Y = -1;
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;
			//rotate model
		case GLFW_KEY_LEFT:
			proj1::ROTATE_Y = -1;//Rotate counterclockwise about Y axis
			break;
		case GLFW_KEY_RIGHT:
			proj1::ROTATE_Y = 1;//Rotate clockwise about Y axis
			break;
		case GLFW_KEY_UP:
			proj1::ROTATE_X = -1;//Rotate down about X axis
			break;
		case GLFW_KEY_DOWN:
			proj1::ROTATE_X = 1;//Rotate up about X axis
			break;
		case GLFW_KEY_Z:
			proj1::ROTATE_CAM_Y = -1;//Rotate counterclockwise about Y axis
			break;
		case GLFW_KEY_C:
			proj1::ROTATE_CAM_Y = 1;//Rotate clockwise about Y axis
			break;
		case GLFW_KEY_D:
			proj1::ROTATE_CAM_X = -1;//Rotate down about X axis
			break;
		case GLFW_KEY_X:
			proj1::ROTATE_CAM_X = 1;//Rotate up about X axis
			break;







		case GLFW_KEY_W://change render mode to lines if not set to lines
			proj1::GL_DRAW_STATE = GL_LINE_LOOP; //Series of connected lines. "GL_LINES" restricts it to only independent line segments, and "GL_LINE_STRIP" doesn't close the loop, which isn't what I want here.
			break;
		case GLFW_KEY_T://change render mode to triangles, if not already set to that
			proj1::GL_DRAW_STATE = GL_TRIANGLE_STRIP;
			break;
		case GLFW_KEY_P://change render mode to points, if not already set to that
			proj1::GL_DRAW_STATE = GL_POINTS;
			break;
		}
	}
}

const GLchar const* read_file(std::string location) {

	std::ifstream content_buf(location, std::ios::in);
	int pos = 0;
	if (content_buf.is_open()) {
		std::string contents((std::istreambuf_iterator<char>(content_buf)), std::istreambuf_iterator<char>());//use stream iterator to iterate over contents until EOF
		GLchar * chars = new GLchar[contents.size() + 1];//make room for null terminator
		const char* const cstr = contents.c_str();
		for (int i = 0; i < contents.size(); ++i) {//copy over
			chars[i] = cstr[i];
		}
		chars[contents.size()] = '\0';//null terminate final position
		return chars;
	}
	else {
		throw file_read_error(location);
	}
}
std::ostream& operator <<(std::ostream& out, const glm::mat4 &mat) {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			out << std::setw(2) << mat[i][j] << " ";
		}
		out << std::endl;
	}
	out << std::endl;
	return out;
}
//Converts each pixel into an uncolored vertex, and returns them in a VBO.
std::vector<float>* image_to_vertices(cimg_library::CImg<float>  const& in) {//Pass by constant reference for efficiency
	std::vector<float>* img_vert = new std::vector<float>();//For every pixel we also to allocate depth. Yeah don't worry it's on the heap
	std::ofstream log;
	log.open(".\\vertex_log.txt");
	for (float x = 0; x < in.width(); ++x) {
		for (float y = 0; y < in.height(); ++y) {
			img_vert->push_back(x);
			img_vert->push_back(y);
			img_vert->push_back(in(x, y, 0, 0)); //make "whiteness" level into depth here
			log << x << " " << y << " " << in(x, y, 0, 0) << std::endl;
		}
	}
	log.close();
	return img_vert;
}

//Ignore what Learn OpenGL says about the function prototypes; do it like you're using GLAD. All you have to do is invoke glew_init()
//GLEW does that for me. It's like GLAD, but it's just what we're using.
int main(char argc, char** argv) {
	const int width = 800, height = 800;//X and Y dimensions of the window.
	const int min_d = 256, max_d = 1024;//Minimum and maximum image sizes
	const float fov_angle = 45.0f;
	float zoomfactor = 1.0f;
	const float ZOOM_SPEED = 0.003f;
	const float CAMERA_ROTATE_SPEED = 0.1f;
	float MAP_TRANSLATE_SPEED = 0.2f;

	std::string vs_loc = ".\\VertexShader.vert";
	std::string fs_loc = ".\\FragmentShader.frag";
	std::string hmap_loc = ".\\Heightmap.png";

	/*GLFW setup*/
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//configure openGL; tell it we're using GL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//Use core profile
	glfwWindowHint(GL_DEPTH_BITS, 24);//Ask for a 24 bit z-buffer

	GLFWwindow* window = glfwCreateWindow(width, height, "Test", NULL, NULL);
	if (window == nullptr) {
		std::cerr << "Failed to intialize GL window for unknown reason." << std::endl;
		exit(-1);
	}
	
	glfwMakeContextCurrent(window);//Set window as active context
	std::cout << "Created window context." << std::endl;
	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LESS);//Set the depth function to render primitives with lower Z value first, for hidden surface removal purposes.


	/*GLEW setup*/
	//Set GLEW to experimental mode. Must come after we define an active context.
	glewExperimental = GL_TRUE;//sets a value in glew.cpp to true. Pretend it's injected above here and that's what we're setting true. Logically though that must mean it's a global. Smells like codesmell. Whatever, I guess it's convenient.
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		std::cerr << "Fatal error: GLEW could not load; extensions therefore will not function. Aborting. Reason given: " << glewGetErrorString(err) << std::endl;
		exit(1);
	}
	else {
		std::cout << "Extension wrangler loaded." << std::endl;
	}
	glViewport(0, 0, width, height);
	glfwGetFramebufferSize(window, const_cast<int*>(&width), const_cast<int*>(&height));
	/*Set up callback functions*/
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mousemove_callback);
	/*Compile vertex shader and insert*/


	const GLchar * const vertexShaderSource = read_file(vs_loc);
	
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);


	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		exit(-2);
	}
	else {
		std::cout << "Successfully loaded vertex shader." << std::endl;
	}
	/*Compile fragment shader and insert*/

	const GLchar * const fragShaderSource = read_file(fs_loc);
	unsigned int fragShader;
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &fragShaderSource, NULL);
	success;
	glCompileShader(fragShader);
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		int len;
		glGetShaderInfoLog(fragShader, 512, &len, infoLog);
		std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		exit(-3);
	}
	else {
		std::cout << "Successfully loaded fragment shader." << std::endl;
	}

	/*Join shaders together in shader program*/
	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragShader);
	glLinkProgram(shaderProgram);
	//check success
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::PROGRAM_LINK::JOIN_FAILED\n" << infoLog << std::endl;
		exit(-255);
	}
	else {
		std::cout << "Successfully linked shaders." << std::endl;
	}
	glDeleteShader(fragShader);
	glDeleteShader(vertexShader);


	glUseProgram(shaderProgram);


	/*Load heightmap from disk*/
	if (std::ifstream(hmap_loc.c_str()).good()) {
		std::cout << "Heightmap found at \"" << hmap_loc << ".\"" << std::endl;
	}

	cimg_library::CImg<float> hmap_png(hmap_loc.c_str());//Read image bytes at location into cimg object. This is now a grid of pixels with an x, y, z, and color value.
	std::cout << "Heightmap loaded." << std::endl
		<< "Height(x): " << hmap_png.height() << std::endl
		<< "Width (y): " << hmap_png.width() << std::endl
		<< "Depth (z): " << hmap_png.depth() << std::endl;

	if (hmap_png.height() < min_d || hmap_png.width() < min_d
		|| hmap_png.height() > max_d || hmap_png.width() > max_d) {
		std::cerr << "Image diagonal must be between " << min_d << " and " << max_d << std::endl;
		exit(2);
	}

	std::vector<float>* vertices = image_to_vertices(hmap_png);
	/*GLfloat verticesss[] = {
		0.0f, 0.5f, 0.0f,  // Top
		0.5f, -0.5f, 0.0f,  // Bottom Right
		-0.5f, -0.5f, 0.0f,  // Bottom Left
	};*/
	//std::vector<GLfloat>*vertices = new std::vector<GLfloat>();

	//for (GLfloat f : verticesss) vertices->push_back(f);
	//for (GLfloat f : *vertices) std::cout << f << std::endl;

	const std::size_t num_vertices = vertices->size()/3;//Guaranteed to be an even division since 3 components are pushed back for each pixel in the conversion function

	std::cout << sizeof(GLfloat)*vertices->size() << std::endl;
	GLuint VAO, VBO;
	glGenVertexArrays(1, &VAO);//Generate 1 array and bind its ID to VAO var
	glGenBuffers(1, &VBO);	//Generate 1 buffer and store its ID in VBO var
	glBindVertexArray(VAO); //Make it our one vertex array


	glBindBuffer(GL_ARRAY_BUFFER, VBO);//My
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vertices->size(), &vertices->front(), GL_STATIC_DRAW);//Tell it to allocate an array buffer with the # of bits in vertices, using the data in vertices, in static draw mode. Pointer must be to vertices.front()

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);//
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0); 

	

	//Tell shader program where to find the transformation matrixes
	GLuint transformLoc = glGetUniformLocation(shaderProgram, "model_matrix");
	GLuint viewMatrixLoc = glGetUniformLocation(shaderProgram, "view_matrix");
	GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection_matrix");



	/*Give initial values to matrices. Will be modified in execution so we need to define this stuff.*/


	glm::mat4 view_matrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), //Give initial camera position
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));//defines origin as  (0, 0, 0), up as (0, 1, 0). First parameter gives initial position.
	glm::mat4 projection_matrix = glm::perspective<float>(fov_angle, (GLfloat)width / (GLfloat)height, 5.0f, 100.0f);//parameter 3 should be positive or else it becomes orthographic.
	glm::mat4 model_matrix(1);
	//model_matrix = glm::translate(model_matrix, glm::vec3(-hmap_png.height()/2, -hmap_png.width()/2, 1.0f));//translate vertices approx. to origin and far away

	//model_matrix = glm::scale(model_matrix, glm::vec3((3.0f / hmap_png.width()), (3.0f / hmap_png.height()), 1.0f));

	model_matrix = glm::scale(model_matrix, glm::vec3((1.0f / 2), (1.0f / 2), 1.0f));//remember: translate->rotate->scale

	//std::cout << projection_matrix << view_matrix << model_matrix;
	//Application/render  loop

	glfwSwapInterval(1);//Enable v-sync

	while (!glfwWindowShouldClose(window))
	{

		assert(glGetError() == GL_NO_ERROR);

		glfwPollEvents();//Needed even if not using polling input; otherwise, OpenGL will think the program has locked up.

		//render commands here

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shaderProgram);


		if (proj1::ROTATE_X != 0 || proj1::ROTATE_Y != 0) {
			model_matrix = glm::rotate(model_matrix, MAP_TRANSLATE_SPEED, glm::vec3(proj1::ROTATE_X, proj1::ROTATE_Y, 0.0f));
		}

		if (proj1::ZOOM < 0 && fov_angle*(zoomfactor - ZOOM_SPEED) > 44) {
			zoomfactor -= ZOOM_SPEED;
			projection_matrix = glm::perspective<float>(fov_angle*zoomfactor, (GLfloat)width / (GLfloat)height, 1.0f, 1000.0f);
		}
		else if (proj1::ZOOM > 0 && fov_angle*(zoomfactor+ZOOM_SPEED) < 46) {
			zoomfactor += ZOOM_SPEED;
			projection_matrix =	glm::perspective<float>(fov_angle*zoomfactor, (GLfloat)width / (GLfloat)height, 1.0f, 1000.0f);
		}
		std::cout << zoomfactor << '\r';
		if (proj1::ROTATE_CAM_X != 0) {
			view_matrix = glm::rotate(view_matrix, CAMERA_ROTATE_SPEED, glm::vec3(proj1::ROTATE_CAM_X, 0, 0));
		}
		if (proj1::ROTATE_CAM_Y != 0) {
			view_matrix = glm::rotate(view_matrix, CAMERA_ROTATE_SPEED, glm::vec3(0, proj1::ROTATE_CAM_Y, 0));
		}




		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(model_matrix));//Pass uniform values to shaders. All uniform are treated as const within the shaders' context.
		glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, glm::value_ptr(view_matrix));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection_matrix));
		

		glBindVertexArray(VAO);
		glDrawArrays(proj1::GL_DRAW_STATE, 0, num_vertices);
		glBindVertexArray(0);


		proj1::ROTATE_X = 0;
		proj1::ROTATE_Y = 0;//reset current rotation
		proj1::ZOOM = 0; //reset zoom action
		proj1::ROTATE_CAM_X = 0;
		proj1::ROTATE_CAM_Y = 0;
		glfwSwapBuffers(window);//update window. Here's your double buffering support.
	
	}
	glfwTerminate();//Free all resources allocated by openGL

	return 0;
}

