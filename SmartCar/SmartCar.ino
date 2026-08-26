#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// Replace with your ESP32-CAM stream address.
const char* CAMERA_STREAM = "http://YOUR_CAMERA_IP:81/stream";

WebServer server(80);

// L298N
#define IN1 26
#define IN2 27
#define IN3 14
#define IN4 13

// Buzzer
#define BUZZER 25

// Ultrasonic
#define TRIG 33
#define ECHO 34

// Headlight
#define LIGHT 32

bool reversing = false;
bool reverseBlocked = false;
bool hornActive = false;
bool turnBeep = false;

unsigned long lastReverseBeep = 0;
bool reverseBeepState = false;

void stopMotors() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

void forward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void backward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

void left() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void right() {
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
}

void updateBuzzer() {
  if (hornActive) {
    digitalWrite(BUZZER, HIGH);
    return;
  }

  if (reverseBlocked) {
    unsigned long now = millis();

    if (now - lastReverseBeep >= 300) {
      lastReverseBeep = now;
      reverseBeepState = !reverseBeepState;
      digitalWrite(BUZZER, reverseBeepState ? HIGH : LOW);
    }
    return;
  }

  digitalWrite(BUZZER, turnBeep ? HIGH : LOW);
}

void hornOn() {
  hornActive = true;
  turnBeep = false;
  digitalWrite(BUZZER, HIGH);
}

void hornOff() {
  hornActive = false;
  updateBuzzer();
}

void lightOn() {
  digitalWrite(LIGHT, HIGH);
}

void lightOff() {
  digitalWrite(LIGHT, LOW);
}

float getDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 30000);

  if (duration == 0)
    return 999.0;

  return duration * 0.0343 / 2.0;
}

void handleRoot() {
  String page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no">

<style>
*{box-sizing:border-box;-webkit-tap-highlight-color:transparent}

body{
  margin:0;
  padding:16px;
  min-height:100vh;
  background:radial-gradient(circle at top,#242424 0%,#090909 45%,#000 100%);
  color:#ffd700;
  font-family:Arial,sans-serif;
  text-align:center;
  user-select:none;
  touch-action:manipulation;
}

h1{
  margin:5px 0 14px;
  font-size:clamp(26px,6vw,38px);
  text-shadow:0 0 15px rgba(255,215,0,.8);
}

.status{
  display:inline-block;
  margin-bottom:12px;
  padding:7px 14px;
  border:1px solid #ffd700;
  border-radius:20px;
  font-size:13px;
  color:#ffe77a;
  background:rgba(255,215,0,.07);
}

.camera{
  width:95%;
  max-width:720px;
  margin:auto;
  padding:4px;
  border-radius:20px;
  background:linear-gradient(135deg,#ffd700,#805d00,#ffd700);
  box-shadow:0 0 28px rgba(255,215,0,.45);
}

.cam{
  display:block;
  width:100%;
  aspect-ratio:16/9;
  object-fit:cover;
  background:#050505;
  border-radius:16px;
}

.controls{
  width:min(95vw,570px);
  margin:17px auto 0;
}

.row{
  display:flex;
  justify-content:center;
  align-items:center;
  gap:8px;
  margin:8px 0;
}

button{
  width:125px;
  min-height:70px;
  padding:10px;
  border:0;
  border-radius:17px;
  background:linear-gradient(145deg,#ffe44d,#f0a800);
  color:#080808;
  font-size:16px;
  font-weight:800;
  box-shadow:0 5px 18px rgba(255,215,0,.35);
  cursor:pointer;
  touch-action:none;
}

button:active,
button.active{
  transform:scale(.93);
  filter:brightness(.88);
}

.stop{
  background:linear-gradient(145deg,#ff3b30,#a80000);
  color:white;
}

.horn{
  width:min(92vw,390px);
  background:linear-gradient(145deg,#ff9f1c,#f05a00);
  color:white;
}

.light-on{
  background:linear-gradient(145deg,#24d65c,#008f36);
  color:white;
}

.light-off{
  background:linear-gradient(145deg,#626262,#292929);
  color:white;
}

@media(max-width:520px){
  button{
    width:29vw;
    min-width:90px;
    height:64px;
    font-size:14px;
  }
  .horn{width:75vw}
}
</style>

<script>
function send(cmd){
  fetch("/" + cmd).catch(()=>{});
}

function startMove(cmd){
  send(cmd);
}

function stopMove(){
  send("S");
}

function hornStart(){
  send("HON");
}

function hornStop(){
  send("HOFF");
}

function lightOn(){
  send("LON");
}

function lightOff(){
  send("LOFF");
}
</script>
</head>

<body>

<h1>🚗 ESP32 SMART CAR</h1>

<div class="status">● Controller Connected</div>

<img class="cam" src="CAMERA_STREAM_URL">

<div class="controls">

<div class="row">
<button
ontouchstart="startMove('F')"
ontouchend="stopMove()"
onmousedown="startMove('F')"
onmouseup="stopMove()">
⬆<br>FORWARD
</button>
</div>

<div class="row">

<button
ontouchstart="startMove('L')"
ontouchend="stopMove()"
onmousedown="startMove('L')"
onmouseup="stopMove()">
⬅<br>LEFT
</button>

<button class="stop" onclick="stopMove()">
⏹<br>STOP
</button>

<button
ontouchstart="startMove('R')"
ontouchend="stopMove()"
onmousedown="startMove('R')"
onmouseup="stopMove()">
RIGHT<br>➡
</button>

</div>

<div class="row">
<button
ontouchstart="startMove('B')"
ontouchend="stopMove()"
onmousedown="startMove('B')"
onmouseup="stopMove()">
⬇<br>BACKWARD
</button>
</div>

<div class="row">
<button
class="horn"
ontouchstart="hornStart()"
ontouchend="hornStop()"
onmousedown="hornStart()"
onmouseup="hornStop()">
📢 HORN<br>(HOLD)
</button>
</div>

<div class="row">
<button class="light-on" onclick="lightOn()">
💡<br>LIGHT ON
</button>

<button class="light-off" onclick="lightOff()">
🌙<br>LIGHT OFF
</button>
</div>

</div>

</body>
</html>
)rawliteral";

  page.replace("CAMERA_STREAM_URL", CAMERA_STREAM);
  server.send(200, "text/html", page);
}

void handleForward() {
  reversing = false;
  reverseBlocked = false;
  turnBeep = false;
  digitalWrite(BUZZER, LOW);
  forward();
  server.send(200, "text/plain", "FORWARD");
}

void handleBackward() {
  turnBeep = false;

  float distance = getDistance();

  if (distance <= 5.0) {
    stopMotors();
    reversing = false;
    reverseBlocked = true;
    reverseBeepState = false;
    lastReverseBeep = millis();

    server.send(200, "text/plain", "OBSTACLE");
    return;
  }

  reverseBlocked = false;
  reversing = true;
  backward();

  server.send(200, "text/plain", "BACKWARD");
}

void handleLeft() {
  reversing = false;
  reverseBlocked = false;
  turnBeep = true;
  left();
  updateBuzzer();

  server.send(200, "text/plain", "LEFT");
}

void handleRight() {
  reversing = false;
  reverseBlocked = false;
  turnBeep = true;
  right();
  updateBuzzer();

  server.send(200, "text/plain", "RIGHT");
}

void handleStop() {
  reversing = false;
  reverseBlocked = false;
  turnBeep = false;

  stopMotors();

  if (!hornActive)
    digitalWrite(BUZZER, LOW);

  server.send(200, "text/plain", "STOP");
}

void handleHornOn() {
  hornOn();
  server.send(200, "text/plain", "HORN ON");
}

void handleHornOff() {
  hornOff();
  server.send(200, "text/plain", "HORN OFF");
}

void handleLightOn() {
  lightOn();
  server.send(200, "text/plain", "LIGHT ON");
}

void handleLightOff() {
  lightOff();
  server.send(200, "text/plain", "LIGHT OFF");
}

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(BUZZER, OUTPUT);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  pinMode(LIGHT, OUTPUT);

  stopMotors();
  digitalWrite(BUZZER, LOW);
  lightOff();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi connected!");

  Serial.print("Controller IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/F", handleForward);
  server.on("/B", handleBackward);
  server.on("/L", handleLeft);
  server.on("/R", handleRight);
  server.on("/S", handleStop);

  server.on("/HON", handleHornOn);
  server.on("/HOFF", handleHornOff);

  server.on("/LON", handleLightOn);
  server.on("/LOFF", handleLightOff);

  server.begin();

  Serial.println("Smart Car web server started.");
}

void loop() {
  server.handleClient();

  if (reversing && getDistance() <= 5.0) {
    stopMotors();
    reversing = false;
    reverseBlocked = true;
    reverseBeepState = false;
    lastReverseBeep = millis();
  }

  if (reverseBlocked && getDistance() > 5.0) {
    reverseBlocked = false;
    reverseBeepState = false;

    if (!hornActive && !turnBeep)
      digitalWrite(BUZZER, LOW);
  }

  updateBuzzer();

  delay(5);
}
