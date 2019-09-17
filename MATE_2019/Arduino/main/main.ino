#include <LSM9DS1_Registers.h>
#include <LSM9DS1_Types.h>
#include <Wire.h>
#include <SPI.h>
#include <SparkFunLSM9DS1.h>

#include <Servo.h>
#include "pins.h"

#define LSM9DS1_M  0x1E // Would be 0x1C if SDO_M is LOW
#define LSM9DS1_AG  0x6B // Would be 0x6A if SDO_AG is LOW

LSM9DS1 imu;

Servo FR;
Servo FL;
Servo BL;
Servo BR;

Servo UL;
Servo UR;
Servo UB;

const int claw = 12;

const int COMMAND_SIZE = 37;
const int MOTOR_NEUTRAL = 1500;

int timer = 0;

void setup() {
  // put your setup code here, to run once:
  Serial2.begin(115200);
  Serial2.setTimeout(80);

  TCCR0A=(1<<WGM01);    //Set the CTC mode   
  OCR0A=0xF9; //Value for ORC0A for 1ms 
  
  TIMSK0|=(1<<OCIE0A);   //Set the interrupt request
  sei(); //Enable interrupt
  
  TCCR0B|=(1<<CS01);    //Set the prescale 1/64 clock
  TCCR0B|=(1<<CS00);
  
  imu.settings.device.commInterface = IMU_MODE_I2C;
  imu.settings.device.mAddress = LSM9DS1_M;
  imu.settings.device.agAddress = LSM9DS1_AG;
  
  FR.attach(2);
  FL.attach(3);
  BL.attach(4);
  BR.attach(5);

  UL.attach(6);
  UR.attach(7);
  UB.attach(8);

  pinMode(claw, OUTPUT);

  FR.writeMicroseconds(MOTOR_NEUTRAL);
  FL.writeMicroseconds(MOTOR_NEUTRAL);
  BL.writeMicroseconds(MOTOR_NEUTRAL);
  BR.writeMicroseconds(MOTOR_NEUTRAL);

  UL.writeMicroseconds(MOTOR_NEUTRAL);
  UR.writeMicroseconds(MOTOR_NEUTRAL);
  UB.writeMicroseconds(MOTOR_NEUTRAL);

  if (!imu.begin())
  {
    while (1);
  }
}

void loop() {
  char driveCommands[COMMAND_SIZE];
  
  if (imu.accelAvailable())
  {
    // Updates ax, ay, and az
    imu.readAccel();
  }

  if (imu.gyroAvailable())
  {
    // Updates gx, gy, and gz
    imu.readGyro();
  }

  float ax = imu.ax;
  float ay = imu.ay;
  float az = imu.az;
  
  float roll = atan2(ay, az) * 180 / PI;
  float pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180 / PI;
  
  String IMUString = ":";
  IMUString = IMUString + (int) pitch + ";";
  IMUString = IMUString + (int) roll + "|";

  // Wait untill there is at least 1 full command to read
  if (Serial2.available() >= COMMAND_SIZE-1)
  {
    timer = 0; // Reset timer if data received
    // Don't read a string that starts in the middle of a command
    if (Serial2.read() == ':')
    {
      // Only send data back if data was received
      writeString(IMUString);
      
      String info = Serial2.readStringUntil('\n');
      info.toCharArray(driveCommands, COMMAND_SIZE-1);
      drive(driveCommands);

      // Clear any backlog commands
      Serial2.flush();
    }
    else
    {
      // Clear invalid command
      Serial2.readStringUntil('\n');
    }
  }
}

// Used to serially push out a String with Serial.write()
void writeString(String stringData) {
  for (int i = 0; i < stringData.length(); i++)
  {
    Serial2.write(stringData[i]);   // Push each char 1 by 1 on each loop pass
  }
}

void drive(char array[])
{
  char *commands[35];
  char *ptr = NULL;
  byte index = 0;
  ptr = strtok(array, ";");
  while(ptr != NULL)
  {
      commands[index] = ptr;
      index++;
      ptr = strtok(NULL, ";");
  }

  // Only run if a command has been received within one second
  if (timer < 1000)
  {
    writeCommands(atoi(commands[0]), atoi(commands[3]), atoi(commands[2]), atoi(commands[1]), 
                  atoi(commands[4]), atoi(commands[5]), atoi(commands[6]), atoi(commands[7]));
  }
  else
  {
    writeCommands(MOTOR_NEUTRAL, MOTOR_NEUTRAL, MOTOR_NEUTRAL, MOTOR_NEUTRAL,
                  MOTOR_NEUTRAL, MOTOR_NEUTRAL, MOTOR_NEUTRAL, atoi(commands[7]));
  }
}

void writeCommands(int FR, int FL, int BL, int BR, int UL, int UR, int UB, int c)
{
  setFR(FR);
  setFL(FL);
  setBL(BL);
  setBR(BR);
  
  setUL(UL);
  setUR(UR);
  setUB(UB);

  digitalWrite(claw, c);
}

ISR(TIMER0_COMPA_vect) // This is the interrupt request
{
  timer++;
}

void setFR(int num)
{
  FR.writeMicroseconds(num);
}

void setFL(int num)
{
  FL.writeMicroseconds(num);
}

void setBL(int num)
{
  BL.writeMicroseconds(num);
}

void setBR(int num)
{
  BR.writeMicroseconds(num);
}

void setUL(int num)
{
  UL.writeMicroseconds(num);
}

void setUR(int num)
{
  UR.writeMicroseconds(num);
}

void setUB(int num)
{
  UB.writeMicroseconds(num);
}
