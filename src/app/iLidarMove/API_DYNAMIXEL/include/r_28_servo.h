#ifndef _R_28_SERVO_HEADER
#define _R_28_SERVO_HEADER

#ifdef __cplusplus
extern "C" {
#endif

// Control table address
#define P_GOAL_POSITION_L	30
#define P_GOAL_POSITION_H	31
#define P_MOVING_SPEED_L        32
#define P_MOVING_SPEED_H        33
#define P_TORQUE_LIMIT_L        34
#define P_TORQUE_LIMIT_H        35
#define P_CW_LIMIT_L            6
#define P_CW_H                  7
#define P_CCW_LIMIT_L           8
#define P_CCW_LIMIT_H           9




#define P_PRESENT_POSITION_L	36
#define P_PRESENT_POSITION_H	37
#define P_MOVING		46
#define P_PRESENT_SPEED_L	38
#define P_PRESENT_SPEED_H	39
#define P_PRESENT_LOAD_L	40
#define P_PRESENT_LOAD_H	41

// Default setting
#define DEFAULT_BAUDNUM		1 // 1Mbps
#define DEFAULT_ID		1


//value
#define JOINT_TYPE 0
#define WHEEL_TYPE 1
#define MULTITURN_TYPE  2


//define errors
#define ERROR_VALUE_GOAL_POS   27


//Status Communication
void PrintCommStatus(const int CommStatus);
void PrintErrorCode(void);

//manage USB2dynamixel
int open_USB2Dyn(const int baudnum,const int deviceIndex);
void close_USB2Dyn();

//servo function
int setAngle(const int servo_id,const int goalPos);
int setSpeed(const int servo_id,const int speedPos);
void getPosition(const int servo_id,int *pos,int *CommStatus);
void isMoving(const int servo_id,int *result,int *CommStatus);
void getOperationType(const int servo_id,int *typeOperation,int *CommStatus);

#ifdef __cplusplus
}
#endif

#endif
