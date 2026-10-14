#pragma once

// GICAR frame parser and command builder (read-only subset). Plain C++, no ESPHome headers.
// Protocol facts and their status: docs/protocol/. Constants: protocol_profile.h.
//
// There is deliberately NO write-command builder here. Writes arrive in a later phase and
// only for registers released by the owner (see CLAUDE.md).

#include <cstddef>
#include <cstdint>

#include "protocol_profile.h"

namespace lm_gicar {

constexpr size_t kMaxFrame = 300;

enum class FrameType : uint8_t {
  UNKNOWN,       // started with a start character but could not be classified
  POLL_REPLY,    // 'R' + address 4000: machine state
  CONFIG_REPLY,  // 'R' + address 0000: config block (setpoint)
  READ_REPLY,    // any other 'R' reply (valid framing, content not decoded)
  Z_STREAM,      // autonomous 'Z' frame while brewing / backflush
  X_REPLY,       // reply to the boot probe
  WRITE_ACK,     // 'W' ... "OK"/"KO"
};

enum class FrameStatus : uint8_t {
  OK,
  BAD_CHECKSUM,
  MALFORMED,  // bad length field or non-hex where hex is required
};

struct Frame {
  FrameType type = FrameType::UNKNOWN;
  FrameStatus status = FrameStatus::MALFORMED;
  char raw[kMaxFrame + 1] = {0};
  size_t len = 0;
};

// --- Checksum and hex helpers --------------------------------------------------------
uint8_t checksum(const char *s, size_t n);          // sum of ASCII values mod 256
bool checksum_ok(const char *frame, size_t len);    // last two chars = checksum of the rest
int hex_nibble(char c);                             // -1 if not hex
int hex_byte(const char *p);                        // two chars -> 0..255, -1 if invalid
int hex_u16(const char *p);                         // four chars -> 0..65535, -1 if invalid

// --- Commands (module -> machine). Returns the length written, 0 on error. -----------
size_t build_read(char *out, size_t cap, uint16_t addr, uint16_t len);
size_t build_probe(char *out, size_t cap);  // "X00000000D8"

// --- Byte-wise frame assembler -------------------------------------------------------
// Frames always start with one of R, W, X, Z. These letters never occur inside a frame
// (payload is uppercase hex), so a start character always begins a new frame.
class FrameParser {
 public:
  explicit FrameParser(const Profile &profile) : profile_(profile) {}

  // Feed one received byte. Returns true if a complete frame is in `out`
  // (check out.status, bad frames are reported too).
  bool feed(uint8_t byte, uint32_t now_ms, Frame &out);

  uint32_t garbage_bytes() const { return garbage_bytes_; }
  uint32_t dropped_partial_frames() const { return dropped_partial_; }

 private:
  void reset_();
  bool finish_(Frame &out, bool malformed);

  const Profile &profile_;
  char buf_[kMaxFrame + 1] = {0};
  size_t len_ = 0;
  size_t expected_ = 0;
  uint32_t last_rx_ms_ = 0;
  uint32_t garbage_bytes_ = 0;
  uint32_t dropped_partial_ = 0;
};

// --- Decoders (valid frames only) ----------------------------------------------------
struct PollData {
  uint8_t type_code = 0;          // 0x03 = LINEA
  char serial[16] = {0};          // ASCII, e.g. "Sn..."
  bool lever_brew = false;        // payload[17]: follows the lever, not the pump
  uint8_t boiler_flags = 0;       // payload[27]
  float coffee_temp_c = 0;        // payload[28..29] / 10
};
struct ConfigData {
  float setpoint_c = 0;           // payload[7..8] / 10 (effective setpoint, ramps while heating)
};
struct ZData {
  bool shot_active = false;
  uint8_t counter = 0;
  float temp_c = 0;               // chars [45..48] / 10
};
struct AckData {
  uint16_t addr = 0;
  uint16_t len = 0;
  bool ok = false;
};

bool decode_poll(const Frame &f, const Profile &p, PollData &out);
bool decode_config(const Frame &f, const Profile &p, ConfigData &out);
bool decode_z(const Frame &f, const Profile &p, ZData &out);
bool decode_ack(const Frame &f, const Profile &p, AckData &out);

}  // namespace lm_gicar
