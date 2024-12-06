#include <Servo.h>
//Mux Pin Out
  //Program dependant LEDs
  #define FlipperButtonLED MUX[0]
  #define StartButtonLED MUX[1] 
  //  MUX[2] Sling Shot LED
  //  MUX[3]
  //  MUX[4]
  //  MUX[5]
  //  MUX[6]
  //  MUX[7]
  //  MUX[8]
  //  MUX[9]
  //  MUX[10]
  //  Simple operation LEDS, these could utilize turnary operators to determine whether they are active. This can be combined with loopTime modulo period time > 
  //  MUX[11] 3 Balls left 
  //  MUX[12] 2 Balls left 
  //  MUX[13] Last Ball      //Add flashing May be linked to other LEDs across the playfield
  //  MUX[14] Press Start 
  //  MUX[15]  
/* #### PinOUT ####
  Pin Configurations preset for Arduino Mega
  1 
  2 Interupt Right Button Interupt  
  3 Interupt 
  4 PWM Upper Ball Drop Servo
  5 PWM Lower Ball Drop Servo
  6 PWM Drop Down Target Servo 
  7
  8
  9  PWM servo Ball Launch
  10 PWM servo Ball Launch
  11 PWM  SlingShot (PWM is optional and should only be considered to reduce the launch power of the Slingshot)
  12 PWM  Right Flipper 
  13  (Warning, this will go HIGH when the Arduino reboots)
  ...
  18 Interupt 
  19 Interupt 
  20 Interupt Slingshot Triggers
  21 Interupt Trigger Mux
      0 LOWEST PRIORITY         H H H
      1 
      2
      3
      4
      5
      6
      7 HIGHEST PRIORITY Drain  L L L 
  ...
  23  Trigger Mux LSB   Active Low!!!
  25  Trigger Mux Bit 2 Active Low!!!
  27  Trigger Mux MSB   Active Low!!!
  ...
  48  mux LSB
  49  Start Button
  50  mux 4s
  51  mux 2s
  52  mux MSB
  53  Mux Enable Pin
*/
//3-target
  bool target1 = false;
  bool target2 = false;
  bool target3 = false;
  bool targetBlink = false;

//Drain
  uint8_t triggerMux = 21;  //Interupt      Pin
  uint8_t encoderA0 = 22;
  uint8_t encoderA1 = 24;
  uint8_t encoderA2 = 26;
  uint8_t lastTriggerAddr; //keeps track of the last trigger to flip
  long ballDrainCD = 0;

//Flipper
  uint8_t R_Button = 3; //Interupt            Pin
  uint8_t R_FLipper = 12; //NEEDS PWM         Pin
  long flipperOnTime = 0;
  bool flipperActive = false;

//SlingShot
  uint8_t slingShot = 11; //                  Pin
  uint8_t slingShotTrigger = 20; //Interupt    Pin
  long slingShotOnTime = 0;
//MultiPlexer for LEDs
  uint8_t muxEnPin = 53;//                    Pin
  uint8_t mux1s = 48;//                       Pin
  uint8_t mux2s = 51;//                       Pin
  uint8_t mux4s = 50;//                       Pin
  uint8_t mux8s = 52;//                       Pin
  uint8_t muxIndex;  // This is the loop count
  bool MUX[15];      //These represent the various LEDs or Switches on the Multiplexer

//Drop Target Set Up  !!!! TODO !!!!
  Servo drop;
  int targetHeight;
  bool calibrated;
  bool dropDisabled;
  uint8_t dropDownServo = 6; //NEEDS PWM     Pin
  uint8_t dropDownTrigger = 35;//            Pin

//Ball Launch Mechanism !!!! TODO !!!!
  Servo ballLaunch1;
  Servo ballLaunch2;
  uint8_t BL_1servo = 9;  //NEEDS PWM        Pin
  uint8_t BL_2servo = 10;  //NEEDS PWM        Pin
  bool launchBall, dropBall, delayStep, ballStep; 
      //Launch Ball is the over all container the others are steps
  long ballLaunchTimer;
  int ballsLoaded;
  int ballQueue = 0;

//Game Utility Items
  uint8_t S_Button = 49;    //               Pin
  int ballsInPlay = 0;
  int ballsLeft;
  bool gameOver;
  unsigned long loopTime; //The start of the loop
  long buttonTimer;
void setup() {
  gameOver = true;
  ballsLeft = 0;
  ballsInPlay = 0;
  ballsLoaded = 5;

  //####  ####  ####  Flipper Setup ####  ####  ####
    pinMode(R_FLipper, OUTPUT);
    pinMode(R_Button, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(R_Button),buttonPress_R, CHANGE);
  //####  ####  ####  LSingShot Setup ####  ####  ####
    pinMode(slingShotTrigger, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(slingShotTrigger), slingShotFire, FALLING);
    pinMode(slingShot, OUTPUT); 

  //####  ####  ####  Triggers Setup ####  ####  ####
    pinMode(triggerMux, INPUT_PULLUP);
    pinMode(encoderA0, INPUT_PULLUP);
    pinMode(encoderA1, INPUT_PULLUP);
    pinMode(encoderA2, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(triggerMux), processTriggerMux , FALLING);

  //####  ####  ####  Multiplexer Setup ####  ####  ####
    pinMode(mux1s,OUTPUT);
    pinMode(mux2s,OUTPUT);
    pinMode(mux4s,OUTPUT);
    pinMode(mux8s,OUTPUT);
    pinMode(muxEnPin,OUTPUT);
    muxIndex = 0;
    for(int i = 0; i<16; i++){
      MUX[i] = false;
    }
  //####  ####  ####  Ball Launch Setup ####  ####  ####
    ballLaunch1.attach(BL_1servo);
    ballLaunch2.attach(BL_2servo);
    pinMode(S_Button,INPUT_PULLUP);
    launchBall = false;
    ballLaunchTimer = 0;
  //####  ####  ####  Drop Down Target Setup ####  ####  ####
    pinMode(dropDownTrigger, INPUT);
    drop.attach(dropDownServo);
    drop.write(0);
    calibrate();
    //loopCounter = 0;
}
//####  ####  ####  ####  ####  ####

//####  ####  ####  Game Loops  ####  ####  ####
  void loop() {
    loopTime = millis();
    timeChecks();
    if(!gameOver){

      if(ballsInPlay==0){
        StartButtonLED = true;
        FlipperButtonLED = false;
        if(!digitalRead(S_Button)){
          FlipperButtonLED = true;
          launchABall();
        }
      } else {
        StartButtonLED = false;
        //Main Game here!




      }
    } else {
      FlipperButtonLED = false;
      StartButtonLED = true;
      if(!digitalRead(S_Button)){
        FlipperButtonLED = true;
        ballsLeft = 3;
        ballsLoaded = 5;
        ballsInPlay = 0;
        gameOver = false;
        launchABall();
      }
    }

    //loopCounter++;
    //loopCounter = loopCounter%3000; 
    delay(1);
  }

  void endGame(){
    gameOver = true;
    resetTargets();
  }

//####  ####  ####  Timing Functions ####  ####  ####
  void timeChecks(){
    flipperCheck();
    muxLoop();
    launchBallOperation();
    checkSlingShot();
    targetTasks();
    //if(!digitalRead(S_Button) && waitFlag(3000, &buttonTimer)){
    //  endGame();
    //}
  }

  bool coolDown(int period,unsigned long *timer){
    if(*timer == 0){
      *timer = loopTime;
      return true;
    }
    if(loopTime >= *timer + period){
      *timer = 0;
      return true;
    } else {
      return false;
    }
  }
  bool waitFlag(int period,unsigned long *timer){
    if(*timer == 0){
      *timer = loopTime;
    }
    if(loopTime >= *timer + period){
      *timer = 0;
      return true;
    } else {
      return false;
    }
  }
//####  ####  ####  Sling Shot Fire   ####  ####  ####  
  void slingShotFire(){
    slingShotOnTime = loopTime;
    MUX[3] = true;
    analogWrite(slingShot, 255);
  }
  void checkSlingShot(){
    if(slingShotOnTime > 0){
      if(waitFlag(300, &slingShotOnTime)){
        digitalWrite(slingShot, LOW);
        MUX[3] = false;
      }
    }
  }

//####  ####  ####  Flipper Button Control  ####  ####  ####  
  void buttonPress_R(){
    if(!digitalRead(R_Button) && FlipperButtonLED){
      flipperActive = true;
      flipperOnTime = loopTime;
      analogWrite(R_FLipper, 255);
    } else {
      flipperActive = false;
      flipperOnTime = 0;
      analogWrite(R_FLipper, 0);
    }
  }
  void flipperCheck(){
    if(flipperActive){
      if(digitalRead(R_Button) || !FlipperButtonLED){
        digitalWrite(R_FLipper, LOW);
        flipperOnTime = 0;
        flipperActive = false;
      } else if(waitFlag(400, &flipperOnTime)){
        analogWrite(R_FLipper, 55);
      }
    }
  }

//####  ####  ####  LED Mux Function ####  ####  ####
  //The mux[] in this function should be replaced by flags or arguements relivant to the output
  //9-12V power supply reccomended
  void muxLoop(){
  //Mux Loop component
  //RESET and Addr
    digitalWrite(muxEnPin,LOW);
    digitalWrite(mux1s,(muxIndex%2 > 0)?HIGH:LOW);
    digitalWrite(mux2s,((muxIndex/2)%2 > 0)?HIGH:LOW);
    digitalWrite(mux4s,((muxIndex/4)%2 > 0)?HIGH:LOW);
    digitalWrite(mux8s,((muxIndex/8)%2 > 0)?HIGH:LOW);
  //Enable OUT ( MUX[#] can be replaces by bool flags representing the values )
  /*
  *   Time relative events like blinking can be accomplished by taking the mod of loop time
  *     -  && (loopTime%1000)<500
  *    Where 1000 is a period of 1000ms, and 500 being the threshold at which the light toggles. 
  *    This allows a 50% on 50% off relationship with a 1 second long period, and half a second interval
  */
    switch(muxIndex){
      case 0: digitalWrite(muxEnPin,FlipperButtonLED?HIGH:LOW);
      break;
      case 1: digitalWrite(muxEnPin,(StartButtonLED && ((loopTime%2000)<1000))?HIGH:LOW);
      break;
      case 2: digitalWrite(muxEnPin,MUX[2]?HIGH:LOW); 
      break;
      case 3: digitalWrite(muxEnPin,MUX[3]?HIGH:LOW); //SLING SHOT LIGHT
      break;
      case 4: digitalWrite(muxEnPin,MUX[4]?HIGH:LOW);
      break;
      case 5: digitalWrite(muxEnPin,ballsInPlay == 0?HIGH:LOW); //MUX[5]
      break;
      case 6: digitalWrite(muxEnPin,ballsInPlay == 1?HIGH:LOW);
      break;
      case 7: digitalWrite(muxEnPin,ballsInPlay == 2?HIGH:LOW);
      break;
      case 8: digitalWrite(muxEnPin,ballsInPlay == 3?HIGH:LOW);
      break;
      case 9: digitalWrite(muxEnPin,((targetBlink && ((loopTime%1000)<500)) || target3)?HIGH:LOW);
      break;
      case 10: digitalWrite(muxEnPin,((targetBlink && ((loopTime%1000)<500)) || target2)?HIGH:LOW);
      break;
      case 11: digitalWrite(muxEnPin,((targetBlink && ((loopTime%1000)<500)) || target1)?HIGH:LOW);
      break;
      case 12: digitalWrite(muxEnPin,(ballsLeft == 2)?HIGH:LOW);                        // 3 Balls Left
      break;
      case 13: digitalWrite(muxEnPin,(ballsLeft == 3)?HIGH:LOW);                        // 2 Balls Left
      break;
      case 14: digitalWrite(muxEnPin,(ballsInPlay==0) && ((loopTime%2000)<1000)?HIGH:LOW); // Last Ball!   
      break;
      case 15: digitalWrite(muxEnPin,(ballsLeft == 1 && (loopTime%1000)>500)?HIGH:LOW);           //Press Start
      break;
    }
    //Loops the index per loop
    muxIndex++;
    muxIndex = muxIndex%16;
  }

//####  ####  ####  Trigger MUX ####  ####  ####
  long lastTriggerTime = 0;
  int triggerAddr;
  
  void processTriggerMux(){
    //Nessicary for encoder to display correct data Encoder used has a data delay of 15 nano seconds. Consider using no operation assembly instead, once I have read more on it
    delay(1); 
    triggerAddr = (digitalRead(encoderA2)?0:4)+(digitalRead(encoderA1)?0:2)+(digitalRead(encoderA0)?0:1); // Mux sensor Bits to Int - might be a little slow
    switch(triggerAddr){
      case 0: //CASE 0 IS SHORTED ATM
      break;
      case 1: 
      targetHit(1); //Bump target
      break;
      case 2: 
      targetHit(2); //Bump target
      break;
      case 3: 
      targetHit(3); //Bump target
      break;
      case 4: 
      break;
      case 5: 
      break;
      case 6: 
      break;
      case 7: 
      ballDrained();  //Drain trigger
      break;
    }
  } 

//####  ####  3 Target  ####  ####  ####  ####
  long targetTimer;

  void targetHit(int targetLoc){
    switch(targetLoc){
      case 1: 
      target1 = true;//!target1;
      break;
      case 2: 
      target2 = true;//!target2;
      break;
      case 3: 
      target3 = true;//!target3;
      break;
    }
  }

  void resetTargets(){
    target1 = false;
    target2 = false;
    target3 = false;
    targetBlink = false;
  }
  //Main gameplay function
  void targetTasks(){
    if(waitFlag(20000,&targetTimer) && targetBlink){
      resetTargets();
    } //else if(target1 && target2 && target3 && targetBlink){
      //resetTargets();
      //launchMultiBall(1);
      //} 
    if(target1 && target2 && target3 && !targetBlink){
      target1 = false;
      target2 = false;
      target3 = false;
      targetBlink = true;
      launchMultiBall(1);
    }
  }
//####  ####  ####  Launch Ball Mech ####  ####  ####

  void ballDrained() {
    if(ballsInPlay>0 && coolDown(1000, &ballDrainCD)){
      ballsInPlay = ballsInPlay - 1;
      if(ballsInPlay<=0){
        //ballsInPlay = 0;
        ballsLeft--;
      }
    }
  }

  void launchMultiBall(int quantity){
    ballQueue += quantity;
    //if(quantity <= (ballsLoaded - ballsLeft)){
      //ballQueue = ballQueue + quantity;
    //}
  }

  void launchABall(){ //Launches a ball, if the process is active, it will queue up a ball
    if(ballsLoaded >= ballsLeft){
      ballsLoaded--;
      ballsInPlay++;
      launchBall = true;
      dropBall = true;
    }
  }

  void launchBallOperation(){ //provides the operation to launch the ball. This will be ran each cycle. It will also check to see if the queue has a waiting ball
    if(launchBall){
      if(dropBall){
        ballLaunch1.write(85);
        ballLaunch2.write(160);
        if(waitFlag(200, &ballLaunchTimer)){
          dropBall = false;
          delayStep = true;
        }
      } else if(delayStep){
        ballLaunch1.write(85);
        ballLaunch2.write(90);
        if(waitFlag(500, &ballLaunchTimer)){
          delayStep = false;
          ballStep = true;
        }
      }else if(ballStep){
        ballLaunch1.write(150);
        ballLaunch2.write(90);
        if(waitFlag(200, &ballLaunchTimer)){
          ballStep = false;
          launchBall = false;
        }
      }
    } else {        
      ballLaunch1.write(85);
      ballLaunch2.write(90);
      if(ballQueue > 0 && !launchBall){        //If there is a ball in the queue, launch it
        ballQueue--;
        launchABall();
      }
    }
    if(ballsLeft<=0){
      endGame();
    }
  }

//####  ####  ####  Drop Down Target Functions ####  ####  ####
  //  COMPLETE!!! Utilize popUp()

  void popUp(){
    if(!dropDisabled){
      drop.write(targetHeight);
      drop.write(0);
    }
  }
  void calibrate(){
    int angle = 0;
    do{
      drop.write(angle);
      delay(30);          //TODO remove delay and utilize millis() and timeChecks() to incorporate a falisafe mid game
      angle++;
    } while(digitalRead(dropDownTrigger));
    targetHeight = angle;
    calibrated = angle>10?true:false;
    drop.write(0);
  }
  void dropEnable(){
      dropDisabled = false;
      drop.write(0);
    }
  void dropDisable(){
    dropDisabled = true;
    drop.write(targetHeight);
  }