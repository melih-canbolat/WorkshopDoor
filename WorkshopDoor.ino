/*--- İ. Melih Canbolat ---*/
/*
 * Controls the servo motor that locks/unlocks the workshop door.
 * 
 * - A keypad is used to enter the password.
 * - "Press 4,1 -> enter the password -> press 4,3" to open the lock.
 * - Locks the system after a certain number of failed attempts.
 * - Does not respond to the keypad unless 4,1 is pressed first (or 4,4)
 * - Press 4,4 to lock immediately.
 * - The buzzer gives feedback according to the action taken.
 * - A button overrides password protection and locks/unlocks w/out password.
 * 
 */


#include <Keypad.h>
#include <Servo.h>

const String password = "1234";     // Password
const int max_attempt = 3;          // Maximum number of attempts to get the password right
const int t_lock = 6;               // Duration of lock for system if max_attempt is exceeded (seconds)
const int open_lock_angle = 20;     // Servo angle for unlocking
const int closed_lock_angle = 140;  // Servo angle for locking

String input;                   // Variable to store user input
int failed_attempt = 0;         // Count of failed attempts
const int servoDelay = 1000;
const int buzzer = 12;          // Buzzer pin
const int super_buttonPin = 2;  // Button that opens/closes the door w/out password (interrupt)
const int servo_pin = 11;       // Servo motor signal pin
const byte ROWS = 4;            // Number of rows
const byte COLS = 4;            // Number of columns
volatile bool lock_state ;      // Lock state; True if locked
volatile unsigned long last_millis;        // Variable to store time for debouncing
const unsigned long debounce_time = 1000;  // Debouncing time in milliseconds

char keys[ROWS][COLS] =
{
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'G','0','R','D'}
};

byte row_Pins[ROWS] = {10, 9, 8, 7};  // Row pins of the keypad
byte col_Pins[COLS] = {6, 5, 4, 3};  // Column pins of the keypad

Keypad keypad = Keypad(makeKeymap(keys), row_Pins, col_Pins, ROWS, COLS);
Servo servo;

void setup()
{
  Serial.begin(115200);
  Serial.println("The system started - door is locked");
  
  servo.attach(servo_pin);
  servo.write(closed_lock_angle);  // Lock the door initially
  lock_state = true;
  delay(servoDelay);
  servo.detach();  // Detach the servo to save power

  pinMode(buzzer, OUTPUT);
  pinMode(super_buttonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(super_buttonPin), isr, FALLING);
//  input.reserve(16);  // Allocate a buffer in memory for manipulation of the string
  keypad.setDebounceTime(50);
}

void loop()
{
//  char key = keypad.getKey();
  char key = keypad.waitForKey();  // Wait until a key is pressed

  if (key == 'G')
  {
    input = "";  // Clear stored input
    beep(2000,200);
    Serial.print("Enter password: ");
    while (true)
    {
      char key = keypad.getKey();
      if (key != 'R' && key !='G' && key)  // If a key is pressed other than 'R' and 'G'
      {
          beep(1000,100);
          Serial.print("*");
          input += key;  // Append new character to input string
      }
      else if (key == 'R')
      {
        if (input == password)
        {
          Serial.println("  Correct Password. The door is opened");
          failed_attempt = 0;
          beep(3000,150);
          beep(5000,150);
          servo.attach(servo_pin);
          servo.write(open_lock_angle);  // Open the door
          lock_state = false;
          delay(servoDelay);
          servo.detach();  // Detach the servo to save power
        }
        else
        {
          Serial.println("  Wrong Password");
          beep(200,500);
          failed_attempt +=1;
          if (failed_attempt >= max_attempt)
          {
            Serial.print("Max allowable attempts exceeded. The system is locked for ");
            Serial.print(t_lock);
            Serial.println(" seconds");
            beep(4000,3000);
            delay(t_lock*1000);  // Lock the system for "t_lock" seconds
            failed_attempt = 0;  // Reset failed attempt count
          }
        }
        break;
      }
    }
  }
  else if (key == 'D')
  {
    Serial.println("The door is locked");
    beep(800,500);
    servo.attach(servo_pin);
    servo.write(closed_lock_angle);  // Lock the door
    lock_state = true;
    delay(servoDelay);
    servo.detach();  // Detach the servo to save power
  }
}

void beep(int freq, int duration)  // Controls the buzzer
{
  tone(buzzer, freq);
  delay(duration);
  noTone(buzzer);
}

void isr() // Interrupt service routine
{
  if (millis() > (last_millis + debounce_time))  // Debouncing
  {
    last_millis = millis();
    Serial.print("Button is pressed - ");
    lock_state = !lock_state;
    switch (lock_state)
    {
      case 0:
        Serial.println("The door is opened");
        servo.attach(servo_pin);
        servo.write(open_lock_angle);  // Open the door
        break;

      case 1:
        Serial.println("The door is locked");
        servo.attach(servo_pin);
        servo.write(closed_lock_angle);  // Lock the door
        break;
    }
  }
}
