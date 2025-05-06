#define HID_CUSTOM_LAYOUT
#define LAYOUT_JAPANESE
#include <HID-Project.h>


const int LED_PIN = 17;
const int FOOT = 21;
const int SWITCH = 2;

const uint8_t CHATTERING = 100;

uint16_t timerCount = 0;
uint8_t mode = 0;

const uint8_t KEY_OUT_NUM = 5;
const uint8_t KEY_IN_NUM = 6;
const uint8_t KEY_OUTPUT[] = {6, 7, 8, 9, 10};
const uint8_t KEY_IN[] = {16, 14, 15, 18, 19, 20};
const uint8_t KEYS[] = {
// DEL   ENT    BS   n    m    ,< 
  0x00, 0x0A, 0x08, 'n', 'm', ',', 
// \_    /    .    l    ;    :
  '\\', '/', '.', 'l', ';', ':', 
//'k', 'j', 'h', 'y', 'u', 'i',
  'k', 'j', 'h', 'y', 'u', 'i',
// [    p    o    0    -    ] 
  '[', 'p', 'o', '0', '-', ']', 
// 9    8    7    @    ^    \|
  '9', '8', '7', '@', '^', '|',
};
// const uint8_t KEYS[] = {
// // DEL   ENT    BS    n     m     ,< 
//   0x4C, 0x28, 0x2A, 0x11, 0x10, 0x36, 
// //  \_    /?    .>    l     ;+    :*
//   0x87, 0x38, 0x37, 0x0F, 0x33, 0x34,
// //   k', 'j',  'h',  'y',  'u', 'i',
//   0x0E, 0x0D, 0x0B, 0x1C, 0x18, 0x0C,
// //  [{    p     o     0     -=    ]} 
//   0x30, 0x13, 0x12, 0x27, 0x2D, 0x32,
// //  9     8     7     @`    ^~    \| 
//   0x26, 0x25, 0x24, 0x2F, 0x2E, 0x89,
// };

const uint8_t SKEYS[] = {
// DEL   ENT    BS    n     m     ,< 
  0x00, 0x0F, 0x18, 0x00, 0x12, 0x16, 
//  \_    /?    .>    l     ;+    :*
  0x00, 0x00, 0x13, 0x14, 0x00, 0x00,
//   k', 'j',  'h',  'y',  'u', 'i',
  0x00, 0x15, 0x00, 0x00, 0x10, 0x17,
//  [{    p     o     0     -=    ]} 
  0x00, 0x00, 0x11, 0x0D, 0x00, 0x00,
//  9     8     7     @`    ^~    \| 
  0x0E, 0x0C, 0x00, 0x01, 0x02, 0x00,
};

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_PIN, OUTPUT);                                                                                                                                                                                                                 
  pinMode(FOOT, INPUT_PULLUP);
  pinMode(SWITCH, INPUT_PULLUP);
  for(int in = 0; in < KEY_IN_NUM; in++){
    pinMode(KEY_IN[in], INPUT_PULLUP);
  }
  for(int out = 0; out < KEY_OUT_NUM; out++){
    pinMode(KEY_OUTPUT[out], OUTPUT);
    digitalWrite(KEY_OUTPUT[out], HIGH);
  }
  Serial.begin(9600);

  // Timer設定 100ms
  const uint16_t TIMER_COUNT = 25000;
  TCCR1A = 0;
  TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);
  OCR1A = TIMER_COUNT - 1;
  TIMSK1 = (1 << OCIE1A);

  bool light = 1;

  BootKeyboard.begin();
  digitalWrite(LED_PIN, mode);

}

ISR(TIMER1_COMPA_vect){
  const uint16_t MAX_COUNT = 36000;  

  if(digitalRead(SWITCH) == LOW){
    autoKey(timerCount);
  }

  timerCount = timerCount < MAX_COUNT ? timerCount+ 1 : 0;

}


void loop() {
  keySearch();
  footKey();
}

void keySearch(){
  static int pressed[KEY_OUT_NUM * KEY_IN_NUM] = {0};
  
  for(int out = 0; out < KEY_OUT_NUM; out++){
    digitalWrite(KEY_OUTPUT[out], LOW);
    for(int in = 0; in < KEY_IN_NUM; in++){
      int p = out*KEY_IN_NUM + in;
      if(digitalRead(KEY_IN[in]) == LOW){
        if(pressed[p] == 0){
          if(p>0){
            BootKeyboard.press(mode == 0? KEYS[p]: SKEYS[p]);
          }else{
            mode = !mode;
            digitalWrite(LED_PIN, mode);
            BootKeyboard.releaseAll();
            for(int i = 0; i < KEY_OUT_NUM * KEY_IN_NUM; i++){
              pressed[i] = 0;
            }
          }
          pressed[p] = CHATTERING;
         }
      }else{
        if(pressed[p] != 0){
          if(pressed[p] >1){
            pressed[p]--;
          }else{
            if(p>0){
              BootKeyboard.release(mode == 0? KEYS[p]: SKEYS[p]);
            }
            pressed[p] = 0;
          }
        }
      }
    }
    digitalWrite(KEY_OUTPUT[out], HIGH);
  }
}

void footKey(){
  const char key = ' ';
  static uint8_t before = 0;
  if(digitalRead(FOOT) == LOW){
    if(before == 0){
      BootKeyboard.press(key);
      before = CHATTERING;
    }
  }else{
    if(before>1){
      before--;
    }
    if(before == 1){
      BootKeyboard.release(key);
      before = 0;
    }
  }
}

void autoKey(uint16_t count){
  static uint8_t key = 0x17;
  uint16_t turn = count % 200;

  if(turn % 10 == 0){
    BootKeyboard.press(key);
  }else if(turn % 10 == 1){
    BootKeyboard.release(key);
  }else if(turn == 99){
    key = 0x17;
  }else if(turn == 199){
    key = 0x16;
  }
}
