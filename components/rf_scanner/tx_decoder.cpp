/*
  Digitainer RF Ferbedienung (433MHz ASK Modutlation)

  Ein Sendeimpuls (Modulation aktiv) ist 560 us lang gefolgt von einer Pause (Modulation ausgeschaltet)
  Eine Puls Pause Folge entspricht einem Bit oder Sync Impuls
  die Länge der Pause ist hier definiert zu:
     _
  0 | |_      550H / 560L = 1120 us
     _
  1 | |_ _ _  560H / 1680L = 2240 us
     ________
  S |        |_ _ _ _ _ 4480H / 2800L = 7280 us AGC Sync (Beginn eines Datenpaketes)

  Protokoll: 21 Bit pro Daten Paket mit MSB immer 0
  T 1 bit toggling with every press of the same key
  z 7 bit checksum
  c 7 bit command
  a 4 bit address
  0TzzzzzzzTcccccccaaaa
*/  

#include "tx_decoder.h"
#include "config.h"
#include "esphome/core/log.h"
#include <ctime>

namespace esphome {
namespace rf_scanner {

TXDecoder txd;

static const char *const TAG = "tx_decoder";

// global variables to be accessed from interrupt handler and Loop
uint64_t Codes[MAX_CODES];
volatile bool dataReady = false;
volatile byte CodeBeg = 0;
volatile byte CodeEnd = 0;
volatile bool scanning = false;

byte CodesIndexIncrement(byte curr)
{
  // increments index of CodesBuffer
  if (++curr >= MAX_CODES) curr = 0;
  return curr;
}

// interrupt service routines for GTech 433MHz transmitters

void IRAM_ATTR DigitainerRecvSignal()
{
  static unsigned long lastRise = 0;
  static unsigned long lastTime = 0;
  static unsigned long lastSync = 0;
  static uint64_t code = 0;
  static byte id = 0;
  static byte cnt = 0;
  bool valid = false;
  bool recv_pin = digitalRead(cfg.Tx.RecvPin);

  unsigned long time_us = micros();

  if (recv_pin) {                               // rising edge
    lastRise = time_us;                         // store for measuring active modulation pulse length
    if (time_us - lastSync > 140000ul) {
      CodeBeg = CodeEnd;
    }
  } else {                                      // falling edge
    unsigned long duration = time_us - lastRise;
    if (duration > 500ul && duration < 650ul) { // detected pulse between 500 and 650us
      // check for valid pulse length
      duration = lastRise - lastTime;           // calculate duration to last rising edge
      lastTime = lastRise;                      // store current rising edge
      if (duration > 800ul && duration < 2600ul) {
        code <<= 1;                             // 0 detected
        if (duration > 1600ul) {                // 1 detected
          code |= 1;                            // just put a 1 to LSB
        }
        valid = true;
        cnt++;
      } else {                                  // wrong length or sync
        if (duration >= 7000ul) {               // sync detected
          if (cnt == 20) {                      // rereived data of 20 bit length
            Codes[id] = code;
            id = CodesIndexIncrement(id);
            CodeEnd = id;                       // set current id one beind last valid for loop()
            valid = true;
            dataReady = true;                   // loop() has to analyze last transmissions
            lastSync = time_us;
          }
          cnt = 0;                              // reset count and code
          code = 0;
        }
      }
    } 
  }
  if (cfg.HasLedPin()) digitalWrite(cfg.Tx.LedPin, !valid); // LED signals stable reception with long flashes for > 6x120 ms
  //if (cfg.HasLedPin()) digitalWrite(D4, dataReady); // for logic analyzer
}

void IRAM_ATTR NecRecvSignal()
{
  static unsigned long lastRise = 0;
  static unsigned long lastTime = 0;
  static unsigned long lastSync = 0;
  static uint64_t code = 0;
  static byte id = 0;
  static byte cnt = 0;
  bool valid = false;

  unsigned long time_us = micros();

  if (!digitalRead(cfg.Tx.RecvPin)) {           // rising edge
    lastRise = time_us;                         // store for measuring active modulation pulse length
    if (time_us - lastSync > 100000ul) {
      CodeBeg = CodeEnd;
    }
  } else {                                      // falling edge
    unsigned long duration = time_us - lastRise;
    if (duration > 500ul && duration < 650ul) { // detected pulse between 500 and 650us
      // check for valid pulse length
      duration = lastRise - lastTime;           // calculate duration to last rising edge
      lastTime = lastRise;                      // store current rising edge
      if (duration > 1000ul && duration < 2400ul) {
        code <<= 1;                             // 0 detected
        if (duration < 2000ul) {                // 1 detected
          code |= 1;                            // just put a 1 to LSB
        }
        valid = true;
        cnt++;
      } else {                                  // wrong length or sync
        if (duration >= 13000ul) {              // sync detected
          if (cnt == 32) {                      // rereived data of 32 bit length
            Codes[id] = code;
            id = CodesIndexIncrement(id);
            CodeEnd = id;                       // set current id one beind last valid for loop()
            valid = true;
            dataReady = true;                   // loop() has to analyze last transmissions
            lastSync = time_us;
          }
          cnt = 0;                              // reset count and code
          code = 0;
        }
      }
    }
  }
  if (cfg.HasLedPin()) digitalWrite(cfg.Tx.LedPin, valid); // LED signals stable reception with long flashes for > 6x120 ms
}

void TXDecoder::Setup()
{
  ESP_LOGE(TAG, "Setup Type=%d Recv=%d Led=%d\r\n", cfg.Tx.Type, cfg.Tx.RecvPin, cfg.Tx.LedPin);
  if (cfg.HasLedPin()) {
    pinMode(cfg.Tx.LedPin, OUTPUT);
    //pinMode(D4, OUTPUT);                      // for logic analyzer
    digitalWrite(cfg.Tx.LedPin, LOW);           // LED signals stable reception with long flashes for
  }

  if (cfg.Tx.Type == TX_DIGITAINER) {
    if (cfg.HasRecvPin()) {
      pinMode(cfg.Tx.RecvPin, INPUT);
      attachInterrupt(digitalPinToInterrupt(cfg.Tx.RecvPin), DigitainerRecvSignal, CHANGE);
    }
  }
  else if (cfg.Tx.Type == TX_NEC) {
    if (cfg.HasRecvPin()) {
      pinMode(cfg.Tx.RecvPin, INPUT);
      attachInterrupt(digitalPinToInterrupt(cfg.Tx.RecvPin), NecRecvSignal, CHANGE);
    }
  }
}

class Histogram
{
public:
  struct histdata {
    uint64_t value;
    int count;
  } data = { 0, 0 };
  int size = 0;

  void addValue(uint64_t value) {
    size++;
    for (auto &record : histogram) {
      if (record.value == value) {
        record.count++;
        return;
      }
    }
    struct histdata record = { value, 1 };
    histogram.push_back(record);
  };

  struct histdata getMaxCount() {
    data.count = 0;
    data.value = 0ul;
    for (auto &record : histogram) {
      if (record.count > data.count) {
        data = record;
      }
    }
    return data;
  };

  struct histdata getMaxCountExcludeValue(uint64_t value) {
    data.count = 0;
    data.value = 0ul;
    for (auto &record : histogram) {
      if (record.value != value && record.count > data.count) {
        data = record;
      }
    }
    return data;
  };

private:
  std::vector<struct histdata> histogram;
};

void TXDecoder::Loop()
{
  static byte LastBeg = MAX_CODES;
  bool decode = dataReady;
  byte rec_cnt = 1;
  byte beg = CodeBeg;
  byte end = CodeEnd;
  dataReady = false;

  if (cfg.Tx.Type == TX_DIGITAINER) {  // catch more tha one sequence
    rec_cnt = 3;
    static uint64_t lastDataReady = 0;
    static bool recvData = false;
    uint64_t time_us = micros();
    if (decode) {
      if (LastBeg != CodeBeg) {
        LastBeg = beg;
        recvData = true;
      }
      lastDataReady = time_us;
      decode = false; // wait for transfer burst to be finished
    }
    else {
      decode = (time_us - lastDataReady > 200000ul) && recvData; // wait for 0.5 seconds analyzing the data
      if (decode) {
        recvData = false;
        beg = LastBeg;
        LastBeg = MAX_CODES;
      }
    }
  }

  if (decode)
  {
    time_t now;
    char strftime_buf[64];
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    std::strftime(strftime_buf, sizeof(strftime_buf), "%a %d.%m.%Y %H:%M:%S", &timeinfo);
    ESP_LOGD(TAG, "===== %s =====", strftime_buf);

    Histogram hist;
    do {
      //ESP_LOGD(TAG, "Received: 0x%s %2d %2d\r\n", String((unsigned long)Codes[beg], 16).c_str(), beg, end);
      hist.addValue(Codes[beg]);
      beg = CodesIndexIncrement(beg);
    } while (beg != end);

    
    hist.getMaxCount();
    ESP_LOGD(TAG, "MaxOccur: 0x%s %2d %2d ", String((unsigned long)hist.data.value, 16).c_str(), hist.size, hist.data.count);
    if (hist.data.count >= rec_cnt) {
      struct rec data;
      if (DecodeRecord(hist.data.value, data)) {
        StoreRecord(data);
        ESP_LOGD(TAG, "valid ");
      }
      else {
        ESP_LOGE(TAG, "error");
      }
    }
    else {
      ESP_LOGD(TAG, "ignored");
    }

    // check for interleaved records
    if (hist.size - hist.data.count >= 2) {
      hist.getMaxCountExcludeValue(hist.data.value);
      ESP_LOGD(TAG, "TxSecond: 0x%s %2d %2d ", String((unsigned long)hist.data.value, 16).c_str(), hist.size, hist.data.count);
      if (hist.data.count >= rec_cnt) {
        struct rec data;
        if (DecodeRecord(hist.data.value, data)) {
          StoreRecord(data);
          ESP_LOGD(TAG, "valid ");
        }
        else {
          ESP_LOGD(TAG, "error");
        }
      }
      else {
        ESP_LOGD(TAG, "ignored");
      }
    }
  }
}

bool TXDecoder::GetRecord(struct rec &record)
{
  bool ret = false;
  if (mLastStoredRec >= 0) {
    int last_read_rec = mLastReadRec + 1;
    if (last_read_rec >= MAX_RECORDS) last_read_rec = 0;
    if (last_read_rec != mLastStoredRec) {
      record = mRecords[last_read_rec];
      mLastReadRec = last_read_rec;
      ret = true;
    }
  }
  return ret;
}

char *TXDecoder::Record2String(struct rec &record)
{
  sprintf(mBuffer, "addr 0x%04X  cmnd=0x%04X",
    record.address,
    record.command
  );
  return mBuffer;
}

void TXDecoder::StoreRecord(struct rec &record)
{
  if (record.valid) {
    if (mLastStoredRec < 0) mLastStoredRec++;
    int last_stored_rec = mLastStoredRec + 1;
    if (last_stored_rec >= MAX_RECORDS) last_stored_rec = 0;
    if (last_stored_rec == mLastReadRec) mLastReadRec++;
    if (mLastReadRec >= MAX_RECORDS) mLastReadRec = 0;
    mRecords[mLastStoredRec] = record;
    mLastStoredRec = last_stored_rec;
  }
}

uint32_t TXDecoder::little_endian(const uint32_t value, const uint8_t bits)
{
  uint32_t mask = 1 << (bits-1);
  uint32_t msb  = mask;
  uint32_t little_endian = 0;
  while (mask) {
    little_endian >>= 1;
    if (value & mask) little_endian |= msb;
    mask >>= 1;
  }
  return little_endian;
}

bool TXDecoder::DecodeRecord(uint64_t value, struct rec &record)
{
  if (cfg.Tx.Type == TX_DIGITAINER) {
    return DigitainerDecodeRecord(value, record);
  }
  else if (cfg.Tx.Type == TX_NEC) {
    return NecDecodeRecord(value, record);
  }
  return false;
}

bool TXDecoder::DigitainerDecodeRecord(uint64_t value, struct rec &record)
{
  /*---------------------------------------------------------------------------------------------------------------------------------------------------
 * RF X10 remote control (MEDION, Pollin 721815) Taken frpm IMRP.org
 *
 * Frame:
 * 1 toggle bit + 7 checksum bits + 1 toggle bit + 7 command bits + 4 channel bits
 *
 * Rule:
 * checksum = (command + 0x0055 + (channel << 4)) & 0x7F
 *
 * Here we store checksum in address, command incl. 4 channel bits in command
 *
 * In irmp_get_data():
 *  irmp_command = command << 4
 *  irmp_address = channel + 1
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

  uint32_t check_msk = 0x80800;
  uint32_t check_val = value & check_msk; 
  uint32_t check_sum;

  // bacic check, if toggle bits are inverted
  //if ((check_val != check_msk) && (check_val != 0)) {
  if (true) {
    record.address = value & 0xf;
    record.command = (value >> 4) & 0x7f;
    check_val = (value >> 12) & 0x7f;
    check_sum = (record.command + 0x55 + (record.address << 4)) & 0x7f;
    record.valid = (check_val == check_sum);
    ESP_LOGD(TAG, "val=0x%02X sum=0x%02X", check_val, check_sum);
    time(&record.timestamp);
  }

  return record.valid;
}

bool TXDecoder::NecDecodeRecord(uint64_t value, struct rec &record)
{
  record.address = little_endian(value >> 16, 16);
  record.command = little_endian(value, 16);
  record.valid = (record.command & 0xff) == ((~record.command >> 8) & 0xff);
  time(&record.timestamp);

  return record.valid;
}

}  // namespace rf_scanner
}  // namespace esphome
