
#include <ros/ros.h>
#include "psoc_arm.h"
#include "conn_interface.h"

using namespace psoc_arm;
using namespace conn;

Psoc_arm::Psoc_arm() :
    nh_(ros::NodeHandle()),
    nh_private_(ros::NodeHandle("~"))
{

    nh_private_.param<std::string>("serial_port", serialName_, "/dev/ttyUSB2");
    nh_private_.param<int>("baudrate", baudrate_, 9600);

    std::string fcu_url(serialName_.c_str());

    try {
        link = ConnInterface::open_url(fcu_url, baudrate_);
        // may be overridden by URL
    }
    catch (conn::DeviceError &ex) {
        //ROS_FATAL("FCU: %s", ex.what());
        //ros::shutdown();
        return;
    }

    link->message_received.connect(boost::bind(&Psoc_arm::receive, this, _1, _2));
    link->port_closed.connect(boost::bind(&Psoc_arm::terminate_cb, this));

    // subscriptions
    command_subscriber_ = nh_.subscribe("arm_command", 1, &Psoc_arm::commandCallback_2, this);

    // publications
    data_publisher_ = nh_.advertise<std_msgs::String>("psoc_arm_data", 1);
}

void Psoc_arm::receive(const uint8_t *bytes, ssize_t nbytes)
{

    received = true;
    char* output = new char[nbytes];
    memcpy(output, bytes, nbytes);
    this->out << std::string(output);

// todo: we will need to do some parsing here but I think we'll leave that up to marshal
   // std_msgs::String result;
   // result.data = out.str();
   // data_publisher_.publish(result);
}

// void Psoc::send(std::string command)
// {
//     const char* data = command.c_str();
//     size_t length = command.length();
//     link->send_bytes((uint8_t*)data, length);
// }

void Psoc_arm::send(uint16_t tur, uint16_t sh)
{
  uint8_t array[6];
  array[0]=0xEA;
  array[1]=0xE3;
  array[2]=tur&0xff;
  array[3]=(tur>>8)&0xff;
  array[4]=sh&0xff;
  array[5]=(sh>>8)&0xff;


  link->send_bytes(array,6);
}

void Psoc_arm::terminate_cb() 
{
}

// void Psoc::commandCallback(const std_msgs::String &command_msg)
// {
//     this->send(command_msg.data);
// }

void Psoc_arm::commandCallback_2(const rover_msgs::Arm &command_msg)
{
  this->send(command_msg.tur,command_msg.sh);
  // this->send(command_msg.lw&0xff,(command_msg.lw>>8)&0xff,command_msg.rw&0xff,(command_msg.rw>>8)&0xff,command_msg.pan&0xff,(command_msg.pan>>8)&0xff,command_msg.tilt&0xff,(command_msg.tilt>>8)&0xff,command_msg.camnum);
}
