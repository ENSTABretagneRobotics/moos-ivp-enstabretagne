TARGET		= libserial_lidar.a
OBJS		= serial_lidar.o
SRCS		= $(OBJS:.o=.c)
INCLUDEDIRS	+= -I../include
LIBDIRS		+= -L../lib
CFLAGS		= $(INCLUDEDIRS) -W -Wall -O2
LDFLAGS		= $(LIBDIRS) -lm  -ldxl

CC			= gcc
AR			= ar


$(TARGET): $(OBJS)
	$(AR) rs $@ $^
	cp $(TARGET) ../../lib
.c.o:
	$(CC) -c $< $(CFLAGS)
	
clean:
	rm -f $(OBJS) $(TARGET)
	@echo "file deleted."

dep:
	gccmakedep $(SRCS)

