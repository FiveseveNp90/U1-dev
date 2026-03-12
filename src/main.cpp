#include <Arduino.h>
#include <Wire.h>
#include "PT7314.h"
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>
#include "def.h"
#include <LittleFS.h>

// Pins

#define pFootsw 19

#define pADC 26
#define pC1 2
#define pC2 3
#define pC3 4
#define pC4 5 // bypass
#define pLED 6

#define pSDA 20
#define pSCL 21

// Pot read ADC threshold
#define potThreshold 5

// Timings in milliseconds
#define debounceThreshold 30
#define autosaveTime 5000

#define presetNum 49 // total presets (0 indexed)
#define panVal 27

// Global variables
int currPreset = 1; // 0 to 49
byte lastPreset = 1;
byte adcVal = 255; // out of range to force initial read
byte midiChan = 1;
bool autosaveFlag = false;
bool lastButtonState = false;
bool buttonState = false;

unsigned long lastDebounceTime = 0;
unsigned long autosavePrevMillis = 0;

preset_type pstA;
preset_type pstB;
preset_type pstC;

preset_type preset;
preset_type copyBuffer;
config_type config;

// filesystem
lfs_t lfs;
lfs_file_t file;

Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

PT7314 pre;

#include "io.h"

void setup()
{

  pinMode(pFootsw, INPUT_PULLUP);
  pinMode(pADC, INPUT);
  pinMode(pC1, OUTPUT);
  pinMode(pC2, OUTPUT);
  pinMode(pC3, OUTPUT);
  pinMode(pC4, OUTPUT);
  pinMode(pLED, OUTPUT);

  Wire.setSCL(pSCL);
  Wire.setSDA(pSDA);
  Wire.begin();
  Wire.setClock(400000); // 100000 / 400000 ?
#if defined(WIRE_HAS_TIMEOUT)
  Wire.setWireTimeout(10000, true);
#endif

  if (!TinyUSBDevice.isInitialized())
  {
    TinyUSBDevice.begin(0);
  }
  usb_midi.setStringDescriptor("DEFECT U(1)");

  // LittleFSConfig cfg;
  // cfg.setAutoFormat(true);
  // LittleFS.setConfig(cfg);
  LittleFS.begin();

  analogWriteFreq(100000);
  analogWriteResolution(7);

  // hardware init
  pre.setInput(0, 0, 0);

  // read filesystem

  loadConfig();

  MIDI.setHandleProgramChange(handlePC);
  MIDI.setHandleControlChange(handleCC);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  midiChan = config.midiChan;
  MIDI.turnThruOff();

  if (TinyUSBDevice.mounted())
  {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }

  loadLastPst();

  if (digitalRead(pFootsw) == LOW)
  { // bypass
    currPreset = 127;
  }
  loadThree();
  loadPreset();
  autosavePrevMillis = millis();
}

void loop()
{
  MIDI.read();

  readFS();

  int ADCread = 0;
  int ADCdiff = 0;

  if (currPreset <= presetNum)
  {
    ADCread = analogRead(pADC) >> 3;
    ADCdiff = ADCread - adcVal;
    if (abs(ADCdiff) >= potThreshold)
    {
      adcVal = ADCread;
      switch (config.mode)
      {
      case 0: // morph 3
        morphPst(ADCread);
        break;
      case 1: // sel 5
        switchPst(ADCread);
        break;

      case 3:
        setGain(ADCread);
        break;
      case 4:
        setLow(ADCread);
        break;
      case 5:
        setMid(ADCread);
        break;
      case 6:
        setHigh(ADCread);
        break;
      case 7:
        setClipping(ADCread);
        break;

      default:
        morphPst(ADCread);
        break;
      }
    }
    unsigned long currentMillis = millis();
    // save last preset
    if ((currentMillis - autosavePrevMillis > autosaveTime) && (lastPreset != currPreset) && autosaveFlag)
    {
      lastPreset = currPreset;
      saveLastPst();
      autosaveFlag = false;
      autosavePrevMillis = millis();
    }
  }
}