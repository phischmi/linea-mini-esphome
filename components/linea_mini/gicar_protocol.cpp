#include "gicar_protocol.h"

#include <cstdio>
#include <cstring>

namespace lm_gicar {

// --- helpers -------------------------------------------------------------------------

uint8_t checksum(const char *s, size_t n) {
  uint32_t sum = 0;
  for (size_t i = 0; i < n; i++)
    sum += static_cast<uint8_t>(s[i]);
  return static_cast<uint8_t>(sum & 0xFF);
}

int hex_nibble(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  return -1;
}

int hex_byte(const char *p) {
  int hi = hex_nibble(p[0]), lo = hex_nibble(p[1]);
  return (hi < 0 || lo < 0) ? -1 : (hi << 4) | lo;
}

int hex_u16(const char *p) {
  int hi = hex_byte(p), lo = hex_byte(p + 2);
  return (hi < 0 || lo < 0) ? -1 : (hi << 8) | lo;
}

bool checksum_ok(const char *frame, size_t len) {
  if (len < 3)
    return false;
  int want = hex_byte(frame + len - 2);
  return want >= 0 && static_cast<uint8_t>(want) == checksum(frame, len - 2);
}

// --- builders ------------------------------------------------------------------------

static size_t finish_command(char *out, size_t cap, size_t n) {
  if (n + 3 > cap)
    return 0;
  std::snprintf(out + n, cap - n, "%02X", checksum(out, n));
  return n + 2;
}

size_t build_read(char *out, size_t cap, uint16_t addr, uint16_t len) {
  int n = std::snprintf(out, cap, "R%04X%04X", addr, len);
  return n < 0 ? 0 : finish_command(out, cap, static_cast<size_t>(n));
}

size_t build_probe(char *out, size_t cap) {
  int n = std::snprintf(out, cap, "X%04X%04X", 0, 0);
  return n < 0 ? 0 : finish_command(out, cap, static_cast<size_t>(n));
}

// --- FrameParser ---------------------------------------------------------------------

static bool is_start(uint8_t c) { return c == 'R' || c == 'W' || c == 'X' || c == 'Z'; }

void FrameParser::reset_() {
  len_ = 0;
  expected_ = 0;
}

bool FrameParser::finish_(Frame &out, bool malformed) {
  std::memcpy(out.raw, buf_, len_);
  out.raw[len_] = '\0';
  out.len = len_;
  out.type = FrameType::UNKNOWN;
  switch (buf_[0]) {
    case 'R': {
      int addr = len_ >= 5 ? hex_u16(buf_ + 1) : -1;
      out.type = addr == profile_.poll_addr     ? FrameType::POLL_REPLY
                 : addr == profile_.config_addr ? FrameType::CONFIG_REPLY
                                                : FrameType::READ_REPLY;
      break;
    }
    case 'Z': out.type = FrameType::Z_STREAM; break;
    case 'X': out.type = FrameType::X_REPLY; break;
    case 'W': out.type = FrameType::WRITE_ACK; break;
  }
  if (malformed)
    out.status = FrameStatus::MALFORMED;
  else
    out.status = checksum_ok(out.raw, out.len) ? FrameStatus::OK : FrameStatus::BAD_CHECKSUM;
  reset_();
  return true;
}

bool FrameParser::feed(uint8_t c, uint32_t now_ms, Frame &out) {
  if (len_ > 0 && static_cast<uint32_t>(now_ms - last_rx_ms_) > profile_.rx_timeout_ms) {
    dropped_partial_++;
    reset_();
  }
  last_rx_ms_ = now_ms;

  if (is_start(c)) {
    if (len_ > 0)
      dropped_partial_++;
    reset_();
  } else if (len_ == 0) {
    garbage_bytes_++;
    return false;
  }

  if (len_ >= kMaxFrame) {  // cannot happen with a sane max_frame_len; resync
    dropped_partial_++;
    reset_();
    return false;
  }
  buf_[len_++] = static_cast<char>(c);

  if (len_ == 1) {
    switch (c) {
      case 'X': expected_ = profile_.x_reply_len; break;
      case 'Z': expected_ = profile_.z_len; break;
      case 'W': expected_ = profile_.w_ack_len; break;
      default: expected_ = 0; break;  // 'R': length comes with the header
    }
  } else if (buf_[0] == 'R' && len_ == 9) {
    int n = hex_u16(buf_ + 5);  // payload length in bytes
    size_t total = n < 0 ? 0 : 9 + 2 * static_cast<size_t>(n) + 2;
    if (total == 0 || total > profile_.max_frame_len || total > kMaxFrame)
      return finish_(out, true);
    expected_ = total;
  }

  if (expected_ != 0 && len_ == expected_)
    return finish_(out, false);
  return false;
}

// --- decoders ------------------------------------------------------------------------

static bool payload_byte(const Frame &f, size_t index, uint8_t &out) {
  size_t pos = 9 + 2 * index;
  if (pos + 2 > f.len - 2)
    return false;
  int v = hex_byte(f.raw + pos);
  if (v < 0)
    return false;
  out = static_cast<uint8_t>(v);
  return true;
}

static bool payload_u16(const Frame &f, size_t index, uint16_t &out) {
  uint8_t hi, lo;
  if (!payload_byte(f, index, hi) || !payload_byte(f, index + 1, lo))
    return false;
  out = static_cast<uint16_t>((hi << 8) | lo);
  return true;
}

bool decode_poll(const Frame &f, const Profile &p, PollData &out) {
  if (f.type != FrameType::POLL_REPLY || f.status != FrameStatus::OK)
    return false;
  PollData d;
  uint8_t b;
  uint16_t t;
  if (!payload_byte(f, p.r_type_byte, d.type_code) || !payload_byte(f, p.r_brew_byte, b) ||
      !payload_byte(f, p.r_flags_byte, d.boiler_flags) || !payload_u16(f, p.r_temp_byte, t))
    return false;
  d.lever_brew = b == p.r_brew_value;
  d.coffee_temp_c = static_cast<float>(t) / p.r_temp_divisor;
  size_t n = p.r_serial_len < sizeof(d.serial) - 1 ? p.r_serial_len : sizeof(d.serial) - 1;
  for (size_t i = 0; i < n; i++) {
    uint8_t c;
    if (!payload_byte(f, p.r_serial_first_byte + i, c))
      return false;
    d.serial[i] = (c >= 32 && c < 127) ? static_cast<char>(c) : '?';
  }
  out = d;
  return true;
}

bool decode_config(const Frame &f, const Profile &p, ConfigData &out) {
  if (f.type != FrameType::CONFIG_REPLY || f.status != FrameStatus::OK)
    return false;
  uint16_t v;
  if (!payload_u16(f, p.cfg_setpoint_byte, v))
    return false;
  out.setpoint_c = static_cast<float>(v) / p.cfg_setpoint_divisor;
  return true;
}

bool decode_z(const Frame &f, const Profile &p, ZData &out) {
  if (f.type != FrameType::Z_STREAM || f.status != FrameStatus::OK || f.len != p.z_len)
    return false;
  int shot = hex_byte(f.raw + p.z_shot_char);
  int counter = hex_byte(f.raw + p.z_counter_char);
  int temp = hex_u16(f.raw + p.z_temp_char);
  if (shot < 0 || counter < 0 || temp < 0)
    return false;
  out.shot_active = shot != 0;
  out.counter = static_cast<uint8_t>(counter);
  out.temp_c = static_cast<float>(temp) / p.z_temp_divisor;
  return true;
}

bool decode_ack(const Frame &f, const Profile &p, AckData &out) {
  if (f.type != FrameType::WRITE_ACK || f.status != FrameStatus::OK || f.len != p.w_ack_len)
    return false;
  int addr = hex_u16(f.raw + 1), len = hex_u16(f.raw + 5);
  if (addr < 0 || len < 0)
    return false;
  out.addr = static_cast<uint16_t>(addr);
  out.len = static_cast<uint16_t>(len);
  out.ok = f.raw[9] == 'O' && f.raw[10] == 'K';
  return true;
}

}  // namespace lm_gicar
