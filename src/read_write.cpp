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
#include "header_files\OculusRiftSensor.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include <dynamixel_sdk.h>                                  // Uses Dynamixel SDK library

// Control table address
#define ADDR_MX_TORQUE_ENABLE           24                  // Control table address is different in Dynamixel model
#define ADDR_MX_GOAL_POSITION           30
#define ADDR_MX_PRESENT_POSITION        36

#define ADDR_MX_TORQUE_LIMIT_HIGH          35
#define ADDR_MX_TORQUE_LIMIT_LOW           34

//
#define ADDR_MX_MAX_TORQUE_HIGH         15
#define ADDR_MX_MAX_TORQUE_LOW          14

// Protocol version
#define PROTOCOL_VERSION                1.0                 // See which protocol version is used in the Dynamixel

// Default setting
#define DXL_ID                          1                   // Dynamixel ID: 1
#define BAUDRATE                        57142
#define DEVICENAME                      "COM4"				// Check which port is being used on your controller
                                                            // ex) Windows: "COM1"   Linux: "/dev/ttyUSB0"

#define TORQUE_ENABLE                   1                   // Value for enabling the torque
#define TORQUE_DISABLE                  0                   // Value for disabling the torque
#define DXL_MINIMUM_POSITION_VALUE      0                 // Dynamixel will rotate between this value
#define DXL_MAXIMUM_POSITION_VALUE      1023              // and this value (note that the Dynamixel would not move when the position value is out of movable range. Check e-manual about the range of the Dynamixel you use.)
#define DXL_POSITION_TO_ANGLE_RATIO		300
#define DXL_MOVING_STATUS_THRESHOLD     10                  // Dynamixel moving status threshold

#define ESC_ASCII_VALUE                 0x1b
#define DXL_INIT_POSITION               512

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



//TODO: Refactor the following methods into a seperate class file for the dynamixel

void initDxl();

/**

void openDxlPort();

void setDxlBaudRate();

void enableDxlTorque(int baudrate);

void writeDxlPosition(int goalPosition);

void readDxlPosition();
**/

int angleToDxlPosition(double angle);


int main()
{
  // Initialize PortHandler instance
  // Set the port path
  // Get methods and members of PortHandlerLinux or PortHandlerWindows
  dynamixel::PortHandler *portHandler = dynamixel::PortHandler::getPortHandler(DEVICENAME);

  // Initialize PacketHandler instance
  // Set the protocol version
  // Get methods and members of Protocol1PacketHandler or Protocol2PacketHandler
  dynamixel::PacketHandler *packetHandler = dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION);
  
  //create OculusRiftSensor instance and initialize it
  OculusRiftSensor OVR;

  //initial angle value for the head set when user looking straight ahead
  double initOVRAngleY;

  int index = 0;
  int dxl_comm_result = COMM_TX_FAIL;             // Communication result
  int dxl_goal_position;
  //int dxl_goal_position[2] = {DXL_MINIMUM_POSITION_VALUE, DXL_MAXIMUM_POSITION_VALUE};         // Goal position

  int torque_max_high;

  uint8_t dxl_error = 0;                          // Dynamixel error
  uint16_t dxl_present_position = 0;              // Present position

  // Open port
  if (portHandler->openPort())
  {
    printf("Succeeded to open the port!\n");
  }
  else
  {
    printf("Failed to open the port!\n");
    printf("Press any key to terminate...\n");
    _getch();
    return 0;
  }

  // Set port baudrate
  if (portHandler->setBaudRate(BAUDRATE))
  {
    printf("Succeeded to change the baudrate!\n");
  }
  else
  {
    printf("Failed to change the baudrate!\n");
    printf("Press any key to terminate...\n");
    _getch();
    return 0;
  }

  dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID, ADDR_MX_TORQUE_LIMIT_LOW, 0x200, &dxl_error);
  if (dxl_comm_result != COMM_SUCCESS)
  {
	  packetHandler->printTxRxResult(dxl_comm_result);
  }
  else if (dxl_error != 0)
  {
	  packetHandler->printRxPacketError(dxl_error);
  }
  else
  {
	  printf("Dynamixel max. torque has been successfully changed \n");
  }

  // Enable Dynamixel Torque
  dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, DXL_ID, ADDR_MX_TORQUE_ENABLE, TORQUE_ENABLE, &dxl_error);
  
  if (dxl_comm_result != COMM_SUCCESS)
  {
    packetHandler->printTxRxResult(dxl_comm_result);
  }
  else if (dxl_error != 0)
  {
    packetHandler->printRxPacketError(dxl_error);
  }
  else
  {
    printf("Dynamixel has been successfully connected \n");
  }

  
  // initialize dynamixel position
  dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID, ADDR_MX_GOAL_POSITION, DXL_INIT_POSITION, &dxl_error);
  if (dxl_comm_result != COMM_SUCCESS)
  {
	  packetHandler->printTxRxResult(dxl_comm_result);
  }
  else if (dxl_error != 0)
  {
	  std::cout << "xxx";
	  packetHandler->printRxPacketError(dxl_error);
  }

  printf("Look straight ahead at the , press any key when ready");
  getch();
  OVR.OVRread();
  initOVRAngleY = OVR.angleY;
  
  while(1)
  {
      
    //wait 3 seconds before writing a new value to the dynamixel 
	Sleep(10);
    OVR.OVRread();

	//TODO: read from oculus, convert int [] values to two angle values, 
	//      use daisy chaining to write goal positions to both servos

	//currently we only have one servo, convert the yaw value to a position value and write it to the servo
	dxl_goal_position = DXL_INIT_POSITION - angleToDxlPosition(OVR.angleY - initOVRAngleY);
	printf("goal position for dynamixel %d", dxl_goal_position);
	if (dxl_goal_position > 0 && dxl_goal_position < 1023){
		//dxl_goal_position = 500;


		// Write goal position
		dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID, ADDR_MX_GOAL_POSITION, dxl_goal_position, &dxl_error);
		if (dxl_comm_result != COMM_SUCCESS)
		{
			packetHandler->printTxRxResult(dxl_comm_result);
		}
		else if (dxl_error != 0)
		{
			std::cout << "xxx";
			packetHandler->printRxPacketError(dxl_error);
		}

		do
		{
			// Read present position
			dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, DXL_ID, ADDR_MX_PRESENT_POSITION, &dxl_present_position, &dxl_error);
			if (dxl_comm_result != COMM_SUCCESS)
			{
				packetHandler->printTxRxResult(dxl_comm_result);
			}
			else if (dxl_error != 0)
			{
				packetHandler->printRxPacketError(dxl_error);
			}

			printf("[ID:%03d] GoalPos:%03d  PresPos:%03d\n", DXL_ID, dxl_goal_position, dxl_present_position);

		} while ((abs(dxl_goal_position - dxl_present_position) > DXL_MOVING_STATUS_THRESHOLD));
	}
    // Change goal position

  }
  

  // Disable Dynamixel Torque
  dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, DXL_ID, ADDR_MX_TORQUE_ENABLE, TORQUE_DISABLE, &dxl_error);
  if (dxl_comm_result != COMM_SUCCESS)
  {
    packetHandler->printTxRxResult(dxl_comm_result);
  }
  else if (dxl_error != 0)
  {
    packetHandler->printRxPacketError(dxl_error);
  }

  // Close port
  portHandler->closePort();

  return 0;
}

/**
	Function for calculating the position value to write to the dynamixel based on the angle of interest for the 
	oculus rift

	TODO this will likely need to be improved

	@param angle 
				an angle value between -90 and 90 degrees
	@return 
			returns the position value to write to the dynamixel servo
**/
int angleToDxlPosition(double angle){

	//if angle is negative then subtract the computed position value from the maximum dxl_position value
	/*
	if (angle < 0){
		return (int)
			(DXL_MAXIMUM_POSITION_VALUE -
			(abs(angle) * (DXL_MAXIMUM_POSITION_VALUE / DXL_POSITION_TO_ANGLE_RATIO)));

	}
	*/
	//otherwise simply multiply angle by conversion ratio

	return (int)(angle * (DXL_MAXIMUM_POSITION_VALUE / DXL_POSITION_TO_ANGLE_RATIO));

	
}

