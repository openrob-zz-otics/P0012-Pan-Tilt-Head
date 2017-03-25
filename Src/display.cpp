#include <headers/display.h>
#include <iostream>
#include <GL/glew.h>


Display::Display(int width, int height, const std::string& title)
{

    //SDL_Init(SDL_INIT_EVERYTHING);
	GLint   windowWidth = 640;     // Define our window width
	GLint   windowHeight = 480;     // Define our window height
	GLfloat fieldOfView = 45.0f;   // FoV
	GLfloat zNear = 0.1f;    // Near clip plane
	GLfloat zFar = 200.0f;  // Far clip plane

	
    
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    m_window = SDL_CreateWindow(title.c_str(), 
                                SDL_WINDOWPOS_CENTERED, 
                                SDL_WINDOWPOS_CENTERED, 
                                width, 
                                height, 
                                SDL_WINDOW_OPENGL);

    m_glContext = SDL_GL_CreateContext(m_window);

    GLenum status = glewInit();

	if (status != GLEW_OK)
	{
		std::cerr << "Glew failed to initialize!" << std::endl;
	}

	// Setup our viewport to be the entire size of the window
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	// Change to the projection matrix and set our viewing volume
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, windowWidth, windowHeight, 0, 0, 1); // Paramters: left, right, bottom, top, near, far

													// Make sure we're changing the model view and not the projection then reset the matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// ----- OpenGL Setup -----

	//glfwSwapInterval(1); 		// Lock to vertical sync of monitor (normally 60Hz, so 60fps)

	glDisable(GL_SMOOTH);		// Disable (gouraud) shading

	glDisable(GL_DEPTH_TEST); 	// Disable depth testing

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // // Set our clear colour to black

	glEnable(GL_LINE_SMOOTH);	// Enable anti-aliasing on lines

    m_isClosed = false;

}

Display::~Display()
{
    SDL_GL_DeleteContext(m_glContext);
    SDL_DestroyWindow(m_window);

    SDL_Quit();
}


void Display::Update()
{

    SDL_GL_SwapWindow(m_window);

    SDL_Event e;

    while(SDL_PollEvent(&e))
    {
        if(e.type == SDL_QUIT)
            m_isClosed = true;
    }

}

void Display::Clear(float r, float g, float b, float a)
{

    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);

}


bool Display::IsClosed()
{

    return m_isClosed;
}






