#ifndef _SERIAL_LIDAR_HEADER
#define _SERIAL_LIDAR_HEADER

#ifdef __cplusplus
extern "C" {
#endif
int start_serial(const int baudrate ,const char *url);
void serial_close();
void serial_clear();
int serial_rx( unsigned char *pPacket, int numPacket );
void serial_set_timeout( int NumRcvByte );
int serial_timeout();
void read_packet(void);
int read_distance(void);

void findLastGoodTrame(char *buffer,int *res);
#ifdef __cplusplus
}
#endif

#endif
