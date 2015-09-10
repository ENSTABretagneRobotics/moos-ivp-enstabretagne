#include "seanetmsg.h"
#include <cstring>
#include <sstream>
#include <cstdio>

#define PRINT_DEBUG_WARNING_MINIKINGCORE(x) printf x 
#define PRINT_DEBUG_ERROR_MINIKINGCORE(x) printf x
#define strtime_m() 0
#define hDev 0

using namespace std;

SeaNetMsg::SeaNetMsg()
{
}

SeaNetMsg::SeaNetMsg(const std::string &data)
    : m_data(data)
{
}

SeaNetMsg::~SeaNetMsg()
{
}

void SeaNetMsg::print_hex(int max_bytes) const
{
  for (int k=0; k < min(max_bytes,(int)m_data.size());++k)
   	printf("%02x ", (unsigned char)m_data.data()[k]);
  if (max_bytes < (int)m_data.size())
    printf("... ");
  printf("[%dB]", (int)m_data.size());
}


bool SeaNetMsg::startsWithHeader(const std::string &data)
{
  if (data.empty()) return false;
  if (data[0] != '@') return false;
  return true;
}

int SeaNetMsg::readMsgLength(const std::string &data)
{
  // Not enough data to read and validate message length
  if (data.size() < 7) return mrNotEnoughData;
  
  // Read hex length
  uint16_t hex_length = 0;
  stringstream ss;
  ss << std::hex << data.substr(1, 4); // Bytes 2-5 are the message length in ASCII hex
  ss >> hex_length;
  
  // Read bin length
  uint16_t bin_length = * (uint16_t*) (data.data()+5);
  
  // Compare both values
  if (hex_length != bin_length) return mrNotAMessage;
  
  return hex_length;
}

SeaNetMsg::MessageTypeID SeaNetMsg::detectMessageType(const std::string &data)
{
  if (data.size() < 11) return mteNoMessageType;
  else return (MessageTypeID) (data[10]); 
}

int SeaNetMsg::numberBytesMissing(const std::string &data, int &full_length)
{
  // If no data, request the size of the smallest message
  if (data.empty()) 
    return 14;
      
  // Check presence of header '@'
  if (data[0] != '@')
    return mrNotAMessage;
  
  // Try to read message length field
  int length = readMsgLength(data);
  if (length == mrNotAMessage) return mrNotAMessage;
  if (length == mrNotEnoughData) return 14 - data.size();
  
  // length does not includes 5 first bytes and terminal '\n'
  full_length = length + 6;
  if (data.size() >= full_length)
    return 0;
  else
    return full_length - data.size();
}

SeaNetMsg * SeaNetMsg::fromData(const std::string &data)
{
  // TODO: Message factory
  return 0;
}

SeaNetMsg_HeadCommand::SeaNetMsg_HeadCommand()
    : SeaNetMsg()
{
  // Initial parameters
  params.adc8on = 1;
  params.cont = 0;//1;
  params.invert = 0;

  // Scan à droite
  params.LeftAngleLimit = 70.;
  params.RightAngleLimit = params.LeftAngleLimit+90.;
  // Scan à gauche
  params.LeftAngleLimit = 360 - 160.;
  params.RightAngleLimit = 360 - 70.;

  params.VelocityOfSound = 1500;
  params.RangeScale = 5;//25;
  params.StepAngleSize = 1.8;
  params.NBins = 400;
  params.IGain = 95;
  
  // Create message
  buildMessage();
}

void SeaNetMsg_HeadCommand::buildMessage()
{
  typedef uint8_t uint8;
  typedef uint16_t uint16;
	union uShort
	{
		uint16 v;  
		uint8 c[2];
	};
	typedef union uShort uShort;

  struct HdCtrlStruct
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
  
  union uHdCtrlUnion
  {
          struct HdCtrlStruct bits;  
          uint8 c[4];
  } HdCtrl;
        
//        CHRONO chrono;
  double duration = 0;
//	uint8 readbuf[22];
  
  uint8 writebuf[66];
//	uint8 buf = 0;
  uShort word;
  double d = 0;
  int adc8on = params.adc8on; // The head will return 4-bit packed echo data (0..15) representing the amplitude
  // of received echoes in a databin if it is set to 0. Otherwise, it will be in 8 bits (0..255)
  int cont = params.cont; // Scanning will be restricted to a sector defined by the directions LeftAngleLimit 
  // and RightAngleLimit if it is set to 0. Otherwise, it will be a continuous rotation and 
  // LeftAngleLimit and RightAngleLimit will be ignored
  double LeftAngleLimit = params.LeftAngleLimit; // For a sector scan (cont = 0), in degrees
  double RightAngleLimit = params.RightAngleLimit; // For a sector scan (cont = 0), in degrees
  int VelocityOfSound = params.VelocityOfSound; // In m/s
  int RangeScale = params.RangeScale; // In meters
  double StepAngleSize = params.StepAngleSize; // In degrees: Ultimate Resolution (0.225°), Mgr Resolution (0.45°), Medium Resolution (0.9°), Low Resolution (1.8°)
  int NBins = params.NBins; // Number of bins generated after a ping
  int IGain = params.IGain; // Initial Gain of the receiver (in units 0..210 = 0..+80dB = 0..100%)

  writebuf[0] = (uint8)0x40; // Message Header = @
  writebuf[1] = (uint8)0x30; // Hex Length of whole binary packet excluding LF Terminator in ASCII = '0'
  writebuf[2] = (uint8)0x30; // Hex Length of whole binary packet excluding LF Terminator in ASCII = '0'
  writebuf[3] = (uint8)0x33; // Hex Length of whole binary packet excluding LF Terminator in ASCII = '3'
  writebuf[4] = (uint8)0x43; // Hex Length of whole binary packet excluding LF Terminator in ASCII = 'C'
  writebuf[5] = (uint8)0x3C; // Binary Word of above Hex Length (LSB), Hex Length = 60
  writebuf[6] = (uint8)0x00; // Binary Word of above Hex Length (MSB), Hex Length = 60
  writebuf[7] = (uint8)0xFF; // Packet Source Identification (Tx Node number 0 - 255)
  writebuf[8] = (uint8)0x02; // Packet Destination Identification (Rx Node number 0 - 255)
  writebuf[9] = (uint8)0x37; // Byte Count of attached message that follows this byte (Set to 0 (zero) in mtHeadData
  // reply to indicate Multi-packet mode NOT used by device) = 55
  writebuf[10] = (uint8)0x13; // Command / Reply Message = mtHeadCommand
  writebuf[11] = (uint8)0x80; // Message Sequence, always = 0x80
  writebuf[12] = (uint8)0x02; // Node number, copy of 8
  writebuf[13] = (uint8)0x01; // If the device is Dual Channel (i.e. SeaKing SONAR) then a 16-byte V3B Gain Parameter block
  // is appended at the end and this byte must be set to 0x1D to indicate this. Else, for Single 
  // channel devices such as SeaPrince and MiniKing, the V3B block is not appended and this byte
  // is set to 0x01 to indicate this
//	buf = 0x0F;
//	if (!adc8on)    {
//		buf = buf & 0xFE; // adc8on = 0
//	}
//	if (!cont)      {
//		buf = buf & 0xFD; // cont = 0
//	}
//	writebuf[14] = (uint8)buf; // HdCtrl (LSB)
//	writebuf[15] = (uint8)0x23; // HdCtrl (MSB)

  HdCtrl.bits.adc8on = adc8on;
  HdCtrl.bits.cont = cont;
  HdCtrl.bits.scanright = 1;
  HdCtrl.bits.invert = 1;
  HdCtrl.bits.motoff = 0;
  HdCtrl.bits.txoff = 0;
  HdCtrl.bits.spare = 0;
  HdCtrl.bits.chan2 = 0;
  HdCtrl.bits.raw = 1;
  HdCtrl.bits.hasmot = 1;
  HdCtrl.bits.applyoffset = 0;
  HdCtrl.bits.pingpong = 0;
  HdCtrl.bits.stareLLim = 0;
  HdCtrl.bits.ReplyASL = 1;
  HdCtrl.bits.ReplyThr = 0;
  HdCtrl.bits.IgnoreSensor = 0;

  writebuf[14] = (uint8)HdCtrl.c[0]; // HdCtrl (LSB)
  writebuf[15] = (uint8)HdCtrl.c[1]; // HdCtrl (MSB)
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
  writebuf[16] = (uint8)11;//0x02; // Device Type (0x02 = Imaging SONAR, '11' for DST)
  writebuf[17] = (uint8)0x66; // Transmitter numbers for channel 1
  writebuf[18] = (uint8)0x66; // Transmitter numbers for channel 1
  writebuf[19] = (uint8)0x66; // Transmitter numbers for channel 1
  writebuf[20] = (uint8)0x05; // Transmitter numbers for channel 1
  writebuf[21] = (uint8)0x66; // Transmitter numbers for channel 2
  writebuf[22] = (uint8)0x66; // Transmitter numbers for channel 2
  writebuf[23] = (uint8)0x66; // Transmitter numbers for channel 2
  writebuf[24] = (uint8)0x05; // Transmitter numbers for channel 2
  // F * 2^32 / 32e6 with F = Transmitter Frequency in Hertz (675 kHz)
  // This sonar has only 1 channel so channel 1 = channel 2
  writebuf[25] = (uint8)0x70; // Receiver numbers for channel 1
  writebuf[26] = (uint8)0x3D; // Receiver numbers for channel 1
  writebuf[27] = (uint8)0x0A; // Receiver numbers for channel 1
  writebuf[28] = (uint8)0x09; // Receiver numbers for channel 1
  writebuf[29] = (uint8)0x70; // Receiver numbers for channel 2
  writebuf[30] = (uint8)0x3D; // Receiver numbers for channel 2
  writebuf[31] = (uint8)0x0A; // Receiver numbers for channel 2
  writebuf[32] = (uint8)0x09; // Receiver numbers for channel 2
  // (F + 455000) * 2^32 / 32e6 with F = Transmitter Frequency in Hertz (675 kHz)
  // This sonar has only 1 channel so channel 1 = channel 2
  if (RangeScale < 0)     {
    RangeScale = 30;
    PRINT_DEBUG_WARNING_MINIKINGCORE(("SetParamMiniKing warning (%s) : %s"
            "(hDev=%#x)\n", 
            strtime_m(), 
            "Negative RangeScale value. Default value of 30 m used. ", 
            hDev));
  }
  d = (double)((RangeScale + 10.0) * 25.0 / 10.0);
  word.v = (uint16)d;
  if (word.v > 350)       {
    word.v = 350;
    PRINT_DEBUG_WARNING_MINIKINGCORE(("SetParamMiniKing warning (%s) : %s"
            "(hDev=%#x)\n", 
            strtime_m(), 
            "Too high RangeScale value. ", 
            hDev));
  }
  if (word.v < 37)        {
    word.v = 37;
    PRINT_DEBUG_WARNING_MINIKINGCORE(("SetParamMiniKing warning (%s) : %s"
            "(hDev=%#x)\n", 
            strtime_m(), 
            "Too low RangeScale value. ", 
            hDev));
  }
  writebuf[33] = (uint8)word.c[0]; // Transmitter Pulse Length in microseconds units (LSB)
  writebuf[34] = (uint8)word.c[1]; // Transmitter Pulse Length in microseconds units (MSB)
  // TxPulseLen = [RangeScale(m) + Ofs] * Mul / 10 (Use defaults; Ofs = 10, Mul =25)
  // Should be constrained to between 50 .. 350
  // microseconds. A typical value is 100 microseconds
  word.v = (uint16)(RangeScale * 10);
  writebuf[35] = (uint8)word.c[0]; // Range Scale setting in decimetre units (LSB)
  writebuf[36] = (uint8)word.c[1]; // Range Scale setting in decimetre units (MSB)
  // The low order 14 bits are set to a value of Rangescale * 10 units.
  // Bit 6, Bit 7 of the MSB are used as a
  // code (0..3) for the Range Scale units : 
  // 0 = Metres, 1 = Feet, 2 = Fathoms, 3 = Yards
  // For example, Rangescale = 30 m
  d = (LeftAngleLimit * 10.0 / 9.0) * 16.0;
  word.v = ((uint16)d)%6400;
//	if (word.v > 6399)
//	{
//		word.v = 6399;
//		PRINT_DEBUG_WARNING_MINIKINGCORE(("SetParamMiniKing warning (%s) : %s"
//				"(hDev=%#x)\n", 
//				strtime_m(), 
//				"Too high LeftAngleLimit value. ", 
//				hDev));
//	}
  writebuf[37] = (uint8)word.c[0]; // Left Angle Limit in 1/16 Gradian units (LSB) (overridden if bit cont of byte HdCtrl is set)
  writebuf[38] = (uint8)word.c[1]; // Left Angle Limit in 1/16 Gradian units (MSB) (overridden if bit cont of byte HdCtrl is set)
  d = (RightAngleLimit * 10.0 / 9.0) * 16.0;
  word.v = ((uint16)d)%6400;
//	if (word.v > 6399)
//	{
//		word.v = 6399;
//			PRINT_DEBUG_WARNING_MINIKINGCORE(("SetParamMiniKing warning (%s) : %s"
//			"(hDev=%#x)\n", 
//			strtime_m(), 
//			"Too high RightAngleLimit value. ", 
//			hDev));
//	}
  writebuf[39] = (uint8)word.c[0]; // Right Angle Limit in 1/16 Gradian units (LSB) (overridden if bit cont of byte HdCtrl is set)
  writebuf[40] = (uint8)word.c[1]; // Right Angle Limit in 1/16 Gradian units (MSB) (overridden if bit cont of byte HdCtrl is set)
  writebuf[41] = (uint8)0x4D; // ADSpan
  writebuf[42] = (uint8)0x28; // ADLow
  // The SONAR receiver has an 80dB dynamic range, and signal 
  // levels are processed internally by the SONAR head such that
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
  if (IGain > 210)
  {
    IGain = 210;
    PRINT_DEBUG_WARNING_MINIKINGCORE(("SetParamMiniKing warning (%s) : %s"
            "(hDev=%#x)\n", 
            strtime_m(), 
            "Too high Gain value. ", 
            hDev));
  }
  if (IGain < 0)
  {
    IGain = 0;
    PRINT_DEBUG_WARNING_MINIKINGCORE(("SetParamMiniKing warning (%s) : %s"
            "(hDev=%#x)\n", 
            strtime_m(), 
            "Negative Gain value. ", 
            hDev));
  }
  writebuf[43] = (uint8)IGain; // Initial Gain of the receiver for channel 1 in units 0..210 = 0..+80dB = 0..100% (default = 0x49)
  writebuf[44] = (uint8)IGain; // Initial Gain of the receiver for channel 2 in units 0..210 = 0..+80dB = 0..100% (default = 0x49)
  writebuf[45] = (uint8)0x7D; // Slope setting for channel 1 in 1/255 units (LSB)
  writebuf[46] = (uint8)0x00; // Slope setting for channel 1 in 1/255 units (MSB)
  writebuf[47] = (uint8)0x7D; // Slope setting for channel 2 in 1/255 units (LSB)
  writebuf[48] = (uint8)0x00; // Slope setting for channel 2 in 1/255 units (MSB)
  // This sonar has only 1 channel so channel 1 = channel 2
  // For a channel at 675 kHz, Default Slope = 125
  writebuf[49] = (uint8)0x19; // Motor Step Delay Time : high speed limit of the scanning motor in units of 10 microseconds, typically = 25
  d = (double)((StepAngleSize * 10.0 / 9.0) * 16.0);
  writebuf[50] = (uint8)d; // Motor Step Angle Size : scanning motor step angle between pings in 1/16 Gradian units
  // Low Resolution = 32 (= 2 Gradians = 1.8 °)
  // Medium Resolution = 16 (= 1 Gradian = 0.9°)
  // High Resolution = 8 (= 0.5 Gradian = 0.45°)
  // Ultimate Resolution = 4 (= 0.25 Gradian = 0.225°)
  d = (double)(((RangeScale * 2.0 / NBins) / VelocityOfSound) / 0.000000640);
  word.v = (uint16)d;
  if (word.v < 5) {
    word.v = 5;
    PRINT_DEBUG_WARNING_MINIKINGCORE(("SetParamMiniKing warning (%s) : %s"
            "(hDev=%#x)\n", 
            strtime_m(), 
            "Too low ADInterval value. Invalid RangeScale, NBins or VelocityOfSound values. ", 
            hDev));
  }
  writebuf[51] = (uint8)word.c[0]; // AD Interval in units of 640 nanoseconds (LSB)
  writebuf[52] = (uint8)word.c[1]; // AD Interval in units of 640 nanoseconds (MSB)
  // Sampling Interval(s) = (RangeScale(m) * 2 / Number of Bins) / VOS (i.e. use Range *2 for Return Path)
  // with VOS = 1500 m/sec (Velocity Of Sound)
  // ADInterval = Sampling interval in units of 640 nanoseconds = Sampling Interval(s) / 640e-9
  // A practical minimum for ADInterval is about 5 (approximatively 3 microseconds)
  word.v = (uint16)NBins;
  if (word.v > 800)       {
    word.v = 800;
    PRINT_DEBUG_WARNING_MINIKINGCORE(("SetParamMiniKing warning (%s) : %s"
            "(hDev=%#x)\n", 
            strtime_m(), 
            "Too high NBins value. ", 
            hDev));
  }
  if (word.v < 5) {
    word.v = 5;
    PRINT_DEBUG_WARNING_MINIKINGCORE(("SetParamMiniKing warning (%s) : %s"
            "(hDev=%#x)\n", 
            strtime_m(), 
            "Too low NBins value. ", 
            hDev));
  }
  writebuf[53] = (uint8)word.c[0]; // Number of sample bins over scan-line (LSB)
  writebuf[54] = (uint8)word.c[1]; // Number of sample bins over scan-line (MSB)
  // The maximum value of NBins is limited to 800
  writebuf[55] = (uint8)0xE8; // MaxADbuf, default = 500, limit = 1000 (LSB)
  writebuf[56] = (uint8)0x03; // MaxADbuf, default = 500, limit = 1000 (MSB)
  writebuf[57] = (uint8)0x64; // Lockout period in microsecond units, default = 100 (LSB)
  writebuf[58] = (uint8)0x00; // Lockout period in microsecond units, default = 100 (MSB)
  writebuf[59] = (uint8)0x40; // Minor Axis of dual-axis device in 1/16 Gradian units (LSB)
  writebuf[60] = (uint8)0x06; // Minor Axis of dual-axis device in 1/16 Gradian units (MSB)
  // For the standard (Single Axis) devices they should be fixed at 1600
  writebuf[61] = (uint8)0x01; // Major Axis in 1/16 Gradian units. Always 1 for SONAR
  writebuf[62] = (uint8)0x00; // Ctl2, extra SONAR Control Functions to be implemented for operating and test purposes
  writebuf[63] = (uint8)0x00; // ScanZ, for Special devices and should both be left at default values of 0 (LSB)
  writebuf[64] = (uint8)0x00; // ScanZ, for Special devices and should both be left at default values of 0 (MSB)
  writebuf[65] = (uint8)0x0A; // Message Terminator = Line Feed
/*
  if (WriteAllRS232Port(hDev, writebuf, 66) != EXIT_SUCCESS)
  { 
    PRINT_DEBUG_ERROR_MINIKINGCORE(("SetParamMiniKing error (%s) : %s"
            "(hDev=%#x)\n", 
            strtime_m(), 
            "Error writing data to the serial port. ", 
            hDev));
    return EXIT_FAILURE;
  }
*/
  // On power-up of the sonar, the 'HeadInf' byte will change in value to 
  // reflect the different states that the sonar is going through during initialisation
  // i) Power applied to sonar
  // 1st Alive: HeadInf = '5D' (Head is in Re-Centre operation and has No Params). Byte 14 = 80h.
  // 2nd Alive: HeadInf = '4D' (No 'Dir' so Transducer is back at centre)
  // 3rd Alive: HeadInf = '4A' (Transducer is now centred and Not Motoring)
  // ii) 'mtHeadCommand' sent to Sonar
  // 4th Alive: HeadInf = 'CA' (Has been 'Sent Cfg'. Acknowledgement of 'ParamsCmd')
  // 5th Alive: HeadInf = '8A' (Has Params. Parameters have been validated. Ready for 'GetData').
  // Byte 14 now set to 00h
        
/*
  StartChrono(&chrono);

  readbuf[20] = 0;

  while (readbuf[20] != 0x8A)
  {
          GetTimeElapsedChrono(&chrono, &duration);
          if (duration > 10)
          {
                  PRINT_DEBUG_ERROR_MINIKINGCORE(("SetParamMiniKing error (%s) : %s"
                          "(hDev=%#x)\n", 
                          strtime_m(), 
                          "The operation timed out. ", 
                          hDev));
                  return EXIT_FAILURE;
          }

          // Wait for an mtAlive message. It should come every 1 second.
          if (ReadMsgMiniKing(hDev, readbuf, 22, 0x04) != EXIT_SUCCESS)
          { 
                  PRINT_DEBUG_ERROR_MINIKINGCORE(("SetParamMiniKing error (%s) : %s"
                          "(hDev=%#x)\n", 
                          strtime_m(), 
                          "The device is not responding correctly. ", 
                          hDev));
                  return EXIT_FAILURE;
          }

          PRINT_DEBUG_MESSAGE_MINIKINGCORE(("mtAlive message readbuf[20]=%#x\n", readbuf[20]));
  }
*/
//        return EXIT_SUCCESS;
  m_data.assign((char*)writebuf, sizeof(writebuf));
}



double SeaNetMsg_HeadData::firstObstacleDist(uint8_t threshold, const double& minDist, const double& maxDist) const
{
  //int minBin = max(0, (int)(minDist*(double)NBins/(double)RangeScale));
  //int maxBin = min(NBins, (int)(maxDist*(double)NBins/(double)RangeScale));
  double binStep = ADInterval_m();
  
  int minBin = max(0, (int)(minDist / binStep));
  int maxBin = min(nBins(), (int)(maxDist / binStep));
  
  int i = minBin;
  while ((i < maxBin) && (bin(i) < threshold))
  {
    i++;
  }

  if (i >= maxBin)
  { 
    return -1;
  }
  else    
  {
    //return (double)i*(double)RangeScale/(double)NBins; // Convert in m.
    return (double)i * binStep; // Convert in m.
  }
}
