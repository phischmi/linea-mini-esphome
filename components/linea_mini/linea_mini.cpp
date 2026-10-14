#include "linea_mini.h"

#include <cmath>
#include <cstring>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace linea_mini {

static const char *const TAG = "linea_mini";

void LineaMiniComponent::setup() {
  // The controller starts on its first loop() call; nothing to do on the bus here.
}

void LineaMiniComponent::loop() {
  const uint32_t now = millis();
  uint8_t byte;
  while (this->available() && this->read_byte(&byte))
    this->controller_.on_byte(byte, now);
  this->controller_.loop(now);
  this->publish_state_();
}

void LineaMiniComponent::write(const uint8_t *data, size_t len) {
  ESP_LOGV(TAG, "TX %.*s", (int) len, reinterpret_cast<const char *>(data));
  this->write_array(data, len);
}

// Copies a frame with non-printable bytes replaced, so damaged frames are safe to log.
static void printable(const ::lm_gicar::Frame &f, char *out, size_t cap) {
  size_t n = f.len < cap - 1 ? f.len : cap - 1;
  for (size_t i = 0; i < n; i++)
    out[i] = (f.raw[i] >= 32 && f.raw[i] < 127) ? f.raw[i] : '.';
  out[n] = '\0';
}

void LineaMiniComponent::on_frame(const ::lm_gicar::Frame &frame) {
  using ::lm_gicar::FrameStatus;
  using ::lm_gicar::FrameType;
  char text[::lm_gicar::kMaxFrame + 1];
  if (frame.status != FrameStatus::OK) {
    printable(frame, text, sizeof(text));
    ESP_LOGW(TAG, "Damaged frame (%s, %u chars): %s",
             frame.status == FrameStatus::BAD_CHECKSUM ? "bad checksum" : "malformed", (unsigned) frame.len, text);
    return;
  }
  if (frame.type == FrameType::READ_REPLY || frame.type == FrameType::UNKNOWN) {
    printable(frame, text, sizeof(text));
    ESP_LOGD(TAG, "Undecoded frame (%u chars): %s", (unsigned) frame.len, text);
    return;
  }
  if (esp_log_level_get(TAG) >= ESP_LOG_VERBOSE) {
    printable(frame, text, sizeof(text));
    ESP_LOGV(TAG, "RX %s", text);
  }
}

void LineaMiniComponent::publish_state_() {
  const auto &s = this->controller_.state();
  const bool up = s.connected;

  if (this->coffee_temperature_sensor_ != nullptr) {
    float v = up ? s.coffee_temp_c : NAN;
    if (!this->published_once_ || !(v == this->last_coffee_ || (std::isnan(v) && std::isnan(this->last_coffee_)))) {
      this->last_coffee_ = v;
      this->coffee_temperature_sensor_->publish_state(v);
    }
  }
  if (this->setpoint_temperature_sensor_ != nullptr) {
    float v = (up && s.setpoint_valid) ? s.setpoint_c : NAN;
    if (!this->published_once_ || !(v == this->last_setpoint_ || (std::isnan(v) && std::isnan(this->last_setpoint_)))) {
      this->last_setpoint_ = v;
      this->setpoint_temperature_sensor_->publish_state(v);
    }
  }
  if (this->frames_ok_sensor_ != nullptr && (!this->published_once_ || s.frames_ok != this->last_ok_)) {
    this->last_ok_ = s.frames_ok;
    this->frames_ok_sensor_->publish_state(s.frames_ok);
  }
  if (this->frames_bad_sensor_ != nullptr && (!this->published_once_ || s.frames_bad != this->last_bad_)) {
    this->last_bad_ = s.frames_bad;
    this->frames_bad_sensor_->publish_state(s.frames_bad);
  }

  // Binary sensors drop their duplicates themselves. Without a link all machine flags read false.
  if (this->connected_binary_sensor_ != nullptr)
    this->connected_binary_sensor_->publish_state(up);
  if (this->brewing_binary_sensor_ != nullptr)
    this->brewing_binary_sensor_->publish_state(up && s.brew_active);
  if (this->pump_binary_sensor_ != nullptr)
    this->pump_binary_sensor_->publish_state(up && s.pump);
  if (this->heating_binary_sensor_ != nullptr)
    this->heating_binary_sensor_->publish_state(up && s.heating);
  if (this->steam_binary_sensor_ != nullptr)
    this->steam_binary_sensor_->publish_state(up && s.steam);
  if (this->standby_binary_sensor_ != nullptr)
    this->standby_binary_sensor_->publish_state(up && s.standby);

  if (this->serial_number_text_sensor_ != nullptr && s.serial[0] != '\0' &&
      std::strncmp(s.serial, this->last_serial_, sizeof(this->last_serial_)) != 0) {
    std::memcpy(this->last_serial_, s.serial, sizeof(this->last_serial_));
    this->serial_number_text_sensor_->publish_state(s.serial);
  }
  this->published_once_ = true;
}

void LineaMiniComponent::dump_config() {
  ESP_LOGCONFIG(TAG,
                "Linea Mini (read-only):\n"
                "  Boot probe: %s\n"
                "  Poll period: %u ms",
                YESNO(this->profile_.send_probe), (unsigned) this->profile_.poll_period_ms);
  LOG_SENSOR("  ", "Coffee temperature", this->coffee_temperature_sensor_);
  LOG_SENSOR("  ", "Setpoint temperature", this->setpoint_temperature_sensor_);
  LOG_SENSOR("  ", "Frames OK", this->frames_ok_sensor_);
  LOG_SENSOR("  ", "Frames bad", this->frames_bad_sensor_);
  LOG_BINARY_SENSOR("  ", "Connected", this->connected_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Brewing", this->brewing_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Pump", this->pump_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Heating", this->heating_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Steam", this->steam_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Standby", this->standby_binary_sensor_);
  LOG_TEXT_SENSOR("  ", "Serial number", this->serial_number_text_sensor_);
}

}  // namespace linea_mini
}  // namespace esphome
