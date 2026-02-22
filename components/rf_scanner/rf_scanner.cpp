#include "rf_scanner.h"
#include "tx_decoder.h"
#include "text_sensor/rf_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace rf_scanner {

Config cfg;

static const char *const TAG = "rf_scanner";

float RfScanner::get_setup_priority() const { return setup_priority::HARDWARE; }

void RfScanner::setup() {
  if (sensor_type_ == DRY_TEST) return;

  // transfer settings to global Config class which is used a wrapper do old tx_decoder code
  dpin_->setup();
  cfg.Tx.RecvPin = dpin_->get_pin();

  if (lpin_) {
    lpin_->setup();
    cfg.Tx.LedPin = lpin_->get_pin();
  }
  cfg.Tx.Type = static_cast<enum eTx433Type>(sensor_type_);
  txd.Setup();
}

void RfScanner::loop() {
  SensorMessage data;

  if (sensor_type_ == DRY_TEST) {
    static uint32_t last_us = 0;
    uint32_t time_us = millis();

    if (time_us > last_us + 10000UL) {
      ESP_LOGI(TAG, "dummy receiver inject data");
      last_us = time_us;
      scan_data(&data);
    } else {
      return;
    }
  } else {
    TXDecoder::rec scan;
    txd.Loop();
    if (txd.GetRecord(scan)) {
      data.addr = scan.address;
      data.cmnd = scan.command;
      data.timestamp = scan.timestamp;
    }
  }

  if (data.timestamp > 0L) {
    for (auto sensor : sensors_) {
      sensor->update_data(&data);
    }
  }
}

void RfScanner::dump_config() {
  ESP_LOGCONFIG(TAG, "RfSensor:");
  ESP_LOGCONFIG(TAG,"  Sensor type: %s", sensor_type_to_str(sensor_type_));
  LOG_PIN("  Data Pin: ", dpin_);
  LOG_PIN("  LED  Pin: ", lpin_);
}

const char *RfScanner::sensor_type_to_str(EnumSensorType type) {
  switch (type) {
    case DRY_TEST:
      return "DRY-TEST";
    case DIGITAINER:
      return "DIGITAINER";
    case NEC:
      return "NEC";
    default:
      return "UNKNOWN";
  }
}

bool RfScanner::scan_data(struct SensorMessage *message)
{
  static float gauge_count = 0;
  size_t sensor_count = sensors_.size();
  if (sensor_type_ == DRY_TEST && sensor_count > 0) {
    message->addr = rand() % 16;
    uint8_t index = rand() % sensors_[0]->get_keymap_size();
    message->cmnd = sensors_[0]->get_keymap_key(index);
    time_t now;
    time(&now);
    message->timestamp = now;
  }
  return true;
}

}  // namespace rf_scanner
}  // namespace esphome
