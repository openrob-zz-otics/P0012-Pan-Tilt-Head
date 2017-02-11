#include <dynamixel_sdk.h>                                  // Uses Dynamixel SDK library


#define TORQUE_ENABLE                   1                   // Value for enabling the torque
#define TORQUE_DISABLE                  0                   // Value for disabling the torque

int getch();

typedef enum {
    RX_24_F,
    MX_28_T,
    
}servo_type;

namespace RX_24F{
    
    //control table addresses for an RX_24F servo
    const int addr_torque_enable = 24;
    const int addr_goal_position = 30;
    const int addr_present_position = 36;
    const int addr_torque_limit_high = 35;
    const int addr_torque_limit_low = 34;
    const int addr_max_torque_high = 15;
    const int addr_max_torque_low = 14;
}

/**
namespace MX_28T{
    
    //control table addresses for an MX_28T servo
    const int addr_torque_enable = 24;
    const int addr_goal_position = 
    
    //TODO fill the rest of the table
}

**/

class Servo {
    
private:
    
    dynamixel::PortHandler *portHandler;
    dynamixel::PacketHandler *packetHandler;
    
    servo_type dxl_type;
    
    int dxl_id;
    double protocol_ver;
    int baudrate;
    
    //control table for this dynamixel servo instance
    int addr_torque_enable;
    int addr_goal_position;
    int addr_present_position;
    int addr_torque_limit_low;
    int addr_torque_limit_high;
    int addr_max_torque_high;
    int addr_max_torque_low;
    
    
    void openPort();
    void setBaudrate(int baudrate);
    void enableTorque();
    void disableTorque();
       

public:

    Servo();
    
    Servo(int dxl_id, 
             double protocol_ver, 
             servo_type type, 
             int baudrate,
             int init_torque, 
             int init_position, 
             char device_name []);
    
    ~Servo();
    
    
    void setTorque(int goal_torque);
    int readPosition();
    void writePosition(int goal_position);
    

};


