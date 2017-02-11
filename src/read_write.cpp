/*******************************************************************************
* Copyright (c) 2016, ROBOTIS CO., LTD.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* * Redistributions of source code must retain the above copyright notice, this
*   list of conditions and the following disclaimer.
*
* * Redistributions in binary form must reproduce the above copyright notice,
*   this list of conditions and the following disclaimer in the documentation
*   and/or other materials provided with the distribution.
*
* * Neither the name of ROBOTIS nor the names of its
*   contributors may be used to endorse or promote products derived from
*   this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/* Author: Ryu Woon Jung (Leon) */

//
// *********     Read and Write Example      *********
// Available DXL model on this example : All models using Protocol 1.0
// This example is tested with a DXL MX-28, and an USB2DYNAMIXEL
// Be sure that DXL MX properties are already set as %% ID : 1 / Baudnum : 1 (Baudrate : 1000000)
//

#ifdef __linux__
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <conio.h>
#endif

//include file for reading from oculus
#include "..\include\OculusRiftSensor.h"
#include "..\include\servo.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>                               


#define DXL_PAN_ANGVEL_TO_TORQUE_RATIO		10					// Conversion from an angular velocity value to torque
#define DXL_PAN_POSITION_TO_ANGLE_RATIO		300
#define DXL_PAN_MOVING_THRESHOLD            10                  // Dynamixel moving status threshold
#define DXL_PAN_TORQUE_THRESHOLD		    100					// Threshold for a torque change in order for it to be applied

// Default setting
#define DXL_PAN_PROTOCOL_VERSION            1.0
#define DXL_PAN_SERVO_TYPE                  RX_24_F
#define DXL_PAN_ID                          2                   // Dynamixel ID: 1
#define DXL_PAN_BAUDRATE                    57142
#define DXL_PAN_DEVICE_NAME                 "COM4"				// Check which port is being used on your controller
                                                                // ex) Windows: "COM1"   Linux: "/dev/ttyUSB0"
#define DXL_PAN_INIT_TORQUE		            0x200

#define DXL_PAN_MIN_POSITION_VALUE          0					// Dynamixel will rotate between this value
#define DXL_PAN_MAX_POSITION_VALUE          1023                // and this value (note that the Dynamixel would not move when the position value is out of movable range. Check e-manual about the range of the Dynamixel you use.)

#define ESC_ASCII_VALUE                     0x1b
#define DXL_INIT_POSITION                   512

int getch()
{
#ifdef __linux__
  struct termios oldt, newt;
  int ch;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  ch = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  return ch;
#elif defined(_WIN32) || defined(_WIN64)
  return _getch();
#endif
}

int kbhit(void)
{
#ifdef __linux__
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if (ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }

  return 0;
#elif defined(_WIN32) || defined(_WIN64)
  return _kbhit();
#endif
}

/**************************************************************************
	Function for calculating the position value to write to the dynamixel based on the angle of interest for the 
	oculus rift

	TODO this will likely need to be improved

	@param angle 
				an angle value between -90 and 90 degrees
	@return 
			returns the position value to write to the dynamixel servo
***************************************************************************/
static int angleToDxlPosition(double angle){


	return (int)(angle * (DXL_PAN_MAX_POSITION_VALUE 
                        / DXL_PAN_POSITION_TO_ANGLE_RATIO));

	
}

/****************************************************************************
** Function to convert angular velocity of the oculus rift about an axis
** to a torque value for the servo. 
**
** CURRENTLY UNUSED!
*****************************************************************************/
static int angVelToDxlTorque(double angVel){

	double abs_vel = abs(angVel) * 1000000;
	if ((abs_vel / DXL_PAN_ANGVEL_TO_TORQUE_RATIO) > 0x3FF){
		return 0x3FF;
	}
	else if ((abs_vel / DXL_PAN_ANGVEL_TO_TORQUE_RATIO) <= 0x030){
		return 0x030;
	}
	else{
		return abs_vel / (double) DXL_PAN_ANGVEL_TO_TORQUE_RATIO;
	}
}

/**********************************************************************
** Function to calculate the torque to write to the dynamixel servo. 
** The goal torque is proportional to the difference in present position 
** and goal position as well as the current angular velocity of the oculus 
** rift about the axis of interest
***********************************************************************/
static int dxlCalcGoalTorque(int dxl_present_position, 
                             int dxl_goal_position,
                             double angVel){
    
    
    int dxl_goal_torque = abs(dxl_present_position - dxl_goal_position) + abs(angVel*30000);
    
    //only write the torque to the motor if within valid range        
    if (dxl_goal_torque > 1023){
        dxl_goal_torque = 1023;
    }
    
    return dxl_goal_torque;
}
/************************************************************************
** Function to calculate the goal position of the dynamixel. Maps the
** current angle of the oculus rift about an axis to a dxl position.
** returns the difference between the calculated position and the initial
** position of the dynamixel
************************************************************************/
static int dxlCalcGoalPosition(double angle){
    
    int dxl_goal_position = DXL_INIT_POSITION - angleToDxlPosition(angle);
            
    //only write the position to the motor if within valid range   
    if (dxl_goal_position < 0){
        dxl_goal_position = 0;
    }
    else if (dxl_goal_position > 1023){
        dxl_goal_position = 1023;
    }
    
    return dxl_goal_position;
    
}

//TODO: Refactor the following methods into a seperate class file for the dynamixel


int main()
{
    
    Servo dxl_pan(DXL_PAN_ID, 
                  DXL_PAN_PROTOCOL_VERSION, 
                  DXL_PAN_SERVO_TYPE,
                  DXL_PAN_BAUDRATE, 
                  DXL_PAN_INIT_TORQUE, 
                  DXL_INIT_POSITION,
                  DXL_PAN_DEVICE_NAME);
                            

    //create OculusRiftSensor instance and initialize it
    OculusRiftSensor OVR;

    //initial angle value for the head set when user looking straight ahead
    double initOVRAngleY;
    
    int dxl_present_position;
    int dxl_goal_position;
    int dxl_goal_torque;
  
    // get the initial orientation of the oculus rift when the user is ready
    
    printf("Look straight ahead at the camera, press any key when ready");
    getch();
    OVR.OVRread();
    initOVRAngleY = OVR.getAngleY();
  
    while(1)
    {
      
        //we take a new reading every 5 milliseconds
        Sleep(5);
        OVR.OVRread();

        //read the present position of the dynamixel
        dxl_present_position = dxl_pan.readPosition();

        //TODO: read from oculus, convert int [] values to two angle values, 
        //      use daisy chaining to write goal positions to both servos

        //currently we only have one servo, convert the yaw value to a position value and write it to the servo
        dxl_goal_position = dxlCalcGoalPosition(OVR.getAngleY() - initOVRAngleY);

        dxl_goal_torque = dxlCalcGoalTorque(dxl_present_position,
                                            dxl_goal_position,
                                            OVR.getAngVelY());
        

        dxl_pan.setTorque(dxl_goal_torque);
        


        // Write goal position
        dxl_pan.writePosition(dxl_goal_position);
        

    }

    return 0;
}



