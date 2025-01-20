#include "EEPROM.h"
#include <M5Unified.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <Wire.h>

constexpr const char* const version = " V1.65 ";

constexpr const int eeprom_address = 0;

constexpr const int wtModeCount = 13;
constexpr const int wtColors[] = { TFT_CYAN, TFT_RED, TFT_GREEN, TFT_BLUE, TFT_PURPLE, TFT_YELLOW, TFT_WHITE, TFT_LIGHTGRAY, TFT_DARKGREY, TFT_DARKGREY, TFT_DARKGREY, TFT_DARKGREY, TFT_BLACK };
constexpr const int wtColorsNeopixel[] = { 0x00ffff, 0xff0000, 0x00ff00, 0x0000ff, 0xff00ff, 0xffff00, 0xffffff, 0xdddddd, 0xbbbbbb, 0x999999, 0x777777, 0x555555, 0x000000 };
constexpr const char* const wtModeName[] = { " Nakamu ", " Broooock ", " Sharken ", " Kintoki ", " Smile ", " Kiriyan ", " WT_Kun ", " All ", " FadeSync ", " FlashSync ", " FadeRand ", " FlashRand ", " Off " };

constexpr const float sendTimeMillis = 100;

constexpr const int rawDataCount = 28;
constexpr const char *codeStrData[] = {
  //Nakamu、Broooock、シャークん、きんとき、スマイル、きりやん、ワイテルくん
   "a10000110" ,"a10000001", "a10000010" ,"a10000100" ,"a10000101" ,"a10000011" ,"a10000111"  //通常点灯
  // "a10000110" ,"a10000001", "a10000010" ,"a10000100" ,"b10000010" ,"a10000011" ,"a10000111" //本当はこっちの方がスマイルさんの紫になるが当日の会場の色とズレる＆フェードイン紫が無いので・・・
  ,"a10011100" ,"a10010010" ,"a10010100" ,"a10011000" ,"a10011010" ,"a10010110" ,"a10011110"  //フェードイン（紫のフェードインが無い）
  ,"a10111000" ,"a10110101" ,"a10111001" ,"a10110000" ,"a10110100" ,"a10111101" ,"a10111100"  //フェードイン２（青の設定ビットわけわからん）
  ,"a10101100" ,"a10100010", "a10100100" ,"a10101000" ,"a10101010" ,"a10100110" ,"a10101110"  //点滅(遅い)
  ,"a10101101" ,"a10100011", "a10100101" ,"a10101001" ,"a10101011" ,"a10100111" ,"a10101111"  //点滅(早い)
};
constexpr const char *codeStrDataFadeRandSlow = "b01100000";
constexpr const char *codeStrDataFadeRandFast = "b01100001";
constexpr const char *codeStrDataFlashRand0 = "b01000101";
constexpr const char *codeStrDataFlashRand1 = "b01000011";
constexpr const char *codeStrDataOff = "a10000000";

/// 4TのパターンBプロトコルメモ
/// https://docs.google.com/spreadsheets/d/1z6DeR_bfE9Xn-Ck9KBPxR3s_5bTczSzXIgt5QId3XPs/edit?usp=sharing

/* ------------------ */

IRsend *irsend; 
uint16_t kIrLed = 0;  // ESP8266 GPIO pin to use.
uint16_t send_data_buff[rawDataCount];

uint16_t kRgbLed = 0;
bool haveDisplay = false;

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
  const uint16_t data1a_0_us = unit_us * 2;  //ここが2と4で機能が変わる
  const uint16_t data1b_0_us = unit_us * 4;  //ここが2と4で機能が変わる

  ///リーダー部
  *send_data_buff++ = leader_1_us;
  *send_data_buff++ = leader_0_us;

  //1のspace時間を判定。
  uint16_t data1_0_us = data1a_0_us;
  if( *data_string++ == 'b' ) data1_0_us = data1b_0_us;

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

  /// Model: https://docs.m5stack.com/ja/products?id=%E3%82%B3%E3%83%B3%E3%83%88%E3%83%AD%E3%83%BC%E3%83%A9%E3%83%BCAtom_id
  /// Model_ID: https://github.com/m5stack/M5GFX/blob/master/src/lgfx/boards.hpp#L59
  switch(M5.getBoard()) {
    case m5::board_t::board_M5StickC:
      kIrLed = 9;
      kRgbLed = 0;
      haveDisplay = true;
      break;
    case m5::board_t::board_M5StickCPlus:
      kIrLed = 9;
      kRgbLed = 0;
      haveDisplay = true;
      break;
    case m5::board_t::board_M5StickCPlus2:
      kIrLed = 19;
      kRgbLed = 0;
      haveDisplay = true;
      break;
    case m5::board_t::board_M5Atom:
      kIrLed = 12;
      kRgbLed = 27;
      haveDisplay = false;
      break;
    case m5::board_t::board_M5AtomMatrix:
      kIrLed = 12;
      kRgbLed = 27;
      haveDisplay = false;
      break;
    case m5::board_t::board_M5AtomU:
      kIrLed = 12;
      kRgbLed = 27;
      haveDisplay = false;
      break;
    case m5::board_t::board_M5AtomS3:
      kIrLed = 4;
      kRgbLed = 0;
      haveDisplay = true;
      break;
    case m5::board_t::board_M5AtomS3Lite:
      kIrLed = 4;
      kRgbLed = 35;
      haveDisplay = false;
      break;
    case m5::board_t::board_M5AtomS3U:
      kIrLed = 12;
      kRgbLed = 35;
      haveDisplay = false;
      break;
  }

  if( haveDisplay ){
    display_h = M5.Display.height();
    display_w = M5.Display.width();
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
      ///ランダムフェード
      wtmode_led = -2;
    }

    if( wtmode_cur == 11 ){
      ///フラッシュモード
      if( 1000 < current - lastMillisMode ){
        if( -3 <= wtmode_led ) wtmode_led = -4;
        else wtmode_led = -3;

        lastMillisMode = current;
      }
    }

    if( wtmode_cur == 12 ){
      ///Offモード
      wtmode_led = -1;
    }
  }

  ///赤外線データ送信処理
  if( -4 <= wtmode_led ){
    float current = millis();

    if( sendTimeMillis < (current - lastMillisSend) ){
      lastMillisSend = current;

      Serial.printf("irsend->sendRaw Ver:%s, ir_pin:%d, rgb_pin:%d, cur:%d, led:%d\n", version, kIrLed, kRgbLed, wtmode_cur, wtmode_led);

      if( wtmode_led == -1 ){
        buildSendData( codeStrDataOff, send_data_buff);
        irsend->sendRaw(send_data_buff, rawDataCount, 38);  // Send a raw data at 38kHz.
      }else if( wtmode_led == -2 ){
        buildSendData( codeStrDataFadeRandSlow, send_data_buff);
        irsend->sendRaw(send_data_buff, rawDataCount, 38);  // Send a raw data at 38kHz.
      }else if( wtmode_led == -3 ){
        buildSendData( codeStrDataFlashRand0, send_data_buff);
        irsend->sendRaw(send_data_buff, rawDataCount, 38);  // Send a raw data at 38kHz.
      }else if( wtmode_led == -4 ){
        buildSendData( codeStrDataFlashRand1, send_data_buff);
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
    if ( haveDisplay && !M5.Display.displayBusy())
    {
      M5.Display.startWrite();
      M5.Display.setClipRect(0, 0, display_w, display_h);
      M5.Display.waitDisplay();

      M5.Display.fillScreen(TFT_BLACK);

      M5.Display.setTextSize(2);
      M5.Display.setCursor(0, 0);

      //選択中のモード
      M5.Display.fillRect(0, 0, display_w, display_h, wtColors[wtmode_chg]);
      M5.Display.drawCentreString(wtModeName[wtmode_chg], display_w / 2, display_h / 2, (uint8_t)1);

      //現在のモード
      const int font_h = 16;
      M5.Display.fillRect(0, 0, display_w, font_h * 2, wtColors[wtmode_cur]);
      M5.Display.drawCentreString("^^^", display_w / 2, font_h * 0, (uint8_t)0);
      M5.Display.drawCentreString(wtModeName[wtmode_cur], display_w / 2, font_h * 1, (uint8_t)0);
      M5.Display.fillRect(0, font_h * 2, display_w, 2, wtColors[wtmode_cur]);
      M5.Display.fillRect(0, font_h * 2 + 2, display_w, 4, TFT_BLACK);

      M5.Display.drawCentreString( version, display_w / 2, display_h - font_h * 2, (uint8_t)0);
      M5.Display.drawCentreString( "@mizunon", display_w / 2, display_h - font_h * 1, (uint8_t)0);

      M5.Display.clearClipRect();
      M5.Display.endWrite();

      M5.Display.display();
      updateDisp = false;
    }

    if( 0 < kRgbLed ){
      neopixelWrite(kRgbLed, wtColorsNeopixel[wtmode_chg] >> 16 & 0x0f, wtColorsNeopixel[wtmode_chg] >> 8 & 0x0f, wtColorsNeopixel[wtmode_chg] & 0x0f );
      delay(10);
    }
  }

  delay(1);
}