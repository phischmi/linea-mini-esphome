#pragma once

// Non-blocking, read-only controller for the GICAR link. Plain C++, host-testable.
// Feed received bytes with on_byte(), call loop() regularly. It sends only the boot
// probe and read requests (poll, config block); there is no write path.

#include <cstddef>
#include <cstdint>

#include "gicar_protocol.h"

namespace linea_mini {
namespace gicar {

// Implemented by the ESPHome component (UART) and by the fake machine in the tests.
class Transport {
 public:
  virtual ~Transport() = default;
  virtual void write(const uint8_t *data, size_t len) = 0;
};

// Optional hook, e.g. for logging unknown or damaged frames.
class Listener {
 public:
  virtual ~Listener() = default;
  virtual void on_frame(const Frame &frame) = 0;  // every completed frame, also bad ones
};

struct MachineState {
  bool connected = false;       // valid frame within connected_timeout_ms
  float coffee_temp_c = 0;      // R payload[28..29] / 10 (updated by polls only)
  float z_temp_c = 0;           // Z frame temperature, only while brewing
  bool brew_active = false;     // lever on Brew (R flag) or Z shot flag, see update_brew_()
  bool pump = false;
  bool heating = false;
  bool steam = false;
  bool solenoid = false;
  bool standby = false;         // boiler flags == 0x00 (V, capture 12); 0x20 is a transition
  uint8_t boiler_flags = 0;     // raw payload[27]
  bool setpoint_valid = false;
  float setpoint_c = 0;         // effective setpoint from the config block
  uint8_t type_code = 0;
  char serial[16] = {0};
  uint32_t frames_ok = 0;
  uint32_t frames_bad = 0;      // bad checksum or malformed
  uint32_t last_frame_age_ms = 0;
};

enum class Phase : uint8_t { PROBING, POLLING };

class Controller {
 public:
  Controller(const Profile &profile, Transport &transport, Listener *listener = nullptr)
      : profile_(profile), transport_(transport), listener_(listener), parser_(profile) {}

  void on_byte(uint8_t byte, uint32_t now_ms);
  void loop(uint32_t now_ms);

  const MachineState &state() const { return state_; }
  Phase phase() const { return phase_; }
  const FrameParser &parser() const { return parser_; }

 private:
  void handle_frame_(const Frame &f, uint32_t now_ms);
  void send_read_(uint16_t addr, uint16_t len);
  void send_probe_();
  void start_polling_(uint32_t now_ms);
  void update_brew_(uint32_t now_ms);

  const Profile &profile_;
  Transport &transport_;
  Listener *listener_;
  FrameParser parser_;
  Frame rx_frame_;  // reused for every completed frame (no per-byte stack churn)
  MachineState state_;
  Phase phase_ = Phase::PROBING;

  bool started_ = false;
  uint32_t start_ms_ = 0;
  uint32_t last_probe_ms_ = 0;
  uint32_t last_poll_ms_ = 0;
  uint32_t last_config_ms_ = 0;
  bool config_requested_ = false;
  bool have_frame_ = false;
  uint32_t last_frame_ms_ = 0;
  bool r_brew_ = false;
  uint32_t r_brew_ms_ = 0;
  bool have_r_ = false;
  bool z_shot_ = false;
  uint32_t z_ms_ = 0;
};

}  // namespace gicar
}  // namespace linea_mini
