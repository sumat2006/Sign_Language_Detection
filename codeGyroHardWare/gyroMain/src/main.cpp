#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <Wire.h>
// This device (Master/Receiver) MAC: FC:01:2C:D9:3B:5C
// Sender device (Slave) MAC: B8:F8:62:E7:09:E0
// uint8_t sender_mac[] = {0xB8, 0xF8, 0x62, 0xE7, 0x09, 0xE0};
uint8_t sender_mac[] = {0xFC, 0x01, 0x2C, 0xD9, 0x3B, 0x5C};
// --- I2C Addresses ---
const uint8_t MUX_ADDRESS = 0x70;
const uint8_t SENSOR_ADDRESS = 0x6B;
// --- QMI8658C Register Addresses ---
const uint8_t QMI8658C_WHO_AM_I = 0x00;
const uint8_t QMI8658C_CTRL1 = 0x02;
const uint8_t QMI8658C_CTRL2 = 0x03;
const uint8_t QMI8658C_CTRL3 = 0x04;
const uint8_t QMI8658C_CTRL7 = 0x08;
const uint8_t QMI8658C_OUTPUT_REG_START = 0x35;
const int NUM_SENSORS = 5;
#define WIFI_CHANNEL 1
float _gyro_lsb_div,_accel_lsb_div;
struct  metaData
{
  // float ax, ay, az;
  // float gx, gy, gz;
  float ax1, ay1, az1;
  float ax2, ay2, az2;
  float ax3, ay3, az3;
  float ax4, ay4, az4;
  float ax5, ay5, az5;

  float gx1, gy1, gz1;
  float gx2, gy2, gz2;
  float gx3, gy3, gz3;
  float gx4, gy4, gz4;
  float gx5, gy5, gz5;
  bool isValid = false; // ใช้ตรวจสอบว่าอ่านค่าได้สำเร็จหรือไ
};
metaData myDataRc;
metaData myData;
esp_now_peer_info_t peerInfo;
// void showMyMAC() {
//   Serial.print("My MAC Address: ");
//   Serial.println(WiFi.macAddress());
  
//   // Also show as byte array format (useful for ESP-NOW)
//   uint8_t mac[6];
//   WiFi.macAddress(mac);
//   Serial.print("As byte array: {0x");
//   for (int i = 0; i < 6; i++) {
//     if (mac[i] < 16) Serial.print("0");
//     Serial.print(mac[i], HEX);
//     if (i < 5) Serial.print(", 0x");
//   }
//   Serial.println("}");
// }

// --- ฟังก์ชันสำหรับเลือกช่อง I2C บน MUX (เหมือนเดิม) ---
void selectMuxChannel(uint8_t bus) {
  if (bus > 7) return;
  Wire.beginTransmission(MUX_ADDRESS);
  Wire.write(1 << bus);
  Wire.endTransmission();
}
// --- ฟังก์ชันสำหรับเขียนค่าลง Register (เหมือนเดิม) ---
void writeRegister(uint8_t reg, uint8_t data) {
  Wire.beginTransmission(SENSOR_ADDRESS);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}
// --- ฟังก์ชันเริ่มต้นการทำงานของ QMI8658C (เหมือนเดิม) ---
bool initQMI8658C() {
  Wire.beginTransmission(SENSOR_ADDRESS);
  Wire.write(QMI8658C_WHO_AM_I);
  Wire.endTransmission(false);
  Wire.requestFrom(SENSOR_ADDRESS, (uint8_t)1);
  if (Wire.read() != 0x05) {
    return false;
  }
  writeRegister(QMI8658C_CTRL1, 0x40);
  writeRegister(QMI8658C_CTRL2, 0b00100101); // Accel: +/- 8g
  writeRegister(QMI8658C_CTRL3, 0b01100101); // Gyro: +/- 2048dps
  writeRegister(QMI8658C_CTRL7, 0b00000011);
  //QMI8658_GYRO_RANGE_32DPS:
  //_gyro_lsb_div = 1024;
  // QMI8658_GYRO_RANGE_64DPS:
  // _gyro_lsb_div = 512;
  // QMI8658_GYRO_RANGE_128DPS:
  // _gyro_lsb_div = 256;
  // QMI8658_GYRO_RANGE_256DPS:
  _gyro_lsb_div = 128;
  // QMI8658_GYRO_RANGE_512DPS:
  // _gyro_lsb_div = 64;
  // QMI8658_GYRO_RANGE_1024DPS:
  // _gyro_lsb_div = 32;
  // QMI8658_GYRO_RANGE_2048DPS:
  // _gyro_lsb_div = 16;
  // QMI8658_GYRO_RANGE_4096DPS:
  // _gyro_lsb_div = 8;
  // QMI8658_ACCEL_RANGE_2G:
  // _accel_lsb_div = 16384; // 2^14
  // QMI8658_ACCEL_RANGE_4G:
  // _accel_lsb_div = 8192;  // 2^13
  // QMI8658_ACCEL_RANGE_8G:
  _accel_lsb_div = 4096;  // 2^12
  // QMI8658_ACCEL_RANGE_16G:
  // _accel_lsb_div = 2048;  // 2^11
  return true;
}



// Function to register the slave as a peer
void RegisterPeer() {
    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, sender_mac, 6);
    peerInfo.channel = WIFI_CHANNEL;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
    }
}

// Callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myDataRc, incomingData, sizeof(myDataRc));
}


void InitEspNow() {
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_now_register_recv_cb(OnDataRecv);
    // esp_now_register_send_cb(OnDataSent);
    RegisterPeer();
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  WiFi.mode(WIFI_AP_STA);
  Wire.begin(15,14);
  if (esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE) != ESP_OK) {
      Serial.println("Error setting WiFi channel");
      return;
  }
  InitEspNow();
}

void loop() {
  // showMyMAC();
  
  // Nothing to do here, everything is handled by the callback
  delay(100);
}