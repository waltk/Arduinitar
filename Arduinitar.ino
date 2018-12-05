/* 
 * Modified version of Arduino "guitar" by Andrew McPherson Queen Mary University of London, September 2012 http://www.eecs.qmul.ac.uk/
 * Added control to switch mappings used in Auduino Lo-Fi granular synthesiser by Peter Knight, Tinker.it http://tinker.it
 * Additional mapping modes - diatonic major and minor and pentatonic major and minor 
 * Using Mozzi audio library by Tim Barrass.
*/

// CHANGE TO MOZZI CONFIG FILE TO STANDARD PLUS MODE BEFORE UPLOADING TO ARDUINO

#include <MozziGuts.h>
#include <Oscil.h> // oscillator 
#include <mozzi_midi.h>
#include <tables/sin2048_int8.h>
#include <tables/saw2048_int8.h>
#include <tables/triangle2048_int8.h>
#include <tables/cos2048_int8.h>
#include <AutoMap.h>
#include <Metronome.h>
#include <IntMap.h>

#define CONTROL_RATE 128 // powers of 2 please

// Basic pitch of the instrument (MIDI 45 = A2)
#define MIDI_NOTE_MIN (Q16n16)(33L << 16L)
 
const int VOLUME_PIN = 0; 
const int PITCH_PIN = 1; 
const int DECAY_PIN = 2;
const int TUNING_PIN = 3;

const int BASS_PIN = 5;
const int GUITAR1_PIN = 2;
const int GUITAR2_PIN = 3;
const int GUITAR3_PIN = 4;

Oscil <2048, AUDIO_RATE> inGuitar1; // sound source
Oscil <2048, AUDIO_RATE> inGuitar2;
Oscil <2048, AUDIO_RATE> inGuitar3;
Oscil <2048, AUDIO_RATE> inBass;


unsigned long pitch;
unsigned int tuning;

AutoMap volumeMap(0, 1024, 0, 255);
//int pitch;
unsigned int volumeRead;
unsigned int masterVolume;
long out1;
long out2;
long out;
long finalOut;

int bassButton; 
int guitar1Button;
int guitar2Button;
int guitar3Button;

int bassVolume;
int guitar1Volume;
int guitar2Volume;
int guitar3Volume;

// Table Mode Button
const int TABLE_PIN = 6;  // the number of the pushbutton pin 
int buttonValue;        // variable for reading the button status
int buttonState;        // variable to hold the button state
int tableMode = 0;        // What scale/mapping mode is in use?

// Map Mode Bbutton
const int MAP_PIN = 8;
int mapbuttonValue;        // variable for reading the button status
int mapbuttonState;        // variable to hold the button state
int mapMode = 0; 

Metronome ledMetro(800);
const int LED2_PIN = 11;
unsigned int modeBlink = 900;

void chooseTable(){
  buttonValue = digitalRead(TABLE_PIN);
  if (buttonValue != buttonState) {
    if (buttonValue == 1) {
      if (tableMode == 0) {
        tableMode = 1;
      }
      else if (tableMode == 1) {
          tableMode = 2;
        }
      else if (tableMode == 2) {
          tableMode = 3;
        }
      else if (tableMode == 3) {
        tableMode = 0;
      }
    }
    buttonState = buttonValue;    
  }

  if(tableMode == 0){
    inGuitar1.setTable(SIN2048_DATA);
    inGuitar2.setTable(SIN2048_DATA);
    inGuitar3.setTable(SIN2048_DATA);
    inBass.setTable(SIN2048_DATA);
  } else if(tableMode == 1){
    inGuitar1.setTable(SAW2048_DATA);
    inGuitar2.setTable(SAW2048_DATA);
    inGuitar3.setTable(SAW2048_DATA);
    inBass.setTable(SAW2048_DATA);
  } else if(tableMode == 3){
    inGuitar1.setTable(TRIANGLE2048_DATA);
    inGuitar2.setTable(TRIANGLE2048_DATA);
    inGuitar3.setTable(TRIANGLE2048_DATA);
    inBass.setTable(TRIANGLE2048_DATA);
  } else if(tableMode == 4){
    inGuitar1.setTable(COS2048_DATA);
    inGuitar2.setTable(COS2048_DATA);
    inGuitar3.setTable(COS2048_DATA);
    inBass.setTable(COS2048_DATA);
  }

  if (tableMode == 0){
    ledMetro.set(modeBlink);
  } else if (tableMode == 1){
        ledMetro.set(modeBlink - 300);
  } else if (tableMode == 2){
        ledMetro.set(modeBlink - 600);
  }  
}

void choosemapMode(){
  buttonValue = digitalRead(MAP_PIN);
  if (mapbuttonValue != mapbuttonState) {
    if (mapbuttonValue == 1) {
      if (mapMode == 0) {
        mapMode = 1;
      }
      else if (mapMode == 1) {
          mapMode = 2;
        }
      else if (mapMode == 2) {
          mapMode = 3;
        }
      else if (mapMode == 3) {
        mapMode = 0;
      }
    }
    mapbuttonState = mapbuttonValue;    
  }
}

// MAPPINGS - START
// Smooth logarithmic mapping
uint16_t antilogTable[] = {
  64830,64132,63441,62757,62081,61413,60751,60097,59449,58809,58176,57549,56929,56316,55709,55109,
  54515,53928,53347,52773,52204,51642,51085,50535,49991,49452,48920,48393,47871,47356,46846,46341,
  45842,45348,44859,44376,43898,43425,42958,42495,42037,41584,41136,40693,40255,39821,39392,38968,
  38548,38133,37722,37316,36914,36516,36123,35734,35349,34968,34591,34219,33850,33486,33125,32768
};
uint16_t mapPhaseInc(uint16_t input) {
  return (antilogTable[input & 0x3f]) >> (input >> 6);
}

// Stepped chromatic mapping
uint16_t midiTable[] = {
  0,17,18,19,20,22,23,24,26,27,29,31,32,34,36,38,41,43,46,48,51,54,58,61,65,69,73,
  77,82,86,92,97,103,109,115,122,129,137,145,154,163,173,183,194,206,218,231,
  244,259,274,291,308,326,346,366,388,411,435,461,489,518,549,581,616,652,691,
  732,776,822,871,923,978,1036,1097,1163,1232,1305,1383,1465,1552,1644,1742,
  1845,1955,2071,2195,2325,2463,2610,2765,2930,3104,3288,3484,3691,3910,4143,
  4389,4650,4927,5220,5530,5859,6207,6577,6968,7382,7821,8286,8779,9301,9854,
  10440,11060,11718,12415,13153,13935,14764,15642,16572,17557,18601,19708,20879,
  22121,23436,24830,26306,27871
};
uint16_t mapMidi(uint16_t input) {
  return (midiTable[(1023-input) >> 3]);
}

//// Stepped Pentatonic mapping
uint16_t pentatonicTable[54] = {
  0,19,22,26,29,32,38,43,51,58,65,77,86,103,115,129,154,173,206,231,259,308,346,
  411,461,518,616,691,822,923,1036,1232,1383,1644,1845,2071,2463,2765,3288,
  3691,4143,4927,5530,6577,7382,8286,9854,11060,13153,14764,16572,19708,22121,26306
};

uint16_t mapPentatonic(uint16_t input) {
  uint8_t value = (1023-input) / (1024/53);
  return (pentatonicTable[value]);
}

// Stepped major Diatonic mapping
uint16_t majordiatonicTable[76] = {
  0,17,19,22,23,26,29,32,34,38,43,46,51,58,65,69,77,86,92,103,115,129,137,154,173,183,206,231,259,274,308,346,366,
  411,461,518,549,616,691,732,822,923,1036,1097,1232,1383,1465,1644,1845,2071,2195,2463,2765,2930,3288,
  3691,4143,4389,4927,5530,5859,6577,7382,8286,8779,9854,11060,11718,13153,14764,16572,17557,19708,22121,23436,26306
};

uint16_t mapmajorDiatonic(uint16_t input) {
  uint8_t value = (1023-input) / (1024/53);
  return (majordiatonicTable[value]);
}

// Stepped minor Diatonic mapping
uint16_t minordiatonicTable[76] = {
  0,17,19,20,23,26,27,31,34,38,41,46,51,54,61,69,77,82,92,103,109,122,137,154,163,183,206,218,244,274,308,326,366,
  411,435,489,549,616,652,732,822,871,978,1097,1232,1305,1465,1644,1742,1955,2195,2463,2610,2930,3288,
  3484,3910,4389,4927,5220,5859,6577,6968,7821,8779,9854,10440,11718,13153,13935,15642,17557,19708,20879,23436,26306
};

uint16_t mapminorDiatonic(uint16_t input) {
  uint8_t value = (1023-input) / (1024/53);
  return (minordiatonicTable[value]);
}

// Stepped major Pentatonic mapping
uint16_t majorpentatonicTable[55] = {
  0,17,19,22,26,29,34,38,43,51,58,69,77,86,103,115,137,154,173,206,231,274,308,346,
  411,461,549,616,691,822,923,1097,1232,1383,1644,1845,2195,2463,2765,3288,
  3691,4389,4927,5530,6577,7382,8779,9854,11060,13153,14764,17557,19708,22121,26306
};

uint16_t mapmajorPentatonic(uint16_t input) {
  uint8_t value = (1023-input) / (1024/53);
  return (majorpentatonicTable[value]);
}

// Stepped minor Pentatonic mapping
uint16_t minorpentatonicTable[55] = {
  0,17,20,23,26,31,34,41,46,51,61,69,82,92,103,122,137,163,183,206,244,274,326,366,
  411,489,549,652,732,822,978,1097,1305,1465,1644,1955,2195,2610,2930,3288,
  3910,4389,5220,5859,6577,7821,8779,10440,11718,13153,15642,17557,20879,23436,26306
};

uint16_t mapminorPentatonic(uint16_t input) {
  uint8_t value = (1023-input) / (1024/53);
  return (pentatonicTable[value]);
}
// MAPPINGS - END

void setup(){
  pinMode(BASS_PIN, INPUT_PULLUP);
  pinMode(GUITAR1_PIN, INPUT_PULLUP);
  pinMode(GUITAR2_PIN, INPUT_PULLUP);
  pinMode(GUITAR3_PIN, INPUT_PULLUP);
  pinMode(TABLE_PIN, INPUT_PULLUP);
  pinMode(LED2_PIN, OUTPUT);

  ledMetro.start(modeBlink);  
  Serial.begin(9600); 
  //Serial.begin(115200); // set up the Serial output for debugging
  startMozzi(CONTROL_RATE); 
}

void updateControl(){

  chooseTable();
  choosemapMode();
  static Q16n16 midiNote, midiNoteInt, baseFrequency;
  static long midiNoteFrac;
  volumeRead = mozziAnalogRead(VOLUME_PIN); // unmapped
  
  // masterVolume = map(volumeRead, 0, 1023, 0, 255); // mapped
    
  masterVolume = volumeMap(volumeRead);
  if (masterVolume < 40) {
    masterVolume = 0;
  } else {
      masterVolume;   
  }

  pitch = mozziAnalogRead(PITCH_PIN);

  if (mapMode = 0) {
    pitch = mapPhaseInc(pitch) / 4;
  } else if (mapMode = 1) {
    pitch = mapMidi(pitch);
  } else if (mapMode = 2) {
    pitch = mapmajorDiatonic(pitch);
  } else if (mapMode = 3) {
    pitch = mapminorDiatonic(pitch);
  } 
    
  tuning = mozziAnalogRead(TUNING_PIN);
  
  // pitch: 0 = minimum value; 1024 = +32 semitones (~2.5 octaves) 
  // 6 bits left to get to decimal point + 5 bits more = 11 bit shift
  
  midiNote = MIDI_NOTE_MIN + (Q16n16)(pitch << 11);

  // Set the amount the pitch is quantized.
  midiNoteInt = (midiNote + 0x00008000) & 0xFFFF0000; // Nearest MIDI note (Q16n16 rounding)
  midiNoteFrac = (long)midiNote - (long)midiNoteInt; /* Fractional part only */
  midiNoteFrac = (midiNoteFrac * (1023 - tuning)) >> 10; /* Scale the fractional part */
  midiNote = midiNoteInt + midiNoteFrac;
  
  baseFrequency = Q16n16_mtof(midiNote);

  inBass.setFreq_Q16n16(baseFrequency); // down 1 octave
  inGuitar1.setFreq_Q16n16(baseFrequency * 3);
  inGuitar2.setFreq_Q16n16(baseFrequency << 2);
  inGuitar3.setFreq_Q16n16(baseFrequency * 5);

  bassButton = digitalRead(BASS_PIN);
  guitar1Button = digitalRead(GUITAR1_PIN);
  guitar2Button = digitalRead(GUITAR2_PIN);
  guitar3Button = digitalRead(GUITAR3_PIN);
  
  // For Debugging
  // Serial.println(finalOut, BIN);
  // Serial.print(pitch);
  // Serial.println(masterVolume);
  // Serial.println("-");
  //Serial.println(inBass.next());
}

int updateAudio(){

  out = ((bassButton * inBass.next()) + (guitar1Button * inGuitar1.next()) + (guitar2Button * inGuitar2.next() ) + (guitar3Button * inGuitar3.next()));
  finalOut = (out * masterVolume);
  return finalOut;  
}

void loop(){
  audioHook();
}





