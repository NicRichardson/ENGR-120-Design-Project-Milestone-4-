#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, in1,    IRsensorM,      sensorReflection)
#pragma config(Sensor, in2,    IRsensorR,      sensorNone)
#pragma config(Sensor, in3,    IRsensorL,      sensorNone)
#pragma config(Sensor, dgtl1,  Button_R,       sensorTouch)
#pragma config(Sensor, dgtl2,  start_button,   sensorTouch)
#pragma config(Sensor, dgtl3,  USS,            sensorSONAR_raw)
#pragma config(Sensor, dgtl5,  Button_L,       sensorTouch)
#pragma config(Sensor, dgtl6,  Button_W,       sensorNone)
#pragma config(Sensor, dgtl12, LED,            sensorDigitalOut)
#pragma config(Sensor, I2C_1,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_2,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Motor,  port1,           R_motor,       tmotorVex393_HBridge, openLoop, driveRight, encoderPort, I2C_2)
#pragma config(Motor,  port2,           A_motor,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port10,          L_motor,       tmotorVex393_HBridge, openLoop, driveLeft, encoderPort, I2C_1)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

typedef enum {
	Initial = 0,
	Scan,
	Forward,
	Turning,
	Deliver,
	End
} T_State;

const int light_threshold = 1024; 	// threshold for the IR sensor to switch between states
const int TH = 1000; 							// threshold for the Ultrasonic sensor
//const int turning_weight = 1; 		// constant for turning weight

// code to set the bool state of the buttons
bool SB_state = false;
bool LB_state = false; // ask about how to use
bool RB_state = false;

void button(){
	if(SensorValue(start_button) && !SB_state){
		SB_state = true;
	}
	if(SensorValue(Button_R) && !RB_state){
		RB_state = true;
	}
	if(SensorValue(Button_L) && !LB_state){
		LB_state = true;
	}
}

// Perform processing of measurements.
// Should be called with rate of at least 20 Hertz for proper detection of puck.
bool monitorLight(int lightLevel){

	static int minLevelIR = 4096;	// Minimum light level seen by IR sensor 1
	static int maxLevelIR = 0;			// Maximum light level seen by IR sensor 1
	static int diffLevelIR = 0;		// Delta between maximum and minimum seen in last 0.1 seconds

	//int lightLevel = SensorValue[IRsensor];
	bool returnValue;

	// Check if 100 msecs have elapsed.
	if ( time1[T1] > 100 )  {

		// 100 msecs have elapsed.  Compute delta of light level.
		diffLevelIR = maxLevelIR - minLevelIR;

		// Reset calculation for next 100 msecs.
		maxLevelIR = 0;
		minLevelIR = 4096;
		clearTimer(T1);

		} else {

		// Check for new minimum/maximum light levels.
		if ( lightLevel < minLevelIR ) {
			minLevelIR = lightLevel;
			} else if ( lightLevel > maxLevelIR ) {
			maxLevelIR = lightLevel;
		}
	}

	// Check if light level difference over threshold.
	if ( diffLevelIR > light_threshold ) {
		returnValue = true;
		} else {
		returnValue = false;
	}

	return(returnValue);
} // end of IR sensor code


// turns the robot in the direction and amount specified
// CW is -1, CCW is 1 for direction
void turn(int direction, int amount){
	while(getMotorEncoder(L_motor) < amount || getMotorEncoder(R_motor) < amount){
		motor[L_motor] = direction * 37;
		motor[R_motor] = direction * 37; // each motor might have the absolute turn multiplier chenge as each might be different
	}
	motor[L_motor] = direction * -37;
	motor[R_motor] = direction * -37;
	motor[L_motor] = 0;
	motor[R_motor] = 0;
}// end turn

// end of pre-processor material


// beginning of main body

// start of task main
task main(){

	T_State robot_state = Initial;


	while(true){

		button();
		resetMotorEncoder(L_motor);
		resetMotorEncoder(R_motor);
		monitorLight(SensorValue(IRsensorM));

		switch (robot_state){
			// when robot is in rest at the beginning, waits for startt button to be pressed
		case Initial:
			if(SB_state == true){
				SB_state = false;
				robot_state = Scan;
				break;
			}
			break;
			// end Initial

			// Scans the area until the beacon is found by rotating
		case Scan:

			while(!monitorLight(SensorValue(IRsensorM))){
				motor[L_motor] = 40;
				motor[R_motor] = 40;
			}
			motor[L_motor] = 0;
			motor[R_motor] = 0;
			robot_state = Forward;
			break;
			// end Scan

			// moves forward until one of three condistion are met, then it'll swich case, after correcting, it'll come back here unless the new case it Deliver
		case Forward:

			// this if statement should never run as it should only be facing forwards when in this state
			if(!monitorLight(SensorValue[IRsensorM])){
				robot_state = Scan;
				break;
			}

			while(SensorValue(USS) >= TH){
				motor[L_motor] = 40; // again , the constant or velosity might have to be changed as each motor migh tbe different
				motor[R_motor] = -40;

				//this makes sure that the forward state only runs if neither of the side bumpers are not pressed
				if(LB_state || RB_state){
					motor[L_motor] = 0; // could reverse for quick sec to break
					motor[R_motor] = 0;
					robot_state = Turning;
					break;
				}
			}
			motor[L_motor] = 0;
			motor[R_motor] = 0;
			robot_state = Deliver;
			break;
			// end Forward

			// Turns the robot in direction specified by with limit switch was pressed
		case Turning:
			// turns CW
			if(LB_state){
				motor[L_motor] = -37;
				motor[R_motor] = 37;
				wait1Msec(1200); 	// might have to change time amount
				motor[L_motor] = 0;
				motor[R_motor] = 0;
				turn(-1, 300); // might also have to change turn amount
				robot_state = Forward;
				break;
			}
			// turns CCW
			if(RB_state){
				motor[L_motor] = -37;
				motor[R_motor] = 37;
				motor[L_motor] = 0;
				motor[A_motor] = 0;
				wait1Msec(1200); 	// might have to change time amount
				turn(1, 300); // mgiht have to change turn amount
				robot_state = Forward;
				break;
			}
			break;
			// end Turning


			// The process of delivering the cable to the beacon, this involves lowering the arm and raising it.
		case Deliver: // need to add friction to the cable giver so when robot is moving, it doesn't pull out too much and get caught
			motor[A_motor] = -30; // will 100% have to change
			wait1Msec(425);
			motor[A_motor] = 5; // could change to lower value so it can keep it's position
			wait1Msec(1200);
			motor[A_motor] = 40;
			wait1Msec(250); // 140 works nice for rising it not all the way
			motor[A_motor] = 0;

			//

			//
			motor[L_motor] = -37;
			motor[R_motor] = 37;
			wait1Msec(500);
			motor[L_motor] = 0;
			motor[R_motor] = 0;
			robot_state = End;
			break;
			// end Deliver

			// End case, this will move the robot away from the beacon and end all operation.
		case End:
			// move away from the beacon by moving backwards then turning the leaving

			SB_state = false;
			robot_state = Initial;



			// this'll trun away from the beacon, if it sees wall, turns the otehr
			// maybe copy
			/*
			turn(1, 300);
			if(SensorValue(USS) < TH){
			turn(-1, 1200);
			}
			*/
			// more can be added, whatever is neede of the ending process
			//	robot_state = Initial;
			break;
			// end End

		}// end switch(robot_state)

	} // end while(true) loop

}// end task main
