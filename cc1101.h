// https://github.com/dbuezas/esphome-cc1101

#ifndef CC1101TRANSCIVER_H
#define CC1101TRANSCIVER_H

#include <ELECHOUSE_CC1101_SRC_DRV.h>

#include "esphome/components/remote_transmitter/remote_transmitter.h"

int CC1101_module_count = 0;
#define get_cc1101(id) (*((CC1101*)id))

class CC1101 : public PollingComponent, public Sensor {
  int _SCK;
  int _MISO;
  int _MOSI;
  int _CSN;
  int _GDO0;  // TX and also RX
  float _bandwidth;
  esphome::remote_transmitter::RemoteTransmitterComponent* _remote_transmitter;
  float _moduleNumber;
  int _last_rssi = 0;

  void setup() {
    pinMode(_GDO0, INPUT);
    ELECHOUSE_cc1101.addSpiPin(_SCK, _MISO, _MOSI, _CSN, _moduleNumber);
    ELECHOUSE_cc1101.setModul(_moduleNumber);
    ELECHOUSE_cc1101.Init();
    ELECHOUSE_cc1101.setRxBW(_bandwidth);
    ELECHOUSE_cc1101.setMHZ(_freq);
    ELECHOUSE_cc1101.SetRx();
  }

 public:
  float _freq;
  CC1101(int SCK, int MISO, int MOSI, int CSN, int GDO0, float bandwidth, float freq,
         esphome::remote_transmitter::RemoteTransmitterComponent* remote_transmitter)
      : PollingComponent(100) {
    _SCK = SCK;
    _MISO = MISO;
    _MOSI = MOSI;
    _CSN = CSN;
    _GDO0 = GDO0;
    _bandwidth = bandwidth;
    _freq = freq;
    _moduleNumber = CC1101_module_count++;
    _remote_transmitter = remote_transmitter;
  }

  void beginTransmission() {
    ELECHOUSE_cc1101.setModul(_moduleNumber);
    ELECHOUSE_cc1101.SetTx();
    pinMode(_GDO0, OUTPUT);
#ifdef USE_ESP32
    _remote_transmitter->setup();
#endif
#ifdef USE_ESP8266
    noInterrupts();
#endif
  }
  void endTransmission() {
    digitalWrite(_GDO0, 0);
    pinMode(_GDO0, INPUT);
#ifdef USE_ESP8266
    interrupts();
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
  bool rssi_on = false;
  void update() override {
    int rssi = 0;
    if (rssi_on) {
      ELECHOUSE_cc1101.setModul(_moduleNumber);
      rssi = ELECHOUSE_cc1101.getRssi();
      if (rssi != _last_rssi) {
        publish_state(rssi);
        _last_rssi = rssi;
      }
    }
  }
};

#endif