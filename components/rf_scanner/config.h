#pragma once

namespace esphome {
namespace rf_scanner {

enum eTx433Type {
  TX_Unknown = -1,
  TX_NEC,
  TX_DIGITAINER
};

class Config
{
  public:

  Config() = default;
  virtual ~Config() {};

  bool UseTx() const { return Tx.Type >= 0; }
  bool HasRecvPin() const { return Tx.RecvPin >= 0; }
  bool HasLedPin() const { return Tx.LedPin >= 0; }

  // Tx 433 parameters
  struct {
    eTx433Type Type = TX_Unknown;
    int RecvPin = -1;
    int LedPin  = -1;
  } Tx;
};

extern Config cfg;

}  // namespace rf_scanner
}  // namespace esphome
