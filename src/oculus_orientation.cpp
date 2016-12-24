// oculus_orientation.cpp : Defines the entry point for the console application.
//

//#include "C:\Users\User\Documents\Visual Studio 2013\Projects\oculus_orientation\oculus_orientation\stdafx.h"
#include <iostream>
#include <iomanip>
#include <OVR_CAPI.h>
#include <windows.h>
#include <thread>

#define COLW setw(15)

using namespace std;

class OculusRift{
    
    private:
    
        ovrResult initialized;
        ovrTrackingState ts;
        ovrSession session;
        
        bool initOVR(){
            
            if (ovr_Initialize(nullptr) == ovrSuccess)
            {
                session = nullptr;
                ovrGraphicsLuid luid;
                initialized = ovr_Create(&session, &luid);
                return true;
            }
            return false;
            
        }
        
        bool closeOVR(){
            
            ovr_Destroy(session);
                
            ovr_Shutdown();
            
            return true;
            // If we've fallen through to this point, the HMD is no longer
            // connected.
        }
    
    public:
        
        double orientX, orientY, orientZ;
    
        OculusRift(){
            
            initOVR();
        }
        
        ~OculusRift(){
            
            closeOVR();
        }

        void OVRread()
        {   // Initialize our session with the Oculus HMD.

            
            int count = 0;
            double x_sum = 0.0;
            double y_sum = 0.0;
            double z_sum = 0.0;
            
            double max_count = 100000.0;
            double sum = 0;
            
                if (initialized == ovrSuccess)
                {   // Then we're connected to an HMD!

                    // Let's take a look at some orientation data.
                
                    while (count < max_count)
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
                    
                    orientX = x_sum / max_count;
                    orientY = y_sum / max_count;
                    orientZ = z_sum / max_count;
                    
                    cout << "Orientation (x,y,z):  " << COLW << orientX << ","
                    << COLW << orientY << "," << COLW << orientZ
                    << endl;
                            
                }
                
        }

}
			


