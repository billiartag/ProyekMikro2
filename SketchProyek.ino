#include <DHT.h>
#include <ArduinoHttpClient.h>
#include <ArduinoMqttClient.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//wifi&etc
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
WiFiUDP Udp;

HttpClient client = HttpClient(wifiClient, "192.168.1.2", 3000);
char ssid[] = "Masu";
char pass[] = "Kosumasu";
char auth[] = "Vn1AMSEvLcREWIwHg8x7vV1yj4r5QHJ2";
//init komponen
DHT dht(D4, DHT11);
LiquidCrystal_I2C lcd(0x27, 16, 2);
//init val
int val_potensio, mode, ctrLED, ctrBeep, giliranBeep, ctrLCD;
int pinPotensio, pinTombol, pinBeep, pinLED2, pinLED3, pinDHT;
int mode_LED0, mode_LED1;
float humidity, temperature;
long prMLimaRibu,prMSatuRibu,prMLimaRatus,prMTigaRatus,prMDuaRatus,curMillis;
String tanggal, waktu;
String hasilMQTT;

//onlineRTC
// NTP Servers:
static const char ntpServerName[] = "time.nist.gov";
const int timeZone = 7;     // GMT+7
unsigned int localPort = 8888;  // local port to listen for UDP packets

time_t getNtpTime();
void digitalClockDisplay();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);
//
void poolMQTT() {
  mqttClient.poll();
}
void printLCD(String atas, String bawah) {
  lcd.setCursor(0,0);
  lcd.print(atas);
  lcd.setCursor(0,1);
  lcd.print(bawah);
  if (!isnan(temperature) && mode == 0) {
    int bulat = (int)temperature;
    lcd.setCursor(12,0);
    lcd.print(bulat);
    lcd.print((char)0xDF);
    lcd.print("C");
  }
}
String tambahNol(int nilai) {
  if (nilai / 10 == 0) {
    return "0" + String(nilai);
  }
  else {
    return String(nilai);
  }
}
int baca_potensio() {
  return map(analogRead(pinPotensio), 0, 1023, 0, 255);
}
void baca_DHT() {
  humidity =  dht.readHumidity();
  temperature = dht.readTemperature();
}
void connectKeMQTT() {
  if (!mqttClient.connect("192.168.1.2", 1883)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    while (1);
  }
  mqttClient.subscribe("proyek/terima");
  Serial.println("subs");
  Serial.println("Connected to MQTT");
}
//
void getWaktuSekarang() {
  waktu = tambahNol(hour()) + ":" + tambahNol(minute()) + ":" + tambahNol(second());
  tanggal = tambahNol(day()) + "/" + tambahNol(month()) + "/" + year();
}
void bunyi_beep() {
  if (mode == 1) {
    if (giliranBeep == 0) {
      tone(pinBeep, 4500, 200);
      giliranBeep = 1;
    }
    else if (giliranBeep == 1) {
      matiin_beep();
      giliranBeep = 0;
    }
  }
}
void matiin_beep() {
  noTone(pinBeep);
}
void analogLampu(int satu, int dua, int tiga) {
  analogWrite(pinLED2, dua);
  analogWrite(pinLED3, tiga);
}
void nyalain_lampu() {
  int nilai_LED = baca_potensio();
  if (mode == 0) {
    if (mode_LED0 == 0) {
      analogLampu(LOW, nilai_LED, LOW);
      mode_LED0 = 1;
    }
    else {
      analogLampu(LOW, LOW, LOW);
      mode_LED0 = 0;
    }
  }
  else if (mode == 1) {
    ctrLED = ctrLED + 1;
    if (mode_LED1 == 0) {
      analogLampu(nilai_LED, LOW, LOW);
      mode_LED1 = 1;
    }
    else if (mode_LED1 == 1) {
      analogLampu(LOW, nilai_LED, LOW);
      mode_LED1 = 2;
    }
    else if (mode_LED1 == 2) {
      analogLampu(LOW, LOW, nilai_LED);
      mode_LED1 = 1;
    }
    if (ctrLED > 25) {
      mode = 0;
      ctrLED = 0;
      ctrLCD = 1;
      mode_LED1 = 0;
    }
  }
  else if (mode == 2) {
    int hasil = ctrLED % 2;

    if (hasil == 0) {
      analogLampu(nilai_LED, nilai_LED, nilai_LED);
    }
    else if (hasil == 1) {
      analogLampu(20, 20, 20);
    }
    ctrLED += 1;
    if (ctrLED > 20) {
      mode = 0;
      ctrLED = 0;
      ctrLCD = 1;
    }
  }
}
void nyalaSuara() {
  bunyi_beep();
  ctrBeep = ctrBeep + 1;
  if (ctrBeep == 10) {
    matiin_beep();
    ctrBeep = 0;
  }
}
void kirimMQTT() {
  mqttClient.poll();
  Serial.print("Sending: ");
  mqttClient.beginMessage("proyek/kirim");
  mqttClient.print(tanggal + ";" + waktu);
  mqttClient.endMessage();
}
String cekMQTT() {
  String path = "/getEntry";
  // send the GET request
  Serial.println("making GET request");
  client.get(path);
  // read the status code and body of the response
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
  return response;
}
void resetLCD() {
  if (ctrLCD != 0) {
    ctrLCD = 0;
    lcd.clear();
  }
}
void serverLCD() {
  if (mode == 0) {
    //show LCD
    resetLCD();
    getWaktuSekarang();
    printLCD(waktu, tanggal);
  }
  else if (mode == 1) {
    resetLCD();
    getWaktuSekarang();
    printLCD("Selamat Datang", waktu);
  }
  else if (mode == 2) {
    //resetLCD();
    if (ctrLCD != 0) {
      ctrLCD = 0;
      lcd.clear();
    }
    //tembak API
    if (hasilMQTT != "") {
      printLCD("Last log: ", hasilMQTT);
    }
    else {
      printLCD("Belum ada log", "^v^");
    }
  }
}
void gantiMode() {
  //mode 0
  if (mode == 0 && digitalRead(pinTombol) == HIGH) {
    if (baca_potensio() >= 200) {
      mode = 2;
      //tembak API
      hasilMQTT = cekMQTT();
      connectKeMQTT();
      mqttClient.poll();
      //show LCD
      ctrLCD = 1;
    }
    else {
      nyalaSuara();
      mode = 1;
      //tembak API MQTT
      kirimMQTT();
      mqttClient.poll();
      //LCD
      ctrLCD = 1;
    }
  }
}
//Online RTC
time_t prevDisplay = 0; // when the digital clock was displayed
/*-------- NTP code ----------*/
const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets
time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address
  int reload = 0;
  while (reload==0){
    while (Udp.parsePacket() > 0) ; // discard any previously received packets
    Serial.println("Transmit NTP Request");
    // get a random server from the pool
    WiFi.hostByName(ntpServerName, ntpServerIP);
    Serial.print(ntpServerName);
    Serial.print(": ");
    Serial.println(ntpServerIP);
    sendNTPpacket(ntpServerIP);
    uint32_t beginWait = millis();
    while (millis() - beginWait < 1500) {
      int size = Udp.parsePacket();
      if (size >= NTP_PACKET_SIZE) {
        Serial.println("Receive NTP Response");
        Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
        unsigned long secsSince1900;
        // convert four bytes starting at location 40 to a long integer
        secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
        secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
        secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
        secsSince1900 |= (unsigned long)packetBuffer[43];
        reload=1;
        return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
      }
      else{
        Serial.println("No NTP Response :-(");
        reload=0;
        }
    }
  }
}
// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
//
void setup() {
  //set pin
  pinPotensio = A0;
  pinTombol = D8; 
  pinBeep = D5;
  pinLED2 = D6;
  pinLED3 = D7;
  pinDHT = D4;
  //set config
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println("Connected to WIFI");
  Wire.begin(D2, D1); //(SDA, SCL) 
  mode = 0;
  //set nilai awal
  ctrLED = 0;
  ctrLCD = 0;
  mode_LED0 = 0;
  mode_LED1 = 1;
  giliranBeep = 0;
  hasilMQTT = "";
  mqttClient.setId("ArduinoES");
  connectKeMQTT();
  mqttClient.subscribe("proyek/terima");
  dht.begin();
  lcd.backlight(); // Enable or Turn On the backlight
  lcd.clear();
  //onlineRTC
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");  
  setSyncProvider(getNtpTime);
  setSyncInterval(300);
}
void loop() {
  // put your main code here, to run repeatedly:
  mqttClient.poll();
  curMillis=millis();
  if(curMillis - prMLimaRibu >=5000){
    baca_DHT();
    prMLimaRibu=curMillis;
  }
  if(curMillis - prMSatuRibu >=1000){
    getWaktuSekarang();
    poolMQTT();
    serverLCD();
    prMSatuRibu=curMillis;
  }
  if(curMillis - prMLimaRatus >=500){
    nyalaSuara();
    prMLimaRatus=curMillis;
  }
  if(curMillis - prMTigaRatus >=300){
    gantiMode();
    prMTigaRatus=curMillis;
  }
  if(curMillis - prMDuaRatus >=200){
    nyalain_lampu();
    prMDuaRatus=curMillis;
  }
}
