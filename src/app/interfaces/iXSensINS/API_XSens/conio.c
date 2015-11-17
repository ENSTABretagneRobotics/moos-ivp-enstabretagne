#include <sys/select.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#include "conio.h"

/*! getch hack

  Don't use in production code!
*/
int _getch(void)
{
	struct termios oldt, newt;
	int c;

	tcgetattr(0, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON|ECHO);
	tcsetattr(0, TCSANOW, &newt);
	c = getchar();
	tcsetattr(0, TCSANOW, &oldt);
	return c;
}

/*! kbhit hack

  Don't use in production code!
*/
int _kbhit()
{
	struct termios oldt, newt;

	struct timeval tv;
	fd_set rfd;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	FD_ZERO(&rfd);
	FD_SET(0, &rfd);

	tcgetattr(0, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON|ECHO);
	tcsetattr(0, TCSANOW, &newt);

	if (select(1, &rfd, 0, 0, &tv) == -1)
		return 0;

	tcsetattr(0, TCSANOW, &oldt);

	return (FD_ISSET(0, &rfd) != 0);
}
