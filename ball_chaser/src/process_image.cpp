#include "ros/ros.h"
#include <sensor_msgs/Image.h>
#include "ball_chaser/DriveToTarget.h"

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    // Call the command_robot service and pass the requested motor commands
    if (!client.call(srv))
    {
	ROS_ERROR("Failed to call service command_robot");
    }
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;

    bool found_ball = false;
    int row = 0;
    int step = 0;
    int i = 0;

    for (row = 0; row < img.height && found_ball == false; row++)
    {
        for (step = 0; step < img.step && found_ball == false; ++step)
        {   
            i = (row*img.step)+step;
            if (img.data[i] == white_pixel)
            {   
                found_ball = true;   
            }
	}
    }
    if (found_ball)
    {
        // Then, identify if this pixel falls in the left, mid, or right side of the image
        int imgThird = img.width/3;
        int col = step/3;
        if (col < imgThird) 
        {
            drive_robot(0.1, 0.1); // Left
        } 
        else if (col >= imgThird && col < 2*imgThird)
        {
            drive_robot(0.5, 0.0); // Mid
        }
        else if (col >= 2*imgThird)
        {
            drive_robot(0.1, -0.1); //Right
        }
        // Depending on the white ball position, call the drive_bot function and pass velocities to it
    }
   else 
   {
        // Request a stop when there's no white ball seen by the camera
        drive_robot(0.0, 0.0); //Stop
   }
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
