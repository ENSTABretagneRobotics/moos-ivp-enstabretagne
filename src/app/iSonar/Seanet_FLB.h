// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef SEANET_H
#define SEANET_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_SEANETTHREAD
#include "OSThread.h"
#endif // DISABLE_SEANETTHREAD

#define TIMEOUT_MESSAGE_SEANET 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_SEANET 2048

#define MAX_NUMBER_OF_STEPS_SEANET 6400

#define MIN_MESSAGE_LEN_SEANET 11

#define TIMEOUT_REBOOT_SEANET 30.0 // In s.
#define TIMEOUT_BBUSER_SEANET 10.0 // In s.
#define TIMEOUT_HEADCOMMAND_SEANET 10.0 // In s.

#define MESSAGE_HEADER_SEANET 0x40 // '@'.
#define MESSAGE_TERMINATOR_SEANET 0x0a // LF.
#define SERIAL_PORT_SONAR_NODE_NUMBER_SEANET 2
#define SERIAL_PORT_PROGRAM_NODE_NUMBER_SEANET 255

#define mtAlive 4
#define mtReboot 16
#define mtSendVersion 23
#define mtVersionData 1
#define mtSendBBUser 24
#define mtBBUserData 6
#define mtFpgaCalibrationData 63
#define mtFpgaVersionData 57
#define mtHeadCommand 19
#define mtSendData 25
#define mtHeadData 2
#define mtAuxData 8

#define RESOLUTION2STEP_ANGLE_SIZE_IN_DEGREES(res) ((res)*0.05625)
#define STEP_ANGLE_SIZE_IN_DEGREES2RESOLUTION(StepAngleSize) ((int)((StepAngleSize)/0.05625))
#define RESOLUTION2NUMBER_OF_STEPS(res) ((int)(360.0/RESOLUTION2STEP_ANGLE_SIZE_IN_DEGREES(res)))
#define NUMBER_OF_STEPS2RESOLUTION(NSteps) ((int)(STEP_ANGLE_SIZE_IN_DEGREES2RESOLUTION(360.0/(NSteps))))

#ifndef STEP_ANGLE_SIZE_IN_DEGREES2NUMBER_OF_STEPS
#define STEP_ANGLE_SIZE_IN_DEGREES2NUMBER_OF_STEPS(StepAngleSize) ((int)(360.0/(StepAngleSize)))
#endif // STEP_ANGLE_SIZE_IN_DEGREES2NUMBER_OF_STEPS

#ifndef NUMBER_OF_STEPS2STEP_ANGLE_SIZE_IN_DEGREES
#define NUMBER_OF_STEPS2STEP_ANGLE_SIZE_IN_DEGREES(NSteps) (360.0/(NSteps))
#endif // NUMBER_OF_STEPS2STEP_ANGLE_SIZE_IN_DEGREES

// Transducer Bearing is the position of the transducer for the current scanline (0..6399 in 1/16 Gradian units), 
// ahead corresponds to 3200. 10 gradians = 9 degrees. 0.05625 = (1/16)*(9/10).
// Angle of the transducer in degrees (in [0;360[ deg).
#define BEARING2ANGLE_IN_DEGREES(bearing) (((((int)(bearing))%6400-3200+6400)%6400)*0.05625)
#define ANGLE_IN_DEGREES2BEARING(angle) ((((int)((angle)/0.05625))%6400+3200+6400)%6400)

union usShort_SEANET
{
	short v;  
	unsigned char c[2];
};
typedef union usShort_SEANET usShort_SEANET;

union uShort_SEANET
{
	unsigned short v;  
	unsigned char c[2];
};
typedef union uShort_SEANET uShort_SEANET;

union usLong_SEANET
{
	long v;  
	unsigned char c[4];
};
typedef union usLong_SEANET usLong_SEANET;

union uLong_SEANET
{
	unsigned long v;  
	unsigned char c[4];
};
typedef union uLong_SEANET uLong_SEANET;

union uFloat_SEANET
{
	float v;  
	unsigned char c[4];
};
typedef union uFloat_SEANET uFloat_SEANET;

struct HDCTRLStruct
{
	unsigned int adc8on : 1;
	unsigned int cont : 1;
	unsigned int scanright : 1;
	unsigned int invert : 1;
	unsigned int motoff : 1;
	unsigned int txoff : 1;
	unsigned int spare : 1;
	unsigned int chan2 : 1;
	unsigned int raw : 1;
	unsigned int hasmot : 1;
	unsigned int applyoffset : 1;
	unsigned int pingpong : 1;
	unsigned int stareLLim : 1;
	unsigned int ReplyASL : 1;
	unsigned int ReplyThr : 1;
	unsigned int IgnoreSensor : 1;
	unsigned int padding : 16;
};
union HDCTRL
{
	struct HDCTRLStruct bits;  
	unsigned char uc[4];
	char c[4];
	unsigned int u;
	int i;
};
typedef union HDCTRL HDCTRL;

//struct SEANETDATA
//{
//
//	unsigned char HeadInf;
//};
//typedef struct SEANETDATA SEANETDATA;

struct SEANET
{
	RS232PORT RS232Port;
	BOOL bDST;
	BOOL bHalfDuplex;
	//BOOL bSingleChannel;
	HDCTRL HdCtrl;
	int LeftAngleLimit; // In [0..359] deg, computed from ScanDirection and ScanWidth.
	int RightAngleLimit; // In [0..359] deg, computed from ScanDirection and ScanWidth.
	int LeftLim; // In 1/16 Grad, computed from LeftAngleLimit.
	int RightLim; // In 1/16 Grad, computed from RightAngleLimit.
	int ADLow; // Computed from Sensitivity.
	int ADSpan; // Computed from Contrast.
	int IGain; // Initial Gain of the receiver (in units 0..210 = 0..+80dB = 0..100%), computed from Gain.
	int Slope; // Slope setting for each channel in 1/255 units.
	int Resolution; // Scanning motor step angle between pings in units of 1/16 Grad, computed from StepAngleSize.
	int NSteps; // Computed from StepAngleSize.
	int HeadStatus;
	HDCTRL HeadHdCtrl; // Current value returned by the sonar.
	int HeadRangescale; // Current value returned by the sonar.
	int HeadIGain; // Current value returned by the sonar.
	int HeadSlope; // Current value returned by the sonar.
	int HeadADLow; // Current value returned by the sonar.
	int HeadADSpan; // Current value returned by the sonar.
	int HeadADInterval; // Current value returned by the sonar.
	int HeadLeftLim; // Current value returned by the sonar.
	int HeadRightLim; // Current value returned by the sonar.
	int HeadSteps; // Current value returned by the sonar.
	int HeadNBins; // Current value returned by the sonar.
	int Dbytes;
	int HeadInf;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	//SEANETDATA LastSeanetData;
	//LastScanLine, LastAngle
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	BOOL bSaveRawData;
	int RangeScale; // In m.
	int Gain; // In %.
	int Sensitivity; // In [0..80] dB.
	int Contrast; // In [0..80] dB.
	int ScanDirection; // In [0..359] deg.
	int ScanWidth; // In [2..360] deg.
	double StepAngleSize; // In deg, angle between 2 pings.
	int NBins; // Corresponds to the resolution of a ping, max 800.
	int adc8on; // BOOL.
	int scanright; // BOOL.
	int invert; // BOOL.
	int stareLLim; // BOOL.
	int VelocityOfSound; // In m/s.
	double alpha_max_err;
	double d_max_err;
	int HorizontalBeam;
	int VerticalBeam;
};
typedef struct SEANET SEANET;

// msg must contain a valid message of msglen bytes.
inline void SetHexBinLengthSeanet(unsigned char* msg, int msglen)
{
	uShort_SEANET BinLength;
	char szHexLength[5];

	BinLength.v = (unsigned short)(msglen-6);
	msg[5] = BinLength.c[0];
	msg[6] = BinLength.c[1];
	memset(szHexLength, 0, sizeof(szHexLength));
	sprintf(szHexLength, "%.04x", (int)BinLength.v);
	memcpy(msg+1, szHexLength, 4);
}

// buf must contain a valid message without invalid data in the beginning.
inline int GetHexLengthSeanet(unsigned char* buf)
{
	char szHexLength[5];
	int HexLength = 0;

	memset(szHexLength, 0, sizeof(szHexLength));
	memcpy(szHexLength, buf+1, 4);
	if (sscanf(szHexLength, "%x", &HexLength) == 1)	return HexLength; else return 0;
}

// buf must contain a valid message without invalid data in the beginning.
inline int GetBinLengthSeanet(unsigned char* buf)
{
	uShort_SEANET BinLength;

	BinLength.c[0] = buf[5];
	BinLength.c[1] = buf[6];

	return (int)BinLength.v;
}

// If this function succeeds, the beginning of buf contains a valid message
// header, hex and bin length but the message might be incomplete or there 
// might be other data at the end.
inline int AnalyseBufHdrHexBinLenSeanet(unsigned char* buf, int buflen, int* pExpectedMsgLen)
{
	int HexLength = 0;

	if (buflen < 7)
	{
		//PRINT_DEBUG_MESSAGE_SEANETCORE(("Invalid number of bytes\n"));
		return EXIT_FAILURE;
	}
	if (buf[0] != MESSAGE_HEADER_SEANET)
	{
		//PRINT_DEBUG_MESSAGE_SEANETCORE(("Invalid message header\n"));
		return EXIT_FAILURE;
	}
	HexLength = GetHexLengthSeanet(buf);
	if (HexLength != GetBinLengthSeanet(buf))
	{
		//PRINT_DEBUG_MESSAGE_SEANETCORE(("Invalid Hex or Bin Length\n"));
		return EXIT_FAILURE;
	}

	*pExpectedMsgLen = HexLength+6;

	return EXIT_SUCCESS;
}

// If this function succeeds, the beginning of buf contains a valid message
// header, hex length, bin length and MID but the message might be incomplete 
// or there might be other data at the end.
inline int AnalyseBufHdrHexBinLenMIDSeanet(unsigned char* buf, int buflen, int mid, int* pExpectedMsgLen)
{
	int HexLength = 0;

	if (buflen < MIN_MESSAGE_LEN_SEANET)
	{
		//PRINT_DEBUG_MESSAGE_SEANETCORE(("Invalid number of bytes\n"));
		return EXIT_FAILURE;
	}
	if (buf[0] != MESSAGE_HEADER_SEANET)
	{
		//PRINT_DEBUG_MESSAGE_SEANETCORE(("Invalid message header\n"));
		return EXIT_FAILURE;
	}
	HexLength = GetHexLengthSeanet(buf);
	if (HexLength != GetBinLengthSeanet(buf))
	{
		//PRINT_DEBUG_MESSAGE_SEANETCORE(("Invalid Hex or Bin Length\n"));
		return EXIT_FAILURE;
	}
	if (buf[10] != (unsigned char)mid)
	{
		//PRINT_DEBUG_MESSAGE_SEANETCORE(("Invalid message ID\n"));
		return EXIT_FAILURE;
	}

	*pExpectedMsgLen = HexLength+6;

	return EXIT_SUCCESS;
}

// If this function succeeds, the beginning of buf contains a valid message
// but there might be other data at the end.
inline int AnalyseBufSeanet(unsigned char* buf, int buflen)
{
	int HexLength = 0;

	if (buflen < MIN_MESSAGE_LEN_SEANET)
	{
		//PRINT_DEBUG_MESSAGE_SEANETCORE(("Invalid number of bytes\n"));
		return EXIT_FAILURE;
	}
	if (buf[0] != MESSAGE_HEADER_SEANET)
	{
		//PRINT_DEBUG_MESSAGE_SEANETCORE(("Invalid message header\n"));
		return EXIT_FAILURE;
	}
	HexLength = GetHexLengthSeanet(buf);
	if (HexLength != GetBinLengthSeanet(buf))
	{
		//PRINT_DEBUG_MESSAGE_SEANETCORE(("Invalid Hex or Bin Length\n"));
		return EXIT_FAILURE;
	}
	if (buflen < HexLength+6)
	{
		//PRINT_DEBUG_MESSAGE_SEANETCORE(("Incomplete message\n"));
		return EXIT_FAILURE;
	}
	if (buf[HexLength+5] != MESSAGE_TERMINATOR_SEANET)
	{ 
		//PRINT_DEBUG_MESSAGE_SEANETCORE(("Invalid message terminator\n"));
		return EXIT_FAILURE;	
	}

	return EXIT_SUCCESS;
}

// If this function succeeds, the beginning of buf contains a valid message
// but there might be other data at the end.
inline int AnalyseBufWithMIDSeanet(unsigned char* buf, int buflen, int mid)
{
	int HexLength = 0;

	if (buflen < MIN_MESSAGE_LEN_SEANET)
	{
		//PRINT_DEBUG_MESSAGE_SEANETCORE(("Invalid number of bytes\n"));
		return EXIT_FAILURE;
	}
	if (buf[0] != MESSAGE_HEADER_SEANET)
	{
		//PRINT_DEBUG_MESSAGE_SEANETCORE(("Invalid message header\n"));
		return EXIT_FAILURE;
	}
	HexLength = GetHexLengthSeanet(buf);
	if (HexLength != GetBinLengthSeanet(buf))
	{
		//PRINT_DEBUG_MESSAGE_SEANETCORE(("Invalid Hex or Bin Length\n"));
		return EXIT_FAILURE;
	}
	if (buf[10] != (unsigned char)mid)
	{
		//PRINT_DEBUG_MESSAGE_SEANETCORE(("Invalid message ID\n"));
		return EXIT_FAILURE;
	}
	if (buflen < HexLength+6)
	{
		//PRINT_DEBUG_MESSAGE_SEANETCORE(("Incomplete message\n"));
		return EXIT_FAILURE;
	}
	if (buf[HexLength+5] != MESSAGE_TERMINATOR_SEANET)
	{ 
		//PRINT_DEBUG_MESSAGE_SEANETCORE(("Invalid message terminator\n"));
		return EXIT_FAILURE;	
	}

	return EXIT_SUCCESS;
}

// If this function succeeds, the beginning of *pFoundMsg should contain a valid message
// but there might be other data at the end. Data in the beginning of buf might have been discarded.
inline int FindMsgSeanet(unsigned char* buf, int buflen, unsigned char** pFoundMsg, int* pFoundMsgTmpLen)
{
	*pFoundMsg = buf;
	*pFoundMsgTmpLen = buflen;

	while (AnalyseBufSeanet(*pFoundMsg, *pFoundMsgTmpLen) != EXIT_SUCCESS)
	{
		(*pFoundMsg)++;
		(*pFoundMsgTmpLen)--;
		if (*pFoundMsgTmpLen < MIN_MESSAGE_LEN_SEANET)
		{
			*pFoundMsg = NULL;
			*pFoundMsgTmpLen = 0;
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

// If this function succeeds, the beginning of *pFoundMsg should contain a valid message
// but there might be other data at the end. Data in the beginning of buf might have been discarded.
inline int FindMsgWithMIDSeanet(unsigned char* buf, int buflen, int mid, unsigned char** pFoundMsg, int* pFoundMsgTmpLen)
{
	*pFoundMsg = buf;
	*pFoundMsgTmpLen = buflen;

	while (AnalyseBufWithMIDSeanet(*pFoundMsg, *pFoundMsgTmpLen, mid) != EXIT_SUCCESS)
	{
		(*pFoundMsg)++;
		(*pFoundMsgTmpLen)--;
		if (*pFoundMsgTmpLen < MIN_MESSAGE_LEN_SEANET)
		{
			*pFoundMsg = NULL;
			*pFoundMsgTmpLen = 0;
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

// If this function succeeds, the beginning of *pFoundMsg should contain the latest valid message
// but there might be other data at the end. Data in the beginning of buf might have been discarded, 
// including valid messages.
inline int FindLatestMsg0Seanet(unsigned char* buf, int buflen, unsigned char** pFoundMsg, int* pFoundMsgTmpLen)
{
	unsigned char* ptr = NULL;
	int len = 0;
	int msglen = 0;

	if (FindMsgSeanet(buf, buflen, &ptr, &len) != EXIT_SUCCESS)
	{
		*pFoundMsg = NULL;
		*pFoundMsgTmpLen = 0;
		return EXIT_FAILURE;
	}
	for (;;) 
	{
		// Save the position of the beginning of the message.
		*pFoundMsg = ptr;
		*pFoundMsgTmpLen = len;

		// Expected total message length.
		msglen = GetBinLengthSeanet(*pFoundMsg)+6;

		// Search just after the message.
		if (FindMsgSeanet(*pFoundMsg+msglen, *pFoundMsgTmpLen-msglen, 
			&ptr, &len) != EXIT_SUCCESS)
		{
			break;
		}
	}

	return EXIT_SUCCESS;
}

// If this function succeeds, the beginning of *pFoundMsg should contain the latest valid message
// but there might be other data at the end. Data in the beginning of buf might have been discarded, 
// including valid messages.
inline int FindLatestMsgWithMID0Seanet(unsigned char* buf, int buflen, int mid, unsigned char** pFoundMsg, int* pFoundMsgTmpLen)
{
	unsigned char* ptr = NULL;
	int len = 0;
	int msglen = 0;

	if (FindMsgWithMIDSeanet(buf, buflen, mid, &ptr, &len) != EXIT_SUCCESS)
	{
		*pFoundMsg = NULL;
		*pFoundMsgTmpLen = 0;
		return EXIT_FAILURE;
	}
	for (;;) 
	{
		// Save the position of the beginning of the message.
		*pFoundMsg = ptr;
		*pFoundMsgTmpLen = len;

		// Expected total message length.
		msglen = GetBinLengthSeanet(*pFoundMsg)+6;

		// Search just after the message.
		if (FindMsgWithMIDSeanet(*pFoundMsg+msglen, *pFoundMsgTmpLen-msglen, 
			mid, &ptr, &len) != EXIT_SUCCESS)
		{
			break;
		}
	}

	return EXIT_SUCCESS;
}

// If this function succeeds, *pFoundMsg should contain the latest valid message 
// (of *pFoundMsgLen bytes).
// Data in the beginning of buf (*pFoundMsg-buf bytes starting at buf address), including valid 
// messages might have been discarded.
// Other data at the end of buf (*pRemainingDataLen bytes, that should not contain any valid message) 
// might be available in *pRemainingData.
inline int FindLatestMsgSeanet(unsigned char* buf, int buflen, 
							   unsigned char** pFoundMsg, int* pFoundMsgLen, 
							   unsigned char** pRemainingData, int* pRemainingDataLen)
{
	unsigned char* ptr = NULL;
	int len = 0;

	if (FindMsgSeanet(buf, buflen, &ptr, &len) != EXIT_SUCCESS)
	{
		*pFoundMsg = NULL;
		*pFoundMsgLen = 0;
		*pRemainingData = buf;
		*pRemainingDataLen = buflen;
		return EXIT_FAILURE;
	}
	for (;;) 
	{
		// Save the position of the beginning of the message.
		*pFoundMsg = ptr;

		// Expected total message length.
		*pFoundMsgLen = GetBinLengthSeanet(*pFoundMsg)+6;

		*pRemainingData = *pFoundMsg+*pFoundMsgLen;
		*pRemainingDataLen = len-*pFoundMsgLen;

		// Search just after the message.
		if (FindMsgSeanet(*pRemainingData, *pRemainingDataLen, 
			&ptr, &len) != EXIT_SUCCESS)
		{
			break;
		}
	}

	return EXIT_SUCCESS;
}

// If this function succeeds, *pFoundMsg should contain the latest valid message 
// (of *pFoundMsgLen bytes).
// Data in the beginning of buf (*pFoundMsg-buf bytes starting at buf address), including valid 
// messages might have been discarded.
// Other data at the end of buf (*pRemainingDataLen bytes, that should not contain any valid message) 
// might be available in *pRemainingData.
inline int FindLatestMsgWithMIDSeanet(unsigned char* buf, int buflen, int mid, 
									  unsigned char** pFoundMsg, int* pFoundMsgLen, 
									  unsigned char** pRemainingData, int* pRemainingDataLen)
{
	unsigned char* ptr = NULL;
	int len = 0;

	if (FindMsgWithMIDSeanet(buf, buflen, mid, &ptr, &len) != EXIT_SUCCESS)
	{
		*pFoundMsg = NULL;
		*pFoundMsgLen = 0;
		*pRemainingData = buf;
		*pRemainingDataLen = buflen;
		return EXIT_FAILURE;
	}
	for (;;) 
	{
		// Save the position of the beginning of the message.
		*pFoundMsg = ptr;

		// Expected total message length.
		*pFoundMsgLen = GetBinLengthSeanet(*pFoundMsg)+6;

		*pRemainingData = *pFoundMsg+*pFoundMsgLen;
		*pRemainingDataLen = len-*pFoundMsgLen;

		// Search just after the message.
		if (FindMsgWithMIDSeanet(*pRemainingData, *pRemainingDataLen, 
			mid, &ptr, &len) != EXIT_SUCCESS)
		{
			break;
		}
	}

	return EXIT_SUCCESS;
}

//// Should be able to use previously remaining data and output any new remaining data...
//inline int GetLatestMsgWithRemainingSeanet(SEANET* pSeanet, int mid, 
//										   unsigned char* databuf, int databuflen, int* pNbdatabytes, 
//										   unsigned char** pRemainingData, int* pRemainingDataLen);

// We suppose that read operations return when a message has just been completely sent, and not randomly.
inline int GetLatestMsgSeanet(SEANET* pSeanet, int mid, unsigned char* databuf, int databuflen, int* pNbdatabytes)
{
	unsigned char recvbuf[2*MAX_NB_BYTES_SEANET];
	unsigned char savebuf[MAX_NB_BYTES_SEANET];
	int BytesReceived = 0, Bytes = 0, recvbuflen = 0;
	unsigned char* ptr = NULL;
	int len = 0;
	unsigned char* remainingdata = NULL;
	int remainingdatalen = 0;
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	memset(savebuf, 0, sizeof(savebuf));
	recvbuflen = MAX_NB_BYTES_SEANET-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	if (ReadRS232Port(&pSeanet->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
	{
		printf("Error reading data from a Seanet. \n");
		return EXIT_FAILURE;
	}
	BytesReceived += Bytes;

	if (BytesReceived >= recvbuflen)
	{
		// If the buffer is full and if the device always sends data, there might be old data to discard...

		while (Bytes == recvbuflen)
		{
			if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_SEANET)
			{
				printf("Error reading data from a Seanet : Message timeout. \n");
				return EXIT_TIMEOUT;
			}
			memcpy(savebuf, recvbuf, Bytes);
			if (ReadRS232Port(&pSeanet->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
			{
				printf("Error reading data from a Seanet. \n");
				return EXIT_FAILURE;
			}
			BytesReceived += Bytes;
		}

		// The desired message should be among all the data gathered, unless there was 
		// so many other messages sent after that the desired message was in the 
		// discarded data, or we did not wait enough...

		memmove(recvbuf+recvbuflen-Bytes, recvbuf, Bytes);
		memcpy(recvbuf, savebuf+Bytes, recvbuflen-Bytes);

		// Only the last recvbuflen bytes received should be taken into account in what follows.
		BytesReceived = recvbuflen;
	}

	// The data need to be analyzed and we must check if we need to get more data from 
	// the device to get the desired message.
	// But normally we should not have to get more data unless we did not wait enough
	// for the desired message...

	while (FindLatestMsgWithMIDSeanet(recvbuf, BytesReceived, mid, &ptr, &len, &remainingdata, &remainingdatalen) != EXIT_SUCCESS)
	{
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_SEANET)
		{
			printf("Error reading data from a Seanet : Message timeout. \n");
			return EXIT_TIMEOUT;
		}
		// The last character must be a 0 to be a valid string for sscanf.
		if (BytesReceived >= 2*MAX_NB_BYTES_SEANET-1)
		{
			printf("Error reading data from a Seanet : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadRS232Port(&pSeanet->RS232Port, (unsigned char*)recvbuf+BytesReceived, 2*MAX_NB_BYTES_SEANET-1-BytesReceived, &Bytes) != EXIT_SUCCESS)
		{
			printf("Error reading data from a Seanet. \n");
			return EXIT_FAILURE;
		}
		BytesReceived += Bytes;
	}

	if (remainingdatalen > 0)
	{
		printf("Error getting data from a Seanet : Unexpected data after a message. \n");
	}

	// Get data bytes.

	memset(databuf, 0, databuflen);
	*pNbdatabytes = len;

	// Check the number of data bytes before copy.
	if (databuflen < *pNbdatabytes)
	{
		printf("Error getting data from a Seanet : Too small data buffer. \n");
		return EXIT_FAILURE;
	}

	// Copy the data bytes of the message.
	if (*pNbdatabytes > 0)
	{
		memcpy(databuf, ptr, *pNbdatabytes);
	}

	return EXIT_SUCCESS;
}

// Note : flush does not exist in TCP mode...
// Possible improvement : mix this function with the other version that discards a lot of data...
// But is it really possible?
//
// GetMsgSeanet() partout sauf pour alive (surtout le 1er)? Pas forcément, seulement quand il y a risque 
// qu'un autre message important suive celui qu'on est en train de recevoir...
//
inline int GetMsgSeanet(SEANET* pSeanet, int mid, unsigned char* databuf, int databuflen, int* pNbdatabytes)
{
	unsigned char recvbuf[MAX_NB_BYTES_SEANET];
	int i = 0;
	int BytesReceived = 0, recvbuflen = 0;
	unsigned char* ptr = NULL;
	int len = 0;
	unsigned char* remainingdata = NULL;
	int remainingdatalen = 0;
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = MAX_NB_BYTES_SEANET-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	// Suppose that there are not so many data to discard.
	// First try to get directly the desired message...

	if (ReadAllRS232Port(&pSeanet->RS232Port, recvbuf, MIN_MESSAGE_LEN_SEANET) != EXIT_SUCCESS)
	{
		printf("Error reading data from a Seanet. \n");
		return EXIT_FAILURE;
	}
	BytesReceived += MIN_MESSAGE_LEN_SEANET;

	i = 0;
	while (AnalyseBufHdrHexBinLenMIDSeanet(recvbuf+i, BytesReceived-i, mid, &len) != EXIT_SUCCESS)
	{
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_SEANET)
		{
			printf("Error reading data from a Seanet : Message timeout. \n");
			return EXIT_TIMEOUT;
		}
		if (BytesReceived+1 > recvbuflen)
		{
			printf("Error reading data from a Seanet : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadAllRS232Port(&pSeanet->RS232Port, recvbuf+BytesReceived, 1) != EXIT_SUCCESS)
		{
			printf("Error reading data from a Seanet. \n");
			return EXIT_FAILURE;
		}
		BytesReceived += 1;
		i++;
	}

	// Notes :
	// BytesReceived+len-(BytesReceived-i) = len+i
	// BytesReceived = MIN_MESSAGE_LEN_SEANET+i
	// len-MIN_MESSAGE_LEN_SEANET = len-(BytesReceived-i)
	if (len+i > recvbuflen)
	{
		printf("Error reading data from a Seanet : Invalid data. \n");
		return EXIT_INVALID_DATA;
	}

	// Read the remaining bytes of the message according to the length found.
	if (len-(BytesReceived-i) > 0)
	{
		if (ReadAllRS232Port(&pSeanet->RS232Port, recvbuf+BytesReceived, len-(BytesReceived-i)) != EXIT_SUCCESS)
		{
			printf("Error reading data from a Seanet. \n");
			return EXIT_FAILURE;
		}
		BytesReceived += (len-(BytesReceived-i));
	}

	if (FindLatestMsgWithMIDSeanet(recvbuf, BytesReceived, mid, 
		&ptr, &len, &remainingdata, &remainingdatalen) != EXIT_SUCCESS)
	{
		printf("Error reading data from a Seanet : Invalid data. \n");
		return EXIT_INVALID_DATA;
	}

	if (remainingdatalen > 0)
	{
		printf("Error getting data from a Seanet : Unexpected data after a message. \n");
	}

	// Get data bytes.

	memset(databuf, 0, databuflen);
	*pNbdatabytes = len;

	// Check the number of data bytes before copy.
	if (databuflen < *pNbdatabytes)
	{
		printf("Error getting data from a Seanet : Too small data buffer. \n");
		return EXIT_FAILURE;
	}

	// Copy the data bytes of the message.
	if (*pNbdatabytes > 0)
	{
		memcpy(databuf, ptr, *pNbdatabytes);
	}

	return EXIT_SUCCESS;
}

// Same as GetLatestMsgSeanet() but do not look for a specific MID and return the mid found.
inline int GetLatestMsgWithoutMIDSeanet(SEANET* pSeanet, int* pMid, unsigned char* databuf, int databuflen, int* pNbdatabytes)
{
	unsigned char recvbuf[2*MAX_NB_BYTES_SEANET];
	unsigned char savebuf[MAX_NB_BYTES_SEANET];
	int BytesReceived = 0, Bytes = 0, recvbuflen = 0;
	unsigned char* ptr = NULL;
	int len = 0;
	unsigned char* remainingdata = NULL;
	int remainingdatalen = 0;
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	memset(savebuf, 0, sizeof(savebuf));
	recvbuflen = MAX_NB_BYTES_SEANET-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	if (ReadRS232Port(&pSeanet->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
	{
		printf("Error reading data from a Seanet. \n");
		return EXIT_FAILURE;
	}
	BytesReceived += Bytes;

	if (BytesReceived >= recvbuflen)
	{
		// If the buffer is full and if the device always sends data, there might be old data to discard...

		while (Bytes == recvbuflen)
		{
			if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_SEANET)
			{
				printf("Error reading data from a Seanet : Message timeout. \n");
				return EXIT_TIMEOUT;
			}
			memcpy(savebuf, recvbuf, Bytes);
			if (ReadRS232Port(&pSeanet->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
			{
				printf("Error reading data from a Seanet. \n");
				return EXIT_FAILURE;
			}
			BytesReceived += Bytes;
		}

		// The desired message should be among all the data gathered, unless there was 
		// so many other messages sent after that the desired message was in the 
		// discarded data, or we did not wait enough...

		memmove(recvbuf+recvbuflen-Bytes, recvbuf, Bytes);
		memcpy(recvbuf, savebuf+Bytes, recvbuflen-Bytes);

		// Only the last recvbuflen bytes received should be taken into account in what follows.
		BytesReceived = recvbuflen;
	}

	// The data need to be analyzed and we must check if we need to get more data from 
	// the device to get the desired message.
	// But normally we should not have to get more data unless we did not wait enough
	// for the desired message...

	while (FindLatestMsgSeanet(recvbuf, BytesReceived, &ptr, &len, &remainingdata, &remainingdatalen) != EXIT_SUCCESS)
	{
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_SEANET)
		{
			printf("Error reading data from a Seanet : Message timeout. \n");
			return EXIT_TIMEOUT;
		}
		// The last character must be a 0 to be a valid string for sscanf.
		if (BytesReceived >= 2*MAX_NB_BYTES_SEANET-1)
		{
			printf("Error reading data from a Seanet : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadRS232Port(&pSeanet->RS232Port, (unsigned char*)recvbuf+BytesReceived, 2*MAX_NB_BYTES_SEANET-1-BytesReceived, &Bytes) != EXIT_SUCCESS)
		{
			printf("Error reading data from a Seanet. \n");
			return EXIT_FAILURE;
		}
		BytesReceived += Bytes;
	}

	if (remainingdatalen > 0)
	{
		printf("Error getting data from a Seanet : Unexpected data after a message. \n");
	}

	// Get data bytes.

	memset(databuf, 0, databuflen);
	*pNbdatabytes = len;

	// Check the number of data bytes before copy.
	if (databuflen < *pNbdatabytes)
	{
		printf("Error getting data from a Seanet : Too small data buffer. \n");
		return EXIT_FAILURE;
	}

	// Copy the data bytes of the message.
	if (*pNbdatabytes > 0)
	{
		memcpy(databuf, ptr, *pNbdatabytes);
	}

	*pMid = databuf[10];

	return EXIT_SUCCESS;
}

// Same as GetMsgSeanet() but do not look for a specific MID and return the mid found.
inline int GetMsgWithoutMIDSeanet(SEANET* pSeanet, int* pMid, unsigned char* databuf, int databuflen, int* pNbdatabytes)
{
	unsigned char recvbuf[MAX_NB_BYTES_SEANET];
	int i = 0;
	int BytesReceived = 0, recvbuflen = 0;
	unsigned char* ptr = NULL;
	int len = 0;
	unsigned char* remainingdata = NULL;
	int remainingdatalen = 0;
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = MAX_NB_BYTES_SEANET-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	// Suppose that there are not so many data to discard.
	// First try to get directly the desired message...

	if (ReadAllRS232Port(&pSeanet->RS232Port, recvbuf, MIN_MESSAGE_LEN_SEANET) != EXIT_SUCCESS)
	{
		printf("Error reading data from a Seanet. \n");
		return EXIT_FAILURE;
	}
	BytesReceived += MIN_MESSAGE_LEN_SEANET;

	i = 0;
	while (AnalyseBufHdrHexBinLenSeanet(recvbuf+i, BytesReceived-i, &len) != EXIT_SUCCESS)
	{
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_SEANET)
		{
			printf("Error reading data from a Seanet : Message timeout. \n");
			return EXIT_TIMEOUT;
		}
		if (BytesReceived+1 > recvbuflen)
		{
			printf("Error reading data from a Seanet : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadAllRS232Port(&pSeanet->RS232Port, recvbuf+BytesReceived, 1) != EXIT_SUCCESS)
		{
			printf("Error reading data from a Seanet. \n");
			return EXIT_FAILURE;
		}
		BytesReceived += 1;
		i++;
	}

	// Notes :
	// BytesReceived+len-(BytesReceived-i) = len+i
	// BytesReceived = MIN_MESSAGE_LEN_SEANET+i
	// len-MIN_MESSAGE_LEN_SEANET = len-(BytesReceived-i)
	if (len+i > recvbuflen)
	{
		printf("Error reading data from a Seanet : Invalid data. \n");
		return EXIT_INVALID_DATA;
	}

	// Read the remaining bytes of the message according to the length found.
	if (len-(BytesReceived-i) > 0)
	{
		if (ReadAllRS232Port(&pSeanet->RS232Port, recvbuf+BytesReceived, len-(BytesReceived-i)) != EXIT_SUCCESS)
		{
			printf("Error reading data from a Seanet. \n");
			return EXIT_FAILURE;
		}
		BytesReceived += (len-(BytesReceived-i));
	}

	if (FindLatestMsgSeanet(recvbuf, BytesReceived, 
		&ptr, &len, &remainingdata, &remainingdatalen) != EXIT_SUCCESS)
	{
		printf("Error reading data from a Seanet : Invalid data. \n");
		return EXIT_INVALID_DATA;
	}

	if (remainingdatalen > 0)
	{
		printf("Error getting data from a Seanet : Unexpected data after a message. \n");
	}

	// Get data bytes.

	memset(databuf, 0, databuflen);
	*pNbdatabytes = len;

	// Check the number of data bytes before copy.
	if (databuflen < *pNbdatabytes)
	{
		printf("Error getting data from a Seanet : Too small data buffer. \n");
		return EXIT_FAILURE;
	}

	// Copy the data bytes of the message.
	if (*pNbdatabytes > 0)
	{
		memcpy(databuf, ptr, *pNbdatabytes);
	}

	*pMid = databuf[10];

	return EXIT_SUCCESS;
}

inline int GetLatestAliveMsgSeanet(SEANET* pSeanet)
{
	unsigned char databuf[22];
	int nbdatabytes = 0;

	// Wait for a mtAlive message. It should come every 1 second.
	memset(databuf, 0, sizeof(databuf));
	nbdatabytes = 0;
	if (GetLatestMsgSeanet(pSeanet, mtAlive, databuf, sizeof(databuf), &nbdatabytes)
		!= EXIT_SUCCESS)
	{ 
		printf("A Seanet is not responding correctly. \n");
		return EXIT_FAILURE;	
	}

	// HeadInf.
	pSeanet->HeadInf = (unsigned char)databuf[20];

	return EXIT_SUCCESS;
}

/*
Reboot a sonar. This should take several seconds.
*/
inline int RebootSeanet(SEANET* pSeanet)
{
	unsigned char reqbuf[] = {MESSAGE_HEADER_SEANET,'0','0','0','8',0x08,0x00,
		SERIAL_PORT_PROGRAM_NODE_NUMBER_SEANET,SERIAL_PORT_SONAR_NODE_NUMBER_SEANET,
		0x03,mtReboot,0x80,SERIAL_PORT_SONAR_NODE_NUMBER_SEANET,MESSAGE_TERMINATOR_SEANET};
	unsigned char databuf[22];
	int nbdatabytes = 0;
	CHRONO chrono;

	// Send mtReboot message.
	if (WriteAllRS232Port(&pSeanet->RS232Port, reqbuf, sizeof(reqbuf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a Seanet. \n");
		return EXIT_FAILURE;
	}

	mSleep(1000); // Wait for the device to reboot.

	StartChrono(&chrono);

	// Check mtAlive message HeadInf byte with Transducer Centred (bit 1=1), not Motoring (bit 2=0), not SentCfg (bit 7=0).
	do
	{
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_REBOOT_SEANET)
		{
			printf("Error reading data from a Seanet : Reboot timeout. \n");
			return EXIT_TIMEOUT;
		}

		// Wait for a mtAlive message. It should come every 1 second.
		memset(databuf, 0, sizeof(databuf));
		nbdatabytes = 0;
		if (GetLatestMsgSeanet(pSeanet, mtAlive, databuf, sizeof(databuf), &nbdatabytes)
			!= EXIT_SUCCESS)
		{ 
			printf("A Seanet is not responding correctly. \n");
			return EXIT_FAILURE;	
		}

		// HeadInf.
		pSeanet->HeadInf = (unsigned char)databuf[20];

		//printf("mtAlive message databuf[20]=%#x\n", (int)databuf[20]);
	}
	while (!(databuf[20]&0x02)||(databuf[20]&0x04)||(databuf[20]&0x80));

	return EXIT_SUCCESS;
}

/*
Get version data of a sonar. This message must be sent to be able to use the 
device although we do not try to interprete the data received.
*/
inline int GetVersionSeanet(SEANET* pSeanet)
{
	unsigned char reqbuf[] = {MESSAGE_HEADER_SEANET,'0','0','0','8',0x08,0x00,
		SERIAL_PORT_PROGRAM_NODE_NUMBER_SEANET,SERIAL_PORT_SONAR_NODE_NUMBER_SEANET,
		0x03,mtSendVersion,0x80,SERIAL_PORT_SONAR_NODE_NUMBER_SEANET,MESSAGE_TERMINATOR_SEANET};
	unsigned char databuf[25];
	int nbdatabytes = 0;

	// Send mtSendVersion message.
	if (WriteAllRS232Port(&pSeanet->RS232Port, reqbuf, sizeof(reqbuf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a Seanet. \n");
		return EXIT_FAILURE;
	}

	// Wait for a mtVersionData message.
	memset(databuf, 0, sizeof(databuf));
	nbdatabytes = 0;
	if (GetLatestMsgSeanet(pSeanet, mtVersionData, databuf, sizeof(databuf), &nbdatabytes)
		!= EXIT_SUCCESS)
	{ 
		printf("A Seanet is not responding correctly. \n");
		return EXIT_FAILURE;	
	}

	return EXIT_SUCCESS;
}

/*
Get various settings of a sonar. This message must be sent to be 
able to use the device and to get information about it.
*/
inline int GetBBUserSeanet(SEANET* pSeanet)
{
	unsigned char reqbuf[] = {MESSAGE_HEADER_SEANET,'0','0','0','8',0x08,0x00,
		SERIAL_PORT_PROGRAM_NODE_NUMBER_SEANET,SERIAL_PORT_SONAR_NODE_NUMBER_SEANET,
		0x03,mtSendBBUser,0x80,SERIAL_PORT_SONAR_NODE_NUMBER_SEANET,MESSAGE_TERMINATOR_SEANET};
	unsigned char databuf[264];
	int nbdatabytes = 0;
	CHRONO chrono;

	// Send mtSendBBUser message.
	if (WriteAllRS232Port(&pSeanet->RS232Port, reqbuf, sizeof(reqbuf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a Seanet. \n");
		return EXIT_FAILURE;
	}

	// Wait for a mtBBUserData message.
	memset(databuf, 0, sizeof(databuf));
	nbdatabytes = 0;
	//if (GetLatestMsgSeanet(pSeanet, mtBBUserData, databuf, sizeof(databuf), &nbdatabytes)
	//	!= EXIT_SUCCESS)
	if (GetMsgSeanet(pSeanet, mtBBUserData, databuf, sizeof(databuf), &nbdatabytes)
		!= EXIT_SUCCESS)
	{ 
		printf("A Seanet is not responding correctly. \n");
		return EXIT_FAILURE;	
	}

	if (databuf[19] == 15) 
	{
		pSeanet->bDST = TRUE;
		if (databuf[146] == 1) pSeanet->bHalfDuplex = TRUE; else pSeanet->bHalfDuplex = FALSE;

		// There is a risk here that mtFpgaCalibrationData and mtFpgaVersionData go as unexpected data 
		// for the mtBBUserData if the computer is too slow...
		// Should just wait for mtAlive message (instead of mtFpgaCalibrationData and mtFpgaVersionData) 
		// to be sure? Or should use GetMsgSeanet()?

		// mtFpgaCalibrationData and mtFpgaVersionData messages should follow the mtBBuserData reply.
		memset(databuf, 0, sizeof(databuf));
		nbdatabytes = 0;
		//if (GetLatestMsgSeanet(pSeanet, mtFpgaCalibrationData, databuf, sizeof(databuf), &nbdatabytes)
		//	!= EXIT_SUCCESS)
		if (GetMsgSeanet(pSeanet, mtFpgaCalibrationData, databuf, sizeof(databuf), &nbdatabytes)
			!= EXIT_SUCCESS)
		{ 
			printf("A Seanet is not responding correctly. \n");
			return EXIT_FAILURE;	
		}
		memset(databuf, 0, sizeof(databuf));
		nbdatabytes = 0;
		//if (GetLatestMsgSeanet(pSeanet, mtFpgaVersionData, databuf, sizeof(databuf), &nbdatabytes)
		//	!= EXIT_SUCCESS)
		if (GetMsgSeanet(pSeanet, mtFpgaVersionData, databuf, sizeof(databuf), &nbdatabytes)
			!= EXIT_SUCCESS)
		{ 
			printf("A Seanet is not responding correctly. \n");
			return EXIT_FAILURE;	
		}
	}
	else
	{
		pSeanet->bDST = FALSE;
		if (databuf[99] == 1) pSeanet->bHalfDuplex = TRUE; else pSeanet->bHalfDuplex = FALSE;
	}

	StartChrono(&chrono);

	// Check mtAlive message HeadInf byte with NoParams (bit 6=1), SentCfg (bit 7=1).
	do
	{
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_BBUSER_SEANET)
		{
			printf("Error reading data from a Seanet : BBUser timeout. \n");
			return EXIT_TIMEOUT;
		}

		// Wait for a mtAlive message. It should come every 1 second.
		memset(databuf, 0, sizeof(databuf));
		nbdatabytes = 0;
		if (GetLatestMsgSeanet(pSeanet, mtAlive, databuf, sizeof(databuf), &nbdatabytes)
			!= EXIT_SUCCESS)
		{ 
			printf("A Seanet is not responding correctly. \n");
			return EXIT_FAILURE;	
		}

		// HeadInf.
		pSeanet->HeadInf = (unsigned char)databuf[20];

		//printf("mtAlive message databuf[20]=%#x\n", (int)databuf[20]);
	}
	while (!(databuf[20]&0x40)||!(databuf[20]&0x80));

	return EXIT_SUCCESS;
}

inline int SetHeadCommandScanReversalSeanet(SEANET* pSeanet)
{
	unsigned char reqbuf[15];

	reqbuf[0] = (unsigned char)MESSAGE_HEADER_SEANET;
	reqbuf[1] = (unsigned char)'0'; // Hex Length of whole binary packet excluding LF Terminator in ASCII
	reqbuf[2] = (unsigned char)'0'; // Hex Length of whole binary packet excluding LF Terminator in ASCII
	reqbuf[3] = (unsigned char)'0'; // Hex Length of whole binary packet excluding LF Terminator in ASCII
	reqbuf[4] = (unsigned char)'9'; // Hex Length of whole binary packet excluding LF Terminator in ASCII
	reqbuf[5] = (unsigned char)0x09; // Binary Word of above Hex Length (LSB)
	reqbuf[6] = (unsigned char)0x00; // Binary Word of above Hex Length (MSB)
	reqbuf[7] = (unsigned char)SERIAL_PORT_PROGRAM_NODE_NUMBER_SEANET; // Packet Source Identification (Tx Node number 0 - 255)
	reqbuf[8] = (unsigned char)SERIAL_PORT_SONAR_NODE_NUMBER_SEANET; // Packet Destination Identification (Rx Node number 0 - 255)
	reqbuf[9] = (unsigned char)0x04; // Byte Count of attached message that follows this byte
	reqbuf[10] = (unsigned char)mtHeadCommand; // Command / Reply Message = mtHeadCommand
	reqbuf[11] = (unsigned char)0x80; // Message Sequence, always = 0x80
	reqbuf[12] = (unsigned char)SERIAL_PORT_SONAR_NODE_NUMBER_SEANET; // Node number, copy of 8
	reqbuf[13] = (unsigned char)0x0F; // If the device is Dual Channel (i.e. SeaKing sonar) then a 16-byte V3B Gain Parameter block
	// is appended at the end and this byte must be set to 0x1D to indicate this. Else, for Single 
	// channel devices such as SeaPrince and MiniKing, the V3B block is not appended and this byte
	// is set to 0x01 to indicate this (Extra Types (see Appendix): "30" = Reduced Command with Gain
	// Parameters only, "15" = Scan Reverse Command (no Head Parameters attached))
	reqbuf[14] = (unsigned char)MESSAGE_TERMINATOR_SEANET; // Message Terminator = Line Feed

	// Send mtHeadCommand message.
	if (WriteAllRS232Port(&pSeanet->RS232Port, reqbuf, sizeof(reqbuf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a Seanet. \n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

inline int SetHeadCommandGainBSeanet(SEANET* pSeanet, int Gain, int Sensitivity, int Contrast)
{
	unsigned char reqbuf[31];
	uShort_SEANET word;

	// Additional bounds checking should be added...

	// Setting various parameters.

	pSeanet->Gain = Gain;
	pSeanet->Sensitivity = Sensitivity;
	pSeanet->Contrast = Contrast;

	pSeanet->ADSpan = 255*pSeanet->Contrast/80; // ADSpan
	pSeanet->ADLow = 255*pSeanet->Sensitivity/80; // ADLow
	// The sonar receiver has an 80dB dynamic range, and signal 
	// levels are processed internally by the sonar head such that
	// 0 .. 80dB = 0 .. 255
	// If adc8on in HdCtrl = 0, then the 80dB receiver signal is mapped to 4-bit, 16 level reply data
	// values, to a display dynamic range defined by ADSpan and ADLow such that:
	// ADSpan = 255 * Span(dB) / 80
	// ADLow = 255 * Low(dB) / 80
	// For example, ADSpan = 38 (12dB), ADLow = 40 (13dB)
	// 4-bit Data Values 0..15 = Signal Amplitudes 0 = 13dB, 15 = 25dB
	// ADSpan = 77 (24 dB) and ADLow = 40 (13dB) are typical values
	// If adc8on = 1 then the full 8-bit, 80dB dynamic range data bin amplitudes are returned to
	// the user:
	// 8-bit data Values 0..255 = Signal Amplitudes 0 = 0dB, 255 = 80dB

	// Initial Gain of the receiver (in units 0..210 = 0..+80dB = 0..100%)
	pSeanet->IGain = pSeanet->Gain*210/100;
	if (pSeanet->IGain > 210)
	{
		pSeanet->IGain = 210;
		printf("Too high Gain value. \n");
	}
	if (pSeanet->IGain < 0)
	{
		pSeanet->IGain = 0;
		printf("Negative Gain value. \n");
	}

	// Slope setting for each channel in 1/255 units
	pSeanet->Slope = (pSeanet->bDST?0x0064:0x007D);

	reqbuf[0] = (unsigned char)MESSAGE_HEADER_SEANET;
	reqbuf[1] = (unsigned char)'0'; // Hex Length of whole binary packet excluding LF Terminator in ASCII
	reqbuf[2] = (unsigned char)'0'; // Hex Length of whole binary packet excluding LF Terminator in ASCII
	reqbuf[3] = (unsigned char)'1'; // Hex Length of whole binary packet excluding LF Terminator in ASCII
	reqbuf[4] = (unsigned char)'9'; // Hex Length of whole binary packet excluding LF Terminator in ASCII
	reqbuf[5] = (unsigned char)0x19; // Binary Word of above Hex Length (LSB)
	reqbuf[6] = (unsigned char)0x00; // Binary Word of above Hex Length (MSB)
	reqbuf[7] = (unsigned char)SERIAL_PORT_PROGRAM_NODE_NUMBER_SEANET; // Packet Source Identification (Tx Node number 0 - 255)
	reqbuf[8] = (unsigned char)SERIAL_PORT_SONAR_NODE_NUMBER_SEANET; // Packet Destination Identification (Rx Node number 0 - 255)
	reqbuf[9] = (unsigned char)0x14; // Byte Count of attached message that follows this byte
	reqbuf[10] = (unsigned char)mtHeadCommand; // Command / Reply Message = mtHeadCommand
	reqbuf[11] = (unsigned char)0x80; // Message Sequence, always = 0x80
	reqbuf[12] = (unsigned char)SERIAL_PORT_SONAR_NODE_NUMBER_SEANET; // Node number, copy of 8
	reqbuf[13] = (unsigned char)0x1E; // If the device is Dual Channel (i.e. SeaKing sonar) then a 16-byte V3B Gain Parameter block
	// is appended at the end and this byte must be set to 0x1D to indicate this. Else, for Single 
	// channel devices such as SeaPrince and MiniKing, the V3B block is not appended and this byte
	// is set to 0x01 to indicate this (Extra Types (see Appendix): "30" = Reduced Command with Gain
	// Parameters only, "15" = Scan Reverse Command (no Head Parameters attached))
	reqbuf[14] = (unsigned char)pSeanet->ADSpan; // ADSpan for channel 1
	reqbuf[15] = (unsigned char)pSeanet->ADSpan; // ADSpan for channel 2
	reqbuf[16] = (unsigned char)pSeanet->ADLow; // ADLow for channel 1
	reqbuf[17] = (unsigned char)pSeanet->ADLow; // ADLow for channel 2
	reqbuf[18] = (unsigned char)pSeanet->IGain; // Initial Gain of the receiver for channel 1 in units 0..210 = 0..+80dB = 0..100% (default = 0x49)
	reqbuf[19] = (unsigned char)pSeanet->IGain; // Initial Gain of the receiver for channel 2 in units 0..210 = 0..+80dB = 0..100% (default = 0x49)
	reqbuf[20] = (unsigned char)0x00; // Spare
	reqbuf[21] = (unsigned char)0x00; // Spare
	word.v = (unsigned short)pSeanet->Slope;
	reqbuf[22] = (unsigned char)word.c[0]; // Slope setting for channel 1 in 1/255 units (LSB)
	reqbuf[23] = (unsigned char)word.c[1]; // Slope setting for channel 1 in 1/255 units (MSB)
	reqbuf[24] = (unsigned char)word.c[0]; // Slope setting for channel 2 in 1/255 units (LSB)
	reqbuf[25] = (unsigned char)word.c[1]; // Slope setting for channel 2 in 1/255 units (MSB)
	// For a channel at 675 kHz, Default Slope = 125
	reqbuf[26] = (unsigned char)0x00; // Slope Delay for channel 1
	reqbuf[27] = (unsigned char)0x00; // Slope Delay for channel 1
	reqbuf[28] = (unsigned char)0x00; // Slope Delay for channel 2
	reqbuf[29] = (unsigned char)0x00; // Slope Delay for channel 2

	// No documentation for Slope Delay, seems to be 0 in some examples (similar parameters as V3B)...

	reqbuf[30] = (unsigned char)MESSAGE_TERMINATOR_SEANET; // Message Terminator = Line Feed

	// Send mtHeadCommand message.
	if (WriteAllRS232Port(&pSeanet->RS232Port, reqbuf, sizeof(reqbuf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a Seanet. \n");
		return EXIT_FAILURE;
	}

	// Should check in the next mtHeadData reply if the parameters were really changed...

	return EXIT_SUCCESS;
}

/*
Set various parameters in a Seanet. This message is important to be able to 
interprete the sonar data.
*/
inline int SetHeadCommandSeanet(SEANET* pSeanet)
{
	unsigned char reqbuf[66];
	unsigned char databuf[22];
	int nbdatabytes = 0;
	CHRONO chrono;
	uShort_SEANET word;
	double d = 0;

	// Additional bounds checking should be added...

	// Setting various parameters.

	pSeanet->HdCtrl.bits.adc8on = pSeanet->adc8on; // The head will return 4-bit packed echo data (0..15) representing the amplitude
	// of received echoes in a databin if it is set to 0. Otherwise, it will be in 8 bits (0..255)
	pSeanet->HdCtrl.bits.cont = (pSeanet->ScanWidth == 360)?1:0; // Scanning will be restricted to a sector defined by the directions LeftAngleLimit 
	// and RightAngleLimit if it is set to 0. Otherwise, it will be a continuous rotation and 
	// LeftAngleLimit and RightAngleLimit will be ignored
	pSeanet->HdCtrl.bits.scanright = pSeanet->scanright;
	pSeanet->HdCtrl.bits.invert = pSeanet->invert;
	pSeanet->HdCtrl.bits.motoff = 0;
	pSeanet->HdCtrl.bits.txoff = 0;
	pSeanet->HdCtrl.bits.spare = 0;
	pSeanet->HdCtrl.bits.chan2 = 0;
	pSeanet->HdCtrl.bits.raw = 1;
	pSeanet->HdCtrl.bits.hasmot = 1;
	pSeanet->HdCtrl.bits.applyoffset = 0;
	pSeanet->HdCtrl.bits.pingpong = 0;
	pSeanet->HdCtrl.bits.stareLLim = pSeanet->stareLLim;
	pSeanet->HdCtrl.bits.ReplyASL = 1;
	pSeanet->HdCtrl.bits.ReplyThr = 0;
	pSeanet->HdCtrl.bits.IgnoreSensor = 0;
	// Bit 0 : adc8on (0=4bit DataBins, 1=8Bit) = 1
	// Bit 1 : cont (0=SectorScan, 1=Continuous) = 1
	// Bit 2 : scanright (ScanDirection 0=Left, 1=Right) = 1
	// Bit 3 : invert (0=Upright, 1=Inverted Orientation) = 1
	// Bit 4 : motoff (0=MotorOn, 1=MotorOff) = 0
	// Bit 5 : txoff (0=Tx on, 1=Tx off. For Test) = 0
	// Bit 6 : spare (0=Normal by default) = 0
	// Bit 7 : chan2 (hSON 0=Use Chan1, 1=Use Chan2) = 0
	// Bit 8 : raw (0=CookedADCmode, 1=RawADC) = 1
	// Bit 9 : hasmot (0=NoMotor, 1=HasMotor) = 1
	// Bit 10 : applyoffset (1=Applied Hdgoffset, 0=Ignore) = 0
	// Bit 11 : pingpong (1=pingpong Chan1/Chan2 e.g. hSSS) = 0
	// Bit 12 : stareLLim (1=Don't Scan, Point at LeftLim) = 0
	// Bit 13 : ReplyASL (1=ASLin ReplyRec, 0=NotIn) = 1
	// Bit 14 : ReplyThr (1=hThrRec Requested) = 0
	// Bit 15 : IgnoreSensor (1=Ignore the Centre Sensor) 0

	if (pSeanet->RangeScale <= 0)	
	{
		pSeanet->RangeScale = 30;
		printf("Negative RangeScale value. Default value of 30 m used. \n");
	}

	pSeanet->LeftAngleLimit = (pSeanet->ScanDirection-pSeanet->ScanWidth/2+360)%360;
	pSeanet->RightAngleLimit = (pSeanet->ScanDirection+pSeanet->ScanWidth/2)%360;

	pSeanet->LeftLim = ((int)((pSeanet->LeftAngleLimit*10.0/9.0)*16.0+3200))%6400;
	pSeanet->RightLim = ((int)((pSeanet->RightAngleLimit*10.0/9.0)*16.0+3200))%6400;

	// Example :
	//  dir 0 deg
	//  width 90 deg
	//  left 315 deg (5600 1/16 grad)
	//  right 45 deg (800 1/16 grad)
	//  (ahead = 3200 1/16 grad)
	//  =>
	//  leftlim 2400 1/16 grad
	//  rightlim 4000 1/16 grad

	pSeanet->ADSpan = 255*pSeanet->Contrast/80; // ADSpan
	pSeanet->ADLow = 255*pSeanet->Sensitivity/80; // ADLow
	// The sonar receiver has an 80dB dynamic range, and signal 
	// levels are processed internally by the sonar head such that
	// 0 .. 80dB = 0 .. 255
	// If adc8on in HdCtrl = 0, then the 80dB receiver signal is mapped to 4-bit, 16 level reply data
	// values, to a display dynamic range defined by ADSpan and ADLow such that:
	// ADSpan = 255 * Span(dB) / 80
	// ADLow = 255 * Low(dB) / 80
	// For example, ADSpan = 38 (12dB), ADLow = 40 (13dB)
	// 4-bit Data Values 0..15 = Signal Amplitudes 0 = 13dB, 15 = 25dB
	// ADSpan = 77 (24 dB) and ADLow = 40 (13dB) are typical values
	// If adc8on = 1 then the full 8-bit, 80dB dynamic range data bin amplitudes are returned to
	// the user:
	// 8-bit data Values 0..255 = Signal Amplitudes 0 = 0dB, 255 = 80dB

	// Initial Gain of the receiver (in units 0..210 = 0..+80dB = 0..100%)
	pSeanet->IGain = pSeanet->Gain*210/100;
	if (pSeanet->IGain > 210)
	{
		pSeanet->IGain = 210;
		printf("Too high Gain value. \n");
	}
	if (pSeanet->IGain < 0)
	{
		pSeanet->IGain = 0;
		printf("Negative Gain value. \n");
	}

	// Slope setting for each channel in 1/255 units
	pSeanet->Slope = (pSeanet->bDST?0x0064:0x007D);

	pSeanet->Resolution = STEP_ANGLE_SIZE_IN_DEGREES2RESOLUTION(pSeanet->StepAngleSize);
	pSeanet->NSteps = STEP_ANGLE_SIZE_IN_DEGREES2NUMBER_OF_STEPS(pSeanet->StepAngleSize);

	reqbuf[0] = (unsigned char)MESSAGE_HEADER_SEANET;
	reqbuf[1] = (unsigned char)'0'; // Hex Length of whole binary packet excluding LF Terminator in ASCII
	reqbuf[2] = (unsigned char)'0'; // Hex Length of whole binary packet excluding LF Terminator in ASCII
	reqbuf[3] = (unsigned char)'3'; // Hex Length of whole binary packet excluding LF Terminator in ASCII
	reqbuf[4] = (unsigned char)'C'; // Hex Length of whole binary packet excluding LF Terminator in ASCII
	reqbuf[5] = (unsigned char)0x3C; // Binary Word of above Hex Length (LSB)
	reqbuf[6] = (unsigned char)0x00; // Binary Word of above Hex Length (MSB)
	reqbuf[7] = (unsigned char)SERIAL_PORT_PROGRAM_NODE_NUMBER_SEANET; // Packet Source Identification (Tx Node number 0 - 255)
	reqbuf[8] = (unsigned char)SERIAL_PORT_SONAR_NODE_NUMBER_SEANET; // Packet Destination Identification (Rx Node number 0 - 255)
	reqbuf[9] = (unsigned char)0x37; // Byte Count of attached message that follows this byte (Set to 0 (zero) in ‘mtHeadData’
	// reply to indicate Multi-packet mode NOT used by device) = 55
	reqbuf[10] = (unsigned char)mtHeadCommand; // Command / Reply Message = mtHeadCommand
	reqbuf[11] = (unsigned char)0x80; // Message Sequence, always = 0x80
	reqbuf[12] = (unsigned char)SERIAL_PORT_SONAR_NODE_NUMBER_SEANET; // Node number, copy of 8
	reqbuf[13] = (unsigned char)0x01; // If the device is Dual Channel (i.e. SeaKing sonar) then a 16-byte V3B Gain Parameter block
	// is appended at the end and this byte must be set to 0x1D to indicate this. Else, for Single 
	// channel devices such as SeaPrince and MiniKing, the V3B block is not appended and this byte
	// is set to 0x01 to indicate this (Extra Types (see Appendix): "30" = Reduced Command with Gain
	// Parameters only, "15" = Scan Reverse Command (no Head Parameters attached))
	reqbuf[14] = (unsigned char)pSeanet->HdCtrl.uc[0]; // HdCtrl (LSB)
	reqbuf[15] = (unsigned char)pSeanet->HdCtrl.uc[1]; // HdCtrl (MSB)
	reqbuf[16] = (unsigned char)(pSeanet->bDST?0x11:0x02); // Device Type (0x02 = Imaging sonar, 0x11 = DST)

	//Frequency = 675000; // Transmitter Frequency in Hertz, to add in configuration file?
	//Should define QWORD union...
	//TXN = Frequency*pow(2,32)/32000000;
	//RXN = (Frequency+455000)*pow(2,32)/32000000;

	reqbuf[17] = (unsigned char)0x66; // Transmitter numbers for channel 1
	reqbuf[18] = (unsigned char)0x66; // Transmitter numbers for channel 1
	reqbuf[19] = (unsigned char)0x66; // Transmitter numbers for channel 1
	reqbuf[20] = (unsigned char)0x05; // Transmitter numbers for channel 1
	reqbuf[21] = (unsigned char)0x66; // Transmitter numbers for channel 2
	reqbuf[22] = (unsigned char)0x66; // Transmitter numbers for channel 2
	reqbuf[23] = (unsigned char)0x66; // Transmitter numbers for channel 2
	reqbuf[24] = (unsigned char)0x05; // Transmitter numbers for channel 2
	// F * 2^32 / 32e6 with F = Transmitter Frequency in Hertz (675 kHz)
	reqbuf[25] = (unsigned char)0x70; // Receiver numbers for channel 1
	reqbuf[26] = (unsigned char)0x3D; // Receiver numbers for channel 1
	reqbuf[27] = (unsigned char)0x0A; // Receiver numbers for channel 1
	reqbuf[28] = (unsigned char)0x09; // Receiver numbers for channel 1
	reqbuf[29] = (unsigned char)0x70; // Receiver numbers for channel 2
	reqbuf[30] = (unsigned char)0x3D; // Receiver numbers for channel 2
	reqbuf[31] = (unsigned char)0x0A; // Receiver numbers for channel 2
	reqbuf[32] = (unsigned char)0x09; // Receiver numbers for channel 2
	// (F + 455000) * 2^32 / 32e6 with F = Transmitter Frequency in Hertz (675 kHz)
	d = (double)((pSeanet->RangeScale + 10.0) * 25.0 / 10.0);
	word.v = (unsigned short)d;
	if (word.v > 350)	
	{
		word.v = 350;
		printf("Too high RangeScale value. \n");
	}
	if (word.v < 37)	
	{
		word.v = 37;
		printf("Too low RangeScale value. \n");
	}
	reqbuf[33] = (unsigned char)word.c[0]; // Transmitter Pulse Length in microseconds units (LSB)
	reqbuf[34] = (unsigned char)word.c[1]; // Transmitter Pulse Length in microseconds units (MSB)
	// TxPulseLen = [RangeScale(m) + Ofs] * Mul / 10 (Use defaults; Ofs = 10, Mul =25)
	// Should be constrained to between 50 .. 350
	// microseconds. A typical value is 100 microseconds
	word.v = (unsigned short)(pSeanet->RangeScale * 10);
	reqbuf[35] = (unsigned char)word.c[0]; // Range Scale setting in decimetre units (LSB)
	reqbuf[36] = (unsigned char)word.c[1]; // Range Scale setting in decimetre units (MSB)
	// The low order 14 bits are set to a value of RangeScale * 10 units.
	// Bit 6, Bit 7 of the MSB are used as a
	// code (0..3) for the Range Scale units : 
	// 0 = Metres, 1 = Feet, 2 = Fathoms, 3 = Yards
	// For example, RangeScale = 30 m
	word.v = (unsigned short)pSeanet->LeftLim;
	reqbuf[37] = (unsigned char)word.c[0]; // Left Angle Limit in 1/16 Gradian units (LSB) (overridden if bit cont of byte HdCtrl is set)
	reqbuf[38] = (unsigned char)word.c[1]; // Left Angle Limit in 1/16 Gradian units (MSB) (overridden if bit cont of byte HdCtrl is set)
	word.v = (unsigned short)pSeanet->RightLim;
	reqbuf[39] = (unsigned char)word.c[0]; // Right Angle Limit in 1/16 Gradian units (LSB) (overridden if bit cont of byte HdCtrl is set)
	reqbuf[40] = (unsigned char)word.c[1]; // Right Angle Limit in 1/16 Gradian units (MSB) (overridden if bit cont of byte HdCtrl is set)
	// The SeaKing direction convention is as follows :
	//  Left 90 deg = 1600
	//  Ahead = 3200
	//  Right 90 deg = 4800
	//  Astern = 0 (or 6399)
	reqbuf[41] = (unsigned char)pSeanet->ADSpan; // ADSpan
	reqbuf[42] = (unsigned char)pSeanet->ADLow; // ADLow
	reqbuf[43] = (unsigned char)pSeanet->IGain; // Initial Gain of the receiver for channel 1 in units 0..210 = 0..+80dB = 0..100% (default = 0x49)
	reqbuf[44] = (unsigned char)pSeanet->IGain; // Initial Gain of the receiver for channel 2 in units 0..210 = 0..+80dB = 0..100% (default = 0x49)
	word.v = (unsigned short)pSeanet->Slope;
	reqbuf[45] = (unsigned char)word.c[0]; // Slope setting for channel 1 in 1/255 units (LSB)
	reqbuf[46] = (unsigned char)word.c[1]; // Slope setting for channel 1 in 1/255 units (MSB)
	reqbuf[47] = (unsigned char)word.c[0]; // Slope setting for channel 2 in 1/255 units (LSB)
	reqbuf[48] = (unsigned char)word.c[1]; // Slope setting for channel 2 in 1/255 units (MSB)
	// For a channel at 675 kHz, Default Slope = 125
	reqbuf[49] = (unsigned char)0x19; // Motor Step Delay Time : high speed limit of the scanning motor in units of 10 microseconds, typically = 25
	reqbuf[50] = (unsigned char)pSeanet->Resolution; // Motor Step Angle Size : scanning motor step angle between pings in 1/16 Gradian units
	d = (double)(((pSeanet->RangeScale * 2.0 / pSeanet->NBins) / pSeanet->VelocityOfSound) / 0.000000640);
	word.v = (unsigned short)d;
	if (word.v < 5)	
	{
		word.v = 5;
		printf("Too low ADInterval value. Invalid RangeScale, NBins or VelocityOfSound values. \n");
	}
	reqbuf[51] = (unsigned char)word.c[0]; // AD Interval in units of 640 nanoseconds (LSB)
	reqbuf[52] = (unsigned char)word.c[1]; // AD Interval in units of 640 nanoseconds (MSB)
	// Sampling Interval(s) = (RangeScale(m) * 2 / Number of Bins) / VOS (i.e. use Range *2 for Return Path)
	// with VOS = 1500 m/sec (Velocity Of Sound)
	// ADInterval = Sampling interval in units of 640 nanoseconds = Sampling Interval(s) / 640e-9
	// A practical minimum for ADInterval is about 5 (approximatively 3 microseconds)
	word.v = (unsigned short)pSeanet->NBins;
	reqbuf[53] = (unsigned char)word.c[0]; // Number of sample bins over scan-line (LSB)
	reqbuf[54] = (unsigned char)word.c[1]; // Number of sample bins over scan-line (MSB)
	reqbuf[55] = (unsigned char)0xE8; // MaxADbuf, default = 500, limit = 1000 (LSB)
	reqbuf[56] = (unsigned char)0x03; // MaxADbuf, default = 500, limit = 1000 (MSB)
	reqbuf[57] = (unsigned char)0x64; // Lockout period in microsecond units, default = 100 (LSB)
	reqbuf[58] = (unsigned char)0x00; // Lockout period in microsecond units, default = 100 (MSB)
	reqbuf[59] = (unsigned char)0x40; // Minor Axis of dual-axis device in 1/16 Gradian units (LSB)
	reqbuf[60] = (unsigned char)0x06; // Minor Axis of dual-axis device in 1/16 Gradian units (MSB)
	// For the standard (Single Axis) devices they should be fixed at 1600
	reqbuf[61] = (unsigned char)0x01; // Major Axis in 1/16 Gradian units. Always 1 for sonar
	reqbuf[62] = (unsigned char)0x00; // Ctl2, extra sonar Control Functions to be implemented for operating and test purposes
	reqbuf[63] = (unsigned char)0x00; // ScanZ, for Special devices and should both be left at default values of 0 (LSB)
	reqbuf[64] = (unsigned char)0x00; // ScanZ, for Special devices and should both be left at default values of 0 (MSB)
	reqbuf[65] = (unsigned char)MESSAGE_TERMINATOR_SEANET; // Message Terminator = Line Feed

	// Should always add V3B parameters (similar as GainB...)?

	// Send mtHeadCommand message.
	if (WriteAllRS232Port(&pSeanet->RS232Port, reqbuf, sizeof(reqbuf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a Seanet. \n");
		return EXIT_FAILURE;
	}

	StartChrono(&chrono);

	// Check mtAlive message HeadInf byte with not NoParams (bit 6=0).
	// Should check everything : Transducer Centred (bit 1=1), not Motoring (bit 2=0), NoParams (bit 6=0), SentCfg (bit 7=1)?
	do
	{
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_HEADCOMMAND_SEANET)
		{
			printf("Error reading data from a Seanet : HeadCommand timeout. \n");
			return EXIT_TIMEOUT;
		}

		// Wait for a mtAlive message. It should come every 1 second.
		memset(databuf, 0, sizeof(databuf));
		nbdatabytes = 0;
		if (GetLatestMsgSeanet(pSeanet, mtAlive, databuf, sizeof(databuf), &nbdatabytes)
			!= EXIT_SUCCESS)
		{ 
			printf("A Seanet is not responding correctly. \n");
			return EXIT_FAILURE;	
		}

		// HeadInf.
		pSeanet->HeadInf = (unsigned char)databuf[20];

		//printf("mtAlive message databuf[20]=%#x\n", (int)databuf[20]);
	}
	while (databuf[20]&0x40);
	//while (!(databuf[20]&0x02)||(databuf[20]&0x04)||(databuf[20]&0x40)||!(databuf[20]&0x80));

	return EXIT_SUCCESS;
}

inline int SendDataRequestSeanet(SEANET* pSeanet)
{
	unsigned char reqbuf[] = {MESSAGE_HEADER_SEANET,'0','0','0','C',0x0C,0x00,
		SERIAL_PORT_PROGRAM_NODE_NUMBER_SEANET,SERIAL_PORT_SONAR_NODE_NUMBER_SEANET,
		0x07,mtSendData,0x80,SERIAL_PORT_SONAR_NODE_NUMBER_SEANET,0x00,0x00,0x00,0x00,MESSAGE_TERMINATOR_SEANET};

	// Send mtSendData message.
	if (WriteAllRS232Port(&pSeanet->RS232Port, reqbuf, sizeof(reqbuf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a Seanet. \n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

inline int HeadDataReplySeanet(SEANET* pSeanet, unsigned char* scanline, double* pAngle)
{
	unsigned char databuf[MAX_NB_BYTES_SEANET];
	int nbdatabytes = 0;
	uShort_SEANET word;
	int j = 0;

	// Wait for a mtHeadData message.
	memset(databuf, 0, sizeof(databuf));
	nbdatabytes = 0;
	//if (GetLatestMsgSeanet(pSeanet, mtHeadData, databuf, sizeof(databuf), &nbdatabytes)
	//	!= EXIT_SUCCESS)
	if (GetMsgSeanet(pSeanet, mtHeadData, databuf, sizeof(databuf), &nbdatabytes)
		!= EXIT_SUCCESS)
	{ 
		printf("A Seanet is not responding correctly. \n");
		return EXIT_FAILURE;	
	}

	// Analyze data.

	//memset(pSeanetData, 0, sizeof(SEANETDATA));

	// Head Status.
	pSeanet->HeadStatus = (unsigned char)databuf[16];

	// HdCtrl.
	word.c[0] = (unsigned char)databuf[18]; // LSB.
	word.c[1] = (unsigned char)databuf[19]; // MSB.
	pSeanet->HeadHdCtrl.i = word.v;

	// Rangescale. Normally, we should check the unit but it should have been requested in m...
	word.c[0] = (unsigned char)databuf[20]; // LSB.
	word.c[1] = (unsigned char)databuf[21]; // MSB.
	pSeanet->HeadRangescale = word.v;

	// Gain.
	pSeanet->HeadIGain = (unsigned char)databuf[26];

	// Slope.
	word.c[0] = (unsigned char)databuf[27]; // LSB.
	word.c[1] = (unsigned char)databuf[28]; // MSB.
	pSeanet->HeadSlope = word.v;

	// ADSpan.
	pSeanet->HeadADSpan = (unsigned char)databuf[29];

	// ADLow.
	pSeanet->HeadADLow = (unsigned char)databuf[30];

	// ADInterval.
	word.c[0] = (unsigned char)databuf[33]; // LSB.
	word.c[1] = (unsigned char)databuf[34]; // MSB.
	pSeanet->HeadADInterval = word.v;

	// LeftLim.
	word.c[0] = (unsigned char)databuf[35]; // LSB.
	word.c[1] = (unsigned char)databuf[36]; // MSB.
	pSeanet->HeadLeftLim = word.v;

	// RightLim.
	word.c[0] = (unsigned char)databuf[37]; // LSB.
	word.c[1] = (unsigned char)databuf[38]; // MSB.
	pSeanet->HeadRightLim = word.v;

	// Steps.
	pSeanet->HeadSteps = (unsigned char)databuf[39];

	// Transducer Bearing. 
	// This is the position of the transducer for the current scanline (0..6399 in 1/16 Gradian units).

	// Ahead corresponds to 3200?

	word.c[0] = (unsigned char)databuf[40]; // LSB.
	word.c[1] = (unsigned char)databuf[41]; // MSB.
	*pAngle = ((word.v-3200+6400)%6400)*0.05625; // Angle of the transducer in degrees (0.05625 = (1/16)*(9/10)).

	// Dbytes.
	word.c[0] = (unsigned char)databuf[42]; // LSB.
	word.c[1] = (unsigned char)databuf[43]; // MSB.
	pSeanet->Dbytes = word.v; // Dbytes defines the number of Range Bins that the sonar will generate for the Head Data reply message.

	// There are 44 bytes of Message Header and Device Parameter Block.

	if (44+pSeanet->Dbytes+1 != nbdatabytes)
	{ 
		printf("A Seanet is not responding correctly. \n");
		return EXIT_FAILURE;	
	}

	// We should take into account ADLow and ADSpan here?

	// Retrieve the scanline data from the data received.
	if (!pSeanet->adc8on)	
	{ 
		// If adc8on = 0, the scanlines data received are in 4 bit (1/2 byte).
		// The amplitude values are between 0 and 15 so we multiply by 16
		// to get values between 0 and 255.
		for (j = 0; j < pSeanet->Dbytes; j++)
		{
			scanline[2*j+0] = (unsigned char)((databuf[j+44]>>4)*16);
			scanline[2*j+1] = (unsigned char)(((databuf[j+44]<<4)>>4)*16);
		}
		pSeanet->HeadNBins = 2*pSeanet->Dbytes;
	}
	else	
	{
		// If adc8on = 1, the scanlines data received are in 8 bit (1 byte).
		// The amplitude values are between 0 and 255.
		memcpy(scanline, databuf+44, pSeanet->Dbytes); // Copy the data received without the header (which was the 44 first bytes).
		pSeanet->HeadNBins = pSeanet->Dbytes;
	}

	// Last data...?

	return EXIT_SUCCESS;
}

// For daisy-chained device support...
// Only the last auxdatabuf is kept...
inline int HeadDataReplyAndAuxDataSeanet(SEANET* pSeanet, 
										 unsigned char* scanline, double* pAngle, 
										 unsigned char* strippedauxdatabuf, int* pNbstrippedauxdatabytes)
{
	unsigned char databuf[MAX_NB_BYTES_SEANET];
	int nbdatabytes = 0;
	int mid = 0;
	int i = 0;
	uShort_SEANET word;
	int j = 0;

	// Use this kind of while for all messages?
	while (mid != mtHeadData)
	{
		// Wait for a message.
		memset(databuf, 0, sizeof(databuf));
		nbdatabytes = 0;
		//if (GetLatestMsgWithoutMIDSeanet(pSeanet, &mid, databuf, sizeof(databuf), &nbdatabytes)
		//	!= EXIT_SUCCESS)
		if (GetMsgWithoutMIDSeanet(pSeanet, &mid, databuf, sizeof(databuf), &nbdatabytes)
			!= EXIT_SUCCESS)
		{ 
			printf("A Seanet is not responding correctly. \n");
			return EXIT_FAILURE;	
		}

		// Message counter...
		i++;
		if (i > 4) 
		{
			printf("A Seanet is not responding correctly. \n");
			return EXIT_FAILURE;	
		}

		if (mid == mtAuxData)
		{
			// Should remove also the final LF...

			//memcpy(auxdatabuf, databuf, nbdatabytes);
			//*pNbauxdatabytes = nbdatabytes;
			memcpy(strippedauxdatabuf, databuf+15, nbdatabytes-15);
			*pNbstrippedauxdatabytes = nbdatabytes-15;
		}
	}

	// Analyze data.

	//memset(pSeanetData, 0, sizeof(SEANETDATA));

	// Head Status.
	pSeanet->HeadStatus = (unsigned char)databuf[16];

	// HdCtrl.
	word.c[0] = (unsigned char)databuf[18]; // LSB.
	word.c[1] = (unsigned char)databuf[19]; // MSB.
	pSeanet->HeadHdCtrl.i = word.v;

	// Rangescale. Normally, we should check the unit but it should have been requested in m...
	word.c[0] = (unsigned char)databuf[20]; // LSB.
	word.c[1] = (unsigned char)databuf[21]; // MSB.
	pSeanet->HeadRangescale = word.v;

	// Gain.
	pSeanet->HeadIGain = (unsigned char)databuf[26];

	// Slope.
	word.c[0] = (unsigned char)databuf[27]; // LSB.
	word.c[1] = (unsigned char)databuf[28]; // MSB.
	pSeanet->HeadSlope = word.v;

	// ADSpan.
	pSeanet->HeadADSpan = (unsigned char)databuf[29];

	// ADLow.
	pSeanet->HeadADLow = (unsigned char)databuf[30];

	// ADInterval.
	word.c[0] = (unsigned char)databuf[33]; // LSB.
	word.c[1] = (unsigned char)databuf[34]; // MSB.
	pSeanet->HeadADInterval = word.v;

	// LeftLim.
	word.c[0] = (unsigned char)databuf[35]; // LSB.
	word.c[1] = (unsigned char)databuf[36]; // MSB.
	pSeanet->HeadLeftLim = word.v;

	// RightLim.
	word.c[0] = (unsigned char)databuf[37]; // LSB.
	word.c[1] = (unsigned char)databuf[38]; // MSB.
	pSeanet->HeadRightLim = word.v;

	// Steps.
	pSeanet->HeadSteps = (unsigned char)databuf[39];

	// Transducer Bearing. 
	// This is the position of the transducer for the current scanline (0..6399 in 1/16 Gradian units).

	// Ahead corresponds to 3200?

	word.c[0] = (unsigned char)databuf[40]; // LSB.
	word.c[1] = (unsigned char)databuf[41]; // MSB.
	*pAngle = ((word.v-3200+6400)%6400)*0.05625; // Angle of the transducer in degrees (0.05625 = (1/16)*(9/10)).

	// Dbytes.
	word.c[0] = (unsigned char)databuf[42]; // LSB.
	word.c[1] = (unsigned char)databuf[43]; // MSB.
	pSeanet->Dbytes = word.v; // Dbytes defines the number of Range Bins that the sonar will generate for the Head Data reply message.

	// There are 44 bytes of Message Header and Device Parameter Block.

	if (44+pSeanet->Dbytes+1 != nbdatabytes)
	{ 
		printf("A Seanet is not responding correctly. \n");
		return EXIT_FAILURE;	
	}

	// We should take into account ADLow and ADSpan here?

	// Retrieve the scanline data from the data received.
	if (!pSeanet->adc8on)	
	{ 
		// If adc8on = 0, the scanlines data received are in 4 bit (1/2 byte).
		// The amplitude values are between 0 and 15 so we multiply by 16
		// to get values between 0 and 255.
		for (j = 0; j < pSeanet->Dbytes; j++)
		{
			scanline[2*j+0] = (unsigned char)((databuf[j+44]>>4)*16);
			scanline[2*j+1] = (unsigned char)(((databuf[j+44]<<4)>>4)*16);
		}
		pSeanet->HeadNBins = 2*pSeanet->Dbytes;
	}
	else	
	{
		// If adc8on = 1, the scanlines data received are in 8 bit (1 byte).
		// The amplitude values are between 0 and 255.
		memcpy(scanline, databuf+44, pSeanet->Dbytes); // Copy the data received without the header (which was the 44 first bytes).
		pSeanet->HeadNBins = pSeanet->Dbytes;
	}

	// Last data...?

	return EXIT_SUCCESS;
}

/*
Get a scanline and the corresponding angle from a Seanet (in fact, 2 scanlines can be 
sent by the device for 1 request depending on the mode, but thanks to its buffering 
capabilities this is not a problem...).

unsigned char* scanline : (INOUT) Valid pointer that will receive the scanline.
double* pAngle : (INOUT) Valid pointer that will receive the angle of the 
scanline (in [0;360[ deg).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int GetHeadDataSeanet(SEANET* pSeanet, unsigned char* scanline, double* pAngle)
{
	if (SendDataRequestSeanet(pSeanet) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	if (HeadDataReplySeanet(pSeanet, scanline, pAngle) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	// Last data...?

	return EXIT_SUCCESS;
}

// For daisy-chained device support...
// Only the last auxdatabuf is kept...
inline int GetHeadDataAndAuxDataSeanet(SEANET* pSeanet, 
									   unsigned char* scanline, double* pAngle, 
									   unsigned char* strippedauxdatabuf, int* pNbstrippedauxdatabytes)
{
	if (SendDataRequestSeanet(pSeanet) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	if (HeadDataReplyAndAuxDataSeanet(pSeanet, scanline, pAngle, strippedauxdatabuf, pNbstrippedauxdatabytes) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	// Last data...?

	return EXIT_SUCCESS;
}

// SEANET must be initialized to 0 before (e.g. SEANET seanet; memset(&seanet, 0, sizeof(SEANET));)!
inline int ConnectSeanet(SEANET* pSeanet, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];

	memset(pSeanet->szCfgFilePath, 0, sizeof(pSeanet->szCfgFilePath));
	sprintf(pSeanet->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pSeanet->szDevPath, 0, sizeof(pSeanet->szDevPath));
		sprintf(pSeanet->szDevPath, "COM1");
		pSeanet->BaudRate = 115200;
		pSeanet->timeout = 1500;
		pSeanet->bSaveRawData = 1;
		pSeanet->RangeScale = 30;
		pSeanet->Gain = 50;
		pSeanet->Sensitivity = 3;
		pSeanet->Contrast = 25;
		pSeanet->ScanDirection = 0;
		pSeanet->ScanWidth = 360;
		pSeanet->StepAngleSize = 3.6;
		pSeanet->NBins = 200;
		pSeanet->adc8on = 1;
		pSeanet->scanright = 0;
		pSeanet->invert = 0;
		pSeanet->stareLLim = 0;
		pSeanet->VelocityOfSound = 1500;
		pSeanet->alpha_max_err = 0.01;
		pSeanet->d_max_err = 0.1;
		pSeanet->HorizontalBeam = 3;
		pSeanet->VerticalBeam = 40;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pSeanet->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSeanet->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSeanet->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSeanet->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSeanet->RangeScale) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSeanet->Gain) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSeanet->Sensitivity) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSeanet->Contrast) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSeanet->ScanDirection) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSeanet->ScanWidth) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pSeanet->StepAngleSize) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSeanet->NBins) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSeanet->adc8on) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSeanet->scanright) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSeanet->invert) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSeanet->stareLLim) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSeanet->VelocityOfSound) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pSeanet->alpha_max_err) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pSeanet->d_max_err) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSeanet->HorizontalBeam) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSeanet->VerticalBeam) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	if ((pSeanet->ScanDirection < 0)||(pSeanet->ScanDirection > 359))
	{
		printf("Invalid parameter : ScanDirection.\n");
		pSeanet->ScanDirection = 0;
	}
	if ((pSeanet->ScanWidth < 2)||(pSeanet->ScanWidth > 360))
	{
		printf("Invalid parameter : ScanWidth.\n");
		pSeanet->ScanWidth = 360;
	}
	if (pSeanet->VelocityOfSound <= 0)
	{
		printf("Invalid parameter : VelocityOfSound.\n");
		pSeanet->VelocityOfSound = 1500;
	}
	if ((pSeanet->HorizontalBeam <= 0)||(pSeanet->HorizontalBeam > 360))
	{
		printf("Invalid parameter : HorizontalBeam.\n");
		pSeanet->HorizontalBeam = 3;
	}
	if ((pSeanet->VerticalBeam <= 0)||(pSeanet->VerticalBeam > 360))
	{
		printf("Invalid parameter : VerticalBeam.\n");
		pSeanet->VerticalBeam = 40;
	}

	// Used to save raw data, should be handled specifically...
	//pSeanet->pfSaveFile = NULL;

	if (OpenRS232Port(&pSeanet->RS232Port, pSeanet->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to Seanet.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pSeanet->RS232Port, pSeanet->BaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)pSeanet->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to Seanet.\n");
		CloseRS232Port(&pSeanet->RS232Port);
		return EXIT_FAILURE;
	}

	if (GetLatestAliveMsgSeanet(pSeanet) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a Seanet.\n");
		CloseRS232Port(&pSeanet->RS232Port);
		return EXIT_FAILURE;
	}

	if (RebootSeanet(pSeanet) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a Seanet.\n");
		CloseRS232Port(&pSeanet->RS232Port);
		return EXIT_FAILURE;
	}

	if (GetVersionSeanet(pSeanet) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a Seanet.\n");
		CloseRS232Port(&pSeanet->RS232Port);
		return EXIT_FAILURE;
	}

	if (GetBBUserSeanet(pSeanet) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a Seanet.\n");
		CloseRS232Port(&pSeanet->RS232Port);
		return EXIT_FAILURE;
	}

	if (SetHeadCommandSeanet(pSeanet) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a Seanet.\n");
		CloseRS232Port(&pSeanet->RS232Port);
		return EXIT_FAILURE;
	}

	printf("Seanet connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectSeanet(SEANET* pSeanet)
{
	if (CloseRS232Port(&pSeanet->RS232Port) != EXIT_SUCCESS)
	{
		printf("Seanet disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("Seanet disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_SEANETTHREAD
THREAD_PROC_RETURN_VALUE SeanetThread(void* pParam);
#endif // DISABLE_SEANETTHREAD

#endif // SEANET_H
