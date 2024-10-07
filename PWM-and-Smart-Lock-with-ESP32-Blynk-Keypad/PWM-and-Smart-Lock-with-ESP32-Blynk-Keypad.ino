#define BLYNK_TEMPLATE_ID "TMPL6Wy5E5M76"
#define BLYNK_TEMPLATE_NAME "PWM CONTROLLER"
#define BLYNK_AUTH_TOKEN "3MmNZkQlmiUeaWmcA-GeXpk5lRHbQIv9"
#define BLYNK_PRINT Serial

#define ROW_NUM 4
#define COLUMN_NUM 4 
#define RELAY_PIN 21 
#define LED_PIN 23 
#define TIMEOUT 5000
#define UNLOCK_DURATION 300000
#define EEPROM_SIZE 512

#include <Keypad.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

const int pwmPin1 = 12;
const int pwmPin2 = 14;

int pwmChannel1 = 0;
int pwmChannel2 = 1;
int freq = 30000;   
int resolution = 8; 
int dutyCycle1 = 0;
int dutyCycle2 = 0;

const char* auth  = BLYNK_AUTH_TOKEN;
const char* ssid = "Redmi 9c";
const char* pass = "westaunyambungkan";

String input_password;
String new_password;
String confirm_password;
bool changing_password = false;
bool verify_old_password = false;
bool confirm_new_password = false;
bool unlocked = false;
unsigned long last_keypress_time = 0;
unsigned long unlock_time = 0;

bool isAdjustingPin1 = true;  

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pin_rows[ROW_NUM] = {19, 18, 5, 17};
byte pin_column[COLUMN_NUM] = {16, 4, 0, 2};

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

BLYNK_WRITE(V1) {
  int vall = param.asInt();
  if(isAdjustingPin1){
    dutyCycle1 = constrain(vall, 0, 255);
    ledcWrite(pwmChannel1, dutyCycle1);
    EEPROM.write(500, dutyCycle1); // Save to EEPROM
    EEPROM.commit();
    Serial.print("PWM Value: ");
    Serial.println(dutyCycle1);
  }
  else{
    dutyCycle2 = constrain(vall, 0, 255);
    ledcWrite(pwmChannel2, dutyCycle2);
    EEPROM.write(501, dutyCycle2); 
    EEPROM.commit();
    Serial.print("PWM2 Value: ");
    Serial.println(dutyCycle2);
    Blynk.virtualWrite(V5, dutyCycle2);
  }
}

BLYNK_WRITE(V2) {
  int vall1 = param.asInt();
  if(vall1 == 1){
    isAdjustingPin1 = !isAdjustingPin1;
    posakhir(); 
  }
}

BLYNK_WRITE(V4){
  int lock = param.asInt();
  if(lock == 1){
    digitalWrite(RELAY_PIN, LOW);
    delay(2500);
    digitalWrite(RELAY_PIN, HIGH);
    Blynk.virtualWrite(V4, LOW);
  }
}

void setup() {
  Serial.begin(9600);
  EEPROM.begin(512);
  dutyCycle1 = EEPROM.read(500);
  dutyCycle2 = EEPROM.read(501);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);
  EEPROM.begin(EEPROM_SIZE);
  //resetPasswordInEEPROM();

  ledcSetup(pwmChannel1, freq, resolution);
  ledcAttachPin(pwmPin1, pwmChannel1);
  ledcWrite(pwmChannel1, dutyCycle1); 

  ledcSetup(pwmChannel2, freq, resolution);
  ledcAttachPin(pwmPin2, pwmChannel2);
  ledcWrite(pwmChannel2, dutyCycle2);

  Blynk.begin(auth, ssid, pass);
}

String readPasswordFromEEPROM() {
  String password = "";
  for (int i = 0; i < EEPROM_SIZE; i++) {
    char c = EEPROM.read(i);
    if (c == 0) break;
    password += c;
  }
  return password;
}

void writePasswordToEEPROM(String password) {
  for (int i = 0; i < EEPROM_SIZE; i++) {
    if (i < password.length()) {
      EEPROM.write(i, password[i]);
    } else {
      EEPROM.write(i, 0);
    }
  }
  EEPROM.commit();
}

void resetPasswordInEEPROM() {
  writePasswordToEEPROM("666666");
  Serial.println("Password direset ke 666666");
}

void loop() {
  char key = keypad.getKey();
  if (key) {
    Serial.print(key);
    last_keypress_time = millis();
    digitalWrite(LED_PIN, LOW);
    static String input = "";
    input += key;

    if (key == '*') {
      Serial.println("");
      Serial.println("Masukan Ulang Sandi: ");
      input_password = "";
      new_password="";
      confirm_password = "";
      changing_password = false; 
      verify_old_password = false; 
      confirm_new_password = false;
    }
    else if (key == '#') {
      if (input_password == "1") {
        unlocked = false;
        Serial.println("");
        Serial.println("Menampilkan data dari EEPROM:");
        for (int i = 0; i < EEPROM_SIZE; i++) {
          char c = EEPROM.read(i);
          if (c != 0) {
            Serial.print("Alamat ");
            Serial.print(i);
            Serial.print(": ");
            Serial.print(c);
            Serial.print(" => ");
            Serial.print((int)c);
            Serial.print(" => ");
            Serial.println(byteToBinary(c));
          }
        }
      }
      if (unlocked) {
        Serial.println("");
        Serial.println("Membuka kunci pintu...");
        digitalWrite(RELAY_PIN, LOW); 
        delay(2000); 
        digitalWrite(RELAY_PIN, HIGH);
      } 
      else if (changing_password) {
        if (verify_old_password) {
          if (confirm_new_password){
            confirm_password = input_password;
            if (new_password == confirm_password){
              writePasswordToEEPROM(new_password);
              Serial.println("Password berhasil diubah.");
              changing_password = false;
              verify_old_password = false;
              confirm_new_password = false;
            }
            else{
              Serial.println("Konfirmasi password tidak cocok, coba lagi.");
              confirm_new_password = false; 
              for (int i = 0; i < 4; i++){
                 digitalWrite(LED_PIN, LOW);
                 delay(300);
                 digitalWrite(LED_PIN, HIGH);
                 delay(300);
              }
            }
          }
          else {
            new_password = input_password;
            Serial.println("Konfirmasi password baru diikuti dengan #");
            confirm_new_password = true;
            input_password = "";
          }
        }
          else {
            String stored_password = readPasswordFromEEPROM();
            if (input_password == stored_password) {
              Serial.println("");
              Serial.println("Password lama benar, masukkan password baru diikuti dengan #");
              verify_old_password = true;
              input_password = "";
            } 
          else {
            Serial.println("");
            Serial.println("Password lama salah, coba lagi.");
            changing_password = false;
          }
        }
      } 
      else {
        String stored_password = readPasswordFromEEPROM();
        if (input_password == stored_password) {
          Serial.println("");
          Serial.println("Password benar, membuka kunci pintu...");
          digitalWrite(RELAY_PIN, LOW); 
          delay(2000);
          digitalWrite(RELAY_PIN, HIGH);
          unlocked = true;
          unlock_time = millis(); 
        } 
        else {
          Serial.println("");
          Serial.println("Password salah, coba lagi.");
          for (int i = 0; i < 4; i++){
             digitalWrite(LED_PIN, LOW);
             delay(100);
             digitalWrite(LED_PIN, HIGH);
             delay(100);
          }
        }
      }
      input_password = "";
    } 
    else if (key == 'D') {
      unlocked = false;
      changing_password = true;
      input_password = "";
      Serial.println("");
      Serial.println("Masukkan password lama diikuti dengan #");
    } 
    else if (key == 'C') {
      unlocked = false;
      Serial.println("");
      Serial.println("Status kunci terbuka direset.");
      for (int i = 0; i < 4; i++){
        digitalWrite(LED_PIN, LOW);
        delay(250);
        digitalWrite(LED_PIN, HIGH);
        delay(250);
      }
    } 
    else {
      input_password += key;
    }
  }
  if (millis() - last_keypress_time > 150) {
      digitalWrite(LED_PIN, HIGH);
    }

    if (millis() - last_keypress_time > TIMEOUT && input_password.length() > 0) {
      Serial.println("");
      Serial.println("Timeout, silakan masukkan password lagi.");
      digitalWrite(LED_PIN, LOW);
      delay(1500);
      digitalWrite(LED_PIN, HIGH);
      input_password = ""; 
      changing_password = false;
      verify_old_password = false;
    }

    if (unlocked && millis() - unlock_time > UNLOCK_DURATION) {
      unlocked = false;
      Serial.println("");
      Serial.println("Waktu kunci terbuka habis, silakan masukkan password lagi.");
      digitalWrite(LED_PIN, LOW);
      delay(1500);
      digitalWrite(LED_PIN, HIGH);
      delay(500);
      digitalWrite(LED_PIN, LOW);
      delay(1500);
      digitalWrite(LED_PIN, HIGH);
    }
  Blynk.run(); 
}

void posakhir() {
  if (isAdjustingPin1) {
    Blynk.virtualWrite(V1, dutyCycle1);
    Blynk.virtualWrite(V3, HIGH);
  } else {
    Blynk.virtualWrite(V5, dutyCycle2);
    Blynk.virtualWrite(V3, LOW);
  }
}
String byteToBinary(byte b) {
  String binary = "";
  for (int i = 7; i >= 0; i--) {
    binary += (b >> i) & 1 ? '1' : '0';
  }
  return binary;
}
