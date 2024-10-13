idf_component_register(SRCS "cube.c" "main.cpp" "AdafruitGFX/glcdfont.c" "AdafruitGFX/LEDCubePort.cpp" "AdafruitGFX/Adafruit_GFX.cpp" "Network/network.c" "Network/http_server.c" "Power/battery.c" "Power/charger.c"
    INCLUDE_DIRS "." 
    REQUIRES esp_timer esp_driver_gpio esp_driver_ledc esp_wifi nvs_flash esp-tls esp_http_server esp32-wifi-manager esp_adc
    EMBED_FILES Network/app_index.html)