// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

//#define BT_DEVICE_NAME ("HSA_Receiver(-_-)")
#define BT_DEVICE_NAME ("HSA_ESP_Receiver(-_-)")

//#define PIN_LRCLK (CONFIG_I2S_LRCK_PIN)
#define PIN_LRCLK (27)

//#define CONFIG_A2DP_SINK_OUTPUT_INTERNAL_DAC

////////////////
// HSA ESP BT //
////////////////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_log.h"

#include "esp_bt.h"          // $IDF_PATH/components/bt/bluedroid/api/

#include "bt_app_core.h"     // ./
#include "bt_app_av.h"       // ./

#include "esp_bt_main.h"     // $IDF_PATH/components/bt/bluedroid/api/
#include "esp_bt_device.h"   // ..
#include "esp_gap_bt_api.h"  // ..
#include "esp_a2dp_api.h"    // ..
#include "esp_avrc_api.h"    // ..

#include "driver/i2s.h"      //esp-idf/components/

// ADD-HSA
#include "driver/gpio.h"
#include "driver/i2c.h"      // ..
#include "esp_err.h"
#include "sdkconfig.h" // generated by "make menuconfig"

#include "ssd1366.h"
#include "font8x8_basic.h"

#define SDA_PIN GPIO_NUM_18
#define SCL_PIN GPIO_NUM_19


/* I2C */
void i2c_master_init()
{
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 1000000};
    i2c_param_config(I2C_NUM_0, &i2c_config);
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
}

void ssd1306_init()
{
    esp_err_t espRc;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);

    i2c_master_write_byte(cmd, OLED_CMD_SET_CHARGE_PUMP, true);
    i2c_master_write_byte(cmd, 0x14, true);

    i2c_master_write_byte(cmd, OLED_CMD_SET_SEGMENT_REMAP, true); // reverse left-right mapping
    i2c_master_write_byte(cmd, OLED_CMD_SET_COM_SCAN_MODE, true); // reverse up-bottom mapping

    i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_ON, true);
    i2c_master_stop(cmd);

    espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
    if (espRc == ESP_OK)
    {
        ESP_LOGI("SSD1306", "OLED configured successfully");
    }
    else
    {
        ESP_LOGE("SSD1306", "OLED configuration failed. code: 0x%.2X", espRc);
    }
    i2c_cmd_link_delete(cmd);
}

void task_ssd1306_display_pattern(void *ignore)
{
    i2c_cmd_handle_t cmd;

    for (uint8_t i = 0; i < 8; i++)
    {
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
        i2c_master_write_byte(cmd, 0xB0 | i, true);
        i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
        for (uint8_t j = 0; j < 128; j++)
        {
            if (!j % 10 == 0)
                i2c_master_write_byte(cmd, 0xFF >> (j % 8), true);
        }
        i2c_master_stop(cmd);
        i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
    }

    vTaskDelete(NULL);
}

void task_ssd1306_display_clear(void *ignore)
{
    i2c_cmd_handle_t cmd;

    uint8_t zero[128];
    for (uint8_t i = 0; i < 8; i++)
    {
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
        i2c_master_write_byte(cmd, 0xB0 | i, true);

        i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
        i2c_master_write(cmd, zero, 128, true);
        i2c_master_stop(cmd);
        i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
    }

    vTaskDelete(NULL);
}

void task_ssd1306_contrast(void *ignore)
{
    i2c_cmd_handle_t cmd;

    uint8_t contrast = 0;
    uint8_t direction = 1;
    while (true)
    {
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
        i2c_master_write_byte(cmd, OLED_CMD_SET_CONTRAST, true);
        i2c_master_write_byte(cmd, contrast, true);
        i2c_master_stop(cmd);
        i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
        vTaskDelay(1 / portTICK_PERIOD_MS);

        contrast += direction;
        if (contrast == 0xFF)
        {
            direction = -1;
        }
        if (contrast == 0x0)
        {
            direction = 1;
        }
    }
    vTaskDelete(NULL);
}

void task_ssd1306_scroll(void *ignore)
{
    esp_err_t espRc;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);

    i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);

    i2c_master_write_byte(cmd, 0x29, true); // vertical and horizontal scroll (p29)
    i2c_master_write_byte(cmd, 0x00, true);
    i2c_master_write_byte(cmd, 0x00, true);
    i2c_master_write_byte(cmd, 0x07, true);
    i2c_master_write_byte(cmd, 0x01, true);
    i2c_master_write_byte(cmd, 0x3F, true);

    i2c_master_write_byte(cmd, 0xA3, true); // set vertical scroll area (p30)
    i2c_master_write_byte(cmd, 0x20, true);
    i2c_master_write_byte(cmd, 0x40, true);

    i2c_master_write_byte(cmd, 0x2F, true); // activate scroll (p29)

    i2c_master_stop(cmd);
    espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
    if (espRc == ESP_OK)
    {
        ESP_LOGI("SSD1306", "Scroll command succeeded");
    }
    else
    {
        ESP_LOGE("SSD1306", "Scroll command failed. code: 0x%.2X", espRc);
    }

    i2c_cmd_link_delete(cmd);

    vTaskDelete(NULL);
}

void task_ssd1306_display_text(const void *arg_text)
{
    char *text = (char *)arg_text;
    uint8_t text_len = strlen(text);

    i2c_cmd_handle_t cmd;

    uint8_t cur_page = 0;

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

    i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
    i2c_master_write_byte(cmd, 0x00, true); // reset column
    i2c_master_write_byte(cmd, 0x10, true);
    i2c_master_write_byte(cmd, 0xB0 | cur_page, true); // reset page

    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    for (uint8_t i = 0; i < text_len; i++)
    {
        if (text[i] == '\n')
        {
            cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

            i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
            i2c_master_write_byte(cmd, 0x00, true); // reset column
            i2c_master_write_byte(cmd, 0x10, true);
            i2c_master_write_byte(cmd, 0xB0 | ++cur_page, true); // increment page

            i2c_master_stop(cmd);
            i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
            i2c_cmd_link_delete(cmd);
        }
        else
        {
            cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

            i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
            i2c_master_write(cmd, font8x8_basic_tr[(uint8_t)text[i]], 8, true);

            i2c_master_stop(cmd);
            i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
            i2c_cmd_link_delete(cmd);
        }
    }

    vTaskDelete(NULL);
}
/* I2C_END */

/* event for handler "bt_av_hdl_stack_up */
enum {
    BT_APP_EVT_STACK_UP = 0,
};

/* handler for bluetooth stack enabled events */
static void bt_av_hdl_stack_evt(uint16_t event, void *p_param);

void app_main()
{
    char *display_string[200];
    char *devname = BT_DEVICE_NAME;
    char *version = "Pre      Ver.005";
    sprintf(display_string, "////////////////\n// HSA ESP BT //\n////////////////\n                \n     Init...    \n                \n                \n%16.16s", version);
    sprintf(display_string, "////////////////\n// HSA ESP BT //\n////////////////\nBluetooth       \nA2DP-Receiver   \n(%14.14s)\n                \n%16.16s", devname, version);

    i2c_master_init();
    ssd1306_init();

    xTaskCreate(&task_ssd1306_display_clear, "ssd1306_display_clear",  10240, NULL, 6, NULL);
    vTaskDelay(100/portTICK_PERIOD_MS);
    //xTaskCreate(&task_ssd1306_display_text, "ssd1306_display_text",  10240,
    //	(void *)"Hello world!\nMulitine is OK!\nAnother line", 6, NULL);
    // Screen -> 16char x 8line
    xTaskCreate(&task_ssd1306_display_text, "ssd1306_display_text", 10240,
                (void *)display_string, 6, NULL);
    //xTaskCreate(&task_ssd1306_contrast, "ssid1306_contrast", 10240, NULL, 6, NULL);

    /* Initialize NVS — it is used to store PHY calibration data */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    i2s_config_t i2s_config = {
#ifdef CONFIG_A2DP_SINK_OUTPUT_INTERNAL_DAC
        .mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN,
#else
        .mode = I2S_MODE_MASTER | I2S_MODE_TX, // Only TX
#endif
        .sample_rate = 44100,
        .bits_per_sample = 16,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT, //2-channels
        //.channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
        .communication_format = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB,
        .dma_buf_count = 8,
        .dma_buf_len = 64, //
        //.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1 //Interrupt level 1
        .intr_alloc_flags = 0, // default interrupt priority
    };

    i2s_driver_install(0, &i2s_config, 0, NULL);
#ifdef CONFIG_A2DP_SINK_OUTPUT_INTERNAL_DAC
    i2s_set_pin(0, NULL);
#else
    i2s_pin_config_t pin_config = {
        .bck_io_num = CONFIG_I2S_BCK_PIN,
        .ws_io_num = PIN_LRCLK,
        .data_out_num = CONFIG_I2S_DATA_PIN,
        .data_in_num = -1 //Not used
    };

    i2s_set_pin(0, &pin_config);
#endif

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if (esp_bt_controller_init(&bt_cfg) != ESP_OK)
    {
        ESP_LOGE(BT_AV_TAG, "%s initialize controller failed\n", __func__);
        return;
    }

    if (esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT) != ESP_OK)
    {
        ESP_LOGE(BT_AV_TAG, "%s enable controller failed\n", __func__);
        return;
    }

    if (esp_bluedroid_init() != ESP_OK)
    {
        ESP_LOGE(BT_AV_TAG, "%s initialize bluedroid failed\n", __func__);
        return;
    }

    if (esp_bluedroid_enable() != ESP_OK)
    {
        ESP_LOGE(BT_AV_TAG, "%s enable bluedroid failed\n", __func__);
        return;
    }

    /* create application task */
    bt_app_task_start_up();

    /* Bluetooth device name, connection mode and profile set up */
    bt_app_work_dispatch(bt_av_hdl_stack_evt, BT_APP_EVT_STACK_UP, NULL, 0, NULL);
}

static void bt_av_hdl_stack_evt(uint16_t event, void *p_param)
{
    ESP_LOGD(BT_AV_TAG, "%s evt %d", __func__, event);
    switch (event)
    {
    case BT_APP_EVT_STACK_UP:
    {
        /* set up device name */
        char *dev_name = BT_DEVICE_NAME;
        esp_bt_dev_set_device_name(dev_name);

        /* initialize A2DP sink */
        esp_a2d_register_callback(&bt_app_a2d_cb);
        esp_a2d_register_data_callback(bt_app_a2d_data_cb);
        esp_a2d_sink_init();

        /* initialize AVRCP controller */
        esp_avrc_ct_init();
        esp_avrc_ct_register_callback(bt_app_rc_ct_cb);

        /* set discoverable and connectable mode, wait to be connected */
        esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
        break;
    }
    default:
        ESP_LOGE(BT_AV_TAG, "%s unhandled evt %d", __func__, event);
        break;
    }
}
