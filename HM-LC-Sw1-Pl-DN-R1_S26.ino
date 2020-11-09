//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// 2018-09-12 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

//#define NDEBUG  

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

// number of relays by defining the device
#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>
#include <secret.h>      // hier sind die HM Key's drin, in extra Datei damit nicht zu Git gesynct wird !
#include <Switch.h>

//pins are fixed by the pcb layout
#define LED_PIN           5  // low-active
#define CONFIG_BUTTON_PIN 4
#define RELAY1_PIN        8


// number of available peers per channel
#define PEERS_PER_CHANNEL 8

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
  {0x01, 0xd8, 0x03},     // Device ID
  "FS26PLG968",           // Device Serial
  {0x00,0xd8},            // Device Model
  0x26,                   // Firmware Version
  as::DeviceType::Switch, // Device Type
  {0x01, 0x00}            // Info Bytes
};

/**
   Configure the used hardware
*/
typedef AvrSPI<10, 11, 12, 13> RadioSPI;
typedef AskSin<StatusLed<LED_PIN>, NoBattery, Radio<RadioSPI, 2> > Hal;

// setup the device with channel type and number of channels
typedef MultiChannelDevice<Hal, SwitchChannel<Hal, PEERS_PER_CHANNEL, List0>, 1> SwitchType;

Hal hal;
SwitchType sdev(devinfo, 0x20);

ConfigToggleButton<SwitchType> cfgBtn(sdev);

void initPeerings (bool first) {
  // create internal peerings - CCU2 needs this
  if ( first == true ) {
    HMID devid;
    sdev.getDeviceID(devid);
    for ( uint8_t i = 1; i <= sdev.channels(); ++i ) {
      Peer ipeer(devid, i);
      sdev.channel(i).peer(ipeer);
    }
  }
}

void setup () {
  DINIT(57600, ASKSIN_PLUS_PLUS_IDENTIFIER);
  bool first = sdev.init(hal);
  sdev.channel(1).init(RELAY1_PIN, false);
  
  buttonISR(cfgBtn, CONFIG_BUTTON_PIN);
  
  sdev.channels(1);
  initPeerings(first);
  sdev.initDone();
  sdev.led().invert(true);  // inverts the LED signal, otherwise the blue LED lights up permanently and flashes only by status requests
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if ( worked == false && poll == false ) {
    hal.activity.savePower<Idle<> >(hal);
  }
}
