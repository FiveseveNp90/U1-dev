#ifndef PT7314_H
#define PT7314_H

//chip **7 bit** address
// Datasheet says 0x88 but that is the 8 bit
// address. Wire.h automatically appends the
// extra (lsb write) bit.
#define PT7314_address 0x44

#define PT7314_master 0b00000000
#define PT7314_latt 0b11000000
#define PT7314_ratt 0b11100000
#define PT7314_input_sel 0b01000000
#define PT7314_bass 0b01100000
#define PT7314_treble 0b01110000

#include <Arduino.h>
class PT7314
{
public:
	PT7314();
	// 0 to 3, 0/1, 0 to 3
	void setInput(int input, bool loudness, int gain);
	// -7 to +7 = attenuation -14 to +14
	void setBass(int gain);
	// -7 to +7 = attenuation -14 to +14
	void setTreble(int gain);
	// Value 0 to 63 = attenuation -78.75 db to 0
	void setMaster(int volume);
	// Value 0 to 31 = attenuation -37.5 db to 0 db (value 0 is mute)
	void setLatten(int att);
	// Value 0 to 31 = attenuation -37.5 db to 0 db (value 0 is mute)
	void setRatten(int att);

private:
	void writeWire(char data);
};

#endif //PT7314_H
