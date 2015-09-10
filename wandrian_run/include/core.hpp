/*
 * core.hpp
 *
 *  Created on: Jul 31, 2015
 *      Author: sontd
 */

#ifndef WANDRIAN_RUN_INCLUDE_COMMON_CORE_HPP_
#define WANDRIAN_RUN_INCLUDE_COMMON_CORE_HPP_

#include <termios.h> // for keyboard input
#include <ros/ros.h>
#include <ecl/threads.hpp>
#include <geometry_msgs/Twist.h> // for velocity commands
#include <nav_msgs/Odometry.h>
#include <kobuki_msgs/BumperEvent.h>
#include <yocs_controllers/default_controller.hpp> // not use but need for bumper event subscriber

namespace wandrian {

class Core {

public:
	Core();
	~Core();
	bool init();
	void spin();

private:
	bool is_verbose;

	bool is_quitting;
	bool is_powered;
	bool is_zero_vel; // avoid zero-vel messages from the beginning
	bool is_logging;
	int file_descriptor;
	double linear_vel_step, linear_vel_max, angular_vel_step, angular_vel_max;

	struct termios terminal;
	ecl::Thread thread;

	geometry_msgs::TwistPtr twist;
	ros::Publisher motor_power_publisher;
	ros::Publisher velocity_publisher;
	ros::Subscriber odom_subscriber;
	ros::Subscriber bumper_subscriber;

	void keyboardInputLoop();
	void processKeyboardInput(char);

	void enablePower();
	void disablePower();
	void subscribeOdometry(const nav_msgs::Odometry::ConstPtr&);
	void subscribeBumper(const kobuki_msgs::BumperEvent::ConstPtr&);
};

}

#endif /* WANDRIAN_RUN_INCLUDE_COMMON_CORE_HPP_ */
