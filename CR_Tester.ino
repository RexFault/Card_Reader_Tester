/** 
 *  Custom Wiegand Card Reader Tester
 *  Will read a card and output it's Facility Code & Card Number to a LCD Screen
 *  Currently Made to support the Standard 26-Bit HID Card Format.
 */

#include <Wiegand.h>
#include <LiquidCrystal_I2C.h>

/* Data Pins (Used for interrupts */
#define PIN_D0 2
#define PIN_D1 3
#define LCD_SDA A4
#define LCD_SCL A5 

//Comment out this line to remove the debug mode output
#define _xDEBUG_ 1

//Wiegand Object Global
Wiegand wiegand;

//LCD Global Object
LiquidCrystal_I2C *outputDisplay = NULL;

// Initialize Wiegand reader
void setup() {
  Serial.begin(115200);
  outputDisplay = new LiquidCrystal_I2C(0x27, 20, 4);

  //Output Setup
  outputDisplay->init();
  outputDisplay->begin(20,4); //Initialize Display
  outputDisplay->backlight(); //Turn on the backlight
  outputDisplay->setCursor(0,0);
  outputDisplay->print(" Card Reader Tester ");
  outputDisplay->setCursor(0,1);
  outputDisplay->print("    Created by:    ");
  outputDisplay->setCursor(0,2);
  outputDisplay->print("   Shane McIntosh");
  delay(3000); //Wait 3 seconds after showing the splash screen;

  //Install listeners and initialize Wiegand reader
  #ifdef _DEBUG_
  wiegand.onReceive(receivedData, "Card read: ");
  wiegand.onReceiveError(receivedDataError, "Unknown Format: ");
  wiegand.onStateChange(stateChanged, "State changed: ");
  wiegand.begin(Wiegand::LENGTH_ANY, true);
  #else
  wiegand.onReceive(receivedData,(const char*)outputDisplay);
  wiegand.onReceiveError(receivedDataError, (const char *)outputDisplay);
  wiegand.onStateChange(stateChanged);
  wiegand.begin(Wiegand::LENGTH_ANY, true);
  #endif

  //initialize pins as INPUTS
  pinMode(PIN_D0, INPUT);
  pinMode(PIN_D1, INPUT);

  //Sends the initial pin state to the Wiegand library
  pinStateChanged();

  Serial.println("Setup Complete");
  outputDisplay->setCursor(0,3);
  outputDisplay->print("System Ready");
}

// Every few milliseconds, check for pending messages on the wiegand reader
// This executes with interruptions disabled, since the Wiegand library is not thread-safe
void loop() {

  wiegand.flush();
  wiegand.setPin0State(digitalRead(PIN_D0));
  wiegand.setPin1State(digitalRead(PIN_D1));
}

// When any of the pins have changed, update the state of the wiegand library
void pinStateChanged() {
  wiegand.setPin0State(digitalRead(PIN_D0));
  wiegand.setPin1State(digitalRead(PIN_D1));
}

// Notifies when a reader has been connected or disconnected.
// Instead of a message, the seconds parameter can be anything you want -- Whatever you specify on `wiegand.onStateChange()`
void stateChanged(bool plugged, const char* message) {
  #ifdef _DEBUG_
    Serial.print(message);
    Serial.println(plugged ? "CONNECTED" : "DISCONNECTED");
  #endif
}

// Notifies when a card was read.
// Instead of a message, the seconds parameter can be anything you want -- Whatever you specify on `wiegand.onReceive()`
#ifdef _DEBUG_
void receivedData(uint8_t* data, uint8_t bits, const char* message) {
    Serial.print(message);
    Serial.print(bits);
    Serial.print("bits / ");
    //Print value in HEX
    uint8_t bytes = (bits+7)/8;
    for (int i=0; i<bytes; i++) {
        Serial.print(data[i] >> 4, 16);
        Serial.print(data[i] & 0xF, 16);
    }
    Serial.println();

    Serial.println("Decoded Format: ");
    Serial.print("Facility Code: ");
    Serial.print(data[0]);
    Serial.println();

    uint16_t cardNumber = ((uint16_t)data[1] << 8) | data[2];
    Serial.print("Card Number: ");
    Serial.print(cardNumber);
    Serial.println();
}
#else
void receivedData(uint8_t* data, uint8_t bits, const char* message) {
    uint8_t bytes = (bits+7)/8;
    uint16_t cardNumber = ((uint16_t)data[1] << 8) | data[2];
    LiquidCrystal_I2C *outputDisplay = (LiquidCrystal_I2C *)message;

    //noInterrupts();
    Serial.println("Decoded Format: ");
    Serial.print("Facility Code: ");
    Serial.print(data[0]);
    Serial.println();

    Serial.print("Card Number: ");
    Serial.print(cardNumber);
    Serial.println();
    
    outputDisplay->clear();
    outputDisplay->setCursor(0,0);
    
    outputDisplay->print("Card Read: ");
    outputDisplay->setCursor(0,1);
    outputDisplay->print("Facility Code: ");
    outputDisplay->print(data[0]);
    outputDisplay->setCursor(0,2);
    outputDisplay->print("Card Number: ");
    outputDisplay->setCursor(0,3);
    outputDisplay->print(cardNumber);
    //interrupts();
}
#endif

// Notifies when an invalid transmission is detected
void receivedDataError(Wiegand::DataError error, uint8_t* rawData, uint8_t rawBits, const char* message) {
  #ifdef _DEBUG_
    Serial.print(message);
    Serial.print(Wiegand::DataErrorStr(error));
    Serial.print(" - Raw data: ");
    Serial.print(rawBits);
    Serial.print("bits / ");

    //Print value in HEX
    uint8_t bytes = (rawBits+7)/8;
    for (int i=0; i<bytes; i++) {
        Serial.print(rawData[i] >> 4, 16);
        Serial.print(rawData[i] & 0xF, 16);
    }
    Serial.println();
    #else
        LiquidCrystal_I2C *outputDisplay = (LiquidCrystal_I2C *)message;
        uint8_t bytes = (rawBits+7)/8;
        char hexBytes[(bytes*2)] = "ABACADAEEE";
        for (int i = 0; i<bytes; i++) {
          sprintf(&hexBytes[(i*2)],"%02x", rawData[i]);
        }
        outputDisplay->clear();
        //outputDisplay->setCursor(0,0);
        //outputDisplay->print("Card Read: ");
        outputDisplay->setCursor(0,0);
        outputDisplay->print("Number of Bytes: ");
        outputDisplay->print(bytes);
        outputDisplay->setCursor(0,1);
        outputDisplay->print("Number of Bits: ");
        outputDisplay->print(bytes*8);
        outputDisplay->setCursor(0,2);
        outputDisplay->print("Raw Card Value: ");
        outputDisplay->setCursor(0,3);
        for (int i = 0; i < bytes*2; i+=2) {
          outputDisplay->print(hexBytes[i]);
          outputDisplay->print(hexBytes[i+1]);
        }
        
    #endif
}
