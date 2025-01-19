#include "EEPROM.h"
#include <M5Unified.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <Wire.h>

constexpr const char* const version = "1.53";

constexpr const int eeprom_address = 0;

constexpr const int wtModeCount = 11;
constexpr const int wtColors[] = { TFT_CYAN, TFT_RED, TFT_GREEN, TFT_BLUE, TFT_PURPLE, TFT_YELLOW, TFT_WHITE, TFT_LIGHTGRAY, TFT_DARKGREY, TFT_DARKGREY, TFT_BLACK };
constexpr const char* const wtModeName[] = { "Nakamu", "Broooock", "Sharken", "Kintoki", "Smile", "Kiriyan", "WT_Kun", "All", "FadeIn", "Flash", "Off" };

constexpr const float sendTimeMillis = 100;

constexpr const int rawPttnCount = 14;
constexpr const int rawDataCount = 28;
constexpr const char *codeStrData[] = {
  //Nakamu、Broooock、シャークん、きんとき、スマイル、きりやん、ワイテルくん

  //通常点灯
   "10000110" ,"10000001", "10000010" ,"10000100" ,"10000101" ,"10000011" ,"10000111"

  //フェードイン（青の設定ビットわけわからん）
  ,"10111000" ,"10110101" ,"10111001" ,"10110000" ,"10110100" ,"10111101" ,"10111100"

  //フェードアウト
  ,"10011100" ,"10010010" ,"10010100" ,"10011000" ,"10011010" ,"10010110" ,"10011110"
};
constexpr const char *codeStrDataOff = "10000000";

/* ------------------ */

IRsend *irsend; 
uint16_t kIrLed = 0;  // ESP8266 GPIO pin to use.
uint16_t send_data_buff[rawDataCount];

int display_h =0;
int display_w =0;

signed int wtmode_led = 0;
int wtmode_cur = 0;
int wtmode_chg = 0;
bool updateDisp = true;

float lastMillisSend = 0;
float lastMillisMode = 0;

void buildSendData( const char *data_string, uint16_t *send_data_buff )
{
  const uint16_t unit_us = 512;

  const uint16_t leader_1_us = unit_us * 8;
  const uint16_t leader_0_us = unit_us * 4;
  const uint16_t trailer_1_us = unit_us;
  const uint16_t trailer_0_us = unit_us * 2;
  const uint16_t data0_0_us = unit_us * 1;
  const uint16_t data1_0_us = unit_us * 2;

  ///リーダー部
  *send_data_buff++ = leader_1_us;
  *send_data_buff++ = leader_0_us;

  ///データ部
  uint8_t data = 0;

  for( int i = 0 ; i < 8 ; i++ ){
    char c = *data_string++;

    if( c == '0' ){
      *send_data_buff++ = unit_us;
      *send_data_buff++ = data0_0_us;

      data = (data << 1) | 0;
    }else{
      *send_data_buff++ = unit_us;
      *send_data_buff++ = data1_0_us;

      data = (data << 1) | 1;
    }
  }

  ///パリティ部
  data = ((data >> 4) & 0b1111) ^ (data & 0b1111);
  for( int i = 0 ; i < 4 ; i++ ){
    if( (data & 0b00001000) == 0 ){
      *send_data_buff++ = unit_us;
      *send_data_buff++ = data0_0_us;
    }else{
      *send_data_buff++ = unit_us;
      *send_data_buff++ = data1_0_us;
    }
    data = (data << 1) & 0b1111;
  }

  ///トレイラー部
  *send_data_buff++ = trailer_1_us;
  *send_data_buff++ = trailer_0_us;
}

void setup(void) {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setTextSize(2);

  display_h = M5.Display.height();
  display_w = M5.Display.width();

  switch(M5.getBoard()) {
    case m5::board_t::board_M5StickC:
      kIrLed = 9;
      break;
    case m5::board_t::board_M5StickCPlus:
      kIrLed = 9;
      break;
    case m5::board_t::board_M5StickCPlus2:
      kIrLed = 19;
      break;
    case m5::board_t::board_M5Atom:
      kIrLed = 12;
      break;
    case m5::board_t::board_M5AtomS3:
      kIrLed = 4;
      break;
    case m5::board_t::board_M5AtomS3Lite:
      kIrLed = 4;
      break;
  }
  irsend = new IRsend(kIrLed);
  irsend->begin();

  Serial.begin(115200);
  Serial.printf("Start. [Ver:%s kIrLed:%d]\n", version, kIrLed);

  if (!EEPROM.begin(500)) {
    Serial.println("Failed to initialise EEPROM");
  }
  wtmode_cur = EEPROM.readInt(eeprom_address);

  if( wtmode_cur <= -1 || wtModeCount <= wtmode_cur ) wtmode_cur = 0;
  wtmode_chg = wtmode_cur;

  //ボード名の取得
  const char* name;
  {
    switch (M5.getBoard()) {
      case m5::board_t::board_M5StickC:       name = "M5StickC";  break;
      case m5::board_t::board_M5StickCPlus:   name = "M5StickCPlus";  break;
      case m5::board_t::board_M5StickCPlus2:  name = "M5StickCPlus2"; break;
      case m5::board_t::board_M5Atom:         name = "M5Atom";  break;
      case m5::board_t::board_M5AtomS3:       name = "M5AtomS3";  break;
      default:                                name = "Other Device";  break;
    }
    Serial.printf("core:%s\n", name);
  }

}

void loop(void) {
  M5.update();  ///ボタン情報とか更新

  static constexpr const int colors[] = { TFT_WHITE, TFT_CYAN, TFT_RED, TFT_YELLOW, TFT_BLUE, TFT_GREEN };
  static constexpr const char* const names[] = { "none", "wasHold", "wasClicked", "wasPressed", "wasReleased", "wasDeciedCount" };

  ///キー入力処理
  {
    auto state = M5.BtnA.wasHold()          ? 1
            : M5.BtnA.wasClicked()          ? 2
            : M5.BtnA.wasPressed()          ? 3
            : M5.BtnA.wasReleased()         ? 4
            : M5.BtnA.wasDeciedClickCount() ? 5
                                            : 0;
    if (state) {
      Serial.printf("BtnA:%s  count:%d\n", names[state], M5.BtnA.getClickCount());

      if( state == 1 ){
        //長押し
        wtmode_cur = wtmode_chg;

        lastMillisSend = 0;
        lastMillisMode = 0;
        updateDisp = true;
      }else if( state == 2 ){
        ///短押し
        wtmode_chg++;
        if( wtModeCount <= wtmode_chg ) wtmode_chg = 0;

        updateDisp = true;
      }
    }

    if (M5.BtnB.wasClicked() || M5.BtnPWR.wasClicked()) {
      EEPROM.writeInt(eeprom_address,wtmode_cur);
      EEPROM.commit();

      delay(100);

      M5.Power.powerOff();
    }
  }

  ///モード別処理
  {
    float current = millis();

    if( 0 <= wtmode_cur && wtmode_cur <= 6 ){
      ///通常点灯モード
      wtmode_led = wtmode_cur;
    }

    if( wtmode_cur == 7 ){
      ///全色モード
      if( 1500 < current - lastMillisMode ){
        wtmode_led++;
        if( 6 <= wtmode_led ) wtmode_led = 0;

        lastMillisMode = current;
      }
    }

    if( wtmode_cur == 8 ){
      ///フェードインモード
      if( 3000 < current - lastMillisMode ){
        wtmode_led++;
        if( wtmode_led < 7 || 12 < wtmode_led ) wtmode_led = 7;

        lastMillisMode = current;
      }
    }

    if( wtmode_cur == 9 ){
      ///フラッシュモード
      if( 300 < current - lastMillisMode ){
        wtmode_led++;
        if( 6 <= wtmode_led ) wtmode_led = 0;

        lastMillisMode = current;
      }
    }

    if( wtmode_cur == 10 ){
      ///Offモード
      wtmode_led = -1;
    }
  }

  ///赤外線データ送信処理
  if( -1 <= wtmode_led ){
    float current = millis();

    if( sendTimeMillis < (current - lastMillisSend) ){
      lastMillisSend = current;

      Serial.printf("irsend->sendRaw Ver:%s, pin:%d, cur:%d, led:%d\n", version, kIrLed, wtmode_cur, wtmode_led);

      if( wtmode_led == -1 ){
        buildSendData( codeStrDataOff, send_data_buff);
        irsend->sendRaw(send_data_buff, rawDataCount, 38);  // Send a raw data at 38kHz.
      }else{
        buildSendData( codeStrData[wtmode_led], send_data_buff);
        irsend->sendRaw(send_data_buff, rawDataCount, 38);  // Send a raw data at 38kHz.
      }

      if( false ){
        Serial.printf("send data>");
        for(int i=0 ; i < rawDataCount ; i++ ) Serial.printf("[%d:%d]", i, send_data_buff[i]);
        Serial.printf("<\n");
      }
    }
  }

  ///画面更新処理
  if( updateDisp ) {
    if (!M5.Display.displayBusy())
    {
      M5.Display.startWrite();
      M5.Display.setClipRect(0, 0, display_w, display_h);
      M5.Display.waitDisplay();

      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setCursor(0, 0);

      M5.Display.fillRect(0, 0, display_w, display_h, wtColors[wtmode_chg]);

      M5.Display.setCursor( (display_w / 2) - (strlen(wtModeName[wtmode_chg]) / 2 * 16) , display_h / 2);
      M5.Display.print(wtModeName[wtmode_chg]);

      M5.Display.clearClipRect();
      M5.Display.endWrite();

      M5.Display.display();
      updateDisp = false;
    }
  }

  delay(1);
}