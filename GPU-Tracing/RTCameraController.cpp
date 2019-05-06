#include "RTCameraController.h"

#include <iostream>
using namespace std;

extern GLFWwindow *window;



void updateRotation( RTCamera &camera )
{
	static bool left_pressed = false;
	static double lastX, lastY;

	double x, y;

    glfwGetCursorPos( window, &x, &y );

    if ( glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_LEFT ) == GLFW_PRESS 
        && !left_pressed )
    {
		left_pressed = true;
		lastX = x;
		lastY = y;
    }
	
     if ( glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_LEFT ) != GLFW_PRESS && left_pressed )
	{
		left_pressed = false;
	}

	if (left_pressed)
	{
		float rotX = ( lastX - x ) / 500.0f;
		camera.turnLeft( rotX );
		float rotY = ( lastY - y ) / 500.0f;
		camera.turnUp( rotY );

        cout << lastX - x << endl;
		cout << lastY - y << endl;
		lastX = x;
		lastY = y;
	}
}

static float moving_speed=0.1;

void updateTranslation(RTCamera& camera)
{
	if ( glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS )
	{
		camera.moveForward( moving_speed );
	}
	
	if ( glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS )
    {
		camera.moveForward( -moving_speed );
	}
	if ( glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS )
	{
		camera.moveLeft( moving_speed );
	}
	if ( glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS )
	{
		camera.moveLeft( -moving_speed );
	}

	if ( glfwGetKey( window, GLFW_KEY_Q ) == GLFW_PRESS )
	{
		camera.moveUp( moving_speed );
	}

	if ( glfwGetKey( window, GLFW_KEY_E ) == GLFW_PRESS )
	{
		camera.moveUp( -moving_speed );
	}
}

bool updateCamera(RTCamera& camera)
{
	updateRotation(camera);
	updateTranslation(camera);
	bool bUpdate = false;
	if (camera.bNeedUpdate())
	{
		bUpdate = true;
	}
	camera.Update();

	return bUpdate;
}

void setCameraSpeed(float speed) {
    moving_speed = speed;
}
