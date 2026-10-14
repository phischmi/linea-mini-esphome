#pragma once

// All protocol constants of the GICAR link in one place. If a machine deviates from
// the documented protocol (docs/protocol/), adjust a profile here, not the logic.
// Every value carries the status from docs/protocol/: V = verified, U = unverified.
//
// Plain C++ without ESPHome headers so it can be unit-tested on the host.

#include <cstddef>
#include <cstdint>

namespace linea_mini {
namespace gicar {

struct Profile {
  // --- Timing -------------------------------------------------------------------------
  uint32_t poll_period_ms;       // V: 760 ms (faster corrupts frames while brewing)
  uint32_t rx_timeout_ms;        // U: a partial frame older than this is junk
  uint32_t connected_timeout_ms; // U: no valid frame for this long -> not connected
  uint32_t probe_retry_ms;       // U: boot probe repeat interval
  uint32_t probe_give_up_ms;     // U: start polling anyway after this long without X reply (O9)
  uint32_t config_refresh_ms;    // U: idle re-read of the config block
  uint32_t z_recent_ms;          // U: a Z frame counts as "shot running" for this long
  uint32_t r_stale_ms;           // U: an R-frame brew flag is trusted for this long

  // --- Commands (module -> machine) ---------------------------------------------------
  bool send_probe;               // V: original module sends X00000000D8 once after boot
  uint16_t poll_addr;            // V: 0x4000
  uint16_t poll_len;             // V: 0x23 (35 bytes)
  uint16_t config_addr;          // V: 0x0000
  uint16_t config_len;           // V: 0x20 (32 bytes)

  // --- Frame lengths (characters, incl. checksum) -------------------------------------
  size_t x_reply_len;            // V: 11  ("X00000001D9")
  size_t z_len;                  // V: 55
  size_t w_ack_len;              // V: 13  ("W00000001OK72")
  size_t max_frame_len;          // sanity cap for the R length field

  // --- R poll reply: payload byte offsets (payload starts at character 9) -------------
  size_t r_type_byte;            // V: 0 (0x03 = LINEA)
  size_t r_serial_first_byte;    // V: 3
  size_t r_serial_len;           // V: 12 (ASCII "Sn...")
  size_t r_brew_byte;            // V: 17 (0x01 = lever on "Brew")
  size_t r_flags_byte;           // V: 27
  size_t r_temp_byte;            // V: 28, uint16 big-endian
  uint16_t r_temp_divisor;       // V: 10
  uint8_t r_brew_value;          // V: 0x01

  // --- Boiler flag bits (r_flags_byte) ------------------------------------------------
  uint8_t flag_heating;          // V: 0x01
  uint8_t flag_pump;             // V: 0x02
  uint8_t flag_solenoid;         // V: 0x08
  uint8_t flag_steam;            // V: 0x10
  uint8_t flags_standby_value;   // V: 0x00 (0x20 = transition, not standby)

  // --- Config block reply (address 0x0000) --------------------------------------------
  size_t cfg_setpoint_byte;      // V: 7, uint16 big-endian
  uint16_t cfg_setpoint_divisor; // V: 10

  // --- Z frame: character offsets (frame starts with "Z6") ---------------------------
  size_t z_shot_char;            // V: 34 (two hex chars; 0x10 brewing, 0x00 stopped)
  size_t z_counter_char;         // V: 40
  size_t z_temp_char;            // V: 45 (four hex chars, divisor 10; NOT 46 / 160)
  uint16_t z_temp_divisor;       // V: 10
};

// Defaults from docs/protocol/ (reference machine: board firmware v2.12).
inline constexpr Profile kDefaultProfile = {
    /*poll_period_ms*/ 760,
    /*rx_timeout_ms*/ 200,
    /*connected_timeout_ms*/ 3000,
    /*probe_retry_ms*/ 2000,
    /*probe_give_up_ms*/ 8000,
    /*config_refresh_ms*/ 60000,
    /*z_recent_ms*/ 300,
    /*r_stale_ms*/ 3000,
    /*send_probe*/ true,
    /*poll_addr*/ 0x4000,
    /*poll_len*/ 0x23,
    /*config_addr*/ 0x0000,
    /*config_len*/ 0x20,
    /*x_reply_len*/ 11,
    /*z_len*/ 55,
    /*w_ack_len*/ 13,
    /*max_frame_len*/ 300,
    /*r_type_byte*/ 0,
    /*r_serial_first_byte*/ 3,
    /*r_serial_len*/ 12,
    /*r_brew_byte*/ 17,
    /*r_flags_byte*/ 27,
    /*r_temp_byte*/ 28,
    /*r_temp_divisor*/ 10,
    /*r_brew_value*/ 0x01,
    /*flag_heating*/ 0x01,
    /*flag_pump*/ 0x02,
    /*flag_solenoid*/ 0x08,
    /*flag_steam*/ 0x10,
    /*flags_standby_value*/ 0x00,
    /*cfg_setpoint_byte*/ 7,
    /*cfg_setpoint_divisor*/ 10,
    /*z_shot_char*/ 34,
    /*z_counter_char*/ 40,
    /*z_temp_char*/ 45,
    /*z_temp_divisor*/ 10,
};

}  // namespace gicar
}  // namespace linea_mini
