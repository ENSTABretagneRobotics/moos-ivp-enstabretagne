//##########################################################
//##                      ENSTA Bretagne                  ##
//##     Code to discuss with teensy with lidarliteV1     ##
//##                                           2015.11.02 ##
//##########################################################
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <linux/serial.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include "serial_lidar.h"
#include <math.h>

int ser_gSocket_fd = -1;
long	ser_glStartTime	= 0;
float	ser_gfRcvWaitTime	= 0.0f;
float	ser_gfByteTransTime	= 0.0f;
unsigned char ser_gbStatusPacket[200] = {'\0'};

/*int main()
{

  char url[20] = "/dev/ttyACM0";
  start_serial(112500,url);
  while(1){
     read_packet();
  }
  return 0;
}*/

int start_serial(const int baudrate ,const char *url){
    struct termios newtio;
    struct serial_struct serinfo;

    if((ser_gSocket_fd = open(url, O_RDWR|O_NOCTTY|O_NONBLOCK)) < 0) {
        fprintf(stderr, "device open error: %s\n", url);
        serial_close();
        return 0;
    }

    newtio.c_cflag        = B115200|CS8|CLOCAL|CREAD;
    newtio.c_iflag        = IGNPAR;
    newtio.c_oflag        = 0;
    newtio.c_lflag        = 0;
    newtio.c_cc[VTIME]    = 0;    // time-out 값 (TIME * 0.1초) 0 : disable
    newtio.c_cc[VMIN]    = 0;    // MIN 은 read 가 return 되기 위한 최소 문자 개수

    tcflush(ser_gSocket_fd, TCIFLUSH);
    tcsetattr(ser_gSocket_fd, TCSANOW, &newtio);
    
    if(ser_gSocket_fd == -1)
        return 0;
    
    if(ioctl(ser_gSocket_fd, TIOCGSERIAL, &serinfo) < 0) {
        fprintf(stderr, "Cannot get serial info:%s\n", url);
        return 0;
    }
    
    serinfo.flags &= ~ASYNC_SPD_MASK;
    serinfo.flags |= ASYNC_SPD_CUST;
    serinfo.custom_divisor = serinfo.baud_base / baudrate;
    
    if(ioctl(ser_gSocket_fd, TIOCSSERIAL, &serinfo) < 0) {
        fprintf(stderr, "Cannot set serial info:%s\n", url);
        return 0;
    }
    
    serial_close();
    
    ser_gfByteTransTime = (float)((1000.0f / baudrate) * 12.0f);
    
    serial_close();
    
    if((ser_gSocket_fd = open(url, O_RDWR|O_NOCTTY | O_NONBLOCK)) < 0) {
        fprintf(stderr, "device open error: %s\n", url);
        serial_close();
        return 0;
    }

    newtio.c_cflag        = B38400|CS8|CLOCAL|CREAD;
    newtio.c_iflag        = IGNPAR;
    newtio.c_oflag        = 0;
    newtio.c_lflag        = 0;
    newtio.c_cc[VTIME]    = 0;    // time-out 값 (TIME * 0.1초) 0 : disable
    newtio.c_cc[VMIN]    = 0;    // MIN 은 read 가 return 되기 위한 최소 문자 개수

    tcflush(ser_gSocket_fd, TCIFLUSH);
    tcsetattr(ser_gSocket_fd, TCSANOW, &newtio);
    printf("baudrate\n");
    return 1;
}




void serial_close()
{
    if(ser_gSocket_fd != -1)
        close(ser_gSocket_fd);
    ser_gSocket_fd = -1;
}

void serial_clear(void)
{
	tcflush(ser_gSocket_fd, TCIFLUSH);
}


int serial_rx( unsigned char *pPacket, int numPacket )
{
	memset(pPacket, 0, numPacket);
	return read(ser_gSocket_fd, pPacket, numPacket);
}

static inline long myclock()
{
	struct timeval tv;
	gettimeofday (&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void serial_set_timeout( int NumRcvByte )
{
	ser_glStartTime = myclock();
	ser_gfRcvWaitTime = (float)(ser_gfByteTransTime*(float)NumRcvByte + 5.0f);
}

int serial_timeout(void)
{
	long time;
	
	time = myclock() - ser_glStartTime;
	
	if(time > ser_gfRcvWaitTime)
		return 1;
	else if(time < 0)
		ser_glStartTime = myclock();
		
	return 0;
}


void read_packet(void)
{
	unsigned char nRead;



	int gbRxGetLength = 0;
        int gbRxPacketLength = 15;
	
	nRead = serial_rx( (unsigned char*)&ser_gbStatusPacket[gbRxGetLength], gbRxPacketLength - gbRxGetLength );
        printf("%s",(char*)ser_gbStatusPacket);
	gbRxGetLength += nRead;
	if( gbRxGetLength < gbRxPacketLength )
	{
		if( serial_timeout() == 1 )
		{
			return;
		}
	}
        return;
}


int read_distance(void)
{
	unsigned char nRead;



	int gbRxGetLength = 0;
        int gbRxPacketLength = 7;
	memset(&ser_gbStatusPacket[gbRxGetLength],'\0',200);

	nRead = serial_rx( (unsigned char*)&ser_gbStatusPacket[gbRxGetLength], 200);
        
	gbRxGetLength += nRead;
	if( gbRxGetLength < gbRxPacketLength )
	{
		if( serial_timeout() == 1 )
		{
			return -1;
		}
	}

        int result = -1;
        findLastGoodTrame((char*)&ser_gbStatusPacket[0],&result);
        //printf("%s",(char*)ser_gbStatusPacket);
        
        //sscanf( (char*)ser_gbStatusPacket,"%d,%d\n",&result,&check);
        //int checking =((int)log10(result) +1);
       //printf(";%d\n",nRead);
        //if (checking!=check || result <= 200 || check < 1 || check > 4){
        //   printf("redo2");
        //   return read_distance();
        //}
        return result;
}


void findLastGoodTrame(char *buffer,int *res){
   int idx=0;
   int oldidx = 0;
   for (idx=0;idx<200;idx++)
   {
      //printf("%c,",buffer[idx]);
      if (buffer[idx]=='\n')
      {  
         if (oldidx!=0)
         {
           sscanf(&buffer[oldidx],"%d\n",res);
           
         }
         oldidx=idx;
      }
      if (idx > 6)
      {
        if (buffer[idx-1]=='\0' && buffer[idx]=='\0')
        {
           return;
        } 
      }
   }

}

