#include <Wire.h> // you know what it is

const int OUT_PIN = A5; // globaly assignde as o/p but later we need to change it
const int IN_PIN = A0; // globaly assignde as i/p but later we need to change it
const float IN_STRAY_CAP_TO_GND = 25.5; // initialise inturnal stray capacitance of architecture approx and tuned
const float IN_CAP_TO_GND = IN_STRAY_CAP_TO_GND; // for calculation
const float R_PULLUP = 29.89; // internal pulup resistance of anal-log pins approx and tuned
// every things are uned in my home envirment
float cap = 0; // initialise the main variable (filter)
const int MAX_ADC_VALUE = 1023; //define max adc value

#define size 10 // define windo size for running avg filter (RAF)
float window[size] = { 0 }; // initialise the main array for RAF

void push(float value) { // function to store real time cappacitance into array EX> [0,0,0] then [13.4,0,0] then [13.7,13,4,0] ..... [x,y,13.7] and so on
  for (int i = 1; i < size; i++) {
    window[i - 1] = window[i];
  }
  window[size - 1] = value;
}

float avg() { // function to add values inside the array when it is running
  float sum = 0.00;
  for (int i = 0; i < size; i++) {
    sum += window[i];
  }
  return sum / size; //calculate the avg when array is running
}

void setup() { // this is setup function if you don't know
  pinMode(OUT_PIN, OUTPUT); // set the impidance of pin A5
  pinMode(IN_PIN, OUTPUT); // se the impidance of pin A0
  // previously mentioned we need to change it
  Serial.begin(9600); // Initialize serial communication
}

void loop() { // annd yes inganors it is an infinite loop known ass void loop
  delay(10); // a delay for nothing yes yes for nothing
  pinMode(IN_PIN, INPUT); // initialise the impidance of the A0 pin
  digitalWrite(OUT_PIN, HIGH); // set A5 at high
  // mens in this line uno is charging the cappacitor vai A5 pin
  int val = analogRead(IN_PIN); // at that time take anal-log read and store it into val
  digitalWrite(OUT_PIN, LOW); // set A5 at high
                                 // mens it is discharging the capacitor
                                 /* every anal-log pins has a intural capacitor cnnected with ground 
   pinmode i/p mens intural capacitor is decharged and vice versa for o/p
   so when we cunnect our capacitor with it, i will create a capacitor votage devider with i/p pins capactor(A0)
   and here we are mesuring the voltage accross i/p pin's capacitor */
  pinMode(IN_PIN, OUTPUT); //set the impidance of pin A5 and charge the inturnal capaitance

  if (val < 1000) { // condition for pF range capacitor
    float val0 = MAX_ADC_VALUE - val; /* i know you know we all know
    to caculate capacitance we need voltage accross our cappacitance not i/p capacitance 
    anal-log read use to masure voltage reffring to the ground 
    so the voltage accross our cappacitor = Vcc(1023) - voltage drop accress i/p pin's capacitor(val)*/
    float capacitance = (float)val * IN_CAP_TO_GND / val0; // formula to calculate capacitance [i/p capacitance * {Volt ip's / Volt our's}]

    cap = capacitance; // for filuring purpas
    push(capacitance); // call push function to store capacitance value into the array
    Serial.print(avg()); // call avg functuon ,calculate avg and print it in same time (yes i am lazy some time)
    Serial.print(","); // print a coma
    Serial.print(cap); // print raw capacitor value
    Serial.println(" pF"); //print unit
    // you can remove print "coma", "cap", and "pF" if you think it is helpfull for me to track the performance of filter
  } else { // if our capacitance is not in between 999pF
    pinMode(OUT_PIN, INPUT_PULLUP); // realy don't know what is going on from 66 to 75
    unsigned long u1 = micros();
    unsigned long t;
    int digVal;

    do {
      digVal = digitalRead(OUT_PIN);
      unsigned long u2 = micros();
      t = u2 > u1 ? u2 - u1 : u1 - u2;
    } while ((digVal < 1) && (t < 400000L));
    // but i do know that it is use to calculate time to charge our cappacior
    pinMode(OUT_PIN, INPUT); // set impidance of pin A5
    val = analogRead(OUT_PIN); // again same way calculate voltage accross i/p capacitor
    digitalWrite(IN_PIN, HIGH); // set A0 high
    int dischargeTime = (int)(t / 1000L) * 5; // calculate the time our capacitor takes to discharg aka relaxation time
    delay(dischargeTime); // delay this time
    pinMode(OUT_PIN, OUTPUT); // set impidance
    digitalWrite(OUT_PIN, LOW); // set both A0 and A5 low
    digitalWrite(IN_PIN, LOW);

    float capacitance = -(float)t / R_PULLUP / log(1.0 - (float)val / (float)MAX_ADC_VALUE); // calculate the capacitance

    if (capacitance > 1000.0) { // same from line no 58
      cap = 0;
      cap = capacitance * 1000; // converting nF to uF to pF
      push(cap);
      Serial.print(avg());
      Serial.print(",");
      Serial.print(cap);
      Serial.println(" uF");
    } else {
      cap = 0;
      cap = capacitance * 1000; // converting nF to pF
      push(cap);
      Serial.print(avg());
      Serial.print(",");
      Serial.print(cap);
      Serial.println(" nF");
    }
  }
  while (micros() % 1000 != 0); // time
}

