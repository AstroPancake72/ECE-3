// PWM Duty Cycle Experiment
// 
// This code does the following:
//  - create a PWM signal that runs the left motor
//    - this is set by the variable enc_bin_cnt (set by Serial Input)
//  - calculate the speed of the left motor using the encoder
//    - this measurement is in arbitrary units (details below)
// 

#include <ECE3.h> // Used for encoder functionality

// NOTE: Function descriptions
//       - getEncoderCount_left()
//           returns how many encoder increments the left wheel has turned
//       - resetEncoderCount_left()
//           sets encoder increments to 0

// Pin Assignments
const int left_nslp_pin = 31;
const int left_dir_pin  = 29;
const int left_pwm_pin  = 40;

// Motor Speed
int left_spd;

// Encoder Variables
long enc_bin_cnt;
const unsigned long enc_bin_len = 50; // 50 ms bins
    // Encoder Speed Calculation Explanation:
    // We wait for a set amount of time (enc_bin_len), and find how many
    // times the encoder has incremented in that period. We call 
    // this period a bin when refering to the encoder. The number 
    // encoder counts per bin is a proportional to speed.

// Serial Communication
char new_duty_cycle[4];

// Prototypes
int get_spd(int prev_spd);

///////////////////////////////////
void setup() {
// This function runs once

  // Pin Settings
  pinMode(left_nslp_pin,OUTPUT);
  pinMode(left_dir_pin,OUTPUT);
  pinMode(left_pwm_pin,OUTPUT);

  // Setting Initial Values
  digitalWrite(left_dir_pin,LOW);
  digitalWrite(left_nslp_pin,HIGH);

  left_spd = 0;
  
  ECE3_Init(); // Used for encoder functionality

  Serial.begin(9600); // data rate for serial data transmission
  
  delay(2000); //Wait 2 seconds before starting 
}

void loop() {
  // Get the new speed from Serial input
  left_spd = get_spd(left_spd);

  // Set the new speed
  analogWrite(left_pwm_pin, left_spd);
  delay(500);

  // Reset the encoder and count how many ticks
  // occur in enc_bin_len milliseconds.
  resetEncoderCount_left();
  delay(enc_bin_len);
  enc_bin_cnt = getEncoderCount_left();

  // Report encoder counts and speed
  Serial.print("Encoder ticks: "); Serial.print(enc_bin_cnt);
  Serial.print(" @ left_spd: "); Serial.print(left_spd);
  Serial.print("\n");
}

int get_spd(int prev_spd) {

  int bytes_avail = Serial.available();
  if(bytes_avail == 0) // no new serial inputs
    return prev_spd;
    
  else if (bytes_avail > 4) { // invalid serial input
    
    do { // eat invalid buffered input
      delay(100);
      Serial.read();
    } while(Serial.available() > 0);  
    Serial.println("INVALID"); 
    //Serial.println(bytes_avail);  
    return prev_spd;
  }
  
  else {
    for (int i = 0; i < bytes_avail; i++) // read out buffer
      new_duty_cycle[i] = Serial.read();
    int sum = 0;
    for (int i = 0; i < bytes_avail; i++) { // calculate new speed
      
      int num = new_duty_cycle[i] - '0';
      if (num == -38)
        break;
      else if (num > 9 || num < 0) { // invalid character
        Serial.println("INVALID"); 
        return prev_spd;
      }
      sum += pow(10, bytes_avail - 2 - i) * num;
    }

    if (sum >= 255)
      return 255;
    else
      return sum;
  }
}

int getIRFusion()
{
  double sensorValues[8];
  int16_t minValues[8] = [596,	527,	619,	619,	505,	685,	643,	713];
  int16_t maxValues[8] = [1516,	1395,	1730,	1208,	1203,	1616,	1492,	1787];
  ECE3_read_IR(sensorValues);

  double fusion;

  for (unsigned int i = 0; i < 8; i++)
  {
    if (sensorValues[i] < minValues[i])
    {
      minValues[i] = sensorValues[i];
    }

    sensorValues[i] -= minValues[i];
    
    if (sensorValues[i] > maxValues[i])
    {
      maxValues[i] = sensorValues[i];
    }

    sensorValues[i] = (1000 * sensorValues[i]/maxValues[i]);

    switch (i)
    {
      case 0:
        fusion -= 8*sensorValues[i];
        break;
      case 1:
        fusion -= 4*sensorValues[i];
        break;
      case 2:
        fusion -= 2*sensorValues[i];
        break;
      case 3:
        fusion -= 1*sensorValues[i];
        break;
      case 4:
        fusion += 8*sensorValues[i];
        break;
      case 5:
        fusion += 4*sensorValues[i];
        break;
      case 6:
        fusion += 2*sensorValues[i];
        break;
      case 7:
        fusion += 1*sensorValues[i];

        /*
        case 0:
        fusion -= 15*sensorValues[i];
        break;
      case 1:
        fusion -= 14*sensorValues[i];
        break;
      case 2:
        fusion -= 12*sensorValues[i];
        break;
      case 3:
        fusion -= 8*sensorValues[i];
        break;
      case 4:
        fusion += 8*sensorValues[i];
        break;
      case 5:
        fusion += 12*sensorValues[i];
        break;
      case 6:
        fusion += 14*sensorValues[i];
        break;
      case 7:
        fusion += 15*sensorValues[i];
        */
    }

  }

  fusion/=4;
  //fusion/=8;

  return fusion;
}
