#include <iostream>
#include <iomanip>
#include <OVR_CAPI.h>
#include <windows.h>
#include <thread>
#include <vector>

#define COLW setw(15)

using namespace std;

class OculusRiftSensor{

private:

	ovrResult _initialized;
	ovrTrackingState _ts;
	ovrSession _session;
    
    //angles for the orientation of the sensor
	double _angleX, _angleY, _angleZ;
    
    //angular velocity about the X, Y and Z axes
    double _angVel_X, _angVel_Y, _angVel_Z;

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
    
    //calculate the angular velocity about an axis given a sequence of angle
    //readings
    double calcAngVel(vector <double> &angles, int size);

public:

	OculusRiftSensor();

	~OculusRiftSensor();

	//read from the sensor and store the current orientation in 
	//variables orientX, orientY, orientZ
	void OVRread();
    
    //getter methods for the sensor's angle values
    double getAngleX();
    double getAngleY();
    double getAngleZ();
    
    //getter methods for the sensor's angular velocity values
    double getAngVelX();
    double getAngVelY();
    double getAngVelZ();
    

};