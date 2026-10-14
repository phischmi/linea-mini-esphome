#pragma once

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"

#include "controller.h"
#include "protocol_profile.h"

namespace esphome {
namespace linea_mini {

// Read-only ESPHome front end of the GICAR controller (controller.h). All protocol logic
// lives in the host-tested plain C++ files; this class only moves bytes and publishes states.
class LineaMiniComponent : public Component,
                           public uart::UARTDevice,
                           public ::linea_mini::gicar::Transport,
                           public ::linea_mini::gicar::Listener {
 public:
  LineaMiniComponent() : controller_(profile_, *this, this) {}

  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_send_probe(bool send_probe) { this->profile_.send_probe = send_probe; }

  SUB_SENSOR(coffee_temperature)
  SUB_SENSOR(setpoint_temperature)
  SUB_SENSOR(frames_ok)
  SUB_SENSOR(frames_bad)
  SUB_BINARY_SENSOR(connected)
  SUB_BINARY_SENSOR(brewing)
  SUB_BINARY_SENSOR(pump)
  SUB_BINARY_SENSOR(heating)
  SUB_BINARY_SENSOR(steam)
  SUB_BINARY_SENSOR(standby)
  SUB_TEXT_SENSOR(serial_number)

  // gicar::Transport
  void write(const uint8_t *data, size_t len) override;
  // gicar::Listener
  void on_frame(const ::linea_mini::gicar::Frame &frame) override;

 protected:
  void publish_state_();

  // profile_ must be declared before controller_ (the controller keeps a reference to it).
  ::linea_mini::gicar::Profile profile_ = ::linea_mini::gicar::kDefaultProfile;
  ::linea_mini::gicar::Controller controller_;

  // Last published values, to publish only on change.
  bool published_once_{false};
  float last_coffee_{NAN};
  float last_setpoint_{NAN};
  uint32_t last_ok_{0};
  uint32_t last_bad_{0};
  char last_serial_[16]{0};
};

}  // namespace linea_mini
}  // namespace esphome
