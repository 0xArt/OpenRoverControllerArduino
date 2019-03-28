#include <Arduino_FreeRTOS.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // RX, TX
const byte pwmOne = 6;
const byte dirOne = 2;
const byte dirTwo = 4;
const byte numBytes = 3;
byte receivedBytes[numBytes];
boolean newData = false;

void Task1( void *pvParameters );
void Task2( void *pvParameters );

void setup() {
  Serial.begin(9600);
  mySerial.begin(115200);
  mySerial.print("$");  // Print three times individually
  mySerial.print("$");
  mySerial.print("$");  // Enter command mode
  delay(100);  // Short delay, wait for the Mate to send back CMD
  mySerial.println("U,9600,N");  // Temporarily Change the baudrate to 9600, no parity
  // 115200 can be too fast at times for NewSoftSerial to relay the data reliably
  mySerial.begin(9600);  // Start bluetooth serial at 9600
  pinMode(pwmOne, OUTPUT);
  pinMode(dirOne, OUTPUT);
  pinMode(dirTwo, OUTPUT);

  //create task
  xTaskCreate(
    Task1
    , (const portCHAR *)"Task1"
    , 128
    , NULL
    , 5
    , NULL );

  xTaskCreate(
    Task2
    , (const portCHAR *)"Task2"
    , 128
    , NULL
    , 4
    , NULL );

}
void loop() {
  // put your main code here, to run repeatedly:
  }
//Tasks
void Task1(void *pvParamaters) {
  (void) pvParamaters;
  //loop here
  for (;;) {
    recvBytes();
    useData();
  }
}

void Task2(void *pvParamaters) {
  (void) pvParamaters;
  //loop here
  for (;;) {
    sendBytes();
    delay(250);
  }
}


void pause()
{
  analogWrite(pwmOne, 0);
  digitalWrite(dirOne, 0);
  digitalWrite(dirTwo, 0);
  Serial.println("Stopping");
}
void forward(byte speed) {
  float duty = float(speed)/100;
  duty = duty*255;
  Serial.println(duty);
  analogWrite(pwmOne, duty);
  digitalWrite(dirOne, 0);
  digitalWrite(dirTwo, 0);
}

void backward(byte speed) {
  float duty = float(speed)/100;
  duty = duty*255;
  analogWrite(pwmOne, duty);
  digitalWrite(dirOne, 1);
  digitalWrite(dirTwo, 1);
  //tone(11, 32);
}

void right(byte speed) {
  float duty = float(speed)/100;
  duty = duty*255;
  analogWrite(pwmOne, duty);
  digitalWrite(dirOne, 0);
  digitalWrite(dirTwo, 1);
}

void left(byte speed) {
  float duty = float(speed)/100;
  duty = duty*255;
  analogWrite(pwmOne, duty);
  digitalWrite(dirOne, 1);
  digitalWrite(dirTwo, 0);
}

void recvBytes() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    byte startMarker = 255;
    byte rb;
    
    while (mySerial.available() > 0 && newData == false) {
        rb = mySerial.read();
        if (recvInProgress == true) {
            if (ndx < numBytes) {
                receivedBytes[ndx] = rb;
                ndx++;
                if(ndx == numBytes){
                  recvInProgress = false;
                  ndx = 0;
                  newData = true;
                }
            }
        }
        if (rb == startMarker) {
            recvInProgress = true;
        }
    }
}

void useData() {
    if (newData == true) {
        if(receivedBytes[0]^receivedBytes[1] == receivedBytes[2]){
          switch (receivedBytes[0]) {
            case 1:
              forward(receivedBytes[1]);
              Serial.print("Moving forward with speed: ");
              Serial.println(receivedBytes[1]);
              break;
            case 2:
              right(receivedBytes[1]);
              break;
            case 3:
              backward(receivedBytes[1]);
              break;
            case 4:
              left(receivedBytes[1]);
              break;
            default:
              pause();
              break;
          }
        }
        else{
          Serial.println("Checksum is NOT correct");
          pause();
        }
        newData = false;
    }
}

void sendBytes() {
  byte zero = 255;
  byte one = (byte)(random(0,255));
  byte two = (byte)(random(0,255));
  byte three = one^two;
  byte toSend[] = {one, two, three};
  mySerial.write(zero);
  mySerial.write(one);
  mySerial.write(two);
  mySerial.write(three);
}
