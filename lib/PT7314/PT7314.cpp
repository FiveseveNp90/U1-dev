#include <Arduino.h>
#include <Wire.h>
#include "PT7314.h"

PT7314::PT7314()
{
}

void PT7314::setInput(int input, bool loudness, int gain)
{
  writeWire(PT7314_input_sel | input | ((!loudness) << 2) | ((3 - gain) << 3));
}

void PT7314::setBass(int gain)
{
  if (gain > 0)
  {
    writeWire(PT7314_bass | (15 - gain));
  }
  else
  {
    writeWire(PT7314_bass | (7 + gain));
  }
}

void PT7314::setTreble(int gain)
{
  if (gain > 0)
  {
    writeWire(PT7314_treble | (15 - gain));
  }
  else
  {
    writeWire(PT7314_treble | (7 + gain));
  }
}

void PT7314::setMaster(int volume)
{
  writeWire(PT7314_master | (63 - volume));
}

void PT7314::setLatten(int att)
{
  writeWire(PT7314_latt | (31 - att));
}
void PT7314::setRatten(int att)
{
  writeWire(PT7314_ratt | (31 - att));
}

void PT7314::writeWire(char msg)
{
  Wire.beginTransmission(PT7314_address);
  Wire.write(msg);
  Wire.endTransmission();
}
