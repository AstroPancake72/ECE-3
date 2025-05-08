#include <ECE3.h>

uint16_t sensorValues[8];

void setup()
{
  ECE3_Init();
  Serial.begin(9600); // set the data rate in bits per second for serial data transmission
  delay(2000);
}


void loop()
{
  // read raw sensor values
  ECE3_read_IR(sensorValues);

  print the sensor values as numbers from 0 to 2500, where 0 means maximum reflectance and
  2500 means minimum reflectance
  for (unsigned char i = 0; i < 8; i++)
  {
    Serial.print(sensorValues[i]);
    Serial.print('\t'); // tab to format the raw data into columns in the Serial monitor
  }
  Serial.println();

  // delay(50);
}

int getIRFusion()
{
  double sensorValues[8];
  int16_t minValues[8] = [596,	527,	619,	619,	505,	685,	643,	713];
  int16_t maxValues[8] = [1516,	1395,	1730,	1208,	1203,	1616,	1492,	1787];
  ECE3_read_IR(sensorValues);

  double fusion = 0;

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
        fusion += 1*sensorValues[i];
        break;
      case 5:
        fusion += 2*sensorValues[i];
        break;
      case 6:
        fusion += 4*sensorValues[i];
        break;
      case 7:
        fusion += 8*sensorValues[i];

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
