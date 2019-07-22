#include <ros/ros.h>
#include <visualization_msgs/Marker.h>
#include <nav_msgs/Odometry.h>

const float dist_thres = 0.4;

// max tries to receive goal from goal_sub
// /target topic from pick_objects
// if fails to receive, uses hard coded goal
// after each try, sleeps for 0.5 seconds.
const int max_wait_goal = 5;
const bool use_goal_pub = false;

class AddMarkers {
public:
  AddMarkers();
  void publishMarkers();

private:
  ros::NodeHandle n;

  ros::Publisher marker_pub;

  ros::Subscriber goal_sub;
  ros::Subscriber odom_sub;

  geometry_msgs::Pose odom;

  geometry_msgs::Pose goal;

  visualization_msgs::Marker marker;

  bool set_pickup;
  bool set_dropoff;
  bool set_timer;

  ros::Time timer_start;

  bool reach_target(const geometry_msgs::Pose &pos, const geometry_msgs::Pose &target);

  void goalCallback(const geometry_msgs::Pose &msg);

  void odomCallback(const nav_msgs::Odometry::ConstPtr &msg);
};

AddMarkers::AddMarkers() {
  marker_pub = n.advertise<visualization_msgs::Marker>("visualization_marker", 1);

  goal_sub = n.subscribe("target", 1, &AddMarkers::goalCallback, this);
  odom_sub = n.subscribe("odom", 1, &AddMarkers::odomCallback, this);

  ROS_INFO("Waiting for a goal location");

  set_pickup = false;
  set_dropoff = false;
  set_timer = false;

  // Set our initial shape type to be a cube
  uint32_t shape = visualization_msgs::Marker::CUBE;

  // Set the frame ID and timestamp.  See the TF tutorials for information on these.
  marker.header.frame_id = "map";
  marker.header.stamp = ros::Time::now();
  marker.ns = "add_markers";
  marker.id = 0;
  marker.type = shape;

  // Set the pose of the marker.  This is a full 6DOF pose relative to the frame/time specified in the header
  marker.pose.position.x = 0.0;
  marker.pose.position.y = 0.0;
  marker.pose.position.z = 0;
  marker.pose.orientation.x = 0.0;
  marker.pose.orientation.y = 0.0;
  marker.pose.orientation.z = 0.0;
  marker.pose.orientation.w = 0.0;

  // Set the scale of the marker -- 1x1x1 here means 1m on a side
  marker.scale.x = 0.4;
  marker.scale.y = 0.4;
  marker.scale.z = 0.4;

  // Set the color -- be sure to set alpha to something non-zero!
  marker.color.r = 0.3f;
  marker.color.g = 0.5f;
  marker.color.b = 0.7f;
  marker.color.a = 0.0;
}

bool AddMarkers::reach_target(const geometry_msgs::Pose &pos, const geometry_msgs::Pose &target) {
  float dx = pos.position.x - target.position.x;
  float dy = pos.position.y - target.position.y;
  float dz = pos.position.z - target.position.z;
  float dist = sqrtf(dx * dx + dy * dy + dz * dz);
  return (dist < dist_thres);
}

void AddMarkers::goalCallback(const geometry_msgs::Pose &msg) {
  goal.position.x = msg.position.x;
  goal.position.y = msg.position.y;
  goal.position.z = msg.position.z;
  goal.orientation.x = msg.orientation.x;
  goal.orientation.y = msg.orientation.y;
  goal.orientation.z = msg.orientation.z;
  goal.orientation.w = msg.orientation.w;

  // display the marker in the pickup zone
  // Set the pose of the marker.
  marker.pose.position.x = goal.position.x;
  marker.pose.position.y = goal.position.y;
  marker.pose.orientation.w = goal.orientation.w;

  if(!set_pickup) {
    ROS_INFO("Obtained the goal position to pick up the object");
    set_pickup = true;
  } else {
    ROS_INFO("Obtained the goal position to drop off the object");
    set_dropoff = true;
  }
}

void AddMarkers::odomCallback(const nav_msgs::Odometry::ConstPtr &msg) {
  odom.position.x = msg->pose.pose.position.x;
  odom.position.y = msg->pose.pose.position.y;
  odom.position.z = msg->pose.pose.position.z;
  odom.orientation.x = msg->pose.pose.orientation.x;
  odom.orientation.y = msg->pose.pose.orientation.y;
  odom.orientation.z = msg->pose.pose.orientation.z;
  odom.orientation.w = msg->pose.pose.orientation.w;

  if(!set_pickup && !set_dropoff) {
    marker.color.a = 0.0;
  } else if (set_pickup && !set_dropoff) {
    marker.color.a = 1.0;
    if(!set_timer && reach_target(odom, goal)) {
      set_timer = true;
      timer_start = ros::Time::now();
    } else if (set_timer && ros::Time::now().toSec() - timer_start.toSec() < 5.0) {
      ros::Duration(0.02).sleep();
    } else {

    }
  } else if (set_pickup && set_dropoff) {
    if(!reach_target(odom, goal)) {
      marker.color.a = 0.0;
    } else {
      marker.color.a = 1.0;
    }
  }
}

void AddMarkers::publishMarkers() {
  marker_pub.publish(marker);
}

int main( int argc, char** argv ) {
  ros::init(argc, argv, "add_markers");
  ROS_INFO("Display markers for the pick up and drop off.");

  AddMarkers addMarkers;
  ros::Rate rate(50);

  while(ros::ok()) {
    addMarkers.publishMarkers();
    ros::spinOnce();
  }


  return 0;
}
