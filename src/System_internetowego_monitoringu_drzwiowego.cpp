#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>
#include <ArduinoWebsockets.h>



#define CAMERA_MODEL_AI_THINKER

#include "camera_pins.h"

const char* ssid = "multimedia_AlpakaNet";
const char* password = "Alpaka321Syb";
const char* websocket_server_host = "34.89.138.247 ";
const uint16_t websocket_server_port = 65080;

bool camState = 0;
 unsigned char * encoded;



using namespace websockets;

 void onMessageCallback(WebsocketsMessage message) {
    Serial.print("Got Message: ");
    Serial.println(message.data());
    const char* mess = (message.data()).c_str();
    
    if (message.data()=="1"){
    camState = 1;
    Serial.println("nagrywamy ;)");
    }
    else if (message.data()=="0") camState=0;
    

}

void onEventsCallback(WebsocketsEvent event, String data) {
    if(event == WebsocketsEvent::ConnectionOpened) {
        Serial.println("Connnection Opened");
    } else if(event == WebsocketsEvent::ConnectionClosed) {
        Serial.println("Connnection Closed");
    } else if(event == WebsocketsEvent::GotPing) {
        Serial.println("Got a Ping!");
    } else if(event == WebsocketsEvent::GotPong) {
        Serial.println("Got a Pong!");
    }
}

WebsocketsClient client;



void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  
  pinMode(2, INPUT);
  digitalWrite(2, LOW);


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
      config.xclk_freq_hz = 10000000;
      config.pixel_format = PIXFORMAT_JPEG;
      //init with high specs to pre-allocate larger buffers
      if(psramFound()){
        config.frame_size = FRAMESIZE_VGA;
        config.jpeg_quality = 45;
        config.fb_count = 1;
        Serial.printf("VGA");
      } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
        Serial.printf("SVGA");
  }


  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  delay(2000);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
   
  }
  Serial.println("");
  Serial.println("WiFi connected");

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");


  // run callback when messages are received
    client.onMessage(onMessageCallback);
    
    // run callback when events are occuring
    client.onEvent(onEventsCallback);
    
  while(!client.connect(websocket_server_host, websocket_server_port, "/")){
    delay(500);
    Serial.print(".");
  }
   Serial.println("Websocket Connected!");

  // Send a ping
    client.ping();

    
}

void loop() {

   client.poll();
   
   if(digitalRead(2)==HIGH){
    Serial.println("wykryłem ruch");
     camera_fb_t *fb = esp_camera_fb_get();
      if(!fb){
        Serial.println("Camera capture failed");
        esp_camera_fb_return(fb);
        return;
      }
    
      if(fb->format != PIXFORMAT_JPEG){
        Serial.println("Non-JPEG data not implemented");
        return;
      }
      
      client.sendBinary((const char*) fb->buf, fb->len);
      esp_camera_fb_return(fb);
      delay(900);
    }  
    
  
   
  if(camState==1){
  camera_fb_t *fb = esp_camera_fb_get();
    if(!fb){
      Serial.println("Camera capture failed");
      esp_camera_fb_return(fb);
      return;
    }
  
    if(fb->format != PIXFORMAT_JPEG){
      Serial.println("Non-JPEG data not implemented");
      return;
    }
    
    client.sendBinary((const char*) fb->buf, fb->len);
    esp_camera_fb_return(fb);

  }
}
