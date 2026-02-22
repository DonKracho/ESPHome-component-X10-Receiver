#pragma once

#define RF_SCANNER
#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "config.h"
#include <string>

namespace esphome {
namespace rf_scanner {

class RfSensor;

enum EnumSensorType {
  DRY_TEST  = -1,
  NEC  = TX_NEC,
  DIGITAINER = TX_DIGITAINER,
};

struct SensorMessage
{
  int16_t addr{-1};
  int16_t cmnd{-1};
  time_t  timestamp{0};
};

class RfScanner : public Component
{
 public:
  RfScanner() = default;

  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override;
  void register_sensor(RfSensor *sensor) { sensors_.push_back(sensor); }
  
  void set_datapin(InternalGPIOPin *pin) { dpin_ = pin; };
  void set_ledpin(InternalGPIOPin *pin)  { lpin_ = pin; };
  void set_sensor_type(EnumSensorType type) { sensor_type_ = type; }
 
  protected:
  const char *sensor_type_to_str(EnumSensorType type);
  EnumSensorType sensor_type_{DIGITAINER};

  bool scan_data(struct SensorMessage *message);
  std::vector<RfSensor*> sensors_;
  InternalGPIOPin *dpin_{nullptr};
  InternalGPIOPin *lpin_{nullptr};
};

}  // namespace rf_scanner
}  // namespace esphome
