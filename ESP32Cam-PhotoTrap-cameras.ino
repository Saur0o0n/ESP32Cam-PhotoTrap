// Set camera settings, one provided below are (in my taste) most efficient for further processing
//

/* OV5640 - 5Mpx camera
*/
void init_ov5640_camera(camera_config_t *config){

  config->ledc_channel = LEDC_CHANNEL_0;
  config->ledc_timer = LEDC_TIMER_0;
  config->pin_d0 = Y2_GPIO_NUM;
  config->pin_d1 = Y3_GPIO_NUM;
  config->pin_d2 = Y4_GPIO_NUM;
  config->pin_d3 = Y5_GPIO_NUM;
  config->pin_d4 = Y6_GPIO_NUM;
  config->pin_d5 = Y7_GPIO_NUM;
  config->pin_d6 = Y8_GPIO_NUM;
  config->pin_d7 = Y9_GPIO_NUM;
  config->pin_xclk = XCLK_GPIO_NUM;
  config->pin_pclk = PCLK_GPIO_NUM;
  config->pin_vsync = VSYNC_GPIO_NUM;
  config->pin_href = HREF_GPIO_NUM;
  config->pin_sscb_sda = SIOD_GPIO_NUM;
  config->pin_sscb_scl = SIOC_GPIO_NUM;
  config->pin_pwdn = PWDN_GPIO_NUM;
  config->pin_reset = RESET_GPIO_NUM;
  config->xclk_freq_hz = 10000000;
  config->pixel_format = PIXFORMAT_JPEG;

  if(psramFound()){
    //config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config->frame_size = FRAMESIZE_QSXGA;
    config->jpeg_quality = 6;
    config->fb_count = 1;  // All examples have 2, but since we are NOT streaming but taking pictures - this is acctualy better (https://github.com/espressif/esp32-camera/blob/master/README.md#important-to-remember)
  } else {
    config->frame_size = FRAMESIZE_SVGA;
    config->jpeg_quality = 10;
    config->fb_count = 1;
  }
}
void set_ov5640_camera(){
  sensor_t * s = esp_camera_sensor_get();

  s->set_brightness(s, 0);     // -2 to 2
  s->set_contrast(s, 0);       // -2 to 2
  s->set_saturation(s, 0);     // -2 to 2
  s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  
  s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
  s->set_awb_gain(s, 0);       // 0 = disable , 1 = enable

  s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
  s->set_aec2(s, 0);           // Auto EXP DSP 0 = disable , 1 = enable
  s->set_ae_level(s, 0);       // -2 to 2
  s->set_aec_value(s, 885);    // 0 to 1200

  s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
  s->set_agc_gain(s, 16);       // 0 to 30
  s->set_gainceiling(s, (gainceiling_t)248);  // 0 to 6


  s->set_bpc(s, 1);            // 0 = disable , 1 = enable
  s->set_wpc(s, 1);            // 0 = disable , 1 = enable
  s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
  s->set_lenc(s, 1);           // 0 = disable , 1 = enable
  s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
  s->set_vflip(s, 0);          // 0 = disable , 1 = enable
  s->set_dcw(s, 1);            // 0 = disable , 1 = enable
  s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
}

/* OV2640 - 3Mpx camera
*/
void init_ov2640_camera(camera_config_t *config){

  config->ledc_channel = LEDC_CHANNEL_0;
  config->ledc_timer = LEDC_TIMER_0;
  config->pin_d0 = Y2_GPIO_NUM;
  config->pin_d1 = Y3_GPIO_NUM;
  config->pin_d2 = Y4_GPIO_NUM;
  config->pin_d3 = Y5_GPIO_NUM;
  config->pin_d4 = Y6_GPIO_NUM;
  config->pin_d5 = Y7_GPIO_NUM;
  config->pin_d6 = Y8_GPIO_NUM;
  config->pin_d7 = Y9_GPIO_NUM;
  config->pin_xclk = XCLK_GPIO_NUM;
  config->pin_pclk = PCLK_GPIO_NUM;
  config->pin_vsync = VSYNC_GPIO_NUM;
  config->pin_href = HREF_GPIO_NUM;
  config->pin_sscb_sda = SIOD_GPIO_NUM;
  config->pin_sscb_scl = SIOC_GPIO_NUM;
  config->pin_pwdn = PWDN_GPIO_NUM;
  config->pin_reset = RESET_GPIO_NUM;
  config->xclk_freq_hz = 20000000;
  config->pixel_format = PIXFORMAT_JPEG;

  if(psramFound()){
    config->frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config->jpeg_quality = 5;
    config->fb_count = 1;  // All examples have 2, but since we are NOT streaming but taking pictures - this is acctualy better (https://github.com/espressif/esp32-camera/blob/master/README.md#important-to-remember)
  } else {
    config->frame_size = FRAMESIZE_SVGA;
    config->jpeg_quality = 10;
    config->fb_count = 1;
  }
}
void set_ov2640_camera(){
  sensor_t * s = esp_camera_sensor_get();

  s->set_brightness(s, 0);     // -2 to 2
  s->set_contrast(s, 0);       // -2 to 2
  s->set_saturation(s, 0);     // -2 to 2
  s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  
  s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_whitebal(s, 1);       // (awb) 0 = disable , 1 = enable
  s->set_awb_gain(s, 1);       // (awb_gain) 0 = disable , 1 = enable

  s->set_exposure_ctrl(s, 1);  // (aec) 0 = disable , 1 = enable
  s->set_aec2(s, 0);           // (aec2) Auto EXP DSP 0 = disable , 1 = enable
  s->set_ae_level(s, 0);       // (ae_level) -2 to 2
  s->set_aec_value(s, 168);    // (aec_value) 0 to 1200

  s->set_gain_ctrl(s, 1);      // (agc) 0 = disable , 1 = enable
  s->set_agc_gain(s, 0);       // (agc_gain) 0 to 30
  s->set_gainceiling(s, (gainceiling_t)0);  // (gainceiling) 0 to 6

  s->set_bpc(s, 0);            // 0 = disable , 1 = enable
  s->set_wpc(s, 1);            // 0 = disable , 1 = enable
  s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
  s->set_lenc(s, 1);           // 0 = disable , 1 = enable
  s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
  s->set_vflip(s, 1);          // 0 = disable , 1 = enable
  s->set_dcw(s, 0);            // 0 = disable , 1 = enable
  s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
}
