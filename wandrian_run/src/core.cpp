/*
 * core.cpp
 *
 *  Created on: Jul 31, 2015
 *      Author: sontd
 */

#include <kobuki_msgs/MotorPower.h>
#include <kobuki_msgs/KeyboardInput.h>
#include <ecl/time.hpp>
#include "../include/core.hpp"

namespace wandrian {

Core::Core() :
		is_bumper_pressed(false), current_position(new Point(0, 0)), current_orientation(
				new Vector(0, 1)), twist(new geometry_msgs::Twist()), linear_vel_step(
				0), linear_vel_max(0), angular_vel_step(0), angular_vel_max(0), robot_size(
				0), starting_point_x(0), starting_point_y(0), is_verbose(false), is_quitting(
				false), is_powered(false), is_zero_vel(true), is_logging(false), file_descriptor(
				0) {
	tcgetattr(file_descriptor, &terminal); // get terminal properties
}

Core::~Core() {
	tcsetattr(file_descriptor, TCSANOW, &terminal);
}

bool Core::initialize() {
	ros::NodeHandle nh("~");

	nh.getParam("is_verbose", is_verbose);
	nh.getParam("plan", plan);
	nh.getParam("robot_size", robot_size);
	nh.getParam("starting_point_x", starting_point_x);
	nh.getParam("starting_point_y", starting_point_y);

	nh.getParam("linear_vel_step", linear_vel_step);
	nh.getParam("linear_vel_max", linear_vel_max);
	nh.getParam("angular_vel_step", angular_vel_step);
	nh.getParam("angular_vel_max", angular_vel_max);

	ROS_INFO_STREAM("[Launch]: Using arg is_verbose(" << is_verbose << ")");
	ROS_INFO_STREAM("[Launch]: Using arg plan(" << plan << ")");
	ROS_INFO_STREAM("[Launch]: Using arg robot_size(" << robot_size << ")");
	ROS_INFO_STREAM(
			"[Launch]: Using arg starting_point_x(" << starting_point_x << ")");
	ROS_INFO_STREAM(
			"[Launch]: Using arg starting_point_y(" << starting_point_y << ")");

	ROS_INFO_STREAM(
			"[Launch]: Using param linear_vel_step(" << linear_vel_step << ")");
	ROS_INFO_STREAM(
			"[Launch]: Using param linear_vel_max(" << linear_vel_max << ")");
	ROS_INFO_STREAM(
			"[Launch]: Using param angular_vel_step(" << angular_vel_step << ")");
	ROS_INFO_STREAM(
			"[Launch]: Using param angular_vel_max(" << angular_vel_max << ")");

	motor_power_publisher = nh.advertise<kobuki_msgs::MotorPower>("motor_power",
			1);
	velocity_publisher = nh.advertise<geometry_msgs::Twist>("velocity", 1);
	odom_subscriber = nh.subscribe<nav_msgs::Odometry>("odom", 1,
			&Core::subscribeOdometry, this);
	bumper_subscriber = nh.subscribe<kobuki_msgs::BumperEvent>("bumper", 1,
			&Core::subscribeBumper, this);

	twist->linear.x = 0.0;
	twist->linear.y = 0.0;
	twist->linear.z = 0.0;
	twist->angular.x = 0.0;
	twist->angular.y = 0.0;
	twist->angular.z = 0.0;

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
			ROS_FATAL_STREAM(
					"[Connection]: Could not connect, trying again after 500ms...");
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
		ROS_ERROR("[Connection]: Could not connect.");
	} else {
		kobuki_msgs::MotorPower motor_power;
		motor_power.state = kobuki_msgs::MotorPower::ON;
		motor_power_publisher.publish(motor_power);
		ROS_INFO("[Connection]: Connected.");
		is_powered = true;
	}

	// start keyboard input thread
	threadKeyboard.start(&Core::startThreadKeyboard, *this);
	return true;
}

void Core::spin() {
	ros::Rate loop_rate(10);

	while (!is_quitting && ros::ok()) {
		// Avoid spamming robot with continuous zero-velocity messages
		if ((twist->linear.x != 0.0) || (twist->linear.y != 0.0)
				|| (twist->linear.z != 0.0) || (twist->angular.x != 0.0)
				|| (twist->angular.y != 0.0) || (twist->angular.z != 0.0)) {
			velocity_publisher.publish(twist);
			is_zero_vel = false;
		} else if (!is_zero_vel) {
			velocity_publisher.publish(twist);
			is_zero_vel = true;
		}
		ros::spinOnce();
		loop_rate.sleep();
	}
	if (is_quitting) { // ros node is still ok, send a disable command
		disablePower();
	} else {
		// just in case we got here not via a keyboard quit request
		is_quitting = true;
		threadKeyboard.cancel();
		threadRun.cancel();
	}
	threadKeyboard.join();
}

void Core::run() {
	// Override this method
}

void Core::stop() {
	twist->linear.x = 0.0;
	twist->angular.z = 0.0;
	velocity_publisher.publish(twist);
}

void Core::startThreadKeyboard() {
	struct termios raw;
	memcpy(&raw, &terminal, sizeof(struct termios));

	raw.c_lflag &= ~(ICANON | ECHO);
	// Setting a new line, then end of file
	raw.c_cc[VEOL] = 1;
	raw.c_cc[VEOF] = 2;
	tcsetattr(file_descriptor, TCSANOW, &raw);

	puts("Available commands");
	puts("---------------------------");
	puts("p: Toggle motor power.");
	puts("l: Toggle logging.");
	puts("r: Start running.");
	puts("q: Quit.");
	char c;
	while (!is_quitting) {
		if (read(file_descriptor, &c, 1) < 0) {
			perror("Read char failed():");
			exit(-1);
		}
		processKeyboardInput(c);
	}
}

void Core::processKeyboardInput(char c) {
	switch (c) {
	case kobuki_msgs::KeyboardInput::KeyCode_Down:
	case kobuki_msgs::KeyboardInput::KeyCode_Up:
	case kobuki_msgs::KeyboardInput::KeyCode_Right:
	case kobuki_msgs::KeyboardInput::KeyCode_Left:
		if (is_powered) {
			if (c == kobuki_msgs::KeyboardInput::KeyCode_Down
					&& twist->linear.x >= -linear_vel_max) { // decrease linear vel
				twist->linear.x -= linear_vel_step;
			} else if (c == kobuki_msgs::KeyboardInput::KeyCode_Up
					&& twist->linear.x <= linear_vel_max) { // increase linear vel
				twist->linear.x += linear_vel_step;
			} else if (c == kobuki_msgs::KeyboardInput::KeyCode_Right
					&& twist->angular.z >= -angular_vel_max) { // decrease angular vel
				twist->angular.z -= angular_vel_step;
			} else if (c == kobuki_msgs::KeyboardInput::KeyCode_Left
					&& twist->angular.z <= angular_vel_max) { // increase angular vel
				twist->angular.z += angular_vel_step;
			}
			ROS_INFO_STREAM(
					"[Vel]: (" << twist->linear.x << ", " << twist->angular.z << ")");
		} else {
			ROS_FATAL_STREAM("[Power]: Disabled");
		}
		break;
	case 'p':
		if (is_powered)
			disablePower();
		else
			enablePower();
		break;
	case 'l':
		is_logging = !is_logging;
		ROS_INFO_STREAM("[Logging]: " << (is_logging ? "On" : "Off"));
		break;
	case 'r':
		ROS_INFO_STREAM("[Run]: " << "Start running");
		threadRun.start(&Core::startThreadRun, *this);
		break;
	case 'q':
		is_quitting = true;
		break;
	default:
		break;
	}
}

void Core::startThreadRun() {
	run();
}

void Core::enablePower() {
	stop();
	ROS_INFO("[Power]: Enabled");
	kobuki_msgs::MotorPower power;
	power.state = kobuki_msgs::MotorPower::ON;
	motor_power_publisher.publish(power);
	is_powered = true;
}

void Core::disablePower() {
	stop();
	ROS_INFO("[Power]: Disabled");
	kobuki_msgs::MotorPower power;
	power.state = kobuki_msgs::MotorPower::OFF;
	motor_power_publisher.publish(power);
	is_powered = false;
}

void Core::subscribeOdometry(const nav_msgs::OdometryConstPtr& odom) {
	double px = odom->pose.pose.position.x;
	double py = odom->pose.pose.position.y;
	double ow = odom->pose.pose.orientation.w;
	double ox = odom->pose.pose.orientation.x;
	double oy = odom->pose.pose.orientation.y;
	double oz = odom->pose.pose.orientation.z;
	current_position->x = px + starting_point_x;
	current_position->y = py + starting_point_y;
	// FIXME: [Tmp]: Set initial orientation to (1, 0)
	current_orientation->x = ow * ow - oz * oz;
	current_orientation->y = 2 * oz * ow;
	if (is_logging && is_verbose)
		ROS_INFO_STREAM(
				"[Odom]: Pos(" << current_position->x << "," << current_position->y << "); " << "Ori(" << current_orientation->x << "," << current_orientation->y << ")");
}

void Core::subscribeBumper(const kobuki_msgs::BumperEventConstPtr& bumper) {
	if (is_logging) {
		std::string state;
		switch (bumper->state) {
		case kobuki_msgs::BumperEvent::PRESSED:
			is_bumper_pressed = true;
			state = "Pressed";
			break;
		case kobuki_msgs::BumperEvent::RELEASED:
			is_bumper_pressed = false;
			state = "Released";
			break;
		default:
			state = "Unknown";
			break;
		}
		ROS_WARN_STREAM("[Bumper]: State(" << state << ")");
	}
}

}
