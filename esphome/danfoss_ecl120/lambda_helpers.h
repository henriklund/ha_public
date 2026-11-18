#pragma once

#include <string>
#include <cstdint>
#include <cstdlib>

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
inline std::string format_schedule(const std::string &x) {

  auto read_dword = [&](int offset) -> uint32_t {
    std::string hex = x.substr(offset, 8);
    return (uint32_t) strtoul(hex.c_str(), nullptr, 16);
  };

  auto format_time = [](uint32_t seconds) -> std::string {
    char buffer[12];
    snprintf(buffer, sizeof(buffer), "%02u:%02u",
            (unsigned int)(seconds / 3600),
            (unsigned int)((seconds % 3600) / 60));
    return std::string(buffer);
  };

  uint32_t p1_start = read_dword(0);
  uint32_t p1_end   = read_dword(8);
  uint32_t p2_start = read_dword(16);
  uint32_t p2_end   = read_dword(24);
  uint32_t p3_start = read_dword(32);
  uint32_t p3_end   = read_dword(40);

  if (p1_end == 0)
    return "No schedule";

  std::string result = "Periode1: " +
      format_time(p1_start) + "-" + format_time(p1_end);

  if (p1_end < 86399) {
    result += " / Periode2: " +
        format_time(p2_start) + "-" + format_time(p2_end);

    if (p2_end < 86399) {
      result += " / Periode3: " +
          format_time(p3_start) + "-" + format_time(p3_end);
    }
  }

  return result;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
inline std::string format_runtime(uint32_t seconds) {
  unsigned long days = seconds / 86400;
  unsigned int hours = (seconds % 86400) / 3600;
  unsigned int minutes = (seconds % 3600) / 60;

  char buffer[32];
  snprintf(buffer, sizeof(buffer),
          "%lud %02uh %02um",
          days, hours, minutes);

  return std::string(buffer);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
inline std::string sensor_temperature_status(int x) {
          switch (x) {
            case 0:
              return "OK";
            case 1:
              return "INVALID";
            case 2:
              return "OPEN CIRCUIT";
            case 3:
              return "SHORT CIRCUIT";
            default:
              return "Unknown";
          }
}
