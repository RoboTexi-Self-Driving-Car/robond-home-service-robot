#!/bin/sh
xterm  -e  " roslaunch turtlebot_gazebo turtlebot_world.launch world_file:=$(rospack find world)/my_world.world " &
sleep 5
xterm  -e  " roslaunch turtlebot_gazebo amcl_demo.launch map_file:=$(rospack find world)/mymap.yaml " &
sleep 5
xterm  -e  " roslaunch turtlebot_rviz_launchers view_navigation.launch "
