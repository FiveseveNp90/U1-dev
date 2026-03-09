#include <Arduino.h>

void setGain(byte val)
{
    preset.gain = val;
    int gain = map(val, 0, 128, 16, 64); // 16 = -20 dB?

    pre.setMaster(gain);
}

void setLow(byte val)
{
    preset.low = val;
    int gain = map(val, 0, 128, 0, 32);

    pre.setLatten(gain);
    pre.setRatten(31 - gain);
    // implement pan law    26 = -6dB
}
void setMid(byte val)
{
    preset.mid = val;

    int gain = map(val, 0, 128, -7, 8);
    pre.setBass(gain);
}
void setHigh(byte val)
{
    preset.high = val;

    int gain = map(val, 0, 128, -7, 8);
    pre.setTreble(gain);
}

void setClipping(byte clip)
{
    preset.clipping = clip;

    int clipBits = map(clip, 0, 128, 0, 9);
    digitalWrite(pC1, bitRead(clipBits, 0));
    digitalWrite(pC2, bitRead(clipBits, 1));
    digitalWrite(pC3, bitRead(clipBits, 2));
}

void setLED(byte value = config.brightness)
{
    config.brightness = value;

    if (currPreset <= presetNum)
    {
        byte scaledVal = map(value, 0, 127, 10, 127);
        analogWrite(pLED, scaledVal);
    }
    else
    {
        analogWrite(pLED, 0);
    }
}

void blinkLED()
{
    analogWrite(pLED, 0);
    delay(50);
    analogWrite(pLED, 127);
    delay(50);
    setLED();
}

// Storage
void savePreset()
{
    String fname;
    fname = "/";
    fname += String(currPreset);
    fname += ".bin";
    File myFile = LittleFS.open(fname, "w");
    if (myFile.write((byte *)&preset, sizeof(preset)))
        myFile.close();
    blinkLED();
}

void saveConfig()
{
    File myFile = LittleFS.open("/config.bin", "w");
    if (myFile.write((byte *)&config, sizeof(config)))
        myFile.close();
    blinkLED();
}

void saveLastPst()
{
    File myFile = LittleFS.open("/last.bin", "w");
    if (myFile.write((byte *)&lastPreset, sizeof(lastPreset)))
        myFile.close();
}

void loadDefaultPst() // edit default values
{
    setGain(0);
    setLow(127);
    setMid(63);
    setHigh(63);
    setClipping(127);
    // savePreset();
}

void factoryPst() // initialize presets
{
    File myFile = LittleFS.open("/0.bin", "w");
    if (myFile.write((byte *)&factory0, sizeof(factory0)))
        myFile.close();
    myFile = LittleFS.open("/1.bin", "w");
    if (myFile.write((byte *)&factory1, sizeof(factory1)))
        myFile.close();
    myFile = LittleFS.open("/2.bin", "w");
    if (myFile.write((byte *)&factory2, sizeof(factory2)))
        myFile.close();
    myFile = LittleFS.open("/3.bin", "w");
    if (myFile.write((byte *)&factory3, sizeof(factory3)))
        myFile.close();
    myFile = LittleFS.open("/4.bin", "w");
    if (myFile.write((byte *)&factory4, sizeof(factory4)))
        myFile.close();

    blinkLED();
    delay(100);
    blinkLED();
    delay(100);
    blinkLED();
    delay(100);
}

void loadPreset()
{
    if (currPreset > presetNum)
    {
        digitalWrite(pC4, LOW); // bypass
        pre.setMaster(0);
    }
    else
    {
        digitalWrite(pC4, HIGH);

        if (config.mode > 0)
        { // load one preset
            String fname;
            fname = "/";
            fname += String(currPreset);
            fname += ".bin";
            if (LittleFS.exists(fname))
            {
                File myFile = LittleFS.open(fname, "r");
                myFile.read((byte *)&preset, sizeof(preset));
                myFile.close();
            }
            else
            {
                loadDefaultPst();
            }
            setGain(preset.gain);
            setLow(preset.low);
            setMid(preset.mid);
            setHigh(preset.high);
            setClipping(preset.clipping);
        }
        else
        { // load 3 presets to morph
            if (!LittleFS.exists("/0.bin"))
            {
                factoryPst();
            }
            File myFile = LittleFS.open("/0.bin", "r");
            myFile.read((byte *)&pstA, sizeof(pstA));
            myFile.close();
            myFile = LittleFS.open("/1.bin", "r");
            myFile.read((byte *)&pstB, sizeof(pstB));
            myFile.close();
            myFile = LittleFS.open("/2.bin", "r");
            myFile.read((byte *)&pstC, sizeof(pstC));
            myFile.close();
        }
    }

    setLED();
    // last preset save flag
    autosaveFlag = true;
    autosavePrevMillis = millis();
}

void loadConfig()
{
    if (LittleFS.exists("/config.bin"))
    {
        File myFile = LittleFS.open("/config.bin", "r");
        myFile.read((byte *)&config, sizeof(config));
        myFile.close();
    }
    else
    {
        config.midiChan = 1;
        config.brightness = 20;
        config.mode = 0;
        saveConfig();
    }
}

void loadLastPst()
{

    if (LittleFS.exists("/last.bin"))
    {
        File myFile = LittleFS.open("/last.bin", "r");
        myFile.read((byte *)&lastPreset, sizeof(lastPreset));
        myFile.close();
        currPreset = lastPreset;
    }
    else
    {
        lastPreset = 0;
        saveLastPst();
        currPreset = lastPreset;
    }
    if (buttonState)
    {
        currPreset = 127;
    }
    else
    {
        currPreset = lastPreset;
    }
}

void sendData()
{
    MIDI.sendControlChange(35, 124, midiChan); // 124 ID response

    if (currPreset > presetNum)
    {
        currPreset = lastPreset;
        loadPreset();
    }

    MIDI.sendProgramChange(currPreset, midiChan);

    MIDI.sendControlChange(35, midiChan, midiChan);
    MIDI.sendControlChange(34, config.brightness, midiChan);
    MIDI.sendControlChange(35, (config.mode + 20), midiChan);

    MIDI.sendControlChange(11, preset.gain, midiChan);
    MIDI.sendControlChange(12, preset.low, midiChan);
    MIDI.sendControlChange(13, preset.mid, midiChan);
    MIDI.sendControlChange(14, preset.high, midiChan);
    MIDI.sendControlChange(15, preset.clipping, midiChan);
}

void copyPreset()
{
    copyBuffer = preset;
}
void pastePreset()
{
    setGain(copyBuffer.gain);
    setLow(copyBuffer.low);
    setMid(copyBuffer.mid);
    setHigh(copyBuffer.high);
    setClipping(copyBuffer.clipping);
    sendData();
}

void morphPst(byte val)
{
    byte morphdGain;
    byte morphdLow;
    byte morphdMid;
    byte morphdHigh;
    byte morphdClip;

    if (val < 64)
    {
        morphdGain = map(val, 0, 63, pstA.gain, pstB.gain);
        morphdLow = map(val, 0, 63, pstA.low, pstB.low);
        morphdMid = map(val, 0, 63, pstA.mid, pstB.mid);
        morphdHigh = map(val, 0, 63, pstA.high, pstB.high);
        morphdClip = map(val, 0, 63, pstA.clipping, pstB.clipping);
    }
    else
    {
        morphdGain = map(val, 64, 127, pstB.gain, pstC.gain);
        morphdLow = map(val, 64, 127, pstB.low, pstC.low);
        morphdMid = map(val, 64, 127, pstB.mid, pstC.mid);
        morphdHigh = map(val, 64, 127, pstB.high, pstC.high);
        morphdClip = map(val, 64, 127, pstB.clipping, pstC.clipping);
    }

    setGain(morphdGain);
    setLow(morphdLow);
    setMid(morphdMid);
    setHigh(morphdHigh);
    setClipping(morphdClip);
}

void switchPst(byte val)
{
    int selPst = map(val, 0, 128, 0, 5);
    currPreset = selPst;
    loadPreset();
}

// Footswitch

void readFS()
{
    bool currentButtonState;
    if (digitalRead(pFootsw) == LOW)
    {
        currentButtonState = true;
    }
    else
    {
        currentButtonState = false;
    }
    if (currentButtonState != lastButtonState)
    {
        lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceThreshold)
    {
        if (currentButtonState != buttonState)
        {
            buttonState = currentButtonState;
            if (buttonState)
            {
                analogWrite(pLED, 0);
            }
            else
            {
                analogWrite(pLED, 70);
            }
        }
    }
    lastButtonState = currentButtonState;

    /*     if (currPreset > presetNum) // if bypassed
        {
            currPreset = lastPreset;
        }
        else
        {
            lastPreset = currPreset;
            currPreset = 127;
        } */
}

// MIDI

void handlePC(byte channel, byte number)
{
    if ((channel == midiChan) && (number <= presetNum))
    {
        currPreset = number;
        loadPreset();
    }
}
void handleCC(byte channel, byte number, byte value)
{
    if (channel == midiChan)
    {
        byte scaledVal;
        switch (number)
        {
        case 11:
            setGain(value);
            break;
        case 12:
            setLow(value);
            break;
        case 13:
            setMid(value);
            break;
        case 14:
            setHigh(value);
            break;
        case 15:
            setClipping(value);
            break;
        case 34:
            setLED(value);
            break;

        case 35: // control
            if ((value > 0) && (value < 17))
            {
                config.midiChan = value;
            }
            if ((value > 19) && (value < 28) && (value != 22))
            {
                config.mode = value - 20;
            }
            switch (value)
            {
            case 110:
                saveConfig();
                break;
            case 120:
                savePreset();
                break;
            case 121:
                copyPreset();
                break;
            case 122:
                pastePreset();
                break;
            case 123:
                sendData();
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
}