#include "controller.h"

namespace linea_mini {
namespace gicar {

static uint32_t elapsed(uint32_t now, uint32_t then) { return static_cast<uint32_t>(now - then); }

void Controller::send_read_(uint16_t addr, uint16_t len) {
  char buf[16];
  size_t n = build_read(buf, sizeof(buf), addr, len);
  if (n > 0)
    transport_.write(reinterpret_cast<const uint8_t *>(buf), n);
}

void Controller::send_probe_() {
  char buf[16];
  size_t n = build_probe(buf, sizeof(buf));
  if (n > 0)
    transport_.write(reinterpret_cast<const uint8_t *>(buf), n);
}

void Controller::start_polling_(uint32_t now_ms) {
  phase_ = Phase::POLLING;
  last_poll_ms_ = now_ms - profile_.poll_period_ms;  // first poll on the next loop()
  config_requested_ = false;
}

void Controller::on_byte(uint8_t byte, uint32_t now_ms) {
  if (parser_.feed(byte, now_ms, rx_frame_))
    handle_frame_(rx_frame_, now_ms);
}

void Controller::handle_frame_(const Frame &f, uint32_t now_ms) {
  if (listener_ != nullptr)
    listener_->on_frame(f);

  if (f.status != FrameStatus::OK) {
    state_.frames_bad++;
    return;
  }
  state_.frames_ok++;
  have_frame_ = true;
  last_frame_ms_ = now_ms;

  switch (f.type) {
    case FrameType::X_REPLY:
      if (phase_ == Phase::PROBING)
        start_polling_(now_ms);
      break;
    case FrameType::POLL_REPLY: {
      PollData d;
      if (!decode_poll(f, profile_, d))
        break;
      if (phase_ == Phase::PROBING)
        start_polling_(now_ms);  // machine already talks: no probe answer needed
      state_.coffee_temp_c = d.coffee_temp_c;
      state_.boiler_flags = d.boiler_flags;
      state_.heating = (d.boiler_flags & profile_.flag_heating) != 0;
      state_.pump = (d.boiler_flags & profile_.flag_pump) != 0;
      state_.solenoid = (d.boiler_flags & profile_.flag_solenoid) != 0;
      state_.steam = (d.boiler_flags & profile_.flag_steam) != 0;
      state_.standby = d.boiler_flags == profile_.flags_standby_value;
      state_.type_code = d.type_code;
      for (size_t i = 0; i < sizeof(state_.serial); i++)
        state_.serial[i] = d.serial[i];
      r_brew_ = d.lever_brew;
      r_brew_ms_ = now_ms;
      have_r_ = true;
      break;
    }
    case FrameType::CONFIG_REPLY: {
      ConfigData c;
      if (decode_config(f, profile_, c)) {
        state_.setpoint_c = c.setpoint_c;
        state_.setpoint_valid = c.setpoint_c > 0;
      }
      break;
    }
    case FrameType::Z_STREAM: {
      ZData z;
      if (!decode_z(f, profile_, z))
        break;
      state_.z_temp_c = z.temp_c;
      z_shot_ = z.shot_active;
      z_ms_ = now_ms;
      break;
    }
    default:
      break;  // ACKs and other read replies are not used in read-only mode
  }
  update_brew_(now_ms);
}

// While brewing the machine answers polls rarely (Z frames take ~99 % of the bandwidth), so
// the brew state combines two sources (U, derived from captures 02/14/15):
//  - the Z shot flag, trusted for z_recent_ms after the last Z frame
//  - the R lever flag, trusted for r_stale_ms after the last R reply
void Controller::update_brew_(uint32_t now_ms) {
  bool z = z_shot_ && elapsed(now_ms, z_ms_) <= profile_.z_recent_ms;
  bool r = have_r_ && r_brew_ && elapsed(now_ms, r_brew_ms_) <= profile_.r_stale_ms;
  state_.brew_active = z || r;
}

void Controller::loop(uint32_t now_ms) {
  if (!started_) {
    started_ = true;
    start_ms_ = now_ms;
    last_probe_ms_ = now_ms;
    last_frame_ms_ = now_ms;
    if (profile_.send_probe)
      send_probe_();
    else
      start_polling_(now_ms);
  }

  state_.connected = have_frame_ && elapsed(now_ms, last_frame_ms_) <= profile_.connected_timeout_ms;
  state_.last_frame_age_ms = have_frame_ ? elapsed(now_ms, last_frame_ms_) : 0;
  update_brew_(now_ms);

  if (phase_ == Phase::PROBING) {
    if (elapsed(now_ms, start_ms_) >= profile_.probe_give_up_ms) {
      start_polling_(now_ms);  // O9: the probe may not be needed or answered
    } else if (elapsed(now_ms, last_probe_ms_) >= profile_.probe_retry_ms) {
      last_probe_ms_ = now_ms;
      send_probe_();
    }
    if (phase_ == Phase::PROBING)
      return;
  }

  if (elapsed(now_ms, last_poll_ms_) >= profile_.poll_period_ms) {
    last_poll_ms_ = now_ms;
    send_read_(profile_.poll_addr, profile_.poll_len);
    return;
  }

  // Config block: once after start, then every config_refresh_ms while idle. Never during a
  // shot (extra traffic is unwelcome) and never in the same tick as a poll.
  bool due = !config_requested_ || elapsed(now_ms, last_config_ms_) >= profile_.config_refresh_ms;
  if (due && !state_.brew_active && elapsed(now_ms, last_poll_ms_) >= 100) {
    config_requested_ = true;
    last_config_ms_ = now_ms;
    send_read_(profile_.config_addr, profile_.config_len);
  }
}

}  // namespace gicar
}  // namespace linea_mini
