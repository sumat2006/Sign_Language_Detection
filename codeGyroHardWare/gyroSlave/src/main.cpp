#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <Wire.h>
// Receiver device (Master) MAC: FC:01:2C:D9:3B:5C
// This device (Slave/Sender) MAC: B8:F8:62:E7:09:E0
// uint8_t receiver_mac[] = {0xFC, 0x01, 0x2C, 0xD9, 0x3B, 0x5C};
uint8_t receiver_mac[] = {0xB8, 0xF8, 0x62, 0xE7, 0x09, 0xE0};
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
metaData myData;
esp_now_peer_info_t peerInfo;
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

// Callback function for send status
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  Serial.print("Packet to: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
  Serial.print(" send status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Function to register the slave as a peer
void RegisterPeer() {
    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, receiver_mac, 6);
    peerInfo.channel = WIFI_CHANNEL;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
    }
}

void InitEspNow() {
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    // esp_now_register_recv_cb(OnMessageReceived);
    esp_now_register_send_cb(OnDataSent);
    RegisterPeer();
}
void setup() {
  Serial.begin(115200);
  while (!Serial);
  WiFi.mode(WIFI_AP_STA);
  Serial.println("\nBare-metal QMI8658C + TCA9548A Test - Table View");
  Wire.begin(15,14);
  if (esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE) != ESP_OK) {
      Serial.println("Error setting WiFi channel");
      return;
  }
  for (int i = 0; i < NUM_SENSORS; i++) {
    selectMuxChannel(i);
    Serial.print("Initializing sensor on MUX channel ");
    Serial.print(i);
    if (initQMI8658C()) {
      Serial.println("... Success!");
    } else {
      Serial.println("... Failed!");
    }
    delay(100);
  }
  Serial.println("\nInitialization complete. Starting readings...");
  InitEspNow();

}

void loop() {
  // --- เฟสที่ 1: อ่านและเก็บข้อมูลจากทุกเซ็นเซอร์ ---
  for (int i = 0; i < NUM_SENSORS; i++) {
    selectMuxChannel(i);

    uint8_t dataBuffer[12];
    Wire.beginTransmission(SENSOR_ADDRESS);
    Wire.write(QMI8658C_OUTPUT_REG_START);
    Wire.endTransmission(false);
    Wire.requestFrom(SENSOR_ADDRESS, (uint8_t)12);

    if (Wire.available() == 12) {
      for (int k = 0; k < 12; k++) {
        dataBuffer[k] = Wire.read();
      }

      int16_t rawAccX = (dataBuffer[1] << 8) | dataBuffer[0];
      int16_t rawAccY = (dataBuffer[3] << 8) | dataBuffer[2];
      int16_t rawAccZ = (dataBuffer[5] << 8) | dataBuffer[4];
      int16_t rawGyroX = (dataBuffer[7] << 8) | dataBuffer[6];
      int16_t rawGyroY = (dataBuffer[9] << 8) | dataBuffer[8];
      int16_t rawGyroZ = (dataBuffer[11] << 8) | dataBuffer[10];
      
      if(i == 0){
        myData.ax1 = (float)rawAccX / _accel_lsb_div;
        myData.ay1 = (float)rawAccY / _accel_lsb_div;
        myData.az1 = (float)rawAccZ / _accel_lsb_div;
        myData.gx1 = (float)rawGyroX / _gyro_lsb_div;
        myData.gy1 = (float)rawGyroY / _gyro_lsb_div;
        myData.gz1 = (float)rawGyroZ / _gyro_lsb_div;
      }else if(i == 1){
        myData.ax2 = (float)rawAccX / _accel_lsb_div;
        myData.ay2 = (float)rawAccY / _accel_lsb_div;
        myData.az2 = (float)rawAccZ / _accel_lsb_div;
        myData.gx2 = (float)rawGyroX / _gyro_lsb_div;
        myData.gy2 = (float)rawGyroY / _gyro_lsb_div;
        myData.gz2 = (float)rawGyroZ / _gyro_lsb_div;
      }else if(i == 2){
        myData.ax3 = (float)rawAccX / _accel_lsb_div;
        myData.ay3 = (float)rawAccY / _accel_lsb_div;
        myData.az3 = (float)rawAccZ / _accel_lsb_div;
        myData.gx3 = (float)rawGyroX / _gyro_lsb_div;
        myData.gy3 = (float)rawGyroY / _gyro_lsb_div;
        myData.gz3 = (float)rawGyroZ / _gyro_lsb_div;
      }else if(i == 3){
        myData.ax4 = (float)rawAccX / _accel_lsb_div;
        myData.ay4 = (float)rawAccY / _accel_lsb_div;
        myData.az4 = (float)rawAccZ / _accel_lsb_div;
        myData.gx4 = (float)rawGyroX / _gyro_lsb_div;
        myData.gy4 = (float)rawGyroY / _gyro_lsb_div;
        myData.gz4 = (float)rawGyroZ / _gyro_lsb_div;
      }else if(i == 4){
        myData.ax5 = (float)rawAccX / _accel_lsb_div;
        myData.ay5 = (float)rawAccY / _accel_lsb_div;
        myData.az5 = (float)rawAccZ / _accel_lsb_div;
        myData.gx5 = (float)rawGyroX / _gyro_lsb_div;
        myData.gy5 = (float)rawGyroY / _gyro_lsb_div;
        myData.gz5 = (float)rawGyroZ / _gyro_lsb_div;
      }
      

      myData.isValid = true;
    } else {
      // หากอ่านไม่สำเร็จ ให้ทำเครื่องหมายว่าข้อมูลไม่ถูกต้อง
      myData.isValid = false;
    }
  }
  // myData.x = random(0, 20);
  // myData.y = random(0, 20);

  // Serial.print("Sending x: ");
  // Serial.print(myData.x);
  // Serial.print(", y: ");
  // Serial.println(myData.y);
  // --- เฟสที่ 2: แสดงผลข้อมูลทั้งหมดในตารางเดียว ---
  Serial.println("\n================================ SENSOR DATA READOUT ================================");
  Serial.println("| CH | Accel X | Accel Y | Accel Z |  Gyro X |  Gyro Y |  Gyro Z |");
  Serial.println("|----|---------|---------|---------|---------|---------|---------|");
  for (int i = 0; i < NUM_SENSORS; i++) {
    if (myData.isValid) {
      if(i == 0){
        Serial.printf("| %2d | %f | %f | %f | %f | %f | %f |\n",
                    i,
                    myData.ax1, myData.ay1, myData.az1,
                    myData.gx1, myData.gy1, myData.gz1);
      }else if(i == 1){
        Serial.printf("| %2d | %f | %f | %f | %f | %f | %f |\n",
                    i,
                    myData.ax2, myData.ay2, myData.az2,
                    myData.gx2, myData.gy2, myData.gz2);
      }else if(i == 2){
       Serial.printf("| %2d | %f | %f | %f | %f | %f | %f |\n",
                    i,
                    myData.ax3, myData.ay3, myData.az3,
                    myData.gx3, myData.gy3, myData.gz3);
      }else if(i == 3){
       Serial.printf("| %2d | %f | %f | %f | %f | %f | %f |\n",
                    i,
                    myData.ax4, myData.ay4, myData.az4,
                    myData.gx4, myData.gy4, myData.gz4);
      }else if(i == 4){
       Serial.printf("| %2d | %f | %f | %f | %f | %f | %f |\n",
                    i,
                    myData.ax5, myData.ay5, myData.az5,
                    myData.gx5, myData.gy5, myData.gz5);
      }
      // ใช้ printf เพื่อจัดรูปแบบทศนิยมและคอลัมน์ให้สวยงาม
      
    } else {
      Serial.printf("| %2d | --- FAILED TO READ DATA ---                                         |\n", i);
    }
  }
  esp_err_t result = esp_now_send(receiver_mac, (uint8_t *) &myData, sizeof(metaData));
  Serial.printf("Size %d\n",sizeof(metaData));
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  
  // Serial.println("---");
  Serial.println("=====================================================================================");
  delay(100);
  // delay(2000);  // Increased delay for easier reading
}