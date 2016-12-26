#include <iostream>
#include <iomanip>
#include <OVR_CAPI.h>
#include <windows.h>
#include <thread>

#define COLW setw(15)

using namespace std;

class OculusRiftSensor{

private:

	ovrResult _initialized;
	ovrTrackingState _ts;
	ovrSession _session;

	//initialize the sensor
	bool initOVR();

	//destroy the sensor
	bool closeOVR();

	//convert the yaw of the sensor to an angle value
	double yawToAngle(double yaw);

	//convert the roll of the sensor to an angle value
	double rollToAngle(double roll);

	//convert the pitch of the sensor to an angle value
	double pitchToAngle(double pitch);

public:

	//public variables for the orientation of the sensor
	double angleX, angleY, angleZ;

	OculusRiftSensor();

	~OculusRiftSensor();

	//read from the sensor and store the current orientation in 
	//variables orientX, orientY, orientZ
	void OVRread();

};