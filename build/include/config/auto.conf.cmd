deps_config := \
	/home/hsasheep/Git/esp/esp-idf/components/app_trace/Kconfig \
	/home/hsasheep/Git/esp/esp-idf/components/aws_iot/Kconfig \
	/home/hsasheep/Git/esp/esp-idf/components/bt/Kconfig \
	/home/hsasheep/Git/esp/esp-idf/components/driver/Kconfig \
	/home/hsasheep/Git/esp/esp-idf/components/esp32/Kconfig \
	/home/hsasheep/Git/esp/esp-idf/components/esp_adc_cal/Kconfig \
	/home/hsasheep/Git/esp/esp-idf/components/ethernet/Kconfig \
	/home/hsasheep/Git/esp/esp-idf/components/fatfs/Kconfig \
	/home/hsasheep/Git/esp/esp-idf/components/freertos/Kconfig \
	/home/hsasheep/Git/esp/esp-idf/components/heap/Kconfig \
	/home/hsasheep/Git/esp/esp-idf/components/libsodium/Kconfig \
	/home/hsasheep/Git/esp/esp-idf/components/log/Kconfig \
	/home/hsasheep/Git/esp/esp-idf/components/lwip/Kconfig \
	/home/hsasheep/Git/esp/esp-idf/components/mbedtls/Kconfig \
	/home/hsasheep/Git/esp/esp-idf/components/openssl/Kconfig \
	/home/hsasheep/Git/esp/esp-idf/components/pthread/Kconfig \
	/home/hsasheep/Git/esp/esp-idf/components/spi_flash/Kconfig \
	/home/hsasheep/Git/esp/esp-idf/components/spiffs/Kconfig \
	/home/hsasheep/Git/esp/esp-idf/components/tcpip_adapter/Kconfig \
	/home/hsasheep/Git/esp/esp-idf/components/wear_levelling/Kconfig \
	/home/hsasheep/Git/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/hsasheep/Git/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/home/hsasheep/Git/esp/a2dp_sink/main/Kconfig.projbuild \
	/home/hsasheep/Git/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/hsasheep/Git/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
