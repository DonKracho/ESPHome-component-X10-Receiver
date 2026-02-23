#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include <ctime>
#include <vector>
#include <map>

#include "../rf_scanner.h"

namespace esphome {
namespace rf_scanner {

class RfSensor : public Component, public Parented<RfScanner>, public text_sensor::TextSensor
{
 public:
  RfSensor() = default;

  void setup() override;
  void loop() override;
  void update();
  void dump_config() override;
  float get_setup_priority() const override;

  void set_channel_sensor(text_sensor::TextSensor *channel_sensor) { channel_sensor_ = channel_sensor; }
  void set_command_sensor(text_sensor::TextSensor *command_sensor) { command_sensor_ = command_sensor; }
  void set_command_name_sensor(text_sensor::TextSensor *command_name_sensor) { command_name_sensor_ = command_name_sensor; }
  void set_device_pad_sensor(text_sensor::TextSensor *device_pad_sensor) { device_pad_sensor_ = device_pad_sensor; }
  void set_motion_pad_sensor(text_sensor::TextSensor *motion_pad_sensor) { motion_pad_sensor_ = motion_pad_sensor; }
  void set_number_pad_sensor(text_sensor::TextSensor *number_pad_sensor) { number_pad_sensor_ = number_pad_sensor; }

  void update_data(struct SensorMessage *data);
  void clear_data();
  uint8_t get_keymap_size() { return key_map_.size(); }
  uint8_t get_keymap_key(int index) { uint8_t ret = 0;  int i = 0; for (auto key : key_map_) { if (index == i++) { ret = key.first; break; } } return ret; }
 
 protected:
  bool hasdata_{false};
  uint32_t last_ms_{0};  // timestamp of last reception
  uint32_t timeout_ms_{600000ul}; // 10 min
  struct SensorMessage data_;

  text_sensor::TextSensor *channel_sensor_{nullptr};
  text_sensor::TextSensor *command_sensor_{nullptr};
  text_sensor::TextSensor *command_name_sensor_{nullptr};
  text_sensor::TextSensor *device_pad_sensor_{nullptr};
  text_sensor::TextSensor *motion_pad_sensor_{nullptr};
  text_sensor::TextSensor *number_pad_sensor_{nullptr};
  
  enum pad {
    NONE,
    NAVIGATION,
    DEVICE_PAD,
    NUMBER_PAD,
    MOTION_PAD
  };

  std::map<const uint8_t, std::pair<std::string, uint8_t>> key_map_{
    { 0x02, {"Power", NONE} },

    { 0x2C, {"TV", DEVICE_PAD} },
    { 0x2D, {"Video", DEVICE_PAD} },
    { 0x04, {"CD/DVD", DEVICE_PAD} },

    { 0x16, {"Text", DEVICE_PAD} },
    { 0x06, {"Audio", DEVICE_PAD} },
    { 0x2E, {"Radio", DEVICE_PAD} },

    { 0x31, {"EPG", DEVICE_PAD} },
    { 0x05, {"Photo", DEVICE_PAD} },
    { 0x2F, {"Info", DEVICE_PAD} },

    { 0x19, {"Menu", NAVIGATION} },

    { 0x1D, {"Left", NAVIGATION} },
    { 0x78, {"Up", NAVIGATION} },
    { 0x79, {"Up", NAVIGATION} },
    { 0x7A, {"Up", NAVIGATION} },
    { 0x7B, {"Up", NAVIGATION} },
    { 0x7C, {"Up", NAVIGATION} },
    { 0x1E, {"OK", NAVIGATION} },
    { 0x70, {"Down", NAVIGATION} },
    { 0x71, {"Down", NAVIGATION} },
    { 0x72, {"Down", NAVIGATION} },
    { 0x73, {"Down", NAVIGATION} },
    { 0x74, {"Down", NAVIGATION} },
    { 0x1F, {"Right", NAVIGATION} },

    { 0x20, {"Back", NAVIGATION} },

    { 0x09, {"Vol+", NONE} },
    { 0x08, {"Vol-", NONE} },
    { 0x1B, {"Select", NONE} },
    { 0x0B, {"Prg+", NONE} },
    { 0x0C, {"Prg-", NONE} },

    { 0x00, {"Mute", NONE} },
    { 0x1C, {"Last", NONE} },

    { 0x32, {"Red/Audio", NONE} },
    { 0x33, {"Green/Subtitle", NONE} },
    { 0x34, {"Yellow/Angel", NONE} },
    { 0x35, {"Blue/Title", NONE} },

    { 0x28, {"Stop", MOTION_PAD} },
    { 0x29, {"Pause", MOTION_PAD} },
    { 0x25, {"Play", MOTION_PAD} },

    { 0x21, {"Step-", MOTION_PAD} },
    { 0x18, {"Snapshot", NONE} },
    { 0x23, {"Step+", MOTION_PAD} },

    { 0x24, {"Rewind", MOTION_PAD} },
    { 0x27, {"Record", MOTION_PAD} },
    { 0x26, {"Forward", MOTION_PAD} },

    { 0x0D, {"1", NUMBER_PAD} },
    { 0x0E, {"2", NUMBER_PAD} },
    { 0x0F, {"3", NUMBER_PAD} },
 
    { 0x10, {"4", NUMBER_PAD} },
    { 0x11, {"5", NUMBER_PAD} },
    { 0x12, {"6", NUMBER_PAD} },
 
    { 0x13, {"7", NUMBER_PAD} },
    { 0x14, {"8", NUMBER_PAD} },
    { 0x15, {"9", NUMBER_PAD} },
 
    { 0x17, {"0", NUMBER_PAD} },
  };
};

}  // namespace rf_scanner
}  // namespace esphome
