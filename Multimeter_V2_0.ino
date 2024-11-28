#include <Adafruit_ADS1X15.h>
Adafruit_ADS1115 ads;
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 11
Adafruit_SSD1306 display(OLED_RESET);

int mode_selector = A3;
int Vcc_read = A1;
int current_switch = A7;
int Baterry_read = A7;
int CapAnalogPin = A0;
int CapAnalogPin2 = A2;
int Buzzer = 2;
int D3 = 3;
int D4 = 4;          //Pins for resistance mode
int D5 = 5;          //Pins for resistance mode
int Induct_OUT = 6;  //Pin for pulse for inductance mode
int D7 = 7;          //For voltage mode, divider for set to GND
int Right_button = 8;
int Induct_IN = 9;
int Left_button = 10;
int dischargePin = 13;
int D12 = 12;  //For voltage mode, divider for set to GND
int chargePin = 11;



int mode = 1;

//Voltage mode
float VoltageReadOffset = 0.0;
float Voltage = 0.0;
float Volt_ref = 0;

//Resistance mode
int R2_1 = 2050;
int R2_2 = 20;
int R2_3 = 195;
int Res_Offset = 0;
bool conductivity = true;

//Capacitance mode
unsigned long startTime;
unsigned long elapsedTime;
float microFarads;
float nanoFarads;
#define resistorValue 10200.00F  //Remember, we've used a 10K resistor to charge the capacitor
bool cap_scale = false;
//Small scale
const float IN_STRAY_CAP_TO_GND = 56.88;
const float IN_CAP_TO_GND = IN_STRAY_CAP_TO_GND;
const float R_PULLUP = 34.8;
const int MAX_ADC_VALUE = 1023;

//Inductance mode
double pulse, frequency, Induct_cap, inductance;

//Current mode
float Sensibility = 0.113;  //Given by the ACS712 datasheet but tweeked a bit


void setup() {
  Serial.begin(9600);
  ads.begin();  //Start the communication with the ADC

  analogReference(DEFAULT);

  pinMode(mode_selector, INPUT);
  pinMode(Vcc_read, INPUT);
  pinMode(current_switch, INPUT);
  pinMode(A6, INPUT);
  pinMode(CapAnalogPin, INPUT);
  pinMode(CapAnalogPin2, OUTPUT);

  pinMode(Buzzer, OUTPUT);
  pinMode(D3, INPUT);
  pinMode(D4, INPUT);
  pinMode(D5, INPUT);
  pinMode(Induct_OUT, OUTPUT);
  pinMode(D7, INPUT);
  pinMode(Left_button, INPUT_PULLUP);
  pinMode(Induct_IN, INPUT);
  pinMode(Right_button, INPUT_PULLUP);
  pinMode(dischargePin, INPUT);
  pinMode(D12, INPUT);
  pinMode(chargePin, OUTPUT);


  digitalWrite(Buzzer, LOW);
  digitalWrite(chargePin, LOW);
  digitalWrite(Induct_OUT, LOW);

  delay(200);


  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(50);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(12, 0);
  display.println("       M21");
  display.println("     MULTIMETER");
  display.display();

  delay(5000);
  display.clearDisplay();
  display.display();
}

void loop() {
  analogReference(DEFAULT);
  //MODE DETECTOR
  if (analogRead(current_switch) > 512) {
    mode = 5;
  } else {
    int val = analogRead(mode_selector);
    if (val > 750) {
      //Serial.println("MODE 1");
      mode = 1;
    } else if (val > 480) {
      //Serial.println("MODE 2");
      mode = 2;
    } else if (val > 360) {
      //Serial.println("MODE 3");
      mode = 3;
    } else {
      //Serial.println("MODE 4");
      mode = 4;
    }
  }
  /////////////////////////END MODE DETECTOR//////////////////////////////






  ////////////////////////////////MODE 1//////////////////////////////////
  if (mode == 1) {
    pinMode(D12, OUTPUT);
    pinMode(D7, OUTPUT);
    digitalWrite(D12, LOW);
    digitalWrite(D7, LOW);
    float adc;  // Leemos el ADC, con 16 bits
    adc = ads.readADC_Differential_0_1();
    //adc = ads.readADC_SingleEnded(0);
    Voltage = 11 * (adc * 0.1875) / 1000 + VoltageReadOffset;
    //I've used a 1K adn 10K divider so outpout is 1/11 that's why
    //we multiply voltage by  11
    //Serial.print(Voltage, 2);
    //Serial.println(" Volts");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(12, 0);
    display.print(" VOLTAGE");

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 20);
    display.println(Voltage);
    display.println("V");
    display.display();


    delay(10);
  }
  /////////////////////////////END MODE 1/////////////////////////////////






  ////////////////////////////////MODE 2//////////////////////////////////
  if (mode == 2) {
    pinMode(D4, INPUT);
    pinMode(D5, INPUT);
    pinMode(D3, OUTPUT);
    digitalWrite(D3, LOW);
    delay(10);

    float adc2;
    float res;

    /*int res_loop = 0;
    while(res_loop < 10)
    {
      adc2 = ads.readADC_SingleEnded(2);
      Voltage = Voltage + (adc2 * 0.1875)/1000;
      res_loop = res_loop + 1;
    }
    Voltage = Voltage/10;    */

    adc2 = ads.readADC_SingleEnded(2);
    Voltage = (adc2 * 0.1875) / 1000;


    res = ((R2_1 * 5.04) / Voltage) - R2_1 - Res_Offset;  //3.422V(AMS1117 3.3V) -0.616V (diode)

    if (res < 2000) {
      Serial.print(res, 2);
      Serial.println(" Ohms");
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(4, 0);
      display.print("RESISTANCE");

      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 20);
      display.println(res);
      display.println("Ohms");
      display.display();
    } else if (res < 20000) {
      pinMode(D3, INPUT);
      pinMode(D4, OUTPUT);
      pinMode(D5, INPUT);
      digitalWrite(D4, LOW);
      delay(10);

      adc2 = ads.readADC_SingleEnded(2);
      Voltage = (adc2 * 0.1875) / 1000;
      res = ((R2_2 * 5) / Voltage) - R2_2;  //3.422V(AMS1117 3.3V) -0.616V (diode)
      Serial.print(res, 2);
      Serial.println(" KOhms");
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(4, 0);
      display.print("RESISTANCE");

      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 20);
      display.println(res);
      display.println("KOhms");
      display.display();
    } else if (res > 20000) {
      pinMode(D5, OUTPUT);
      digitalWrite(D5, LOW);
      pinMode(D3, INPUT);
      pinMode(D4, INPUT);
      delay(10);
      adc2 = ads.readADC_SingleEnded(2);
      Voltage = (adc2 * 0.1875) / 1000;
      res = ((R2_3 * 5) / Voltage) - R2_3;  //3.422V(AMS1117 3.3V) -0.616V (diode)
      if (res < 2000) {
        Serial.print(res, 2);
        Serial.println(" KOhms");
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(4, 0);
        display.print("RESISTANCE");

        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 20);
        display.println(res);
        display.println("KOhms");
        display.display();
      } else {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(4, 0);
        display.print("RESISTANCE");

        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 20);
        display.println("INSERT");
        display.println("RESISTOR");
        display.display();
      }
    }
    delay(50);
  }  //end mode 2
  /////////////////////////////END MODE 2/////////////////////////////////







  ////////////////////////////////MODE 3//////////////////////////////////
  if (mode == 3) {
    //This is the scale for 1uF to max value
    if (!cap_scale) {
      pinMode(CapAnalogPin, INPUT);
      pinMode(CapAnalogPin2, OUTPUT);
      pinMode(chargePin, OUTPUT);

      digitalWrite(CapAnalogPin2, LOW);
      digitalWrite(chargePin, HIGH);
      startTime = micros();
      while (analogRead(CapAnalogPin) < 648) {}  //end while

      elapsedTime = micros() - startTime;
      microFarads = ((float)elapsedTime / resistorValue) - 0.01097;

      if (microFarads > 1) {
        Serial.print(microFarads);
        Serial.println(" uF");
        delay(500);
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(12, 0);
        display.print("CAPACITOR");

        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 15);
        display.println(microFarads);
        display.println("uF");
        display.display();
      }

      else {
        /*nanoFarads = microFarads * 1000.0; 
        Serial.print(nanoFarads);
        Serial.println(" nF");  
        delay(500);  */
        cap_scale = true;  //We change the scale to MIN - 1uF
      }

      digitalWrite(chargePin, LOW);
      pinMode(dischargePin, OUTPUT);
      digitalWrite(dischargePin, LOW);         //discharging the capacitor
      while (analogRead(CapAnalogPin) > 0) {}  //This while waits till the capaccitor is discharged
      pinMode(dischargePin, INPUT);            //this sets the pin to high impedance
      //Serial.println("Discharging");
    }  //end of upper scale


    if (cap_scale) {
      pinMode(dischargePin, INPUT);
      pinMode(chargePin, INPUT);
      pinMode(CapAnalogPin2, OUTPUT);
      pinMode(CapAnalogPin, INPUT);

      digitalWrite(CapAnalogPin2, HIGH);
      int val = analogRead(CapAnalogPin);
      digitalWrite(CapAnalogPin2, LOW);

      if (val < 1000) {
        pinMode(CapAnalogPin, OUTPUT);
        float capacitance = (float)val * IN_CAP_TO_GND / (float)(MAX_ADC_VALUE - val);

        Serial.print(capacitance, 0);
        Serial.println(" pF");
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(10, 0);
        display.print("CAPACITOR");

        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 20);
        display.println(capacitance);
        display.println("pF");
        display.display();
        delay(400);
      }

      else {
        pinMode(CapAnalogPin, OUTPUT);
        delay(1);
        pinMode(CapAnalogPin2, INPUT_PULLUP);
        unsigned long u1 = micros();
        unsigned long t;
        int digVal;
        do {
          digVal = digitalRead(CapAnalogPin2);
          unsigned long u2 = micros();
          t = u2 > u1 ? u2 - u1 : u1 - u2;
        } while ((digVal < 1) && (t < 400000L));

        pinMode(CapAnalogPin2, INPUT);
        val = analogRead(CapAnalogPin2);
        digitalWrite(CapAnalogPin, HIGH);
        int dischargeTime = (int)(t / 1000L) * 5;
        delay(dischargeTime);
        pinMode(CapAnalogPin2, OUTPUT);
        digitalWrite(CapAnalogPin2, LOW);
        digitalWrite(CapAnalogPin, LOW);
        float capacitance = -(float)t / R_PULLUP / log(1.0 - (float)val / (float)MAX_ADC_VALUE);

        if (capacitance > 1000.0) {
          /*Serial.print(capacitance / 1000.0, 2);
          Serial.println(" uF");*/
          cap_scale = false;  //We change the scale to 1uF - max
        } else {
          Serial.print(capacitance);
          Serial.println(" nF");
          display.clearDisplay();
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(10, 0);
          display.print("CAPACITOR");

          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0, 20);
          display.println(capacitance);
          display.println("nF");
          display.display();
        }
      }
      while (micros() % 1000 != 0)
        ;
    }  ////end of lower scalee
  }    //end mode 3
  /////////////////////////////END MODE 3/////////////////////////////////




  ////////////////////////////////MODE 4//////////////////////////////////
  if (mode == 4) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(3, 0);
    display.print("INDUCTANCE");
    display.display();

    digitalWrite(Induct_OUT, HIGH);
    delay(5);  //give some time to charge inductor.
    digitalWrite(Induct_OUT, LOW);
    delayMicroseconds(100);                  //make sure resination is measured
    pulse = pulseIn(Induct_IN, HIGH, 5000);  //returns 0 if timeout
    if (pulse > 0.1)                         //if a timeout did not occur and it took a reading:
    {
      //#error insert your used capacitance value here. Currently using 2uF. Delete this line after that
      Induct_cap = 2.E-6;  // - insert value here

      frequency = 1.E6 / (2 * pulse);
      inductance = 1. / (Induct_cap * frequency * frequency * 4. * 3.14159 * 3.14159);  //one of my profs told me just do squares like this
      inductance *= 1E6;                                                                //note that this is the same as saying inductance = inductance*1E6

      //Serial print
      Serial.print(inductance);
      Serial.println("uH");


      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 20);
      display.println(inductance);
      display.println("uH");
      display.display();
      delay(100);
    }
  }
  //end mode 4
  ////////////////////////////////MODE 5//////////////////////////////////
  if (mode == 5) {
    int read_loop = 0;
    float Sens_volt = 0;
    while (read_loop < 100) {
      float adc3;  // Leemos el ADC, con 16 bits
      adc3 = ads.readADC_SingleEnded(3);
      Sens_volt = Sens_volt + ((adc3 * 0.1875) / 1000);
      read_loop = read_loop + 1;
    }
    //133 233
    Sens_volt = Sens_volt / 100;
    Serial.println(Sens_volt);
    float I = -0.004 + ((Sens_volt - 2.5075) / Sensibility);
    Serial.print(I, 3);
    Serial.println(" A");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(10, 0);
    display.print(" CURRENT");

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 20);
    display.println(I, 3);
    display.println("A");
    display.display();
    delay(50);
  }
}
