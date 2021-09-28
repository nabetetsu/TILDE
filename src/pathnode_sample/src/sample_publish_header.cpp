#include <chrono>
#include <cstdio>
#include <memory>
#include <utility>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_components/register_node_macro.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"

#include "std_msgs/msg/string.hpp"

#include "pathnode/sub_timing_advertise_node.hpp"
#include "pathnode/timing_advertise_publisher.hpp"

using namespace std::chrono_literals;

namespace pathnode_sample
{
// Create a Talker class that subclasses the generic rclcpp::Node base class.
// The main function below will instantiate the class as a ROS node.
class TalkerWithHeader : public pathnode::SubTimingAdvertiseNode
{
public:
  explicit TalkerWithHeader(const rclcpp::NodeOptions & options)
      : SubTimingAdvertiseNode("talker", options)
  {
    // Create a function for when messages are to be sent.
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    auto publish_message =
      [this]() -> void
      {
        msg_ = std::make_unique<std_msgs::msg::String>();
        msg_->data = "Hello World: " + std::to_string(count_++);
        RCLCPP_INFO(this->get_logger(), "Publishing: '%s'", msg_->data.c_str());
        // Put the message into a queue to be processed by the middleware.
        // This call is non-blocking.
        pub_->publish(std::move(msg_));

        msg_pc_ = std::make_unique<sensor_msgs::msg::PointCloud2>();
        msg_pc_->header.stamp = rclcpp::Time(0);
        pub_pc_->publish(std::move(msg_pc_));

      };
    // Create a publisher with a custom Quality of Service profile.
    rclcpp::QoS qos(rclcpp::KeepLast(7));
    pub_ = this->create_timing_advertise_publisher<std_msgs::msg::String>("chatter", qos);
    pub_pc_ = this->create_timing_advertise_publisher<sensor_msgs::msg::PointCloud2>("pc", qos);

    // Use a timer to schedule periodic message publishing.
    timer_ = this->create_wall_timer(1s, publish_message);
  }

private:
  size_t count_ = 1;
  std::unique_ptr<std_msgs::msg::String> msg_;
  std::unique_ptr<sensor_msgs::msg::PointCloud2> msg_pc_;
  pathnode::TimingAdvertisePublisher<std_msgs::msg::String>::SharedPtr pub_;
  pathnode::TimingAdvertisePublisher<sensor_msgs::msg::PointCloud2>::SharedPtr pub_pc_;
  rclcpp::TimerBase::SharedPtr timer_;
};

}  // namespace pathnode_sample

RCLCPP_COMPONENTS_REGISTER_NODE(pathnode_sample::TalkerWithHeader)