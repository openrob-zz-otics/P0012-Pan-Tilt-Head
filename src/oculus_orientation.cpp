// oculus_orientation.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <iomanip>
#include <OVR_CAPI.h>
#include <windows.h>
#include <thread>

#define COLW setw(15)

using namespace std;

int main()
{   // Initialize our session with the Oculus HMD.

	int count = 0;
	double x_sum = 0.0;
	double y_sum = 0.0;
	double z_sum = 0.0;
	double max_count = 100000.0;
	double sum = 0;

	if (ovr_Initialize(nullptr) == ovrSuccess)
	{
		ovrSession session = nullptr;
		ovrGraphicsLuid luid;
		ovrResult result = ovr_Create(&session, &luid);

		if (result == ovrSuccess)
		{   // Then we're connected to an HMD!

			// Let's take a look at some orientation data.
			ovrTrackingState ts;

			while (true)
			{
				//cout << "success";
				//this_thread::sleep_for(chrono::seconds(1));
				Sleep(150);

				ts = ovr_GetTrackingState(session, 0, true);

				ovrPoseStatef tempHeadPose = ts.HeadPose;
				ovrPosef tempPose = tempHeadPose.ThePose;
				ovrQuatf tempOrient = tempPose.Orientation;

				/*
				cout << "Orientation (x,y,z):  " << COLW << tempOrient.x << ","
				<< COLW << tempOrient.y << "," << COLW << tempOrient.z
				<< endl;
				*/

				// Wait a bit to let us actually read stuff.
				//std::this_thread::sleep_for(std::chrono::milliseconds(100));
				/*
				count++;
				x_sum = x_sum + tempOrient.x;
				y_sum = y_sum + tempOrient.y;
				z_sum = z_sum + tempOrient.z;
				//if (count == max_count) {
					cout << "Orientation (x,y,z):  " << COLW << x_sum / max_count << ","
						<< COLW << y_sum / max_count << "," << COLW << z_sum / max_count
						<< endl;
					count = 0;
					x_sum = 0.0;
					y_sum = 0.0;
					z_sum = 0.0;
				//}
				*/
				cout << "Orientation (x,y,z):  " << COLW << tempOrient.x << ","
					 << COLW << tempOrient.y << "," << COLW << tempOrient.z
					<< endl;
			}

			ovr_Destroy(session);
		}
		ovr_Shutdown();
		// If we've fallen through to this point, the HMD is no longer
		// connected.
	}

	return 0;
}

