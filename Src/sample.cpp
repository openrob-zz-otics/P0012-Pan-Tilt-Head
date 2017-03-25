#include <opencv\highgui.h>
#include <opencv\cv.h>
#include <iostream>
#include <GL\glew.h>
#include <headers\display.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>

using namespace cv;
using namespace std;

cv::Mat frame;

GLint windowWidth = 640;	//display width of our OpenGL window
GLint windowHeight = 480;	//dispaly height of our OpenGL window

std::vector <cv::Point> drawPoints;	//global array of points to draw using OpenGL

/* Path to a training set for facial recognition */
String face_cascade_name = "c:/openrobotics/opencv/sources/data/haarcascades/haarcascade_frontalface_alt.xml";
CascadeClassifier face_cascade;
RNG rng(12345);

// Structure to hold a colour as RGB values in the range 0 to 255
struct Colour3ub
{
	uint8_t redComponent;
	uint8_t greenComponent;
	uint8_t blueComponent;

	Colour3ub(){}
	Colour3ub(uint8_t redComponent, uint8_t greenComponent, uint8_t blueComponent)
	{
		this->redComponent = redComponent;
		this->greenComponent = greenComponent;
		this->blueComponent = blueComponent;
	}
};

string intToString(int number) {


	std::stringstream ss;
	ss << number;
	return ss.str();
}

void detectAndDisplay(Mat frame);

/* Function to draw a point at a specified location with a specified colour */
static void glDrawPoint(const GLfloat & xLoc, const GLfloat & yLoc, const Colour3ub & theColour)
{
	/* Set the colour */
	glColor3ub(theColour.redComponent, theColour.greenComponent, theColour.blueComponent);
	glPointSize(40);
	glEnable(GL_POINT_SMOOTH);

	// Draw the point
	glBegin(GL_POINTS);
	glVertex2f(xLoc, yLoc);
	glEnd();

	/* Reset the colour variable so we don't affect texture rendering  */
	glColor3ub(255, 255, 255);
	

}

static GLuint matToTexture(cv::Mat &mat, GLenum minFilter, GLenum magFilter, GLenum wrapFilter)
{
	// Generate a number for our textureID's unique handle
	GLuint textureID;
	glGenTextures(1, &textureID);
 
	// Bind to our texture handle
	glBindTexture(GL_TEXTURE_2D, textureID);
 
	// Catch silly-mistake texture interpolation method for magnification
	if (magFilter == GL_LINEAR_MIPMAP_LINEAR  ||
	    magFilter == GL_LINEAR_MIPMAP_NEAREST ||
	    magFilter == GL_NEAREST_MIPMAP_LINEAR ||
	    magFilter == GL_NEAREST_MIPMAP_NEAREST)
	{
		cout << "You can't use MIPMAPs for magnification - setting filter to GL_LINEAR" << endl;
		magFilter = GL_LINEAR;
	}
 
	// Set texture interpolation methods for minification and magnification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
 
	// Set texture clamping method
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapFilter);
 
	// Set incoming texture format to:
	// GL_BGR       for CV_CAP_OPENNI_BGR_IMAGE,
	// GL_LUMINANCE for CV_CAP_OPENNI_DISPARITY_MAP,
	// Work out other mappings as required ( there's a list in comments in main() )
	GLenum inputColourFormat = GL_BGR;
	if (mat.channels() == 1)
	{
		inputColourFormat = GL_LUMINANCE;
	}
 
	// Create the texture
	glTexImage2D(GL_TEXTURE_2D,     // Type of texture
	             0,                 // Pyramid level (for mip-mapping) - 0 is the top level
	             GL_RGB,            // Internal colour format to convert to
	             mat.cols,          // Image width  i.e. 640 for Kinect in standard mode
	             mat.rows,          // Image height i.e. 480 for Kinect in standard mode
	             0,                 // Border width in pixels (can either be 1 or 0)
	             inputColourFormat, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
	             GL_UNSIGNED_BYTE,  // Image data type
	             mat.ptr());        // The actual image data itself
 
	// If we're using mipmaps then generate them. Note: This requires OpenGL 3.0 or higher
	if (minFilter == GL_LINEAR_MIPMAP_LINEAR  ||
	    minFilter == GL_LINEAR_MIPMAP_NEAREST ||
	    minFilter == GL_NEAREST_MIPMAP_LINEAR ||
	    minFilter == GL_NEAREST_MIPMAP_NEAREST)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
	}
 
	return textureID;
}

int main(int argc, char* argv[])
{
	/* Create our OpenGL window*/
	Display display(windowWidth, windowHeight, "OpenCV_GLTest");

	//-- 1. Load the cascade
	if (!face_cascade.load(face_cascade_name)) { printf("--(!)Error loading\n"); return -1; };


	GLuint textureTrash;
	GLuint texture[1];
	//cvNamedWindow("img", CV_WINDOW_AUTOSIZE);
	//frame = cvLoadImage("C:/Users/neema/Pictures/androidparty.jpg", IMREAD_COLOR);
	//depth = cvLoadImage("C:/Users/neema/Pictures/androidparty.jpg", IMREAD_COLOR);
	// Create a our video capture using the Kinect and OpenNI
	// Note: To use the cv::VideoCapture class you MUST link in the highgui lib (libopencv_highgui.so)
	
	cout << "Opening Camera Device ..." << endl;

	/* Capture a video feed from any available camera, in this case the web cam */
	cv::VideoCapture capture(CV_CAP_ANY);

	// Set sensor to 640x480@30Hz mode as opposed to 1024x768@15Hz mode (which is available for image sensor only!)
	// Note: CV_CAP_OPENNI_IMAGE_GENERATOR_OUTPUT_MODE = CV_CAP_OPENNI_IMAGE_GENERATOR + CV_CAP_PROP_OPENNI_OUTPUT_MODE
	//capture.set(CV_CAP_OPENNI_IMAGE_GENERATOR_OUTPUT_MODE, CV_CAP_OPENNI_VGA_30HZ); // default
	cout << "done." << endl;

	// Check that we have actually opened a connection to the sensor
	if (!capture.isOpened())
	{
		cout << "Can not open a capture object." << endl;
		return -1;
	}

	do {

		if (!capture.grab())
		{
			cout << "Could not grab camera... Skipping frame." << endl;
		}
		else
		{
			/*
			Frame retrieval formats:
			data given from depth generator:
			OPENNI_DEPTH_MAP         - depth values in mm (CV_16UC1)
			OPENNI_POINT_CLOUD_MAP   - XYZ in meters (CV_32FC3)
			OPENNI_DISPARITY_MAP     - disparity in pixels (CV_8UC1)
			OPENNI_DISPARITY_MAP_32F - disparity in pixels (CV_32FC1)
			OPENNI_VALID_DEPTH_MASK  - mask of valid pixels (not occluded, not shaded etc.) (CV_8UC1)

			data given from RGB image generator:
			OPENNI_BGR_IMAGE - color image (CV_8UC3)
			OPENNI_GRAY_IMAGE - gray image (CV_8UC1)
			*/

			// Retrieve desired sensor data
			capture.retrieve(frame, CV_CAP_OPENNI_GRAY_IMAGE);


			if (!frame.empty())
			{
				/* Find faces in the frame and draw a rectangle over top using OpenCV*/
				detectAndDisplay(frame);
			}
			
			/* Flip the frame so that when we draw the texture using OpenGL it appears right side up*/
			cv::flip(frame, frame, 0);

			/* Convert the matrix to a texture*/
			GLuint tex = matToTexture(frame, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP);

			glBindTexture(GL_TEXTURE_2D, tex);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glLoadIdentity();

			glEnable(GL_TEXTURE_2D);

			/* Actually draw the texture to our buffer*/
			glBindTexture(GL_TEXTURE_2D, tex);
			glBegin(GL_QUADS);
			glTexCoord2f(1, 1);
			glVertex3f(0, 0, 0);
			glTexCoord2f(0, 1);
			glVertex3f(windowWidth, 0, 0);
			glTexCoord2f(0, 0);
			glVertex3f(windowWidth, windowHeight, 0);
			glTexCoord2f(1, 0);
			glVertex3f(0, windowHeight, 0);
			glEnd();

			glDeleteTextures(1, &tex);
			glDisable(GL_TEXTURE_2D);

			/* Draw the points we saved using OpenCV's facial detection feature, these are just the corners
			 * of the rectangle
			 */
			for (int j = 0; j < drawPoints.size(); j++) {
				glDrawPoint( 640 - drawPoints.at(j).x, drawPoints.at(j).y, Colour3ub(255, 0, 0));
			}

			/* Swap the buffers to draw to our OpenGL window*/
			display.Update();

			/* THIS ONLY WORKS IF WE HAVE AN OPENCV WINDOW DISPLAYED
			char key = cvWaitKey(2);     //Capture Keyboard stroke
			if ((int)key == 27) {
				break;      //If you hit ESC key loop will break.
			}
			*/

		}
	} while (!display.IsClosed()); //Loop until we close the OpenGL window
	
	
	cvDestroyAllWindows();
	return 0;

}

void detectAndDisplay(Mat frame)
{
	std::vector<Rect> faces;
	Mat frame_gray;

	cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
	equalizeHist(frame_gray, frame_gray);
	//-- Detect faces
	face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));
	drawPoints.clear();
	for (size_t i = 0; i < faces.size(); i++)
	{
	
		cv::Rect rect(faces[i].x, faces[i].y, faces[i].width, faces[i].height);
		cv::rectangle(frame, rect, Scalar(0, 255, 255));
		
		drawPoints.push_back(Point(faces[i].x, faces[i].y));
		drawPoints.push_back(Point(faces[i].x, faces[i].y + faces[i].height));
		drawPoints.push_back(Point(faces[i].x + faces[i].width, faces[i].y));
		drawPoints.push_back(Point(faces[i].x + faces[i].width, faces[i].y + faces[i].height));

	}
}



