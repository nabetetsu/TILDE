// Copyright 2021 Research Institute of Systems Planning, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <utility>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"
#include "std_msgs/msg/string.hpp"
#include "rosgraph_msgs/msg/clock.hpp"

#include "tilde/tilde_node.hpp"
#include "tilde/tilde_publisher.hpp"
#include "tilde_msg/msg/pub_info.hpp"

using tilde::TildeNode;
using tilde::InputInfo;
using tilde_msg::msg::PubInfo;

class TestTildeNode : public ::testing::Test
{
public:
  void SetUp() override
  {
    rclcpp::init(0, nullptr);
  }

  void TearDown() override
  {
    rclcpp::shutdown();
  }
};

std::string str(const builtin_interfaces::msg::Time & time)
{
  std::string s = "";
  s += std::to_string(time.sec);
  s += ".";
  s += std::to_string(time.nanosec);
  return s;
}

/**
 * consider the following system:
 *   sensor_node -> main_node -> checker_node
 *
 * Check main_node PubInfo stamps by checker_node.
 * The type of message is PointCloud2.
 */
TEST_F(TestTildeNode, simple_case) {
  rclcpp::NodeOptions options;
  options.append_parameter_override("use_sim_time", true);
  auto sensor_node = std::make_shared<rclcpp::Node>("sensorNode", options);
  auto main_node = std::make_shared<TildeNode>("pubNode", options);
  auto checker_node = std::make_shared<rclcpp::Node>("checkerNode", options);

  auto sensor_pub = sensor_node->create_publisher<sensor_msgs::msg::PointCloud2>(
    "in_topic", 1);
  auto clock_pub = sensor_node->create_publisher<rosgraph_msgs::msg::Clock>(
    "/clock", 1);

  // apply "/clock"
  rosgraph_msgs::msg::Clock clock_msg;
  clock_msg.clock.sec = 123;
  clock_msg.clock.nanosec = 456;

  clock_pub->publish(clock_msg);
  rclcpp::spin_some(sensor_node);
  rclcpp::spin_some(main_node);

  // prepare pub/sub
  auto main_pub = main_node->create_tilde_publisher<sensor_msgs::msg::PointCloud2>(
    "out_topic", 1);
  auto main_sub = main_node->create_tilde_subscription<sensor_msgs::msg::PointCloud2>(
    "in_topic", 1,
    [&main_pub](sensor_msgs::msg::PointCloud2::UniquePtr msg) -> void
    {
      std::cout << "main_sub_callback" << std::endl;
      (void)msg;
      main_pub->publish(std::move(msg));
    });

  bool checker_sub_called = false;
  auto checker_sub = checker_node->create_subscription<tilde_msg::msg::PubInfo>(
    "out_topic/info/pub", 1,
    [&checker_sub_called, clock_msg](tilde_msg::msg::PubInfo::UniquePtr pub_info_msg) -> void
    {
      checker_sub_called = true;
      std::cout << "checker_sub_callback" << std::endl;
      std::cout << "pub_info_msg: \n" <<
        "pub_time: " << str(pub_info_msg->output_info.pub_time) << "\n" <<
        "pub_time_steady: " << str(pub_info_msg->output_info.pub_time_steady) << "\n" <<
        std::endl;
      EXPECT_EQ(
        pub_info_msg->output_info.pub_time,
        clock_msg.clock);
      EXPECT_EQ(
        pub_info_msg->output_info.has_header_stamp,
        true);
    });

  // do scenario
  auto sensor_msg = std::make_unique<sensor_msgs::msg::PointCloud2>();
  sensor_msg->header.stamp = sensor_node->now();
  sensor_pub->publish(std::move(sensor_msg));

  rclcpp::spin_some(sensor_node);
  rclcpp::spin_some(main_node);
  rclcpp::spin_some(checker_node);
  EXPECT_TRUE(checker_sub_called);
}

/**
 * consider the following system:
 *   sensor_node -> main_node -> checker_node
 *
 * Check main_node PubInfo stamps by checker_node.
 * The type of message is std_msgs::msg::String.
 */
TEST_F(TestTildeNode, no_header_case) {
  rclcpp::NodeOptions options;
  options.append_parameter_override("use_sim_time", true);
  auto sensor_node = std::make_shared<rclcpp::Node>("sensorNode", options);
  auto main_node = std::make_shared<TildeNode>("pubNode", options);
  auto checker_node = std::make_shared<rclcpp::Node>("checkerNode", options);

  auto sensor_pub = sensor_node->create_publisher<std_msgs::msg::String>(
    "in_topic", 1);
  auto clock_pub = sensor_node->create_publisher<rosgraph_msgs::msg::Clock>(
    "/clock", 1);

  // apply "/clock"
  rosgraph_msgs::msg::Clock clock_msg;
  clock_msg.clock.sec = 123;
  clock_msg.clock.nanosec = 456;

  clock_pub->publish(clock_msg);
  rclcpp::spin_some(sensor_node);
  rclcpp::spin_some(main_node);

  // prepare pub/sub
  auto main_pub = main_node->create_tilde_publisher<std_msgs::msg::String>(
    "out_topic", 1);
  auto main_sub = main_node->create_tilde_subscription<std_msgs::msg::String>(
    "in_topic", 1,
    [&main_pub](std_msgs::msg::String::UniquePtr msg) -> void
    {
      std::cout << "main_sub_callback" << std::endl;
      (void)msg;
      main_pub->publish(std::move(msg));
    });

  bool checker_sub_called = false;
  auto checker_sub = checker_node->create_subscription<tilde_msg::msg::PubInfo>(
    "out_topic/info/pub", 1,
    [&checker_sub_called, clock_msg](tilde_msg::msg::PubInfo::UniquePtr pub_info_msg) -> void
    {
      checker_sub_called = true;
      std::cout << "checker_sub_callback" << std::endl;
      std::cout << "pub_info_msg: \n" <<
        "pub_time: " << str(pub_info_msg->output_info.pub_time) << "\n" <<
        "pub_time_steady: " << str(pub_info_msg->output_info.pub_time_steady) << "\n" <<
        std::endl;
      EXPECT_EQ(
        pub_info_msg->output_info.pub_time,
        clock_msg.clock);
      EXPECT_EQ(
        pub_info_msg->output_info.has_header_stamp,
        false);
    });

  // do scenario
  auto msg = std::make_unique<std_msgs::msg::String>();
  sensor_pub->publish(std::move(msg));

  rclcpp::spin_some(sensor_node);
  rclcpp::spin_some(main_node);
  rclcpp::spin_some(checker_node);
  EXPECT_TRUE(checker_sub_called);
}

TEST_F(TestTildeNode, enable_tilde) {
  rclcpp::NodeOptions options;
  options.append_parameter_override("use_sim_time", true);
  options.append_parameter_override("enable_tilde", false);

  auto sensor_node = std::make_shared<rclcpp::Node>("sensorNode", options);
  auto main_node = std::make_shared<TildeNode>("pubNode", options);
  auto checker_node = std::make_shared<rclcpp::Node>("checkerNode", options);

  bool enable_tilde = true;
  main_node->get_parameter("enable_tilde", enable_tilde);
  EXPECT_EQ(enable_tilde, false);

  auto sensor_pub = sensor_node->create_publisher<sensor_msgs::msg::PointCloud2>(
    "in_topic", 1);
  auto clock_pub = sensor_node->create_publisher<rosgraph_msgs::msg::Clock>(
    "/clock", 1);

  // apply "/clock"
  rosgraph_msgs::msg::Clock clock_msg;
  clock_msg.clock.sec = 123;
  clock_msg.clock.nanosec = 456;

  clock_pub->publish(clock_msg);
  rclcpp::spin_some(sensor_node);
  rclcpp::spin_some(main_node);

  // prepare pub/sub
  auto main_pub = main_node->create_tilde_publisher<sensor_msgs::msg::PointCloud2>(
    "out_topic", 1);
  auto main_sub = main_node->create_tilde_subscription<sensor_msgs::msg::PointCloud2>(
    "in_topic", 1,
    [&main_pub](sensor_msgs::msg::PointCloud2::UniquePtr msg) -> void
    {
      std::cout << "main_sub_callback" << std::endl;
      (void)msg;
      main_pub->publish(std::move(msg));
    });

  bool checker_sub_called = false;
  auto checker_sub = checker_node->create_subscription<tilde_msg::msg::PubInfo>(
    "out_topic/info/pub", 1,
    [&checker_sub_called, clock_msg](tilde_msg::msg::PubInfo::UniquePtr pub_info_msg) -> void
    {
      (void) pub_info_msg;
      checker_sub_called = true;
      EXPECT_TRUE(false);  // expect never comes here
    });

  // do scenario
  auto sensor_msg = std::make_unique<sensor_msgs::msg::PointCloud2>();
  sensor_msg->header.stamp = sensor_node->now();
  sensor_pub->publish(std::move(sensor_msg));

  rclcpp::spin_some(sensor_node);
  rclcpp::spin_some(main_node);
  rclcpp::spin_some(checker_node);
  EXPECT_EQ(checker_sub_called, false);
}