#include <ECE3.h>

uint16_t sensorValues[8];
int16_t minValues[8] = { 596, 527, 619, 619, 505, 685, 643, 713 };
int16_t maxValues[8] = { 1516, 1395, 1730, 1208, 1203, 1616, 1492, 1787 };

const int left_nslp_pin = 31;  // nslp ==> awake & ready for PWM
const int left_dir_pin = 29;
const int right_dir_pin = 30;
const int left_pwm_pin = 40;
const int right_pwm_pin = 39;
const int right_nslp_pin = 11;

uint16_t inside;
uint16_t outside1;
uint16_t outside2;

int cur;
int pre;
bool phantom;

bool halfway;

void setup() {
  ECE3_Init();
  pinMode(left_nslp_pin, OUTPUT);
  pinMode(left_dir_pin, OUTPUT);
  pinMode(left_pwm_pin, OUTPUT);
  pinMode(right_nslp_pin, OUTPUT);
  pinMode(right_dir_pin, OUTPUT);
  pinMode(right_pwm_pin, OUTPUT);

  digitalWrite(left_dir_pin, LOW);
  digitalWrite(left_nslp_pin, HIGH);
  digitalWrite(right_dir_pin, LOW);
  digitalWrite(right_nslp_pin, HIGH);
  cur = getIRFusion();
  pre = cur;
  halfway = false;
  phantom = false;
  Serial.begin(9600);  // set the data rate in bits per second for serial data transmission
  delay(2000);
}


void loop() {
  ECE3_read_IR(sensorValues);
  int spd = 35;

  bool straight = false;

  //checks for inside to go straight
  inside = sensorValues[3] + sensorValues[4];
  // inside /= 2;
  inside += 300;

  uint16_t right = sensorValues[0] + sensorValues[1] + sensorValues[2];
  uint16_t left = sensorValues[5] + sensorValues[6] + sensorValues[7];

  // if (inside > sensorValues[0] && inside > sensorValues[1] && inside > sensorValues[2] && inside > sensorValues[5] && inside > sensorValues[6] && inside > sensorValues[7]) {
  //   straight = true;
  // }

  if (inside > right + left)
  {
    straight = true;
  }

  // checks for split or arch

  if (!straight && !halfway && left >= 2200 && right >= 2200 && (right - 400) <= left) {
    sensorValues[0] = 0;
    sensorValues[1] = 0;
  } else if (!straight && halfway && right >= 2200 && left >= 2200 && (left - 400) <= right) {
    sensorValues[6] = 0;
    sensorValues[7] = 0;
  }

  cur = getIRFusion();

  int pd = pid(pre, cur);

  if (straight) {
    pd = 0;
  }


  //checks if at full black line
  bool all = false;
  int sensor_sum = 0;
  for (int i = 0; i < 8; i++) {
    sensor_sum+=sensorValues[i];
  }

  if (sensor_sum < 7500) {
      phantom = false;
  }
  else {
    if (phantom)
    {
      all = true;
      halfway = true;
    }
    phantom = true;
  }

  if (all) {
    pd = 0;
  }

  //checks if lost track
  bool none = false;
  for (int i = 0; i < 8; i++) {
    if (sensorValues[i] > 600) {
      break;
    }
    if (i == 7) {
      none = true;
    }
  }

  if (none) {
    pd = -30;
  }

  //left spd
  int leftSpd = (spd - pd);
  if (leftSpd < 0) {
    digitalWrite(left_dir_pin, HIGH);
    leftSpd *= -1;
    if (leftSpd > 45) {
      leftSpd = 45;
    }
  } else {
    digitalWrite(left_dir_pin, LOW);
  }

  if (leftSpd > 60) {
    leftSpd = 60;
  }


  //right spd
  int rightSpd = (spd + pd);
  if (rightSpd < 0) {
    digitalWrite(right_dir_pin, HIGH);
    rightSpd *= -1;
    if (rightSpd > 45) {
      rightSpd = 45;
    }
  } else {
    digitalWrite(right_dir_pin, LOW);
  }

  if (rightSpd > 60) {
    rightSpd = 60;
  }

  if (phantom && !all)
  {
    leftSpd = 0;
    rightSpd = 0;
  }

  if (all) {
    digitalWrite(left_dir_pin, LOW);
    digitalWrite(right_dir_pin, HIGH);
    leftSpd = 80;
    rightSpd = 80;
  }


  analogWrite(left_pwm_pin, leftSpd);
  analogWrite(right_pwm_pin, rightSpd);

  if (all) {
    delay(1000);
  }

  //stuff for kd
  pre = cur;

  // print the sensor values as numbers from 0 to 2500, where 0 means maximum reflectance and
  // 2500 means minimum reflectance
  // for (unsigned char i = 0; i < 8; i++)
  // {
  //   Serial.print(sensorValues[i]);
  //   Serial.print('\t'); // tab to format the raw data into columns in the Serial monitor
  // }
  // Serial.println();


  // delay(50);
}

int getIRFusion() {

  double fusion = 0;

  for (unsigned int i = 0; i < 8; i++) {
    if (sensorValues[i] < minValues[i]) {
      minValues[i] = sensorValues[i];
    }

    sensorValues[i] -= minValues[i];

    if (sensorValues[i] > maxValues[i]) {
      maxValues[i] = sensorValues[i];
    }

    sensorValues[i] = (1000 * sensorValues[i] / maxValues[i]);
    switch (i) {
      case 0:
        fusion -= 8 * sensorValues[i];
        break;
      case 1:
        fusion -= 4 * sensorValues[i];
        break;
      case 2:
        fusion -= 2 * sensorValues[i];
        break;
      case 3:
        fusion -= 1 * sensorValues[i];
        break;
      case 4:
        fusion += 1 * sensorValues[i];
        break;
      case 5:
        fusion += 2 * sensorValues[i];
        break;
      case 6:
        fusion += 4 * sensorValues[i];
        break;
      case 7:
        fusion += 8 * sensorValues[i];

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

  fusion /= 4;
  //fusion/=8;

  return fusion;
}

int pid(int prev, int cur) {
  double kp = 0.1;
  double kd = 2;

  return (int)(kp * cur + (cur - prev) * kd);
}
