#include <Printers.h>
#include <XBee.h>
#include <SoftwareSerial.h>
// Pin that the thermistor is connected to
#define PINOTERMISTOR A5
// Nominal temperature value for the thermistor
#define TERMISTORNOMINAL 1000
// Nominl temperature depicted on the datasheet
#define TEMPERATURENOMINAL 25
// Number of samples
#define NUMAMOSTRAS 5
// Beta value for our thermistor
#define BCOEFFICIENT 3977
// Value of the series resistor
#define SERIESRESISTOR 1000

XBee xbee = XBee();
const int XBee_wake = 9;
XBeeAddress64 addr64 = XBeeAddress64(0xffffffff, 0xffffffff);
int amostra[NUMAMOSTRAS];
int i;
void setup(void) {
  Serial.begin(9600);
  Serial1.begin(9600);
  xbee.setSerial(Serial1);
  pinMode(PINOTERMISTOR, INPUT);
}

void loop(void) {
  pinMode(XBee_wake, OUTPUT);
  digitalWrite(XBee_wake, LOW);
  float media;
  for (i = 0; i < NUMAMOSTRAS; i++) {
    amostra[i] = 1023 - analogRead(PINOTERMISTOR);
    delay(10);
  }

  media = 0;
  for (i = 0; i < NUMAMOSTRAS; i++) {
    media += amostra[i];
  }
  media /= NUMAMOSTRAS;
  // Convert the thermal stress value to resistance
  media = 1023 / media - 1;
  media = SERIESRESISTOR / media;

  //Calculate temperature using the Beta Factor equation
  float temperatura;
  temperatura = media / TERMISTORNOMINAL;     // (R/Ro)
  temperatura = log(temperatura); // ln(R/Ro)
  temperatura /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  temperatura += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  temperatura = 1.0 / temperatura;                 // Invert the value
  temperatura -= 273.15;                         // Convert it to Celsius

  //Serial.print("The sensor temperature is: ");
  //Serial.print(temperatura);
  //Serial.println(" *C");
  uint8_t payload[] = { ' ', ' '};
  int temp = temperatura;

  if (temp < 10) {
    payload[0] = '0';
    payload[1] = '0' + temp;      // Converting int to hex could also used highByte() lowByte()
  }
  else if (temp >= 10) {
    payload[0] = '0' + (int)(temp / 10);
    payload[1] = '0' + temp % 10;
  }
  ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
  xbee.send(zbTx);
  pinMode(XBee_wake, INPUT); // put pin in a high impedence state
  digitalWrite(XBee_wake, HIGH);
  delay(1000);
}
