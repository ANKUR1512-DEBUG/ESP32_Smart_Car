#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_server.h"

// AI Thinker ESP32-CAM
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

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

httpd_handle_t page_httpd = NULL;
httpd_handle_t stream_httpd = NULL;

static esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t *fb = NULL;

  esp_err_t res = httpd_resp_set_type(
    req,
    "multipart/x-mixed-replace;boundary=frame"
  );

  if (res != ESP_OK)
    return res;

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr(req, "Cache-Control", "no-cache");

  char header[64];

  while (true) {

    fb = esp_camera_fb_get();

    if (!fb) {
      res = ESP_FAIL;
      break;
    }

    int hlen = snprintf(
      header,
      sizeof(header),
      "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n",
      fb->len
    );

    res = httpd_resp_send_chunk(
      req,
      "\r\n--frame\r\n",
      strlen("\r\n--frame\r\n")
    );

    if (res == ESP_OK)
      res = httpd_resp_send_chunk(req, header, hlen);

    if (res == ESP_OK)
      res = httpd_resp_send_chunk(
        req,
        (const char*)fb->buf,
        fb->len
      );

    esp_camera_fb_return(fb);

    if (res != ESP_OK)
      break;
  }

  return res;
}

static esp_err_t index_handler(httpd_req_t *req) {

  const char* html =
    "<html>"
    "<head>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>ESP32-CAM</title>"
    "</head>"
    "<body style='background:#050505;color:#ffd700;text-align:center;font-family:Arial'>"
    "<h1>ESP32-CAM</h1>"
    "<p>Camera server is running.</p>"
    "<p>Use /stream for the MJPEG stream.</p>"
    "</body>"
    "</html>";

  httpd_resp_set_type(req, "text/html");

  return httpd_resp_send(
    req,
    html,
    HTTPD_RESP_USE_STRLEN
  );
}

void startCameraServer() {

  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  config.server_port = 80;
  config.ctrl_port = 32768;

  httpd_uri_t index_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = index_handler,
    .user_ctx = NULL
  };

  if (httpd_start(&page_httpd, &config) == ESP_OK)
    httpd_register_uri_handler(page_httpd, &index_uri);

  config.server_port = 81;
  config.ctrl_port = 32769;

  httpd_uri_t stream_uri = {
    .uri = "/stream",
    .method = HTTP_GET,
    .handler = stream_handler,
    .user_ctx = NULL
  };

  if (httpd_start(&stream_httpd, &config) == ESP_OK)
    httpd_register_uri_handler(stream_httpd, &stream_uri);
}

void setup() {

  Serial.begin(115200);
  Serial.setDebugOutput(false);

  delay(1000);

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

  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {

    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;

    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_LATEST;

  } else {

    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;

    config.fb_location = CAMERA_FB_IN_DRAM;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  }

  esp_err_t err = esp_camera_init(&config);

  if (err != ESP_OK) {

    Serial.print("Camera init failed: 0x");
    Serial.println(err, HEX);

    return;
  }

  sensor_t* sensor = esp_camera_sensor_get();

  sensor->set_framesize(
    sensor,
    FRAMESIZE_QVGA
  );

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi connected!");

  Serial.print("Camera IP: ");
  Serial.println(WiFi.localIP());

  startCameraServer();

  Serial.print("Stream URL: http://");
  Serial.print(WiFi.localIP());
  Serial.println(":81/stream");
}

void loop() {
  delay(1000);
}
