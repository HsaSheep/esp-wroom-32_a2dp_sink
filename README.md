ESP-WROOM-32 Bluetooth Receiver and Controller
==============================================

[esp-idf](https://github.com/espressif/esp-idf.git "Github")

[esp-idf I2C](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/i2c "Github")

[ESP-WROOM-32 Datasheet](http://akizukidenshi.com/download/ds/espressifsystems/esp_wroom_32_datasheet_en.pdf "Github")

[I2C LCD](http://akizukidenshi.com/download/ds/xiamen/AQM0802.pdf "秋月電子通商")

### MEMO ###

Todo :

* i2clcd
* -> interface
* button
* reconnect

Pin :

* IF

| ESP pin   | Name         |
| :-------- | :----------- |
| GPIO12    | Switch_Left  |
| GPIO13    | Switch_Enter |
| GPIO14    | Switch_Right |

* I2C

| ESP pin   | Name         |
| :-------- | :----------- |
| GPIO18    | SDA          |
| GPIO19    | SCL          |

* I2S

| ESP pin   | Name         |
| :-------- | :----------- |
| GPIO22    | LRCK         |
| GPIO25    | DATA         |
| GPIO26    | BCK          |


# ----- ORIGINAL README ----- #

ESP-IDF A2DP-SINK demo
======================

Demo of A2DP audio sink role

This is the demo of API implementing Advanced Audio Distribution Profile to receive an audio stream.

For the I2S codec, pick whatever chip or board works for you; this code was written using a PCM5102 chip, but other I2S boards and chips will probably work as well. The default I2S connections are shown below, but these can be changed in menuconfig:

| ESP pin   | I2S signal   |
| :-------- | :----------- |
| GPIO22    | LRCK         |
| GPIO25    | DATA         |
| GPIO26    | BCK          |

If the internal DAC is selected, analog audio will be available on GPIO25 and GPIO26. The output resolution on these pins will always be limited to 8 bit because of the internal structure of the DACs.


After the program is started, other bluetooth devices such as smart phones can discover a device named "ESP_SPEAKER". Once a connection is established, audio data can be transmitted. This will be visible in the application log including a count of audio data packets.
