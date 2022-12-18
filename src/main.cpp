#include <PS5BT.h>
#include <usbhub.h>


// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>
#include <ros.h>
#include <geometry_msgs/Point.h>

ros::NodeHandle  nh;
geometry_msgs::Point mecanum_msg;
geometry_msgs::Point intake_msg;
ros::Publisher mecanum_publisher("mecanum_fromArduino", &mecanum_msg);
ros::Publisher intake_publisher("intake_fromArduino", &intake_msg);

USB Usb;
//USBHub Hub1(&Usb); // Some dongles have a hub inside
BTD Btd(&Usb); // You have to create the Bluetooth Dongle instance like so

/* You can create the instance of the PS5BT class in two ways */
// This will start an inquiry and then pair with the PS5 controller - you only have to do this once
// You will need to hold down the PS and Share button at the same time, the PS5 controller will then start to blink rapidly indicating that it is in pairing mode
// PS5BT PS5(&Btd, PAIR);

// After that you can simply create the instance like so and then press the PS button on the device
PS5BT PS5(&Btd);

bool printAngle = false, printTouch = false;
uint16_t lastMessageCounter = -1;
uint8_t player_led_mask = 0;
bool microphone_led = false;
uint32_t ps_timer;
// car moving
double x,y,z;
double speed_ratio=20;
double radius_ratio=1500;

// intake moving
double tilt=0;
double stretch = 0;
double suck = 0;

void setup(){
#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  if (Usb.Init() == -1) {
    Serial.print(F("\r\nOSC did not start"));
    while (1); // Halt
  }
  Serial.print(F("\r\nPS5 Bluetooth Library Started"));

 nh.initNode();
 nh.advertise(mecanum_publisher);
 nh.advertise(intake_publisher);
 Serial.begin(57600);
}



void loop() {
  Usb.Task();

  if (PS5.connected() && lastMessageCounter != PS5.getMessageCounter()) {
    lastMessageCounter = PS5.getMessageCounter();
    
    /* move */
    if (PS5.getAnalogHat(LeftHatX) > 137 || PS5.getAnalogHat(LeftHatX) < 117 ) {
      x=(PS5.getAnalogHat(LeftHatX)-127)/speed_ratio;
      Serial.print(x);
    }else{
      x=0;
    }
    if (PS5.getAnalogHat(LeftHatY) > 137 || PS5.getAnalogHat(LeftHatY) < 117 ) {
      y=-(PS5.getAnalogHat(LeftHatY)-127)/speed_ratio;
      Serial.print(y);
    }else{
      y=0;
    }

    /* omega */
    if (PS5.getAnalogButton(L2) || PS5.getAnalogButton(R2)) { // These are the only analog buttons on the PS5 controller
      z=(PS5.getAnalogButton(L2)-PS5.getAnalogButton(R2))/radius_ratio;
      Serial.println(z);
    }else{
      z=0;
    }

    /* tilt */
    if (PS5.getButtonPress(UP)) {
      Serial.print(F("\r\ntilt_front"));
      tilt = 1;
    }
    else if(PS5.getButtonPress(DOWN)){
      Serial.print(F("\r\ntilt_behind"));
      tilt = -1;
    } 
    else{
      Serial.print(F("\r\ntilt_none"));
      tilt = 0;      
    }

    /* stretch */
    if (PS5.getAnalogHat(RightHatY) > 137) {
      stretch = 1;
    }
    else if(PS5.getAnalogHat(RightHatY) < 117){
      stretch = -1;
    }
    else{
      stretch = 0;
    }

    /* suck */
    if (PS5.getButtonClick(TRIANGLE)) {
      Serial.print(F("\r\nTriangle"));
      suck = 1 ;
    }
    if (PS5.getButtonClick(CIRCLE)) {
      Serial.print(F("\r\nCircle"));
      suck = 0 ;
    }
    if (PS5.getButtonClick(CROSS)) {
      Serial.print(F("\r\nCross"));
      suck = -1;
    }
    if (PS5.getButtonClick(SQUARE)) {
      Serial.print(F("\r\nSQUARE"));
      suck = 0;
    }

    //tilt

    // //stretch
    // double stretch_before = 0;
    // int stretch_open =0 ;
    // timer_escape
    // stretch = stretch;
    // if(stretch_before != stretch){
    //   if(stretch-stretch_before >0){
    //     stretch_open=1;
    //   }
    //   else(stretch-stretch_before <0){
    //     stretch_open=-1;
    //   }
    //   stretch_before = stretch;
    // }
    // else{
    //   stretch_open =0;
    // }
    // if(stretch_open==1){
    //   high;
    //   low;
    // }
    // else if(stretch_open==-1){
    //   low;
    //   high;
    // }
    // else if(stretch_open==0){
    //   low;
    //   low;
    // }
    

    // // suck
    // if(suck==1){
    //   high;
    //   low;
    // }
    // else if(suck==-1){
    //   low;
    //   high;
    // }
    // else if(suck==1){
    //   low;
    //   low;
    // }


    // Hold the PS button for 1 second to disconnect the controller
    // This prevents the controller from disconnecting when it is reconnected,
    // as the PS button is sent when it reconnects
    if (PS5.getButtonPress(PS)) {
      if (millis() - ps_timer > 1000)
        PS5.disconnect();
    } else
      ps_timer = millis();

    if (PS5.getButtonClick(PS))
      Serial.print(F("\r\nPS"));

    // if (PS5.getButtonClick(TRIANGLE)) {
    //   Serial.print(F("\r\nTriangle"));
    //   stretch = 1 ;
    // }
    // if (PS5.getButtonClick(CIRCLE)) {
    //   Serial.print(F("\r\nCircle"));
    //   tilt = tilt + 1 ;
    // }
    // if (PS5.getButtonClick(CROSS)) {
    //   Serial.print(F("\r\nCross"));
    //   Serial.print(F("\r\nstretch"));
    //   stretch = stretch - 1 ;
    // }
    // if (PS5.getButtonClick(SQUARE)) {
    //   Serial.print(F("\r\nSQUARE"));
    //   tilt = tilt - 1 ;
    // }
    // if (PS5.getButtonClick(UP)) {
    //   Serial.print(F("\r\nUp"));
    //   suck = 1;
    // } 
    // if (PS5.getButtonClick(RIGHT)) {
    //   Serial.print(F("\r\nRight"));
    //   suck = 0;
    // } 
    // if (PS5.getButtonClick(DOWN)) {
    //   Serial.print(F("\r\nDown"));
    //   suck = -1;
    // } 
    // if (PS5.getButtonClick(LEFT)) {
    //   Serial.print(F("\r\nLeft"));
    //   suck = 0;
    // }

    if (PS5.getButtonClick(L1))
      Serial.print(F("\r\nL1"));
    if (PS5.getButtonClick(L3))
      Serial.print(F("\r\nL3"));
    if (PS5.getButtonClick(R1))
      Serial.print(F("\r\nR1"));
    if (PS5.getButtonClick(R3))
      Serial.print(F("\r\nR3"));

    if (PS5.getButtonClick(CREATE))
      Serial.print(F("\r\nCreate"));
    if (PS5.getButtonClick(OPTIONS)) {
      Serial.print(F("\r\nOptions"));
      printAngle = !printAngle;
    }
    if (PS5.getButtonClick(TOUCHPAD)) {
      Serial.print(F("\r\nTouchpad"));
      printTouch = !printTouch;
    }
    if (PS5.getButtonClick(MICROPHONE)) {
      Serial.print(F("\r\nMicrophone"));
      microphone_led = !microphone_led;
      PS5.setMicLed(microphone_led);
    }

    if (printAngle) { // Print angle calculated using the accelerometer only
      Serial.print(F("\r\nPitch: "));
      Serial.print(PS5.getAngle(Pitch));
      Serial.print(F("\tRoll: "));
      Serial.print(PS5.getAngle(Roll));
    }


  }

    mecanum_msg.x=x;
    mecanum_msg.y=y;
    mecanum_msg.z=z;

    intake_msg.x=tilt;
    intake_msg.y=stretch;
    intake_msg.z=suck;

    nh.spinOnce();
    mecanum_publisher.publish(&mecanum_msg);
    intake_publisher.publish(&intake_msg);
    delay(20);
}