#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, in1,    IRsensorL,      sensorReflection)
#pragma config(Sensor, in2,    IRsensorR,      sensorReflection)
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

const int light_threshold = 512; 	// threshold for the IR sensor to switch between states
const int TH = 1000; 							// threshold for the Ultrasonic sensor
const int turning_weight = 1; 		// constant for turning weight

// code to set the bool state of the buttons
bool SB_state = false;
bool LB_state = false;
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
// it will return diffLevelIR
int monitorLight(int IRsensor){

	static int minLevelIR = 4096;	// Minimum light level seen by IR sensor 1
	static int maxLevelIR = 0;			// Maximum light level seen by IR sensor 1
	static int diffLevelIR = 0;		// Delta between maximum and minimum seen in last 0.1 seconds

	int lightLevel = SensorValue[IRsensor];

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
	return(diffLevelIR);
} // end of IR sensor code


// turns the robot in the direction and amount specified
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
		int diff_IRR_IRL = monitorLight(IRsensorR) - monitorLight(IRsensorL);

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

			// Scans the area until the beacon is found by comparing left and right IR signals and turning proportionally
		case Scan: // logical falicy: what if not facing around the beacon? edit: kind of fixed, a bit rough

			if(((monitorLight(IRsensorL) < light_threshold ) && (monitorLight(IRsensorR) < light_threshold))){
				turn(1, 300); // might have to change value of direction and/or amount
				if(((monitorLight(IRsensorL) < light_threshold ) && (monitorLight(IRsensorR) < light_threshold))){
					turn(-1, 1200); // might have to change value of direction and/or amount
				}
			}
			while(diff_IRR_IRL > 10 || diff_IRR_IRL < -10){ // changed it becasue at low motor speeds it can't run and it'll never get to where it wants, was: diff_IRR_IRL != 0
				motor[L_motor] = diff_IRR_IRL * turning_weight; // turing_weight will have to be changed
				motor[R_motor] = diff_IRR_IRL * turning_weight;
			}
			robot_state = Forward;
			break;
			// end Scan

			// moves forward until one of three condistion are met, then it'll swich case, after correcting, it'll come back here unless the new case it Deliver
		case Forward:

			// this if statement should never run, only in as safty measure
			if(diff_IRR_IRL > 10 || diff_IRR_IRL < -10){
				robot_state = Scan;
				break;
			}

			while(SensorValue(USS) > TH){ // assuming taht there is no way it would be in forward without facing the beacon

				motor[L_motor] = 37; // again , the constant or velosity might have to be changed as each motor migh tbe different
				motor[R_motor] = -37;

				if(LB_state || RB_state){
					motor[L_motor] = 0; // could reverse for quick sec to break
					motor[R_motor] = 0;
					robot_state = Turning;
					break;
				}
			}
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
				turn(-1, 300); // might also have to change turn amount
				robot_state = Forward;
				break;
			}
			// turns CCW
			if(RB_state){
				motor[L_motor] = 37;
				motor[R_motor] = -37;
				wait1Msec(1200); 	// might have to change time amount
				turn(1, 300); // mgiht have to change turn amount
				robot_state = Forward;
				break;
			}
			break;
			// end Turning


			// The process of delivering the cable to the beacon, this involves lowering the arm and raising it.
		case Deliver:

			break;
			// end Deliver

			// End case, this will move the robot away from the beacon and end all operation.
		case End:

			break;
			// end End

		}// end switch(robot_state)

	} // end while(true) loop

}// end task main
