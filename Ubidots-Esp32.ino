#include "BluetoothSerial.h"
#include "UbidotsEsp32Mqtt.h"

const char *pin = "1234"; // Cambia esto a un PIN más seguro si es necesario
String device_name = "G4";
const char *UBIDOTS_TOKEN = "BBFF-8hWhyjESFKCkQC9H2bCoiXnI9XGday";
const char *WIFI_SSID = "PROMECITY";
const char *WIFI_PASS = "2010GYESZ";
const char *DEVICE_LABEL = "Inventario";
const int servoPin = 2;  // Pin donde está conectado el servo
int angle = 90;           // Ángulo inicial del servo (0-180 grados)
Ubidots ubidots(UBIDOTS_TOKEN);
BluetoothSerial SerialBT;
int res560;
int res330;
int res1000;

void setup() {
  Serial.begin(115200);
  SerialBT.begin(device_name);
  Serial.printf("Device name: \"%s\".\n", device_name.c_str());
  SerialBT.connect("raspberrypi");    // Nombre del dispositivo Bluetooth del esclavo
 
  Serial.println("ESP32 esclavo iniciado");
  pinMode(servoPin, OUTPUT);  // Configura el pin del servo como salida
  
  #ifdef USE_PIN
    SerialBT.setPin(pin);
    Serial.println("Using PIN");
  #endif
  
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(16, INPUT);
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setup();
}

void loop() {
  if (!ubidots.connected()) {
    ubidots.reconnect();
  }
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  
  if (SerialBT.available()) {
    int receivedValue = SerialBT.parseInt();
    Serial.print("Raspberry envia: ");
    Serial.println(receivedValue);
    if (receivedValue == 560) {
      handleReceived560();
      int targetAngle = 0;
      moveServoToTarget(targetAngle);
    }
     if (receivedValue == 330) {
      handleReceived330();
      int targetAngle = 60;
      moveServoToTarget(targetAngle);
    } 
      //Revisar cambiar a 10k
      if (receivedValue == 1000) {
      handleReceived1k();
      int targetAngle = 140;
    moveServoToTarget(targetAngle);
    }
  }
  
  if (digitalRead(4) == HIGH) {
    handleRemove560();
    
    delay(200);  
  }
   if (digitalRead(5) == HIGH) {
    handleRemove330();
    
    delay(200);  
  }
  if (digitalRead(16) == HIGH) {
    handleRemove1k();
    
    delay(200);  
  }
  delay(20);
}



void handleReceived560() {
 
  
  res560++;
  Serial.print("Valor recibido 560. Contador: ");
  Serial.println(res560);
  ubidots.add("Resistencia 560", res560);
   ubidots.publish(DEVICE_LABEL);
}
void handleReceived330() {
 
  
  res330++;
  Serial.print("Valor recibido 330. Contador: ");
  Serial.println(res330);
  ubidots.add("Resistencia 330", res330);
   ubidots.publish(DEVICE_LABEL);
}
void handleReceived1k() {
 
  
  res1000++;
  Serial.print("Valor recibido 1k. Contador: ");
  Serial.println(res1000);
  ubidots.add("Resistencia 1k", res1000);
   ubidots.publish(DEVICE_LABEL);
}

//Botones
void handleRemove560() {  
  res560--;
  if (res560 < 0) {
    res560 = 0;
  }
  
  Serial.print("Quitando 560. Contador: ");
  Serial.println(res560);
  ubidots.add("Resistencia 560", res560);
   ubidots.publish(DEVICE_LABEL);
}
void handleRemove330() {  
  res330--;
  if (res330 < 0) {
    res330 = 0;
  }
  
  Serial.print("Quitando 330. Contador: ");
  Serial.println(res330);
  ubidots.add("Resistencia 330", res330);
   ubidots.publish(DEVICE_LABEL);
}
void handleRemove1k() {  
  res1000--;
  if (res1000 < 0) {
    res1000 = 0;
  }
  
  Serial.print("Quitando 1k. Contador: ");
  Serial.println(res1000);
  ubidots.add("Resistencia 1k", res1000);
   ubidots.publish(DEVICE_LABEL);
}

void updateServo(int targetAngle) {
  int pulseWidth = map(targetAngle, 0, 180, 500, 2500);
  digitalWrite(servoPin, HIGH);
  delayMicroseconds(pulseWidth);
  digitalWrite(servoPin, LOW);
}

void moveServoToTarget(int targetAngle) {
  if (targetAngle != angle) {
    int increment = (targetAngle > angle) ? 5 : -5;
    
    while (angle != targetAngle) {
      angle += increment;
      angle = constrain(angle, 0, 180);
      updateServo(angle);
      delay(15);  // Pequeña pausa entre pasos para que el movimiento sea suave
    }
    
    Serial.println("Movimiento completado.");
  } else {
    Serial.println("El ángulo ya está en la posición deseada.");
  }
}
