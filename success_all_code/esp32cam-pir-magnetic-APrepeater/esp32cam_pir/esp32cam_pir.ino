#include <WiFi.h>
#include "esp_camera.h"
#include "esp_system.h"

hw_timer_t *timer = NULL;
void IRAM_ATTR resetModule() {
  ets_printf("reboot\n");
  esp_restart();
}
#include <TridentTD_LineNotify.h>
#define SSID        "MyAP"   // WiFi name
#define PASSWORD    "88888888"   // PASSWORD
#define LINE_TOKEN  "pIqPU7Qv5Cej7HCNVGwKJ7ooK9xb0d5MWaLOGFLg4wM"
// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
//#define MAGNETIC 2

const int Led_Flash = 4;
const int Led_run = 13;
int PIR_Sensor = 12;
const int pingPin = 15;
int inPin = 14;
boolean startTimer = false;
unsigned long time_now = 0;
int time_capture = 0;
int magneticState = 2;

void setup() {

  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  pinMode(2, INPUT);
  pinMode(Led_Flash, OUTPUT);
  pinMode(Led_run, OUTPUT);
  WiFi.begin(SSID, PASSWORD);
  Serial.printf("WiFi connecting to %s\n",  SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(400);
  }
  Serial.printf("\nWiFi connected\nIP : ");
  Serial.println(WiFi.localIP());
  LINE.setToken(LINE_TOKEN);

  timer = timerBegin(0, 80, true); //timer 0, div 80Mhz
  timerAttachInterrupt(timer, &resetModule, true);
  timerAlarmWrite(timer, 20000000, false); //set time in us 15s
  timerAlarmEnable(timer); //enable interrupt

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    // FRAMESIZE_ +
    //QQVGA/160x120//QQVGA2/128x160//QCIF/176x144//HQVGA/240x176
    //QVGA/320x240//CIF/400x296//VGA/640x480//SVGA/800x600//XGA/1024x768
    //SXGA/1280x1024//UXGA/1600x1200//QXGA/2048*1536
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  }     else {
    config.frame_size = FRAMESIZE_QQVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}
void loop() {
  long duration, cm;
//  magneticState = digitalRead(MAGNETIC);
  pinMode(2, INPUT);
  pinMode(pingPin, OUTPUT);

  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);
  pinMode(inPin, INPUT);
  duration = pulseIn(inPin, HIGH);

  cm = microsecondsToCentimeters(duration);

  delay(100);

  timerWrite(timer, 0); //reset timer (feed watchdog)
  long tme = millis();
  if ((digitalRead(2) == LOW) && (digitalRead(12) == 1)){
    Serial.println("Magnetic = detected & PIR = detected");
   
    
    Camera_capture();
    Serial.println("OK");
    startTimer = true;
  } 
////////////////////////////////////////////////////////////////////////// PIR
  if (digitalRead(12) == 0) {
    time_capture = 0;
    Serial.println("PIR = 0");
    delay(300);
  }

  if (digitalRead(12) == 1) {
    time_capture = 0;
    Serial.println("PIR = 1");
    delay(300);
  }
////////////////////////////////////////////////////////////////////// Magnetic
  if (digitalRead(2) == LOW){
    time_capture = 1;
    Serial.println("Magnetic = 1");
    delay(300);
  }

  if (digitalRead(2) == HIGH){
    time_capture = 0;
    Serial.println("Magnetic = 0");
    delay(300);
  }
/////////////////////////////////////////////////////////////////////
}

long microsecondsToCentimeters(long microseconds) {
  return microseconds / 29 / 2;
}

void Camera_capture() {
  digitalWrite(Led_Flash, HIGH);
  delay(100);
  digitalWrite(Led_Flash, LOW);
  delay(100);
  digitalWrite(Led_Flash, HIGH);
  camera_fb_t * fb = NULL;
  delay(200);
  // Take Picture with Camera
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  digitalWrite(Led_Flash, LOW);
  Send_line(fb->buf, fb->len);
  esp_camera_fb_return(fb);
  // Serial.println("Going to sleep now");
  // esp_deep_sleep_start();
  // Serial.println("This will never be printed");

}

void Send_line(uint8_t *image_data, size_t   image_size) {
  LINE.notifyPicture("ตรวจพบการเคลื่อนไหว", image_data, image_size);
}
