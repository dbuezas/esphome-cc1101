// https://github.com/dbuezas/esphome-cc1101

#ifndef CC1101TRANSCIVER_H
#define CC1101TRANSCIVER_H

#include <ELECHOUSE_CC1101_SRC_DRV.h>

int CC1101_module_count = 0;
#define get_cc1101(id) (*((CC1101 *)id))

class CC1101 : public PollingComponent, public Sensor {
  int _SCK;
  int _MISO;
  int _MOSI;
  int _CSN;
  int _GDO0; // TX (and also RX if ESP8266)
#ifdef USE_ESP32
  int _GDO2; // RX
#endif
  float _bandwidth;
  
  float _moduleNumber;
  int _last_rssi = 0;

  void setup() {
#ifdef USE_ESP32
    pinMode(_GDO0, OUTPUT);
    pinMode(_GDO2, INPUT);
#else
    pinMode(_GDO0, INPUT);
#endif
    ELECHOUSE_cc1101.addSpiPin(_SCK, _MISO, _MOSI, _CSN, _moduleNumber);
    ELECHOUSE_cc1101.setModul(_moduleNumber);
    ELECHOUSE_cc1101.Init();
    ELECHOUSE_cc1101.setRxBW(_bandwidth);
    ELECHOUSE_cc1101.setMHZ(_freq);
    ELECHOUSE_cc1101.SetRx();
  }

 public:
  float _freq;
  CC1101(int SCK, int MISO, int MOSI, int CSN, int GDO0, 
#ifdef USE_ESP32
         int GDO2, 
#endif
         float bandwidth, float freq)
      : PollingComponent(100) {
    _SCK = SCK;
    _MISO = MISO;
    _MOSI = MOSI;
    _CSN = CSN;
    _GDO0 = GDO0;
#ifdef USE_ESP32
    _GDO2 = GDO2;
#endif
    _bandwidth = bandwidth;
    _freq = freq;
    _moduleNumber = CC1101_module_count++;
  }

  void beginTransmission() {
    ELECHOUSE_cc1101.setModul(_moduleNumber);
    ELECHOUSE_cc1101.SetTx();
#ifndef USE_ESP32
    pinMode(_GDO0, OUTPUT);
    noInterrupts();
#endif
  }
  void endTransmission() {
#ifndef USE_ESP32
    interrupts();
    pinMode(_GDO0, INPUT);
#endif
    ELECHOUSE_cc1101.setModul(_moduleNumber);
    ELECHOUSE_cc1101.SetRx();
    ELECHOUSE_cc1101.SetRx();  // yes, twice
  }
  void setBW(float bandwidth) {
    ELECHOUSE_cc1101.setModul(_moduleNumber);
    ELECHOUSE_cc1101.setRxBW(bandwidth);
  }
  void setFreq(float freq) {
    ELECHOUSE_cc1101.setModul(_moduleNumber);
    ELECHOUSE_cc1101.setMHZ(freq);
  }
  bool rssi_on;
  void update() override {
    int rssi = 0;
    if (rssi_on) {
      ELECHOUSE_cc1101.setModul(_moduleNumber);
      rssi = ELECHOUSE_cc1101.getRssi();
    }
    if (rssi != _last_rssi) {
      publish_state(rssi);
      _last_rssi = rssi;
    }
  }
};

#endif