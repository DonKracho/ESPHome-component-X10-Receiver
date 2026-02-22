#include "rf_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace rf_scanner {

static const char *const TAG = "RfSensor";

float RfSensor::get_setup_priority() const { return setup_priority::DATA; }

void RfSensor::setup() {
	parent_->register_sensor(this);
    last_ms_ = millis();	// initialize timeout counter
	clear_data();			// report status sensor offline
}

void RfSensor::loop() {
  uint32_t time_ms = millis();

  // check for sensor being offline
  if (time_ms > last_ms_ + timeout_ms_) {
    clear_data();
  }
}

void RfSensor::dump_config() {
  LOG_TEXT_SENSOR("", "RF Sensor", this);
  
  LOG_TEXT_SENSOR("  ", "Channel", this->channel_sensor_);
  LOG_TEXT_SENSOR("  ", "Command", this->command_sensor_);
  LOG_TEXT_SENSOR("  ", "Command Name", this->command_name_sensor_);
  LOG_TEXT_SENSOR("  ", "Device Pad", this->device_pad_sensor_);
  LOG_TEXT_SENSOR("  ", "Motion Pad", this->motion_pad_sensor_);
  LOG_TEXT_SENSOR("  ", "Number Pad", this->number_pad_sensor_);
}

void RfSensor::update() {
	String res;
	last_ms_ = millis();	// update timeout counter
	
	if (hasdata_) {
		status_clear_warning();
		res = "{ chn: 0x" + String(data_.addr, HEX) + ", cmd: 0x" + String(data_.cmnd, HEX) + ", utc: " + String(data_.timestamp) + " }";
		if (channel_sensor_ != nullptr)
			channel_sensor_->publish_state(std::to_string(data_.addr));
		if (command_sensor_ != nullptr)
			command_sensor_->publish_state(std::to_string(data_.cmnd));
		if (command_name_sensor_ != nullptr)
			command_name_sensor_->publish_state(key_map_[data_.cmnd].first);
		if (device_pad_sensor_ != nullptr && key_map_[data_.cmnd].second == DEVICE_PAD)
			device_pad_sensor_->publish_state(key_map_[data_.cmnd].first);
		if (motion_pad_sensor_ != nullptr && key_map_[data_.cmnd].second == MOTION_PAD)
			motion_pad_sensor_->publish_state(key_map_[data_.cmnd].first);
		if (number_pad_sensor_ != nullptr && key_map_[data_.cmnd].second == NUMBER_PAD)
			number_pad_sensor_->publish_state(key_map_[data_.cmnd].first);
        publish_state(res.c_str());
	} else {
		status_set_warning();
		res = "no activity";
		// do not publish this state in order to rtigger actuiob due to sub senseor states!
	}
}

void RfSensor::update_data(struct SensorMessage *data) {
	data_ = *data;
	hasdata_ = true;
	update();
}

void RfSensor::clear_data() {
	SensorMessage data;
	data_ = data;
	hasdata_ = false;
	update();
}

}  // namespace rf_scanner
}  // namespace esphome
