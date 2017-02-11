#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <conio.h>
#include "..\include\servo.h"


Servo::Servo(int dxl_id, 
             double protocol_ver, 
             servo_type type, 
             int baudrate,
             int init_torque, 
             int init_position, 
             char device_name []){
    
    
    switch(type){
        
        case(RX_24_F):{
            
            //set the control table values according to the 
            //RX_24F dynamixel namespace
            
            this->addr_torque_enable = RX_24F::addr_torque_enable;
            this->addr_goal_position = RX_24F::addr_goal_position;
            this->addr_present_position = RX_24F::addr_present_position;
            this->addr_torque_limit_low = RX_24F::addr_torque_limit_low;
            this->addr_torque_limit_high = RX_24F::addr_torque_limit_high;
            this->addr_max_torque_high = RX_24F::addr_max_torque_high;
            this->addr_max_torque_low = RX_24F::addr_max_torque_low;
			this->dxl_id = dxl_id;
            
            // Initialize PortHandler instance
            // Set the port path
            // Get methods and members of PortHandlerLinux or PortHandlerWindows
            portHandler = dynamixel::PortHandler::getPortHandler("COM4");
            
            // Initialize PacketHandler instance
            // Set the protocol version
            // Get methods and members of Protocol1PacketHandler or Protocol2PacketHandler
            packetHandler = dynamixel::PacketHandler::getPacketHandler(1.0);
            
            //open the port for the provided device name
            openPort();
            
            //set the baudrate for communication with this servo
            setBaudrate(baudrate);
            
			//initalize the torque value
			setTorque(init_torque);
			
			enableTorque();
            
            //intialize the dynamixel position
            writePosition(init_position);
            break;  
        }
        
        //TODO: provide intialization for MX_28 and other dynamixel servo
        //types

    }
                
}


Servo::~Servo(){
    
    //disable the dynamixle torque and close the port
    disableTorque();
    portHandler->closePort();
}


void Servo::openPort(){

    // Open Port
    if (portHandler->openPort())
    {
        printf("Succeeded to open the port!\n");
    }
    else
    {
        printf("Failed to open the port!\n");
        printf("Press any key to terminate...\n");
        _getch();
        exit(EXIT_FAILURE);
    }

}


void Servo::enableTorque(){
    
     // Enable Dynamixel Torque
    uint8_t dxl_error = 0;
    int dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, dxl_id, addr_torque_enable, TORQUE_ENABLE, &dxl_error);
  
    if (dxl_comm_result != COMM_SUCCESS)
    {
        packetHandler->printTxRxResult(dxl_comm_result);
    }
    else if (dxl_error != 0)
    {
        packetHandler->printRxPacketError(dxl_error);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Dynamixel has been successfully connected \n");
    }
    
}

void Servo::disableTorque(){
    
    // Disable Dynamixel Torque
    uint8_t dxl_error = 0;
    int dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, dxl_id, addr_torque_enable, TORQUE_DISABLE, &dxl_error);
    if (dxl_comm_result != COMM_SUCCESS)
    {
        packetHandler->printTxRxResult(dxl_comm_result);
    }
    else if (dxl_error != 0)
    {
        packetHandler->printRxPacketError(dxl_error);
        exit(EXIT_FAILURE);
    }
}

void Servo::setBaudrate(int baudrate){
    
    // Set Port Baudrate
    if (portHandler->setBaudRate(baudrate))
    {
        printf("Succeeded to change the baudrate!\n");
    }
    else
    {
        printf("Failed to change the baudrate!\n");
        printf("Press any key to terminate...\n");
        _getch();
        exit(EXIT_FAILURE);
    }
}

void Servo::setTorque(int goal_torque){
    
    uint8_t dxl_error = 0;  
    int dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, dxl_id, addr_torque_limit_low, goal_torque, &dxl_error);
                            
    if (dxl_comm_result != COMM_SUCCESS)
    {
        packetHandler->printTxRxResult(dxl_comm_result);
    }
    else if (dxl_error != 0)
    {
        packetHandler->printRxPacketError(dxl_error);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Dynamixel max. torque has been successfully changed \n");
    }
    
}

void Servo::writePosition(int goal_position){
    
	uint8_t dxl_error;
    int dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, dxl_id, addr_goal_position, goal_position, &dxl_error);
    if (dxl_comm_result != COMM_SUCCESS)
    {
        packetHandler->printTxRxResult(dxl_comm_result);
    }
    else if (dxl_error != 0)
    {
        std::cout << "xxx";
        packetHandler->printRxPacketError(dxl_error);
        exit(EXIT_FAILURE);
    }
}

int Servo::readPosition(){
    
    uint16_t dxl_present_position;
    uint8_t dxl_error = 0;
    
    int dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, dxl_id, addr_present_position, &dxl_present_position, &dxl_error);
	if (dxl_comm_result != COMM_SUCCESS)
	{
		packetHandler->printTxRxResult(dxl_comm_result);
	}
	else if (dxl_error != 0)
	{
		packetHandler->printRxPacketError(dxl_error);
        exit(EXIT_FAILURE);
	}
    
    return dxl_present_position;   
    
}

