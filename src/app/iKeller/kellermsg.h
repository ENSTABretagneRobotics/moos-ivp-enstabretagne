#ifndef KELLERMSG_H
#define KELLERMSG_H

#include <string>
#include <stdint.h>

typedef unsigned char uint8;

class KellerMsg
{
protected:
  std::string m_data;

public:
  enum MessageReadError {
    mrNotAMessage = -1,
    mrNotEnoughData = -2,
  };

  enum MessageTypeID {
    msgNoMessageType = -1,
    msgReadOutCalibFloat = 30,
    msgWriteCalibFloat = 31,
    msgReadOutConfig = 32,
    msgWriteConfig = 33,
    msgInitDevice= 48,
    msgWriteBusAdresses = 66,
    msgReadOutSerial = 69,
    msgReadOutPressureTemperatureFloat = 73,
    msgReadOutPressureTemperatureInt = 74,
    msgZeroingFunctions = 95,

  };

protected:
  uint8_t read_uchar_at(int offset) const {return *(uint8_t *)(m_data.data()+offset);}
  uint16_t read_ushort_at(int offset) const {return *(uint16_t *)(m_data.data()+offset);}
  uint32_t read_ulong_at(int offset) const {return *(uint32_t *)(m_data.data()+offset);}

  void set_uchar_at(int offset, uint8_t value) {m_data[offset] = value;}
  void set_ushort_at(int offset, uint16_t value) {m_data[offset] = value & 0xFF; m_data[offset+1] = (value & 0xFF00) >> 8;}
  void set_ulong_at(int offset, uint32_t value) {m_data[offset] = value & 0xFF; m_data[offset+1] = (value & 0xFF00) >> 8;  m_data[offset+2] = (value & 0xFF0000) >> 16; m_data[offset+3] = (value & 0xFF000000) >> 24; }

public:
  KellerMsg();
  KellerMsg(const std::string &data);
  virtual ~KellerMsg();

  const std::string & data() const {return m_data;};
  MessageTypeID messageType() const {return (MessageTypeID) read_uchar_at(2);}
  void setMessageType(MessageTypeID msg_type) {set_uchar_at(2, (uint8_t)msg_type);}
  void print_hex(int max_bytes = 50) const;

public: // Static member functions
  static bool startsWithHeader(const std::string &data);
  static MessageTypeID detectMessageType(const std::string &data);
  static int numberBytesMissing(const std::string &data, int &full_length);
};

class KellerMsg_InitDeviceRequest : public KellerMsg
{
public:
  KellerMsg_InitDeviceRequest() : KellerMsg() {
    const char msg[] = {0xfa,0x30,0x04,0x43};
    m_data.assign(msg, sizeof(msg));
  }
};
class KellerMsg_ReadOutPressureTemperatureFloatRequest : public KellerMsg
{
public:
  KellerMsg_ReadOutPressureTemperatureFloatRequest() : KellerMsg() {
    const char msg[] = {0xfa,0x49,0x01,0xa1,0xa7};
    m_data.assign(msg, sizeof(msg));
  }
};

// class KellerMsg_InitDeviceResponse : public KellerMsg
// {
// public:
//   KellerMsg_InitDeviceResponse() : KellerMsg() {};

//   // Parameters of the device
//   struct KELLERPARAMS       {
//     uint8 Class;  // Device ID code, 5: Series 30 digital pressure transmitter (33, 35, 36, 39)
//     uint8 Group;  // Subdivision within a device class
//                   // 1: Series 30 transmitter from 1999 or later
//                   // 20: Series 30 transmitter from 2002 or later
//                   // 21: Series 30 transmitter version X2
//     uint8 Year;   // Firmware version
//     uint8 Week;   // Firmware version
//     uint8 BUF;    // Length of the internal receive buffer
//     uint8 STAT;   // Status information
//                   // 0: Device addressed for first time after switching on.
//                   // 1: Device was already initialised
//   };
// };
/*
Calculation of CRC-16 checksum over an amount of bytes in the serial buffer.
The calculation is done without the 2 bytes from CRC-16 (receive-mode).

uint8* SC_Buffer : (IN) Buffer from the serial interface.
int SC_Amount : (IN) Amount of bytes which are received or should be transmitted (without CRC-16).
uint8* crc_h : (INOUT) Valid pointer that should receive the MSB of the CRC-16.
uint8* crc_l: (INOUT) Valid pointer that should receive the LSB of the CRC-16.

Return : Nothing.
*/
inline void CalcCRC16(uint8* SC_Buffer, int SC_Amount, uint8* crc_h, uint8* crc_l)
{
  // Locals.
  unsigned int Crc;
  unsigned char n, m, x;

  // Initialization.
  Crc = 0xFFFF;
  m = (unsigned char)SC_Amount;
  x = 0;

  // Loop over all bits.
  while (m > 0)
  {
    Crc ^= SC_Buffer[x];
    for (n = 0; n < 8 ; n++)
    {
      if (Crc &1)
      {
        Crc >>= 1;
        Crc ^= 0xA001;
      }
      else
        Crc >>= 1;
    }
    m--;
    x++;
  }

  // Result.
  *crc_h = (uint8)((Crc>>8)&0xFF);
  *crc_l = (uint8)(Crc&0xFF);
}

#endif // KELLERMSG_H
