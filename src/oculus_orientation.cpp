// oculus_orientation.cpp : Defines the entry point for the console application.
//

//#include "C:\Users\User\Documents\Visual Studio 2013\Projects\oculus_orientation\oculus_orientation\stdafx.h"
#include <iostream>
#include <iomanip>
#include <OVR_CAPI.h>
#include "header_files\OculusRiftSensor.h"
#include <windows.h>
#include <thread>
#include <vector>

#define COLW setw(15)

using namespace std;

/************************************************************************

Constructor and Destructor methods

************************************************************************/
OculusRiftSensor::OculusRiftSensor(){

	initOVR();
}

OculusRiftSensor::~OculusRiftSensor(){

	closeOVR();
}

bool OculusRiftSensor::initOVR(){

	if (ovr_Initialize(nullptr) == ovrSuccess)
	{
		_session = nullptr;
		ovrGraphicsLuid luid;
		_initialized = ovr_Create(&_session, &luid);
		return true;
	}
	return false;
}

bool OculusRiftSensor::closeOVR(){

	ovr_Destroy(_session);

	ovr_Shutdown();

	return true;
	// If we've fallen through to this point, the HMD is no longer
	// connected.

}

/************************************************************************

This function reads data from the OculusRiftSensor, updates the angle 
and angular velocity values

************************************************************************/
void OculusRiftSensor::OVRread(){

	// Initialize our session with the Oculus HMD.


	double count = 0;
	double x_sum = 0.0;
	double y_sum = 0.0;
	double z_sum = 0.0;

	double max_count = 100000;
    double store_angle_count = 10000;
  
    
    vector <double> anglesX;
    vector <double> anglesY; 
    vector <double> anglesZ;
    
	double sum = 0;

	double yaw = 0;
	double pitch = 0;
	double roll = 0;

	if (_initialized == ovrSuccess)
	{   // Then we're connected to an HMD!

		// Let's take a look at some orientation data.

		while (count < max_count)
		{
			_ts = ovr_GetTrackingState(_session, 0, true);

			ovrPoseStatef tempHeadPose = _ts.HeadPose;
			ovrPosef tempPose = tempHeadPose.ThePose;
			ovrQuatf tempOrient = tempPose.Orientation;


			// Wait a bit to let us actually read stuff.
			//std::this_thread::sleep_for(std::chrono::milliseconds(100));

			x_sum = x_sum + tempOrient.x;
			y_sum = y_sum + tempOrient.y;
			z_sum = z_sum + tempOrient.z;
            
            if((count % store_angle_count) == 0){
                
                anglesX.push_back(pitchToAngle(x_sum / count));
                anglesY.puch_back(yawToAngle(y_sum / count));
                anglesZ.push_back(rollToAngle(z_sum / count));
                
            }
			count++;

		}

		pitch = x_sum / max_count;
		yaw = y_sum / max_count;
		roll = z_sum / max_count;

		//store the converted angle values 
		_angleX = pitchToAngle(pitch);
		_angleY = yawToAngle(yaw);
		_angleZ = rollToAngle(roll);
        
        //calculate and store the angular velocity about each axis
        _angVel_X = calcAngVel(anglesX);
        _angVel_Y = calcAngVel(angleY);
        _angVel_Z = calcAngVel(angleZ);

		std::cout << "Orientation (x,y,z):  " << COLW << angleX << ","
			<< COLW << angleY << "," << COLW << angleZ
			<< endl;

	}
    
    
}

/************************************************************************

Function to calculate the angular velocity about an axis given a sequence
of angle readings

************************************************************************/
double calcAngVel(vector <double> angles){
    
    double difference_sum = 0;
    
    for(int i = 1; i < angles.size; i++){
        difference_sum += (angles[i] - angles[i-1]);
    }
    
    //TODO: need to fix this by doing experiments and determining if
    //this is accurate
    
    return difference_sum / angles.size;
]
}
/************************************************************************

Functions to convert yaw, pitch and roll values to angle values (degrees)

************************************************************************/

double OculusRiftSensor::yawToAngle(double yaw){
	
	return (yaw / 0.50)*90.0;
	
}

double OculusRiftSensor::pitchToAngle(double pitch){

	return(pitch / 0.50)*90.0;

}

double OculusRiftSensor::rollToAngle(double roll){

	return (roll / 0.50)*9.0;

}

/************************************************************************

Getter methods for the private member variables within the 
OculusRiftSensor

************************************************************************/

double OculusRiftSensor::getAngleX(){
    return _angleX;
}

double OculusRiftSensor::getAngleY(){
    return _angleY;
}

double OculusRiftSensor::getAngleZ(){
    return _angleZ;
}

double OculusRiftSensor::getAngVelX(){
    return _angVel_X;
}

double OculusRiftSensor::getAngVelY(){
    return _angVel_Y;
}

double OculusRiftSensor::getAngleZ(){
    return _angVel_Z;
}

		
			
