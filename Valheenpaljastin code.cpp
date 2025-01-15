#include <avr/wdt.h>          // Vahtikoiran kirjasto 
#include <Wire.h>             // I2C-kirjasto 
#include <DFRobot_RGBLCD1602.h>  // LCD-kirjasto 


// LCD:n I2C-osoite ja asetukset 

DFRobot_RGBLCD1602 lcd(0x3E, 16, 2);  // Osoite 0x3E, 16 merkkiä, 2 riviä 
 

// Perusasetukset ja muuttujat 

int baselineA0 = 0;  // A0:n perusarvo (kalibroitu) 

int baselineA1 = 0;  // A1:n perusarvo (kalibroitu) 

long previousMillis = 0;  // Aikapäivämäärä (käytetään ajastukseen) 

const int interval = 1000;  // Aikaväli (millisekunteina) 

 

// Liukuva keskiarvo (Moving Average) suodatus 

int getSmoothedValue(int pin, int samples) { 

  long total = 0; 

  for (int i = 0; i < samples; i++) { 

    total += analogRead(pin); 

    delay(5); 

  } 

  return total / samples; 

} 

 

void setup() { 

  Serial.begin(9600);  // Aloitetaan sarjaliikenne 

 

  // Asetetaan LED-pinien tilaksi lähtötila 

  pinMode(2, OUTPUT);  // Vihreä LED 

  pinMode(3, OUTPUT);  // Keltainen LED 

  pinMode(4, OUTPUT);  // Punainen LED 

 

  // LED-käynnistysanimaatio 

  digitalWrite(2, HIGH); 

  delay(500); 

  digitalWrite(3, HIGH); 

  delay(500); 

  digitalWrite(4, HIGH); 

  delay(500); 

  digitalWrite(2, LOW); 

  digitalWrite(3, LOW); 

  digitalWrite(4, LOW); 

 

  // LCD:n käynnistys ja viestin näyttö 

  lcd.init();  // Alustetaan näyttö 

  lcd.setBacklight(255);  // Taustavalo päälle 

  lcd.print("Kalibrointi..."); 

  delay(2000); 

 

  // Kalibrointi 

  long totalA0 = 0; 

  long totalA1 = 0; 

  for (int i = 0; i < 100; i++) { 

    totalA0 += analogRead(A0); 

    totalA1 += analogRead(A1); 

    delay(50); 

  } 

  baselineA0 = totalA0 / 100; 

  baselineA1 = totalA1 / 100; 

  lcd.clear(); 

  lcd.print("Valmis!");  // Kalibrointi valmis 

  delay(2000); 

 

  // Käynnistetään vahtikoira 

  wdt_enable(WDTO_8S); 

} 

 

void loop() { 

  wdt_reset();  // Nollataan vahtikoira 

 

  long currentMillis = millis(); 

  int sensorValueA0 = getSmoothedValue(A0, 10); 

  int sensorValueA1 = getSmoothedValue(A1, 10); 

  int adjustedValueA0 = sensorValueA0 - baselineA0; 

  int adjustedValueA1 = sensorValueA1 - baselineA1; 

 

  // LED-logiikka ja näytön viestit 

  if (currentMillis - previousMillis > interval) { 

    lcd.clear();  // Tyhjennetään LCD ennen uuden viestin tulostusta 

    if (adjustedValueA0 > 150 || adjustedValueA1 > 150) { 

      digitalWrite(4, HIGH);  // Punainen LED päälle 

      digitalWrite(3, LOW); 

      digitalWrite(2, LOW); 

      lcd.print("Valhe!");  // Näytölle teksti 

    } else if (adjustedValueA0 > 50 || adjustedValueA1 > 50) { 

      digitalWrite(3, HIGH);  // Keltainen LED päälle 

      digitalWrite(4, LOW); 

      digitalWrite(2, LOW); 

      lcd.print("Epailyttava"); 

    } else if (adjustedValueA0 > 20 || adjustedValueA1 > 20) { 

      digitalWrite(2, HIGH);  // Vihreä LED päälle 

      digitalWrite(3, LOW); 

      digitalWrite(4, LOW); 

      lcd.print("Totta!"); 

    } else { 

      digitalWrite(2, LOW);  // Sammutetaan kaikki LEDit 

      digitalWrite(3, LOW); 

      digitalWrite(4, LOW); 

      lcd.print("Odotetaan kysymysta"); //Neutraali tila 

    } 

    previousMillis = currentMillis; 

  } 

 

  // Tulostetaan Serial Plotteriin 

  Serial.print(sensorValueA0); 

  Serial.print(" "); 

  Serial.println(sensorValueA1); 

 

  delay(20); 

} 