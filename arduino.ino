#include <SoftwareSerial.h>
#include <Servo.h>

#include <MFRC522.h>
#include <SPI.h>

#define Rx 4
#define Tx 5
SoftwareSerial myserial(Rx,Tx);

/*Nối chân RFID
 Reset nối chân 9
 SDA nối chân 10
 MOSI nối chân 11
 MISO nối chân 12
 SCK nối chân 13
 */
#define RST_PIN 9
#define SS_PIN 10

int UID[4], i;
char v[50];
int isOpen = 0, isClose = 1;
int isOpen2 = 0, isClose2 = 1;
//Chân 6, 7 là của cảm biến vật thể
int Object1 = 6;
int Object2 = 7;
int Pre1 = HIGH;
int Pre2 = HIGH;

MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo servo;
Servo servo2;

void setup() {
  Serial.begin(9600);
  myserial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  //Servo thứ nhất nối chân 6, Servo thứ hai nối chân 7
  //Dây cam là dây tín hiệu, dây đỏ là 5V, dây nâu là GND
  servo.attach(2, 880, 2120);
  servo2.attach(3, 880, 2120);
  pinMode(Object1, INPUT);
  pinMode(Object2, INPUT);
}

void loop() {
  int isObject1 = digitalRead(Object1);
  int isObject2 = digitalRead(Object2);
  GetUUID();
  char data[50];
  strcpy(data, v);
  myserial.print(data);
  if (isOpen == 0 && isOpen2 == 0)
  {
  if (myserial.available()) {
    String receivedValue = myserial.readString(); // Đọc giá trị từ ESP32
    Serial.print(receivedValue);
    if (receivedValue == "0") 
    {
      Serial.println("Open door 1");
      Open90Degree(servo);
      isOpen = 1;
    } 
    else if (receivedValue == "1") 
    {
      Serial.println("Open door 2");
      Open90Degree(servo2);
      isOpen2 = 1;
     }
     else
     {
      servo.write(90);
      servo2.write(90);
     }
  }
  }
  else if (isOpen == 1 || isOpen2 == 1)
  {
    if (isObject1 == HIGH && Pre1 == LOW && isOpen == 1)
    {
      Serial.println("Close door 1");
      Close90Degree(servo);
      isOpen = 0;
      memset(v, 0, 50);
    }
    if (isObject2 == HIGH && Pre2 == LOW && isOpen2 == 1)
    {
      Serial.println("Close door 2");
      Close90Degree(servo2);
      isOpen2 = 0;
      memset(v, 0, 50);
    }
  }
  Pre1 = isObject1;
  Pre2 = isObject2;
  delay(1000);
}

void GetUUID()
{
  if (!mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  if (!mfrc522.PICC_ReadCardSerial())
  {
    return;
  }

  memset(v, 0, 50);
  
  Serial.print("\nUUID  của thẻ: ");

  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    char v1[10];
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");

    UID[i] = mfrc522.uid.uidByte[i];
    sprintf(v1, "%d", UID[i]);
    strcat(v, v1);
    
    Serial.print(UID[i]);
  }

  Serial.print(" ");
}

void Open90Degree(Servo x)
{
  x.write(75);
  delay(500);
  x.write(90);
}

void Close90Degree(Servo x)
{
  x.write(100);
  delay(580);
  x.write(90);
}
