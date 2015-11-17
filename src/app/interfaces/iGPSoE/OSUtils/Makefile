# Makefile for Linux, tested with Ubuntu 10.10. 
# You might need to install C/C++ development tools by typing :
#    sudo apt-get install build-essential
# in a terminal.

LIB = libosutils.a

CC = gcc
CFLAGS += -g
#CFLAGS += -O3
CFLAGS += -Wall
#CFLAGS += -Wextra -Winline

CFLAGS += -D _DEBUG -D _DEBUG_DISPLAY -D _DEBUG_MESSAGES 
CFLAGS += -D ENABLE_KILL_THREAD -D ENABLE_CANCEL_THREAD 
CFLAGS += -D ENABLE_SET_THREAD_DEFAULT_PROCESSOR -D ENABLE_PRIORITY_HANDLING
CFLAGS += -D ENABLE_WAIT_FOR_THREAD_TIMEOUT
CFLAGS += -D ENABLE_GETTIMEOFDAY_WIN32

CFLAGS += -I. -I../OSUtils 

default: $(LIB)

OSCore.o: ../OSUtils/OSCore.c ../OSUtils/OSCore.h
	$(CC) $(CFLAGS) -c $<

OSCriticalSection.o: ../OSUtils/OSCriticalSection.c ../OSUtils/OSCriticalSection.h OSThread.o
	$(CC) $(CFLAGS) -c $<

OSEv.o: ../OSUtils/OSEv.c ../OSUtils/OSEv.h OSThread.o
	$(CC) $(CFLAGS) -c $<

OSFdList.o: ../OSUtils/OSFdList.c ../OSUtils/OSFdList.h OSNet.o
	$(CC) $(CFLAGS) -c $<

OSIPEv.o: ../OSUtils/OSIPEv.c ../OSUtils/OSIPEv.h OSTime.o
	$(CC) $(CFLAGS) -c $<

OSIPMutex.o: ../OSUtils/OSIPMutex.c ../OSUtils/OSIPMutex.h OSThread.o
	$(CC) $(CFLAGS) -c $<

OSIPSem.o: ../OSUtils/OSIPSem.c ../OSUtils/OSIPSem.h OSTime.o
	$(CC) $(CFLAGS) -c $<

OSMisc.o: ../OSUtils/OSMisc.c ../OSUtils/OSMisc.h OSTime.o
	$(CC) $(CFLAGS) -c $<

OSMq.o: ../OSUtils/OSMq.c ../OSUtils/OSMq.h OSThread.o
	$(CC) $(CFLAGS) -c $<

OSNet.o: ../OSUtils/OSNet.c ../OSUtils/OSNet.h OSTime.o
	$(CC) $(CFLAGS) -c $<

OSProcess.o: ../OSUtils/OSProcess.c ../OSUtils/OSProcess.h OSTime.o
	$(CC) $(CFLAGS) -c $<

OSRS232Port.o: ../OSUtils/OSRS232Port.c ../OSUtils/OSRS232Port.h OSTime.o
	$(CC) $(CFLAGS) -c $<

OSSem.o: ../OSUtils/OSSem.c ../OSUtils/OSSem.h OSTime.o
	$(CC) $(CFLAGS) -c $<

OSTCPSock.o: ../OSUtils/OSTCPSock.c ../OSUtils/OSTCPSock.h OSNet.o
	$(CC) $(CFLAGS) -c $<

OSThread.o: ../OSUtils/OSThread.c ../OSUtils/OSThread.h OSTime.o
	$(CC) $(CFLAGS) -c $<

OSTime.o: ../OSUtils/OSTime.c ../OSUtils/OSTime.h OSCore.o
	$(CC) $(CFLAGS) -c $<

OSTimer.o: ../OSUtils/OSTimer.c ../OSUtils/OSTimer.h OSEv.o
	$(CC) $(CFLAGS) -c $<

$(LIB): OSTimer.o OSTime.o OSThread.o OSTCPSock.o OSSem.o OSRS232Port.o OSProcess.o OSNet.o OSMq.o OSMisc.o OSIPSem.o OSIPMutex.o OSIPEv.o OSFdList.o OSEv.o OSCriticalSection.o OSCore.o
	ar -rcs $@ $^

clean:
	rm -f $(LIB) *.o *.obj core *.gch
