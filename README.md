# Arduino_Pinball
 ECE180 Final Project: Arduino Pinball Machine. 
 Author Dustin Kirkpatrick

 This started as my ECE180 final project, however I intend to continue working and developing the code as I build and add to the pinball machine.
 Some things I would like to impliment:

  * Class defined objects, things like Flippers, Targets, TargetGroups, and similar modular devices that benefit from having data like coodown timers, 
    flags, or other information that could be encapsulated for the sake of keeping the game operation code clean. Once I am more familiar with C++'s class definition and structure I will be
    working on converting the methods and associated variables into class definitions and constructors for easier use. 

  * Pop-Bumper, this will be fairly similar in code to the catapult.

  * Ball Launch, or Kicker Similar to Pop-Bumpers and Sling-shots

  * Servo controlled path redirector, The plan for this on my personal machine will be for a trap door to open on a ramp, capturing the ball behind the drop target. 

 Known Issues:

  * Circuit -> interuptPin -> triggerMux() -> drainBall() -> int ballsInPlay
    When a ball drains and hits the switch while there is more than one ball in play, it "drains" all balls. The code in question reffers to the int "ballsInPlay", the switch is set up such that the pinball falls on it, this causes the lever to bounce 
    up and down breifly. The switch is connected to a 1 X 8:3 16DIP encoder and is the highest priority input. The encoder I am using is TEXAS INSTRUMENTS / SN74LS148N Part# 296-3651-5-ND.
    The encoder is connected with the GS out to an interupt pin, with A0, A1, and A2 connected to non-interupt pins. The encoder is handled by the TriggerMux section of the code. When the drain is triggered, the method
    drainBall() is run. drainBall checks to see if ballsInPlay > 0 to ensure there is not a negative value. It then decrements ballsInPlay. Than checks to see if the game is over. (This might move in the future)

    - I have tried adding a cooldown timer arguement so that the switch "bouncing" and activating multiple times doesn't cause the ballsInPlay to be decremented more than once
      This partially works, however 9 times out of 10 it doesn't work as intended. 

    - I have tried manually pushing and holding the switch to ensure physical bouncing is not at fault. This resulted in the same problem, I plan to digitally short the active low pin on the encoder to guarentee it
      is not the switch. 
      