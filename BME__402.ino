/*
*/

// constants won't change. They're used here to set pin numbers:
const int calcium = 2;
const int creat = 3;
const int uric = 4;
const int LED = 5;
const int buttonPin = 6;  // the number of the pushbutton pin
const int rs = 8, en = 9, d4 = 10, d5 = 11, d6 = 12, d7 = 13;


// variables will change:
int calciumState = 0;
int creatState = 0;
int uricState = 0;
int buttonState = 0;  // variable for reading the pushbutton status
int test = 0;  // variable for specifying which test is selected
double avg = 0.0;


//include the library code:
#include <LiquidCrystal.h>
// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
  
void setup() {
  // set up serial communication at 9600 bits per second (baud):
  Serial.begin(9600);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);
  // initialize the feedback at triple switch to determine which test
  // resistor is connected
  pinMode(calcium, INPUT);
  pinMode(creat, INPUT);
  pinMode(uric, INPUT);
  // initialize the LED pin as an output:
  pinMode(LED, OUTPUT);
  lcd.begin(16, 2);
}

void loop() {
  digitalWrite(LED, LOW);  // LED is off
  lcd.clear();
  lcd.print("Select a test,");
  lcd.setCursor(0,1);
  lcd.print("then press GO");
  waitForButton();

  test = whichTest();  // figure out which test is desired
  displayTestSelected();  // display selected test on screen
  waitForButton();

  if (test>0 && test == whichTest()) {  // checking for errors: if test never specified or if switch has been changed
    askIfMistakeMade();  // display option to start over in case wrong test selected
    if (!mistakeMade()) {  // if no mistake has been made
      lcd.clear();
      lcd.print("Place cuvette,");
      lcd.setCursor(0,1);
      lcd.print("shut lid, hit GO");
      waitForButton();
      dataAcquisition();  // obtain average voltage value from voltage divider
      // ensuring test has not been switched and no error in data collecting
      if (whichTest() == test && avg > 0.0) {
        measureConc();  // calculate calcium concentration from data
      }
      // display error: either the test selector was switched part way (third instance), or error in data acquisition (third instance)
      else {
        lcd.clear();
        lcd.print("Error 203/303.");
      }
      // Test fully completed, pressing button starts program over
    }
  }
  // display error: either the test selector not initialized, or test selector was switched part way
  else {
    lcd.clear();
    lcd.print("Error 102/201.");
  }
  waitForButton();
}

// Method that determines if button was pressed
// @return boolean that answers question: True or false, was the button pressed?
bool ok() {
  buttonState = digitalRead(buttonPin);  // read input from one end of switch
  // if the button is pressed then the circuit will be connected and the pin will read a voltage
  if (buttonState == HIGH) {
    return true;
  }
  // if the button is not pressed then the circuit will be open and the pin will not read a voltage
  else {
    return false;
  }
}

void waitForButton() {
  bool cont = false;
  while (!cont) {  // wait for button to be pressed
    cont = ok();
  }
  delay(500);  // delay for stability TODO: figure out shortest amount of time where this still works
}

// Method that determines which test has been selected
// @return integer that is specific to a test
int whichTest() {
  calciumState = digitalRead(calcium);
  creatState = digitalRead(creat);
  uricState = digitalRead(uric);
  // reads the input from the net before each specific resistor
  // nets that are at voltage have been selected
  if (calciumState == HIGH) {
    return 1;  // 1 is unique to calcium
  }
  else if (creatState == HIGH) {
    return 2;  // 2 is unique to creatinine
  }
  else if (uricState == HIGH) {
    return 3;  // 3 is unique to uric acid
  }
  else {
    return 0;  // do not assign test selector if error has occured
  }
}

// Method that displays on lcd screen which test has been selected
void displayTestSelected() {
  lcd.clear();
  // 1 is unique to the calcium test
  if (test == 1) {
    lcd.print("Calcium test");
    lcd.setCursor(0,1);
    lcd.print("selected.");
  }
  // 2 is unique to the creatinine test
  else if (test == 2) {
    lcd.print("Creatinine test");
    lcd.setCursor(0,1);
    lcd.print("selected.");
  }
  // 3 is unique to the uric acid test
  else if (test == 3) {
    lcd.print("Uric acid test");
    lcd.setCursor(0,1);
    lcd.print("selected.");
  }
  // display error: the test selector was not initialized (first instance)
  else {
    lcd.print("Error: 101.");
  }
}

// Method that displays instructions in case a mistake was made
void askIfMistakeMade() {
  // tell user to hold button to reset program
  lcd.clear();
  lcd.print("Hold GO to");
  lcd.setCursor(0,1);
  lcd.print("restart ");
  lcd.setCursor(7, 1);
  // display which test was selected
  if (test == 1) {
    lcd.print("(Calci)");
  }
  else if (test == 2) {
    lcd.print("(Crea)");
  }
  else if (test == 3) {
    lcd.print("(Uric)");
  }
  // display error: the test selector was not initialized (third instance)
  else {
    lcd.clear();
    lcd.print("Error: 103");
  }
}

// Method that allows user to reset the program if a mistake was made
// @return boolean that answers question: True or false, was an error made?
bool mistakeMade() {
  long ttime = millis();  // set point in time
  // stay in loop for at most five seconds 
  while(millis() < ttime + 3000) {
    // if the button is pressed
    if (ok()) {
      // tell user to keep holding button
      lcd.clear();
      lcd.print("Keep holding...");
      delay(2000);  // wait appropriate time to ensure button wasn't accidentally pressed
      // if the button is still pressed
      if (ok()) {
        // inform user that program has been reset
        lcd.clear();
        lcd.print("Reset.");
        delay(3000);
        return true;  // a mistake has been made
      }
    }
  }
  return false;  // a mistake has not been made
}

// Method that turns on/off LED and reads output from voltage divider
// @return average voltage output from voltage divider
void dataAcquisition () {
  int counter = 0;  // initialize loop counter
  double sum = 0.0;  // initialize sum of all values (for averaging)
  double voltageValues[350];  // initialize array to store values in
  double aIn = A0;  // initialize pin as analog input
  // display that the test is being performed on screen
  lcd.clear();
  lcd.print("Reading...");
  if (test == 2) {
    long ttime = millis();
    long temptime = ttime;
    int pposition = 1;
    while (millis() < ttime + 179000) {
      if (millis() > temptime + 3000) {
        lcd.clear();
        lcd.setCursor(0, pposition);
        pposition = abs(pposition - 1);
        lcd.print("Reading...");
        temptime = temptime + 3000;
      }
    }
  }
  else {
    long ttime = millis();
    long temptime = ttime;
    int pposition = 1;
    while (millis() < ttime + 15000) {
      if (millis() > temptime + 3000) {
        lcd.clear();
        lcd.setCursor(0, pposition);
        pposition = abs(pposition - 1);
        lcd.print("Reading...");
        temptime = temptime + 3000;
      }
    }
  }
  digitalWrite(LED, HIGH);  // turn on LED
  delay(3000); // delay for stability
  // run the loop 350 times or break if the test selector was switched part way
  while (counter < 350 && test == whichTest()) {
    double sensorValue = analogRead(aIn); // read the input on analog pin 0:
    voltageValues[counter] = (sensorValue) * 5 / 1023; // store voltage value into array
    counter = counter + 1; // move the counter up for every loop iteration
  }
  digitalWrite(LED, LOW);  // turn LED off
  // display error: either the test selector was switched part way (second instance), or error in data acquisition (first instance)
  if (counter != 350) {
    lcd.clear();
    lcd.print("Error: 202/301.");
    test = 0;
    waitForButton();
  }
  // sum all values and find average
  else {
    for (int i = 0; i < 350; ++i) {
      sum += voltageValues[i];
    }
  avg = sum / 350;
  }
}

// Method for converting voltage value to concentration
// @param average voltage value from data acquisition
void measureConc() {
  if (avg > 0.0) {
    double conc = 0.0;
    lcd.clear();
    if (test == 1) {
      conc = 1.2055 * avg - 2.8284;
      lcd.print("Calcium: ");
      lcd.setCursor(0,1);
      lcd.print(conc, 4);
    }
    else if (test == 2) {
      conc = 4.5621*avg - 10.674;
      lcd.print("Creatinine:");
      lcd.setCursor(0,1);
      lcd.print(conc, 4);
      lcd.setCursor(6, 1);
      lcd.print(" mg/ml");
    }
      else if (test == 3) {
      conc = 0.3991*avg - 0.011;
      lcd.print("Uric Acid:");
      lcd.setCursor(0,1);
      lcd.print(conc, 4);
      lcd.setCursor(5, 1);
      lcd.print(" mg/ml");
    }
    // display error: the test selector was not initialized (fourth instance)
    else {
      lcd.clear();
      lcd.print("Error: 104.");
    }
  }
  else {
    // display error: error in data acquisition (second instance)
    lcd.clear();
    lcd.print("Error: 302.");
  }
  avg = 0.0;
}
