//    Copyright 2022 Christoph Hellmann Santos
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.

#ifndef PROXY_DEVICE_DRIVER__PROXY_DEVICE_DRIVER_HPP_
#define PROXY_DEVICE_DRIVER__PROXY_DEVICE_DRIVER_HPP_
#include <string>

#include "proxy_device_driver/visibility_control.h"
#include "base_device_driver/base_device_driver.hpp"

namespace ros2_canopen
{
class ProxyDeviceDriver : public BaseDeviceDriver
{
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr nmt_state_publisher;
  rclcpp::Publisher<ros2_canopen_interfaces::msg::COData>::SharedPtr rpdo_publisher;
  rclcpp::Subscription<ros2_canopen_interfaces::msg::COData>::SharedPtr tpdo_subscriber;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr nmt_state_reset_service;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr nmt_state_start_service;
  rclcpp::Service<ros2_canopen_interfaces::srv::CORead>::SharedPtr sdo_read_service;
  rclcpp::Service<ros2_canopen_interfaces::srv::COWrite>::SharedPtr sdo_write_service;

  std::mutex sdo_mtex;

protected:
  void on_nmt(canopen::NmtState nmt_state);

  void on_tpdo(const ros2_canopen_interfaces::msg::COData::SharedPtr msg);

  void on_rpdo(COData d);

  void on_nmt_state_reset(
    const std_srvs::srv::Trigger::Request::SharedPtr request,
    std_srvs::srv::Trigger::Response::SharedPtr response);

  void on_nmt_state_start(
    const std_srvs::srv::Trigger::Request::SharedPtr request,
    std_srvs::srv::Trigger::Response::SharedPtr response);

  void on_sdo_read(
    const ros2_canopen_interfaces::srv::CORead::Request::SharedPtr request,
    ros2_canopen_interfaces::srv::CORead::Response::SharedPtr response);

  void on_sdo_write(
    const ros2_canopen_interfaces::srv::COWrite::Request::SharedPtr request,
    ros2_canopen_interfaces::srv::COWrite::Response::SharedPtr response);

public:
  explicit ProxyDeviceDriver(const rclcpp::NodeOptions & options)
  : BaseDeviceDriver(options) {}

  void init(
    ev::Executor & exec,
    canopen::AsyncMaster & master,
    uint8_t node_id) noexcept override
  {
    RCLCPP_INFO(this->get_logger(), "Intitialising ProxyDeviceDriver");
    BaseDeviceDriver::init(exec, master, node_id);
    nmt_state_publisher = this->create_publisher<std_msgs::msg::String>(
      std::string(
        this->get_name()).append("/nmt_state").c_str(), 10);
    tpdo_subscriber = this->create_subscription<ros2_canopen_interfaces::msg::COData>(
      std::string(this->get_name()).append("/tpdo").c_str(),
      10,
      std::bind(&ProxyDeviceDriver::on_tpdo, this, std::placeholders::_1));

    rpdo_publisher = this->create_publisher<ros2_canopen_interfaces::msg::COData>(
      std::string(this->get_name()).append("/rpdo").c_str(), 10);

    nmt_state_reset_service = this->create_service<std_srvs::srv::Trigger>(
      std::string(this->get_name()).append("/nmt_reset_node").c_str(),
      std::bind(
        &ros2_canopen::ProxyDeviceDriver::on_nmt_state_reset,
        this,
        std::placeholders::_1,
        std::placeholders::_2));

    nmt_state_start_service = this->create_service<std_srvs::srv::Trigger>(
      std::string(this->get_name()).append("/nmt_start_node").c_str(),
      std::bind(
        &ros2_canopen::ProxyDeviceDriver::on_nmt_state_start,
        this,
        std::placeholders::_1,
        std::placeholders::_2));

    sdo_read_service = this->create_service<ros2_canopen_interfaces::srv::CORead>(
      std::string(this->get_name()).append("/sdo_read").c_str(),
      std::bind(
        &ros2_canopen::ProxyDeviceDriver::on_sdo_read,
        this,
        std::placeholders::_1,
        std::placeholders::_2));

    sdo_write_service = this->create_service<ros2_canopen_interfaces::srv::COWrite>(
      std::string(this->get_name()).append("/sdo_write").c_str(),
      std::bind(
        &ros2_canopen::ProxyDeviceDriver::on_sdo_write,
        this,
        std::placeholders::_1,
        std::placeholders::_2));
  }
};
}  // namespace ros2_canopen

#endif  // PROXY_DEVICE_DRIVER__PROXY_DEVICE_DRIVER_HPP_
