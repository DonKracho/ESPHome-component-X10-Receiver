#include <Arduino.h>

#define MAX_CODES 30
#define MAX_RECORDS 20

namespace esphome {
namespace rf_scanner {

class TXDecoder {
public:
  struct rec
  {
    bool valid{false};     // record is valid
    int16_t address{0};    // address
    int16_t command{0};    // command
    time_t  timestamp{0};  // time this record was created
  };

  TXDecoder() {};
  ~TXDecoder() {};

  void Loop();
  void Setup();
  bool GetRecord(struct rec &record);
  char *Record2String(struct rec &record);
  char *Record2Debug(struct rec &record);

private:
  uint32_t little_endian(const uint32_t value, const uint8_t bits);
  bool DecodeRecord(uint64_t value, struct rec &record);
  bool DigitainerDecodeRecord(uint64_t value, struct rec &record);
  bool NecDecodeRecord(uint64_t value, struct rec &record);
  void StoreRecord(struct rec &record);
  int  mLastReadRec = -1;
  int  mLastStoredRec = -1;
  struct rec mRecords[MAX_RECORDS];
  char mBuffer[64];
};

extern TXDecoder txd;

}  // namespace rf_scanner
}  // namespace esphome
