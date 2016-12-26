// oculus_orientation.cpp : Defines the entry point for the console application.
//

//#include "C:\Users\User\Documents\Visual Studio 2013\Projects\oculus_orientation\oculus_orientation\stdafx.h"
#include <iostream>
#include <iomanip>
#include <OVR_CAPI.h>
#include "header_files\OculusRiftSensor.h"
#include <windows.h>
#include <thread>

#define COLW setw(15)

using namespace std;

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

double OculusRiftSensor::yawToAngle(double yaw){
	
	return (yaw / 0.40)*90.0;
	
}

double OculusRiftSensor::pitchToAngle(double pitch){

	return(pitch / 0.40)*90.0;

}

double OculusRiftSensor::rollToAngle(double roll){

	return (roll / 0.40)*9.0;

}

OculusRiftSensor::OculusRiftSensor(){

	initOVR();
}

OculusRiftSensor::~OculusRiftSensor(){

	closeOVR();
}

void OculusRiftSensor::OVRread(){

	// Initialize our session with the Oculus HMD.


	int count = 0;
	double x_sum = 0.0;
	double y_sum = 0.0;
	double z_sum = 0.0;

	double max_count = 100000.0;
	double sum = 0;

	double yaw = 0;
	double pitch = 0;
	double roll = 0;

	if (_initialized == ovrSuccess)
	{   // Then we're connected to an HMD!

		// Let's take a look at some orientation data.

		while (count < max_count)
		{
			//cout << "success";
			//this_thread::sleep_for(chrono::seconds(1));
			Sleep(100);

			_ts = ovr_GetTrackingState(_session, 0, true);

			ovrPoseStatef tempHeadPose = _ts.HeadPose;
			ovrPosef tempPose = tempHeadPose.ThePose;
			ovrQuatf tempOrient = tempPose.Orientation;

			/*
			cout << "Orientation (x,y,z):  " << COLW << tempOrient.x << ","
			<< COLW << tempOrient.y << "," << COLW << tempOrient.z
			<< endl;
			*/

			// Wait a bit to let us actually read stuff.
			//std::this_thread::sleep_for(std::chrono::milliseconds(100));

			x_sum = x_sum + tempOrient.x;
			y_sum = y_sum + tempOrient.y;
			z_sum = z_sum + tempOrient.z;
			count++;


			/*
			cout << "Orientation (x,y,z):  " << COLW << tempOrient.x << ","
			<< COLW << tempOrient.y << "," << COLW << tempOrient.z
			<< endl;
			*/
		}

		pitch = x_sum / max_count;
		yaw = y_sum / max_count;
		roll = z_sum / max_count;

		//store the converted angle values 
		angleX = pitchToAngle(pitch);
		angleY = yawToAngle(yaw);
		angleZ = rollToAngle(roll);

		std::cout << "Orientation (x,y,z):  " << COLW << angleX << ","
			<< COLW << angleY << "," << COLW << angleZ
			<< endl;

	}
}

		
			
