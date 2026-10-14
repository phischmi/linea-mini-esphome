// Host tests for the GICAR parser and the read-only controller. No test framework needed.
// Run: cmake -S tests -B build/tests && cmake --build build/tests && build/tests/test_gicar

#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include "controller.h"
#include "gicar_protocol.h"

using namespace linea_mini::gicar;

static int g_failed = 0;
static int g_checks = 0;
#define CHECK(cond)                                                              \
  do {                                                                           \
    g_checks++;                                                                  \
    if (!(cond)) {                                                               \
      g_failed++;                                                                \
      std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);                \
    }                                                                            \
  } while (0)
#define CHECK_NEAR(a, b) CHECK(std::fabs((a) - (b)) < 0.001)

// --- fixtures ------------------------------------------------------------------------

static std::vector<std::string> fixture_frames(const char *file) {
  std::vector<std::string> out;
  std::ifstream in(std::string(FIXTURE_DIR) + "/" + file);
  std::string line;
  while (std::getline(in, line)) {
    if (line.empty() || line[0] == '#')
      continue;
    out.push_back(line.substr(0, line.find(' ')));
  }
  return out;
}

static const Profile &P = kDefaultProfile;

// Feeds a whole string byte by byte, collecting completed frames.
static std::vector<Frame> parse_all(const std::string &s, uint32_t start_ms = 0) {
  FrameParser parser(P);
  std::vector<Frame> frames;
  uint32_t t = start_ms;
  for (char c : s) {
    Frame f;
    if (parser.feed(static_cast<uint8_t>(c), t, f))
      frames.push_back(f);
    t += 1;
  }
  return frames;
}

// --- tests ---------------------------------------------------------------------------

static void test_fixtures_checksums() {
  for (const char *file : {"r_frames.txt", "z_frames.txt", "config_block.txt", "acks.txt", "commands.txt"}) {
    auto frames = fixture_frames(file);
    CHECK(!frames.empty());
    for (auto &f : frames)
      CHECK(checksum_ok(f.c_str(), f.size()));
  }
}

static void test_builders() {
  char buf[32];
  size_t n = build_read(buf, sizeof(buf), 0x4000, 0x23);
  CHECK(std::string(buf, n) == "R40000023DB");
  n = build_read(buf, sizeof(buf), 0x0000, 0x20);
  CHECK(std::string(buf, n) == "R00000020D4");
  n = build_probe(buf, sizeof(buf));
  CHECK(std::string(buf, n) == "X00000000D8");
  CHECK(build_read(buf, 5, 0x4000, 0x23) == 0);  // buffer too small
}

struct RExpect {
  float temp;
  uint8_t flags;
  bool lever;
};

static void test_poll_fixtures() {
  // Order matches tests/fixtures/r_frames.txt.
  const RExpect exp[] = {{93.0f, 0x61, false}, {93.0f, 0x60, false}, {83.0f, 0x70, false}, {82.0f, 0x71, false},
                         {87.0f, 0x7B, true},  {95.0f, 0x00, false}, {95.0f, 0x20, false}};
  auto frames = fixture_frames("r_frames.txt");
  CHECK(frames.size() == sizeof(exp) / sizeof(exp[0]));
  for (size_t i = 0; i < frames.size() && i < sizeof(exp) / sizeof(exp[0]); i++) {
    auto parsed = parse_all(frames[i]);
    CHECK(parsed.size() == 1);
    if (parsed.size() != 1)
      continue;
    CHECK(parsed[0].type == FrameType::POLL_REPLY);
    CHECK(parsed[0].status == FrameStatus::OK);
    CHECK(parsed[0].len == 81);
    PollData d;
    CHECK(decode_poll(parsed[0], P, d));
    CHECK_NEAR(d.coffee_temp_c, exp[i].temp);
    CHECK(d.boiler_flags == exp[i].flags);
    CHECK(d.lever_brew == exp[i].lever);
    CHECK(d.type_code == 0x03);
    CHECK(std::strcmp(d.serial, "Sn2011010551") == 0);
  }
}

static void test_z_fixtures() {
  const float temps[] = {93.0f, 83.0f, 94.0f};
  auto frames = fixture_frames("z_frames.txt");
  CHECK(frames.size() == 3);
  for (size_t i = 0; i < frames.size() && i < 3; i++) {
    auto parsed = parse_all(frames[i]);
    CHECK(parsed.size() == 1);
    if (parsed.size() != 1)
      continue;
    CHECK(parsed[0].type == FrameType::Z_STREAM);
    CHECK(parsed[0].len == 55);
    ZData z;
    CHECK(decode_z(parsed[0], P, z));
    CHECK(z.shot_active);
    CHECK_NEAR(z.temp_c, temps[i]);
    if (i == 0)
      CHECK(z.counter == 0x90);
  }
}

static void test_config_and_reads() {
  auto frames = fixture_frames("config_block.txt");
  CHECK(frames.size() == 2);
  auto p0 = parse_all(frames[0]);
  CHECK(p0.size() == 1 && p0[0].type == FrameType::CONFIG_REPLY && p0[0].len == 75);
  ConfigData c;
  CHECK(!p0.empty() && decode_config(p0[0], P, c));
  CHECK_NEAR(c.setpoint_c, 95.0f);
  auto p1 = parse_all(frames[1]);  // R00E2 reply (4 bytes), generic read
  CHECK(p1.size() == 1 && p1[0].type == FrameType::READ_REPLY && p1[0].status == FrameStatus::OK);
}

static void test_acks_and_x() {
  auto frames = fixture_frames("acks.txt");
  CHECK(frames.size() == 8);
  size_t acks = 0;
  for (auto &f : frames) {
    auto parsed = parse_all(f);
    CHECK(parsed.size() == 1);
    if (parsed.size() != 1)
      continue;
    if (parsed[0].type == FrameType::WRITE_ACK) {
      AckData a;
      CHECK(decode_ack(parsed[0], P, a));
      CHECK(a.ok);
      acks++;
    } else {
      CHECK(parsed[0].type == FrameType::X_REPLY && parsed[0].status == FrameStatus::OK);
    }
  }
  CHECK(acks == 7);
  AckData a;
  auto p = parse_all("W00000001OK72");
  CHECK(!p.empty() && decode_ack(p[0], P, a) && a.addr == 0x0000 && a.len == 1);
  p = parse_all("W04060001OK7C");
  CHECK(!p.empty() && decode_ack(p[0], P, a) && a.addr == 0x0406);
}

static void test_bad_checksum_and_malformed() {
  std::string good = fixture_frames("r_frames.txt")[0];
  std::string bad = good;
  bad[30] = bad[30] == '0' ? '1' : '0';  // flip one payload char
  auto p = parse_all(bad);
  CHECK(p.size() == 1 && p[0].status == FrameStatus::BAD_CHECKSUM);
  PollData d;
  CHECK(p.size() == 1 && !decode_poll(p[0], P, d));  // never decode bad frames

  auto m = parse_all("R4000ZZZZ");  // 'Z' inside the header: starts a new frame, no output
  CHECK(m.empty());
  m = parse_all("R4000GGGGxx");  // bad length field
  CHECK(m.size() == 1 && m[0].status == FrameStatus::MALFORMED);
  m = parse_all("R4000FFFF");  // length beyond the sanity cap
  CHECK(m.size() == 1 && m[0].status == FrameStatus::MALFORMED);
}

static void test_resync_and_stream() {
  auto r = fixture_frames("r_frames.txt");
  auto z = fixture_frames("z_frames.txt");
  auto a = fixture_frames("acks.txt");
  // garbage, then frames, with a truncated frame in between and non-printable noise
  std::string stream = std::string("\xFC.. ") + r[0] + z[0] + "R400000" + z[1] + a[0] + r[4].substr(0, 40) + r[4];
  FrameParser parser(P);
  std::vector<Frame> frames;
  uint32_t t = 0;
  for (char c : stream) {
    Frame f;
    if (parser.feed(static_cast<uint8_t>(c), t++, f))
      frames.push_back(f);
  }
  // expected: r[0], z[0], z[1], ack, r[4]  (the truncated "R400000" and "r[4] head" are dropped)
  CHECK(frames.size() == 5);
  if (frames.size() == 5) {
    CHECK(frames[0].type == FrameType::POLL_REPLY && frames[0].status == FrameStatus::OK);
    CHECK(frames[1].type == FrameType::Z_STREAM && frames[2].type == FrameType::Z_STREAM);
    CHECK(frames[3].type == FrameType::WRITE_ACK);
    CHECK(frames[4].type == FrameType::POLL_REPLY && frames[4].status == FrameStatus::OK);
  }
  CHECK(parser.garbage_bytes() >= 4);
  CHECK(parser.dropped_partial_frames() == 2);
}

static void test_rx_timeout() {
  FrameParser parser(P);
  Frame f;
  std::string r = fixture_frames("r_frames.txt")[0];
  uint32_t t = 0;
  for (size_t i = 0; i < 30; i++)
    parser.feed(static_cast<uint8_t>(r[i]), t++, f);
  t += 1000;  // line goes silent, rest of an old frame must not complete it
  bool done = false;
  for (size_t i = 30; i < r.size(); i++)
    done |= parser.feed(static_cast<uint8_t>(r[i]), t++, f);
  CHECK(!done);
  CHECK(parser.dropped_partial_frames() >= 1);
}

// --- controller with a fake transport -----------------------------------------------

struct FakeTransport : Transport {
  std::vector<std::pair<uint32_t, std::string>> sent;
  uint32_t *clock;
  explicit FakeTransport(uint32_t *c) : clock(c) {}
  void write(const uint8_t *d, size_t n) override {
    sent.emplace_back(*clock, std::string(reinterpret_cast<const char *>(d), n));
  }
};

static void feed_str(Controller &c, const std::string &s, uint32_t &now) {
  for (char ch : s)
    c.on_byte(static_cast<uint8_t>(ch), now);
}

static size_t count_sent(const FakeTransport &t, const std::string &frame) {
  size_t n = 0;
  for (auto &s : t.sent)
    n += s.second == frame;
  return n;
}

static void test_controller_boot_and_poll() {
  uint32_t now = 0;
  FakeTransport tr(&now);
  Controller c(P, tr);
  CHECK(c.phase() == Phase::PROBING);
  for (; now < 4500; now += 10)
    c.loop(now);
  // probe sent at t=0, again every 2000 ms; no polls yet
  CHECK(count_sent(tr, "X00000000D8") == 3);
  CHECK(count_sent(tr, "R40000023DB") == 0);
  CHECK(!c.state().connected);

  feed_str(c, "X00000001D9", now);  // machine boots and answers
  CHECK(c.phase() == Phase::POLLING);
  size_t before = tr.sent.size();
  uint32_t t0 = now;
  for (; now < t0 + 3100; now += 10)
    c.loop(now);
  CHECK(count_sent(tr, "R40000023DB") == 5);  // t0, +760, +1520, +2280, +3040
  CHECK(count_sent(tr, "R00000020D4") == 1);  // config read once
  CHECK(tr.sent.size() > before);
  // poll spacing
  std::vector<uint32_t> polls;
  for (auto &s : tr.sent)
    if (s.second == "R40000023DB")
      polls.push_back(s.first);
  for (size_t i = 1; i < polls.size(); i++)
    CHECK(polls[i] - polls[i - 1] >= 760 && polls[i] - polls[i - 1] < 780);
}

static void test_controller_state() {
  uint32_t now = 0;
  FakeTransport tr(&now);
  Controller c(P, tr);
  c.loop(now);
  auto r = fixture_frames("r_frames.txt");
  auto z = fixture_frames("z_frames.txt");
  auto cfg = fixture_frames("config_block.txt");

  now = 100;
  feed_str(c, r[0], now);  // idle, heating on
  c.loop(now);
  CHECK(c.phase() == Phase::POLLING);  // a valid state reply also ends probing
  CHECK(c.state().connected);
  CHECK_NEAR(c.state().coffee_temp_c, 93.0f);
  CHECK(c.state().heating && !c.state().pump && !c.state().steam && !c.state().standby);
  CHECK(!c.state().brew_active);
  CHECK(std::strcmp(c.state().serial, "Sn2011010551") == 0);
  CHECK(c.state().type_code == 3);

  now = 200;
  feed_str(c, cfg[0], now);
  CHECK(c.state().setpoint_valid);
  CHECK_NEAR(c.state().setpoint_c, 95.0f);

  now = 900;
  feed_str(c, r[2], now);  // steam on
  CHECK(c.state().steam);

  now = 1000;
  feed_str(c, z[0], now);  // Z stream: brewing
  CHECK(c.state().brew_active);
  CHECK_NEAR(c.state().z_temp_c, 93.0f);
  c.loop(now + 250);
  CHECK(c.state().brew_active);   // still within z_recent_ms
  c.loop(now + 400);
  CHECK(!c.state().brew_active);  // Z silent, R flag still says lever off

  now = 2000;
  feed_str(c, r[4], now);  // lever on Brew, pump+solenoid
  CHECK(c.state().brew_active && c.state().pump && c.state().solenoid);
  c.loop(now + 2900);
  CHECK(c.state().brew_active);   // R flag trusted for r_stale_ms
  c.loop(now + 3100);
  CHECK(!c.state().brew_active);  // stale

  now = 10000;
  feed_str(c, r[5], now);  // standby
  CHECK(c.state().standby && !c.state().heating);
  feed_str(c, r[6], now);  // transition 0x20
  CHECK(!c.state().standby);

  // damaged frame: counted, state unchanged
  std::string bad = r[0];
  bad[20] = bad[20] == '0' ? '1' : '0';
  uint32_t bad_before = c.state().frames_bad;
  feed_str(c, bad, now);
  CHECK(c.state().frames_bad == bad_before + 1);
  CHECK(!c.state().heating);  // unchanged from r[6]

  // connection timeout
  c.loop(now + 3001);
  CHECK(!c.state().connected);
}

static void test_controller_probe_give_up_and_config_refresh() {
  uint32_t now = 0;
  FakeTransport tr(&now);
  Controller c(P, tr);
  for (; now < 8100; now += 10)
    c.loop(now);
  CHECK(c.phase() == Phase::POLLING);  // machine never answered the probe: poll anyway (O9)
  CHECK(count_sent(tr, "X00000000D8") == 4);  // t=0,2000,4000,6000

  // config re-read every 60 s while idle
  for (; now < 135000; now += 10)
    c.loop(now);
  size_t cfg = count_sent(tr, "R00000020D4");
  CHECK(cfg >= 2 && cfg <= 3);
}

static void test_controller_no_probe_profile() {
  Profile p = kDefaultProfile;
  p.send_probe = false;  // profile switch: skip the boot probe
  uint32_t now = 0;
  FakeTransport tr(&now);
  Controller c(p, tr);
  c.loop(now);
  CHECK(c.phase() == Phase::POLLING);
  CHECK(count_sent(tr, "X00000000D8") == 0);
  CHECK(count_sent(tr, "R40000023DB") == 1);
}

static void test_controller_is_read_only() {
  uint32_t now = 0;
  FakeTransport tr(&now);
  Controller c(P, tr);
  auto r = fixture_frames("r_frames.txt");
  auto z = fixture_frames("z_frames.txt");
  for (; now < 200000; now += 7) {
    c.loop(now);
    if (now % 760 < 7)
      feed_str(c, r[(now / 760) % r.size()], now);
    if (now % 91 < 7)
      feed_str(c, z[(now / 91) % z.size()], now);
  }
  CHECK(!tr.sent.empty());
  for (auto &s : tr.sent) {
    // only read requests and the boot probe may ever leave the controller
    bool read = s.second[0] == 'R' && s.second.size() == 11;
    bool probe = s.second == "X00000000D8";
    CHECK(read || probe);
    CHECK(s.second[0] != 'W');
  }
}

struct CountingListener : Listener {
  int total = 0, bad = 0;
  void on_frame(const Frame &f) override {
    total++;
    bad += f.status != FrameStatus::OK;
  }
};

static void test_listener() {
  uint32_t now = 0;
  FakeTransport tr(&now);
  CountingListener l;
  Controller c(P, tr, &l);
  std::string r = fixture_frames("r_frames.txt")[0];
  std::string bad = r;
  bad[40] = bad[40] == '0' ? '1' : '0';
  feed_str(c, r, now);
  feed_str(c, bad, now);
  CHECK(l.total == 2 && l.bad == 1);
}

int main() {
  test_fixtures_checksums();
  test_builders();
  test_poll_fixtures();
  test_z_fixtures();
  test_config_and_reads();
  test_acks_and_x();
  test_bad_checksum_and_malformed();
  test_resync_and_stream();
  test_rx_timeout();
  test_controller_boot_and_poll();
  test_controller_state();
  test_controller_probe_give_up_and_config_refresh();
  test_controller_no_probe_profile();
  test_controller_is_read_only();
  test_listener();
  std::printf("%d checks, %d failed\n", g_checks, g_failed);
  return g_failed == 0 ? 0 : 1;
}
