####Project Properties:

- Go to: 

__C/C++ General__ > __Paths and Symbols__ > __Include__ > __GNU C++__

 Add:

 `/opt/ros/ROS_distro/include`
 
####Setup:

Change to catkin root directory then run

	. devel/setup.bash
	. src/wandrian_run/setup.sh

####Build for testing:

	cd wandrian_run/test/
	./test.sh
	
####Running:

`roslaunch wandrian_run run.launch plan:=spiral_stc robot_size:=0.5 starting_point_x:=0.75 starting_point_y:=-0.75 is_verbose:=false`
