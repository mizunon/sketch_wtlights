# WTLights Controller for M5StickC / ATOM / ATOMS3
ワイテルズさんのイベント「WhiteTails 最初で最後が武道館」のLEDライトを無改造でメンバーカラーで光らせられます。あの日の思い出を、おうちでもう一度。  
また、製作中に判明したのですが、他のライブのLEDライトでも、赤外線でシンクロするライトなら光る可能性があります（動作の保障はできません）。

## 概要
本プログラムを対応するM5開発キットに書きこむ事で、ライブのLEDライトを光らせる赤外線を送信。ライブのLEDライトがそれに反応して光ります。

## 機能と使い方
### 基本的な使い方
- ライブのLEDライトの電源を入れて、白く光っている事を確認します。
- 本プログラムを書きこんだM5開発キットをUSB電源に接続するか、（存在する場合は）本体の電源ボタンを押すと、電源が入って赤外線の送信を自動的に開始します。
  - 初期状態では、Nakamuさんの水色で光らせるモードでスタートします。
- 各機種の上面ボタンを短く押すとライティングモードを仮選択できます。
  - 仮選択されているライティングモードは、中央に大きく表示されます。画面が無い機種の場合はボタン中央のLEDに色で表示されます。
- 上面ボタンを長押しすると、選択中のライティングモードを決定して、LEDライトをそのモードで光らせられます。
  - 決定して送信中のライティングモードは画面上部に表示され、今どのモードを使っているかわかるようになっています。
  - 画面の無いAtom Liteでは、決定して送信中のライティングモードはわかりません。ライブのLEDライトを見て判断してください。
- M5StickCシリーズでは、側面ボタンを短押しすると、送信中のライティングモードを記憶して、本体の電源を切ることができます。
  - 次回起動時からは、これで記憶されたモードで自動的に赤外線の送信が開始されます。

### ライティングモード
様々な光らせ方で、LEDライトを楽しめます。
- メンバーカラー+ワイテルくん単色点灯
  - Nakamuさん→Broooockさん→シャークんさん→きんときさん→スマイルさん→きりやんさん→ワイテルくんさん
- 全メンバーカラー シンクロ点灯ループ「All」モード
  - 1.5秒ごとに全メンバーカラーを点灯してループします。
  - 複数のライトがあっても、全て同じカラーで点灯します。
- 全メンバーカラー シンクロフェードイン点灯ループ「FadeSync」モード
  - 3秒ごとに全メンバーカラーをフェードインしながら点灯してループします。
  - 複数のライトがあっても、全て同じカラーでフェードイン点灯します。
- 全メンバーカラー シンクロフラッシュ点灯ループ「FlashSync」モード
  - 0.5秒ごとに全メンバーカラーをフラッシュ点灯してループします。
  - 複数のライトがあっても、全て同じカラーで点灯します。
- 全メンバーカラー ランダムフェードイン点灯ループ「FadeRand」モード
  - 全メンバーカラーをフェードインしながら点灯してループします。
  - 複数のライトがある場合、ばらばらのカラーでフェードイン点灯します。（ランダムなので、結果的に同じカラーになってしまうことはあります）
- 全メンバーカラー ランダムフラッシュ点灯ループ「FlashRand」モード
  - 全メンバーカラーをフラッシュ点灯してループします。
  - 複数のライトがある場合、ばらばらのカラーでフラッシュ点灯します。（ランダムなので、結果的に同じカラーになってしまうことはあります）
- 消灯「Off」モード
  - ライトを消灯できます。電源OFFと区別がつかないので、電池の消耗に気を付けてください。（何度もやった）

### モード記憶機能
- M5StickCシリーズでは、側面ボタン(BtnB,BtnPWR)を短押しすると、送信中のライティングモードを記憶して、本体の電源を切ることができます。
  - 次回起動時からは、これで記憶されたモードで自動的に赤外線の送信が開始されます。
  - このモードはEEPROMのアドレス0から記録されます。なんらかの理由でここが書き換わると、起動時の動作も変わります。


## 対応機種
M5Stack開発キットシリーズの以下の開発キットに対応しています。  
一番のオススメは「M5StickC Plus2」です。  
このキットは、秋葉原にお店がある「[千石電商](https://www.sengoku.co.jp/shop_01.html)」さんや、「[スイッチサイエンス](https://www.switch-science.com/)」さん、Amazonで買えます。
- M5StickC, M5StickC Plus, M5StickC Plus2 （4千円前後）
  - バッテリーが内蔵されています。外部のUSB電源でも動きます。
  - 書き込みも動作も安定していて、赤外線出力も悪くない＆バッテリー内臓でいつでもどこでも光らせられる。
    - M5StickC Plus2以外のM5StickC, M5StickC Plusは旧機種なので、これから買う人はPlus2一択です。
  - 購入リンク
    - M5StickC Plus2 https://ssci.to/6470
    - M5StickC Plus https://ssci.to/9350
- ATOM Lite, ATOM Matrix （2～3千円）
  - バッテリーは内蔵されていません。外部のUSB電源が必要です。
  - このシリーズは赤外線送信パワーが弱いので、価格が安いこと以外はオススメしません。
  - 書き込み後にシリアルモニタを再接続（シリアルモニタボタンを2回押す）しないと、正常動作しない場合があります。
  - 購入リンク
    - ATOM Matrix https://ssci.to/6260
    - ATOM Lite https://ssci.to/6262
- ATOMS3,ATOMS3 Lite（2～3千円）
  - バッテリーは内蔵されていません。外部のUSB電源が必要です。
  - このシリーズは書き込み方法に下記のクセがあるのと、赤外線送信パワーが弱いので、小さくてかわいいこと以外はオススメしません。
    - 書き込み前にリセットボタンを3秒長押しして裏側LEDが緑点灯するのを確認しないといけない。
    - 書き込み後にリセットバタンを短押しして、リセットしないといけない。
  - 購入リンク
    - ATOMS3 https://ssci.to/8670
    - ATOMS3 Lite https://ssci.to/8778

## 注意点
- **実際のライブの現場や、公共の場所、知らない方がいる場所等、または他人の迷惑になりそうな場合は、絶対に使用しないでください。**
  - 同様の赤外線ライトが使われている現場で使用した場合は、通常の進行を妨げる恐れがあり、業務妨害に該当する恐れがあります。
- 複数台起動すると赤外線信号が競合してとんでもない光り方をします。ビカビカ光り出した場合は、2台の本体が起動してないかチェックしてみてください。
- M5開発キットの赤外線LEDは、テレビ等のリモコンと比べるととても弱い光です。キットによってはさらに弱く30cmでも届かなくなるので、なるべくLEDライトに近づけてあげてください。
- M5開発キットの赤外線LEDは、テレビ等のリモコンのように赤外線LEDがむき出しになっておらず、小さい穴の奥にあります。その為、かなり指向性があり、向きが重要です。
- ライブLEDライトは、**開発キットからの赤外線が2秒間受信できないと自動的に消灯**します。一度ついたけど消えた場合は、上記の赤外線LEDの距離や向きを変えて試してみてください。

## インストール方法
- Arduino IDEで、本プログラム「[sketch_wtlights.ino](https://github.com/mizunon/sketch_wtlights/blob/main/sketch_wtlights.ino)」を「Sketch」メニューから「Upload」して、開発キットに本プログラムを書き込んでください。
- Arduino IDE自体のインストールは下記の方のWebページがわかりやすいです。
  - あれさん / Arduino IDE 2.0でM5Stack開発環境を構築する
    - https://note.com/ats030/n/n0a3358b6d466
- このプログラムは「M5Unified」ライブラリを使用しています。「M5Unified」ライブラリをインストールしてから次の作業に移ってください。
  - 「M5Unified」ライブラリをインストールする手順で、依存関係のある他のライブラリも自動的にインストールするか聞かれるので、これらもインストールする事を選択して下さい。
  - 「ツール」→「ライブラリを管理…」でライブラリマネージャを開き、「M5Unified by M5Stack」をインストール。
- Aiduino IDE下部の「Output」部分に下記のように表示され、エラーが出なければ書き込みは成功です。一回でうまくいかない時もあるので、何回か試してみてください。
    - 下記のように表示されても、シリアルモニタの関連で動作が止まるときがあるので、その場合は、一度USBを抜き差ししてみたり、Arduino IDEのシリアルモニタをOFFしてからONにしてみてください。
```Console
Sketch uses 500777 bytes (15%) of program storage space. Maximum is 3145728 bytes.
Global variables use 26848 bytes (8%) of dynamic memory, leaving 300832 bytes for local variables. Maximum is 327680 bytes.
esptool.py v4.5.1
Serial port COM3
Connecting......
Chip is ESP32-PICO-D4 (revision v1.0)
Features: WiFi, BT, Dual Core, 240MHz, Embedded Flash, VRef calibration in efuse, Coding Scheme None
Crystal is 40MHz
MAC: e8:9f:6d:0a:56:1c
Uploading stub...
Running stub...
Stub running...
Changing baud rate to 1500000
Changed.
Configuring flash size...
Flash will be erased from 0x00001000 to 0x00005fff...
Flash will be erased from 0x00008000 to 0x00008fff...
Flash will be erased from 0x0000e000 to 0x0000ffff...
Flash will be erased from 0x00010000 to 0x0008afff...
Compressed 18992 bytes to 13110...
Writing at 0x00001000... (100 %)
Wrote 18992 bytes (13110 compressed) at 0x00001000 in 0.4 seconds (effective 357.4 kbit/s)...
Hash of data verified.
Compressed 3072 bytes to 137...
Writing at 0x00008000... (100 %)
Wrote 3072 bytes (137 compressed) at 0x00008000 in 0.1 seconds (effective 385.3 kbit/s)...
Hash of data verified.
Compressed 8192 bytes to 47...
Writing at 0x0000e000... (100 %)
Wrote 8192 bytes (47 compressed) at 0x0000e000 in 0.1 seconds (effective 581.1 kbit/s)...
Hash of data verified.
Compressed 501520 bytes to 270343...
Writing at 0x00010000... (5 %)
Writing at 0x00017eed... (11 %)
Writing at 0x0002366e... (17 %)
Writing at 0x0003afb7... (23 %)
Writing at 0x0004032c... (29 %)
Writing at 0x000457d5... (35 %)
Writing at 0x0004b011... (41 %)
Writing at 0x000507d5... (47 %)
Writing at 0x00055f5d... (52 %)
Writing at 0x0005b369... (58 %)
Writing at 0x00060a99... (64 %)
Writing at 0x00065ce6... (70 %)
Writing at 0x0006bfc6... (76 %)
Writing at 0x0007493e... (82 %)
Writing at 0x0007cae1... (88 %)
Writing at 0x00081ef4... (94 %)
Writing at 0x000878d3... (100 %)
Wrote 501520 bytes (270343 compressed) at 0x00010000 in 5.1 seconds (effective 783.7 kbit/s)...
Hash of data verified.

Leaving...
Hard resetting via RTS pin...
```

## TODO
- 振動反応モード
- 音声反応モード
- シリアルコントロールモード

## 開発メモ
- 赤外線データの2Tと4Tのナゾを解き明かしたい。他にもモードがありそう。
- ロガーつくってライブに行く。


## 他のライブのLEDライトのメモ
### 動作確認済機種
動作確認したら追加。

### 動作しない事の確認済機種
動作しないことを確認したら追加。

### 未確認候補
動作未確認のメモです！実際に動作しなくても責任はとれません。
- 「BABYMETAL LEGEND-MM 神器」
  - この方のを見る限りプロトコル同じっぽい
  - https://qiita.com/tatsuki1119/items/7e2444da04b4cc24f004
- 「Yuzuru Hanyu ICE STORY 2023 “GIFT”」
  - 形がまんま
  - https://x.com/jp_GIFTofficial/status/1629725616835788800
- 「Kazuki Kato Concert Tour 2024 ～Respection～♪」
  - 形がまんま
  - https://x.com/kazuki_kato1007/status/1760652176911962221


## 開発環境
- Arduino IDE: Version: 2.3.4
  - Date: 2024-12-03T10:03:03.490Z
  - CLI Version: 1.1.1
- Board:
  - M5Stack 2.1.3
- Libralies:
  - M5Stack: 0.4.6
  - M5Unified: 0.2.2


## 作成者
Takuhiro Mizuno / [@mizunon](https://twitter.com/mizunon)


## ライセンス
このプロジェクトはMITライセンスのもとで公開されています。  
"WTLights Controller" is under [MIT license](https://en.wikipedia.org/wiki/MIT_License).

