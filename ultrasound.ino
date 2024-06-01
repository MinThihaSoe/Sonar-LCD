#include <LiquidCrystal.h>
volatile uint16_t t_ovf_count = 0; //unsign short variable

// Initialized pin number connected to LCD module
const int RS = 22, E = 23, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(RS, E, d4, d5, d6, d7);
volatile float distance = 0; 

void setup() {
  
  cli();
  //Setting up input capture pin 
  DDRL = 0b00000000;  // Define all the L registor pins as input pins
  PORTL = 0b00000001; // Turn on pull-up resistor at PL0(ICP4)

  //Setting up trigger pulse output pin
  DDRB  = 0b00100000;   // Setting PB5->OC1A (pin 11) in PORTB to output pin
  PORTB = 0b00000000;   // Sinking the output (Ensuring no output at start)
  Serial.begin(19200);
  
  // Timer 1 setting
  TCCR1A = 0b10000010;  // Bit [1:0] = WGM1[1:0] = 10 to select mode 14. Where ICRn is TOP.
  TCCR1B = 0b00011010;  // Prescalar determined by Bit [2:0], f_clk/8 = 010
                        // Bit [4:3] = 11, Waveform Generation Mode = fast pwm
  TCCR1C = 0b00000000;
  ICR1   = 19999;        // Defining Top (100Hz PWM signal)
  OCR1A  = 19;           // Initialize duty cycle (10 us)
  
  // Timer 4 setting 
  TCCR4A = 0b00000000; // Bit [1:0] = WGM1[1:0] = 00 to select normal mode 1
  TCCR4B = 0b01000010; // Bit [2:0] = set clock speed prescaler 1/8 = 010,
                       // Bit [6] = ICES4 = 1 rising edge will trigger the capture event
  TCCR4C = 0b00000000; 
  TIMSK4 = 0b00100001; // Bit [5] = ICIE4 (Timer/Counter4, Input Capture Interrupt Enable)
                       // Bit [0] = TOIE4 (Timer/Counter4, Overflow Interrupt Enable)
  // Set up the LCD's number of columns and rows
  lcd.begin(16, 2);  // Assuming a 16x2 LCD
  // Print a message to the LCD
  lcd.print("Distance is");
  sei(); 
}

void loop() {
  lcd.setCursor(0, 1);
  lcd.print(String(distance, 4)+ " meters    ");
  delay(100);
}

// Input capture using timer 4
ISR(TIMER4_CAPT_vect)
{
  uint16_t t_event;
  static uint16_t t_last = 0;
  static bool pulse_complete = false;

  t_event = ICR4;     // ICR4 stores the data of the input capture and store it in t_event
  TCCR4B ^= 1 << 6;   // Switching the trigger condition ON at Bit [6]
  TIFR4 = 1<<5;       // ICF4 (Interrupt flag is cleared) 

  if(pulse_complete)
  {
    distance = (((float)t_event-(float)t_last)*343)/1000000;
    Serial.print(t_ovf_count);
    Serial.print(",");
    Serial.println((int32_t)t_event-(int32_t)t_last);
    }
  else
  {
    t_last = t_event;
    t_ovf_count =0;
  }
  pulse_complete =! pulse_complete;
}

ISR(TIMER4_OVF_vect)
{
  t_ovf_count ++;
}