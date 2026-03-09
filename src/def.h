#include <Arduino.h>
typedef struct
{
    byte gain;
    byte low;
    byte mid;
    byte high;
    byte clipping;
} preset_type;

typedef struct
{
    byte midiChan;
    byte brightness;
    byte mode;
} config_type;

// factory presets
preset_type factory0 = {
    .gain = 0,
    .low = 127,
    .mid = 63,
    .high = 63,
    .clipping = 127,
};
preset_type factory1 = {
    .gain = 30,
    .low = 127,
    .mid = 63,
    .high = 63,
    .clipping = 127,
};
preset_type factory2 = {
    .gain = 60,
    .low = 127,
    .mid = 63,
    .high = 63,
    .clipping = 127,
};
preset_type factory3 = {
    .gain = 90,
    .low = 127,
    .mid = 63,
    .high = 63,
    .clipping = 127,
};
preset_type factory4 = {
    .gain = 127,
    .low = 127,
    .mid = 63,
    .high = 63,
    .clipping = 127,
};

/*

PT inputs
HPF		L
full	R

2040 I/O
knob    ADC0    GPIO26
fsw GPIO19
SDA GPIO20
SCL GPIO21
C1	GPIO2	clip A (lsb)
C2	GPIO3	clip B
C3	GPIO4	clip C
C4	GPIO5	bypass
LED GPIO6


MIDI mapping
CC	function        range

11 	Gain            0 — 20
12	Low
13 	Mid         	-14—14 step 2
14 	High        	-14—14 step 2
15 	Clipping        0 — 7
34	LED Brightness  10-100

35	reserved:		value

    byte midiChan	1 — 16
    byte mode   	20 — 27 (22 invalid)

    save config		110
    save preset		120
    copy preset     121
    paste preset    122

    request data    123		respond 124

*/