/*
 * core.cpp
 *
 *  Created on: Jul 31, 2015
 *      Author: sontd
 */

#include "../include/core.hpp"
#include <kobuki_msgs/MotorPower.h>
#include <ecl/time.hpp>

namespace wandrian {

Core::Core() :
		last_zero_vel_sent(true), accept_incoming(true), power_status(false), cmd(
				new geometry_msgs::Twist()), quit_requested(false), key_file_descriptor(
				0) {
	tcgetattr(key_file_descriptor, &original_terminal_state); // get terminal properties
}

Core::~Core() {
	tcsetattr(key_file_descriptor, TCSANOW, &original_terminal_state);
}

bool Core::init() {
	ros::NodeHandle nh("~");

	ROS_INFO_STREAM("Core: ");

	velocity_publisher = nh.advertise<geometry_msgs::Twist>("cmd_vel", 1);
	motor_power_publisher = nh.advertise<kobuki_msgs::MotorPower>("motor_power",
			1);

	cmd->linear.x = 1.0;
	cmd->linear.y = 0.0;
	cmd->linear.z = 0.0;
	cmd->angular.x = 0.0;
	cmd->angular.y = 0.0;
	cmd->angular.z = 0.0;

	ecl::MilliSleep millisleep;
	int count = 0;
	bool connected = false;
	while (!connected) {
		if (motor_power_publisher.getNumSubscribers() > 0) {
			connected = true;
			break;
		}
		if (count == 6) {
			connected = false;
			break;
		} else {
			ROS_WARN_STREAM("KeyOp: could not connect, trying again after 500ms...");
			try {
				millisleep(500);
			} catch (ecl::StandardException &e) {
				ROS_ERROR_STREAM("Waiting has been interrupted.");
				ROS_DEBUG_STREAM(e.what());
				return false;
			}
			++count;
		}
	}
	if (!connected) {
		ROS_ERROR("KeyOp: could not connect.");
		ROS_ERROR("KeyOp: check remappings for enable/disable topics).");
	} else {
		kobuki_msgs::MotorPower power_cmd;
		power_cmd.state = kobuki_msgs::MotorPower::ON;
		motor_power_publisher.publish(power_cmd);
		ROS_INFO("KeyOp: connected.");
		power_status = true;
	}

	// start keyboard input thread
	thread.start(&Core::keyboardInputLoop, *this);
	return true;
}

void Core::spin() {
	ros::Rate loop_rate(10);

	while (!quit_requested && ros::ok()) {
		// Avoid spamming robot with continuous zero-velocity messages
		if ((cmd->linear.x != 0.0) || (cmd->linear.y != 0.0)
				|| (cmd->linear.z != 0.0) || (cmd->angular.x != 0.0)
				|| (cmd->angular.y != 0.0) || (cmd->angular.z != 0.0)) {
			velocity_publisher.publish(cmd);
			last_zero_vel_sent = false;
		} else if (last_zero_vel_sent == false) {
			velocity_publisher.publish(cmd);
			last_zero_vel_sent = true;
		}
		accept_incoming = true;
		ros::spinOnce();
		loop_rate.sleep();
	}
	if (quit_requested) { // ros node is still ok, send a disable command
		disable();
	} else {
		// just in case we got here not via a keyboard quit request
		quit_requested = true;
		thread.cancel();
	}
	thread.join();
}

void Core::keyboardInputLoop() {
	struct termios raw;
	memcpy(&raw, &original_terminal_state, sizeof(struct termios));

	raw.c_lflag &= ~(ICANON | ECHO);
	// Setting a new line, then end of file
	raw.c_cc[VEOL] = 1;
	raw.c_cc[VEOF] = 2;
	tcsetattr(key_file_descriptor, TCSANOW, &raw);

	puts("Reading from keyboard");
	puts("---------------------------");
	puts("q : quit.");
	char c;
	while (!quit_requested) {
		if (read(key_file_descriptor, &c, 1) < 0) {
			perror("read char failed():");
			exit(-1);
		}
		processKeyboardInput(c);
	}
}

void Core::remoteKeyInputReceived(const kobuki_msgs::KeyboardInput& key) {
	processKeyboardInput(key.pressedKey);
}

void Core::processKeyboardInput(char c) {
	switch (c) {
	case 'q': {
		quit_requested = true;
		break;
	}
	default: {
		break;
	}
	}
}

void Core::disable() {
	cmd->linear.x = 0.0;
	cmd->angular.z = 0.0;
	velocity_publisher.publish(cmd);
	accept_incoming = false;

	if (power_status) {
		ROS_INFO(
				"KeyOp: die, die, die (disabling power to the device's motor system).");
		kobuki_msgs::MotorPower power_cmd;
		power_cmd.state = kobuki_msgs::MotorPower::OFF;
		motor_power_publisher.publish(power_cmd);
		power_status = false;
	} else {
		ROS_WARN("KeyOp: Motor system has already been powered down.");
	}
}

}
