#include <HardwareSerial.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>

#define RX_PIN 13 // Chọn chân RX ảo
#define TX_PIN 15 // Chọn chân TX ảo
//biến cờ dùng để so sánh và gửi tín hiệu về cho arduino
char data;

// Cấu hình Wi-Fi
const char* ssid = "vuong";
const char* password = "1234567800";

// Cấu hình MQTT
const char* mqttServer = "mqtt.flespi.io";
const int mqttPort = 1883;
const char* mqttUsername = "moJruoiv8YC1nbgzW2ffeFNi4cnCwNyIZ7Xjj6ChS2834esO7yU91xhVxvnT8JpV";
const char* mqttPassword = "";
const char* topic1 = "topic1";
const char* topic2 = "topic2";

WiFiClient espClient;  //espClient là đối tượng của lớp WiFiClient để kết nối ESP32 với mạng Wi-Fi.
PubSubClient mqttClient(espClient);//mqttClient là đối tượng của lớp PubSubClient để thực hiện kết nối và giao tiếp với broker MQTT.mqttClient là đối tượng của lớp PubSubClient để thực hiện kết nối và giao tiếp với broker MQTT.
//biến newData là một cờ dùng để kiểm tra xem có dữ liệu mới từ Arduino Uno R3 được nhận hay không.volatile bool newData = false;
char Rxbuff[50];
//là một ngắt (interrupt) được gọi khi có dữ liệu mới từ Arduino Uno R3 được nhận.
volatile bool newData = false;
void IRAM_ATTR serialEvent() {
  newData = true;
}
//Callback được gọi khi nhận được dữ liệu từ topic2 và xuất ra Monitor. 
//Nếu dữ liệu nhận được là '1', biến flag_e sẽ được thiết lập là true và được gửi từ ESP32 qua giao tiếp Serial tới Arduino Uno R3.
void callback(char* topic, byte* payload, unsigned int length) {
   if (strcmp(topic, topic2) == 0) {
    Serial.print("Received message from topic2: ");
    for (int i = 0; i < length; i++) {
      data = (char)payload[i];
      Serial.print(data);
      if(data == '1')          //check data lấy được từ topic2 coi đúng kh nếu đúng thì sẽ trả về biến cờ là 1
      {
        char value = '1';
        Serial1.print(value); // Gửi dữ liệu từ ESP32 sang Arduino Uno R3
      }
      else if (data == '0')
      {
        char value = '1';
        Serial1.print(value);
      }
      else 
      {
        char v = '8';
        Serial1.print(v);
      }
    }
    Serial.println();
  }
}

bool flag_e;
void callback1(char* topic, byte* payload, unsigned int length) {
   if (strcmp(topic, topic2) == 0) {
    Serial.print("Received message from topic2: ");
    for (int i = 0; i < length; i++) {
      char data = (char)payload[i];
      Serial.print(data);
      if(data == '1')          //check data lấy được từ topic2 coi đúng kh nếu đúng thì sẽ trả về biến cờ là 1
      {
        //flag_e=true;
        //Serial1.print(flag_e); // Gửi dữ liệu từ ESP32 sang Arduino Uno R3
        char vn = '1';
        Serial1.print(vn); // Gửi dữ liệu từ ESP32 sang Arduino Uno R3
      }
      else if (data == '0')
      {
        //flag_e=false;
        //Serial1.print(flag_e);
        char vn = '0';
        Serial1.print(vn); // Gửi dữ liệu từ ESP32 sang Arduino Uno R3
      }
      else
      {
        char vn = '8';
        Serial1.print(vn); // Gửi dữ liệu từ ESP32 sang Arduino Uno R3
      }
      }
    }
    Serial.println();
  }
//}

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);  //Khởi tạo kết nối Serial và chân kết nối ảo RX/TX.
  // Kết nối Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi...");
    delay(1000);
  }
  Serial.println("Connected to WiFi");
  // Kết nối MQTT
  mqttClient.setServer(mqttServer, mqttPort);
  //gọi hàm callback để kiểm tra coi có giá trị trên flespi hay không nếu có thì lấy giá trị
  //gọi hàm này để lấy giá trị từ topic2 về cho esp32
  mqttClient.setCallback(callback1);
  //kiểm tra kết nối của mqtt và subscribe vào các topic cần thiết
  while (!mqttClient.connected()) {
    if (mqttClient.connect("ESP32Client", mqttUsername, mqttPassword)) {
      Serial.println("Connected to MQTT");
      mqttClient.subscribe(topic1);
      mqttClient.subscribe(topic2);
    } else {
      Serial.print("Failed to connect to MQTT, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
  // Thiết lập interrupt để đọc dữ liệu từ Serial1
  attachInterrupt(digitalPinToInterrupt(RX_PIN), serialEvent, CHANGE);
}

void loop()
{
  //Kiểm tra cờ newData để xem có dữ liệu mới từ Arduino Uno R3 hay không.
  if (newData) {
    //Nếu có dữ liệu mới, các bước sau được thực hiện:
    //Tạm ngưng ngắt bằng hàm noInterrupts() để đảm bảo dữ liệu không bị ghi đè trong quá trình đọc.
    noInterrupts();
    memset(Rxbuff, 0, sizeof(Rxbuff));
    int len = Serial1.readBytes(Rxbuff, sizeof(Rxbuff) - 1);
    //Đặt cờ newData về false để chỉ ra rằng dữ liệu đã được xử lý.
    //Kích hoạt lại ngắt bằng hàm interrupts() sau khi dữ liệu đã được đọc.
    newData = false;
    interrupts();
    if (len > 0) {
      Rxbuff[len] = '\0';
      Serial.print("Received data from arduino: ");
      Serial.println(Rxbuff);
      mqttClient.publish("topic1", Rxbuff);
    }
  }
  // Thực hiện xử lý các gói tin MQTT đến và duy trì kết nối
  mqttClient.loop();
  
}
