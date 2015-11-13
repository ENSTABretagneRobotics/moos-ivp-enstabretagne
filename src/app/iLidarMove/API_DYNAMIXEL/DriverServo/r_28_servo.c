//##########################################################
//##                      ENSTA Bretagne                  ##
//##          Simplified library code for Dynamixel.      ##
//##     Inspiration from ROBOTIS              2015.11.02 ##
//##########################################################
#include <stdio.h>
#include <termio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "dynamixel.h"
#include "r_28_servo.h"

void setMovement(int servo_id,const int angleMin,const int angleMax);//private function

int open_USB2Dyn(const int baudnum,const int deviceIndex){
     if( dxl_initialize(deviceIndex, baudnum) == 0 )
	{
		printf( "Failed to open USB2Dynamixel!\n" );
		return 0;
	}
	else
		printf( "Succeed to open USB2Dynamixel!\n" );
     return 1;
}

void close_USB2Dyn(){
  dxl_terminate();
}

int setAngle(const int servo_id,const int goalPos){
  if (goalPos>1023 || goalPos <0)
     return ERROR_VALUE_GOAL_POS; 
  dxl_write_word( servo_id , P_GOAL_POSITION_L, goalPos);
  return 1;
}

int setSpeed(const int servo_id,const int speedPos){
  if (speedPos>1023 || speedPos <0)
     return ERROR_VALUE_GOAL_POS; 
  dxl_write_word( servo_id , P_MOVING_SPEED_L, speedPos);
  return 1;
}

void getPosition(const int servo_id,int *pos,int *CommStatus){
    *pos = dxl_read_word( servo_id, P_PRESENT_POSITION_L );
    *CommStatus = dxl_get_result();
    if(*CommStatus != COMM_RXSUCCESS)
       PrintCommStatus(*CommStatus);
}

void isMoving(const int servo_id,int *result,int *CommStatus){
    *result =  dxl_read_byte( servo_id, P_MOVING);
    *CommStatus = dxl_get_result();
    if(*CommStatus != COMM_RXSUCCESS)
       PrintCommStatus(*CommStatus);
    PrintErrorCode();
}

void getOperationType(const int servo_id,int *typeOperation,int *CommStatus){
     int cw_val =  dxl_read_word( servo_id, P_CW_LIMIT_L);
     int ccw_val =  dxl_read_word( servo_id, P_CCW_LIMIT_L);
     if (cw_val+ccw_val==0)
        *typeOperation =  WHEEL_TYPE;
     else if (cw_val<1024 && ccw_val<1024)
        *typeOperation = JOINT_TYPE;
     else 
        *typeOperation = MULTITURN_TYPE;
     *CommStatus = dxl_get_result();
     if(*CommStatus != COMM_RXSUCCESS)
       PrintCommStatus(*CommStatus);
}

void setMovement(int servo_id,const int angleMin,const int angleMax){
     int GoalPos[2] = {angleMin,angleMax};
     int index =0;
     int Moving,CommStatus;
     while(1){
       // Write goal position
       dxl_write_word( servo_id, P_GOAL_POSITION_L, GoalPos[index] );
       do
       {
           // Check moving done
           Moving = dxl_read_byte( servo_id, P_MOVING );
           CommStatus = dxl_get_result();
           if( CommStatus == COMM_RXSUCCESS )
           {
               if( Moving == 0 )
               {
                   // Change goal position
                   if( index == 0 )
                       index = 1;
                   else
                       index = 0;					
               }
           }
           else
           {
               PrintCommStatus(CommStatus);
               break;
           }
        }while(Moving == 1);
     }
}

// Print communication result
void PrintCommStatus(const int CommStatus)
{
	switch(CommStatus)
	{
	case COMM_TXFAIL:
		printf("COMM_TXFAIL: Failed transmit instruction packet!\n");
		break;

	case COMM_TXERROR:
		printf("COMM_TXERROR: Incorrect instruction packet!\n");
		break;

	case COMM_RXFAIL:
		printf("COMM_RXFAIL: Failed get status packet from device!\n");
		break;

	case COMM_RXWAITING:
		printf("COMM_RXWAITING: Now recieving status packet!\n");
		break;

	case COMM_RXTIMEOUT:
		printf("COMM_RXTIMEOUT: There is no status packet!\n");
		break;

	case COMM_RXCORRUPT:
		printf("COMM_RXCORRUPT: Incorrect status packet!\n");
		break;

	default:
		printf("This is unknown error code!\n");
		break;
	}
}

// Print error bit of status packet
void PrintErrorCode()
{
	if(dxl_get_rxpacket_error(ERRBIT_VOLTAGE) == 1)
		printf("Input voltage error!\n");

	if(dxl_get_rxpacket_error(ERRBIT_ANGLE) == 1)
		printf("Angle limit error!\n");

	if(dxl_get_rxpacket_error(ERRBIT_OVERHEAT) == 1)
		printf("Overheat error!\n");

	if(dxl_get_rxpacket_error(ERRBIT_RANGE) == 1)
		printf("Out of range error!\n");

	if(dxl_get_rxpacket_error(ERRBIT_CHECKSUM) == 1)
		printf("Checksum error!\n");

	if(dxl_get_rxpacket_error(ERRBIT_OVERLOAD) == 1)
		printf("Overload error!\n");

	if(dxl_get_rxpacket_error(ERRBIT_INSTRUCTION) == 1)
		printf("Instruction code error!\n");
}
