#ifndef SEANETMSG_H
#define SEANETMSG_H

#include <string>
#include <stdint.h>


class SeaNetMsg
{
protected:
	std::string m_data;
  char m_destinationNode;

public:
  enum MessageReadError {
    mrNotAMessage = -1,
    mrNotEnoughData = -2,
  };

    /*
    * Procedure pour le changement de configuration des modems (voir CompteRenduModemTritech.pdf joint)
    *  1) Réception de messages mtAlive avec bit headInf = 0x40 (reception en boucle)
    *  2) Envoi mtSendVersion
    *  3) Réception mtVersionData
    *  4) Envoi mtSendBBUser
    *  5) Réception mtBBUserData //voir doc, cette commande conditionnela configuration
    *  6) Réception mtFpgaVersionData
    *  7) Réception de messages mtAlive jusqu'à bit headInf = 0xc0 (reception en boucle),
    *  8) Envoi mtEraseSector
    *  9) Réception mtPgrAck
    * 10) Envoi mtProgBlock //voir doc, commande conditionnée par commande du point 5)
    * 11) Réception mtPgrAck
    * 12) Envoi mtProgBlock
    * 13) Réception mtPgrAck
    * 14) Envoi mtReBoot
    */

  enum MessageTypeID {
    mteNoMessageType = -1,
    mtNull = 0,
    mtVersionData,
    mtHeadData,
    mtAlive = 4,
    mtPgrAck,
    mtBBUserData,
    mtAuxData = 8,
    mtReBoot = 16,
    mtHeadCommand = 19,
    mtEraseSector,
    mtProgBlock,
    mtSendVersion = 23,
    mtSendBBuser,
    mtSendData,
    mtFpgaVersionData=57,
  };

protected:
  uint8_t read_uchar_at(int offset) const {return *(uint8_t *)(m_data.data()+offset);}
  uint16_t read_ushort_at(int offset) const {return *(uint16_t *)(m_data.data()+offset);}
  uint32_t read_ulong_at(int offset) const {return *(uint32_t *)(m_data.data()+offset);}

  void set_uchar_at(int offset, uint8_t value) {m_data[offset] = value;}
  void set_ushort_at(int offset, uint16_t value) {m_data[offset] = value & 0xFF; m_data[offset+1] = (value & 0xFF00) >> 8;}
  void set_ulong_at(int offset, uint32_t value) {m_data[offset] = value & 0xFF; m_data[offset+1] = (value & 0xFF00) >> 8;  m_data[offset+2] = (value & 0xFF0000) >> 16; m_data[offset+3] = (value & 0xFF000000) >> 24; }

public:
  SeaNetMsg();
  SeaNetMsg(const char &destinationNode);
  SeaNetMsg(const std::string &data);
  SeaNetMsg(const std::string &data, const char &destinationNode);
  virtual ~SeaNetMsg();

  const std::string & data() const {return m_data;};
  const char & destinationNode() const {return m_destinationNode;};
  MessageTypeID messageType() const {return (MessageTypeID) read_uchar_at(10);}
  void setMessageType(MessageTypeID msg_type) {set_uchar_at(10, (uint8_t)msg_type);}

  void print_hex(int max_bytes = 50) const;

public: // Static member functions
  static bool startsWithHeader(const std::string &data);
  static int readMsgLength(const std::string &data);
  static MessageTypeID detectMessageType(const std::string &data);
  static int numberBytesMissing(const std::string &data, int &full_message_length);
  static SeaNetMsg * fromData(const std::string &data);
};

class SeaNetMsg_SendVersion : public SeaNetMsg
{
  //
public:
  SeaNetMsg_SendVersion(const char & destinationNode) : SeaNetMsg(destinationNode) {
    const char msg[] = {0x40,0x30,0x30,0x30,0x38,0x08,0x00,0xFF,m_destinationNode,0x03,0x17,0x80,m_destinationNode,0x0A};
    m_data.assign(msg, sizeof(msg));
  }
  SeaNetMsg_SendVersion() : SeaNetMsg() {
    const char msg[] = {0x40,0x30,0x30,0x30,0x38,0x08,0x00,0xFF,m_destinationNode,0x03,0x17,0x80,m_destinationNode,0x0A};
    m_data.assign(msg, sizeof(msg));
  }
};

class SeaNetMsg_SendBBUser : public SeaNetMsg
{
public:
  SeaNetMsg_SendBBUser(const char & destinationNode) : SeaNetMsg(destinationNode) {
    const char msg[] = {0x40,0x30,0x30,0x30,0x38,0x08,0x00,0xFF,m_destinationNode,0x03,0x18,0x80,m_destinationNode,0x0A};
    m_data.assign(msg, sizeof(msg));
  }
  SeaNetMsg_SendBBUser() : SeaNetMsg() {
    const char msg[] = {0x40,0x30,0x30,0x30,0x38,0x08,0x00,0xFF,m_destinationNode,0x03,0x18,0x80,m_destinationNode,0x0A};
    m_data.assign(msg, sizeof(msg));
  }
};

class SeaNetMsg_ReBoot : public SeaNetMsg
{
public:
  SeaNetMsg_ReBoot(const char & destinationNode) : SeaNetMsg(destinationNode) {
    const char msg[] = {0x40,0x30,0x30,0x30,0x38,0x08,0x00,0xFF,m_destinationNode,0x03,0x10,0x80,m_destinationNode,0x0A};
    m_data.assign(msg, sizeof(msg));
  }
  SeaNetMsg_ReBoot() : SeaNetMsg() {
    const char msg[] = {0x40,0x30,0x30,0x30,0x38,0x08,0x00,0xFF,m_destinationNode,0x03,0x10,0x80,m_destinationNode,0x0A};
    m_data.assign(msg, sizeof(msg));
  }
};
class SeaNetMsg_EraseSector : public SeaNetMsg
{
public:
  SeaNetMsg_EraseSector(const char & destinationNode, const char & sectorAddr_high, const char & sectorAddr_low) : SeaNetMsg(destinationNode) {
    const char msg[] = {0x40,0x30,0x30,0x30,0x41,0x0A,0x00,0xFF,m_destinationNode,0x05,0x14,0x80,m_destinationNode,sectorAddr_high,sectorAddr_low ,0x0A};
    m_data.assign(msg, sizeof(msg));
  }
};
class SeaNetMsg_FirstProgBlock : public SeaNetMsg
{
public:
  SeaNetMsg_FirstProgBlock(const char & destinationNode, const char & blockNum, const char & role) : SeaNetMsg(destinationNode) {
    //On construit le message global
    const char msg[] = {0x40,0x30,0x30,0x38,0x41,0x8A,0x00,0xFF,m_destinationNode,0x85,0x15,0x80,m_destinationNode,blockNum,0x03,0xff , 0x00 , 0x34 , 0x12 , 0x40 , 0x00 , 0x0f , 0x06 , 0x00 , 0x00 , 0x00 , 0x00 , 0x05 , 0x00 , 0x04 , 0x00 , 0x02 , 0x00 , 0x02 , 0x00 , 0x03 , 0x00 , 0x03 , 0x00 , 0x06 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x03 , 0x00 , 0x06 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x16 , 0x01 , 0x03 , role , 0x03 , 0x0c , 0x03 , 0x01 , 0x00 , 0x00 , 0x00 , 0x01 , 0x01 , 0x01 , 0x00 , 0x00 , 0x01 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x0A};

    // m_data.reserve(sizeof(msg)+sizeof(payload)+1);
    m_data.assign(msg, sizeof(msg));
  }
};
class SeaNetMsg_SecondProgBlock : public SeaNetMsg
{
public:
  SeaNetMsg_SecondProgBlock(const char & destinationNode, const char & blockNum) : SeaNetMsg(destinationNode) {
    //On construit le message global
    const char msg[] = {0x40,0x30,0x30,0x38,0x41,0x8A,0x00,0xFF,m_destinationNode,0x85,0x15,0x80,m_destinationNode,blockNum,0x03, 0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00, 0x0A};
    // m_data.reserve(sizeof(msg)+sizeof(payload)+1);
    m_data.assign(msg, sizeof(msg));
  }
};

class SeaNetMsg_SendData : public SeaNetMsg
{
public:
  SeaNetMsg_SendData(const char & destinationNode) : SeaNetMsg(destinationNode) {
    const char msg[] = {0x40,0x30,0x30,0x30,0x43,0x0C,0x00,0xFF,m_destinationNode,0x07,0x19,0x80,m_destinationNode,0xCA,0x64,0xB0,0x03,0x0A}; //18
    m_data.assign(msg, sizeof(msg));
  }
  SeaNetMsg_SendData() : SeaNetMsg() {
    const char msg[] = {0x40,0x30,0x30,0x30,0x43,0x0C,0x00,0xFF,m_destinationNode,0x07,0x19,0x80,m_destinationNode,0xCA,0x64,0xB0,0x03,0x0A}; //18
    m_data.assign(msg, sizeof(msg));
  }

  void setTime(double time_in_sec) { uint32_t t_ms = time_in_sec*1000.0; set_ulong_at(13,t_ms);}

};

class SeaNetMsg_HeadCommand : public SeaNetMsg
{
public:
  SeaNetMsg_HeadCommand(); /*: SeaNetMsg() {
      const char msg[] = {0x40,0x30,0x30,0x30,0x43,0x0C,0x00,0xFF,0x02,0x07,0x19,0x80,0x02,0xCA,0x64,0xB0,0x03,0x0A}; //18
      m_data.assign(msg, sizeof(msg));
  }*/

  // Parameters of the device
  struct SNRPARAMS       {
    int adc8on; // The head will return 4-bit packed echo data (0..15) representing the amplitude
    // of received echoes in a databin if it is set to 0. Otherwise, it will be in 8 bits (0..255)
    int cont; // Scanning will be restricted to a sector defined by the direction LeftAngleLimit
    // and RightAngleLimit if it is set to 0. Otherwise, it will be a continuous rotation and
    // LeftAngleLimit and RightAngleLimit will be ignored
    int invert; // Allow the rotation direction to be reversed if the sonar head is mounted inverted,
    // i.e. when the sonar transducer boot is pointing downward rather than up (Default = 0 = Sonar
    // mounted upright, transducer boot pointing up)
    double LeftAngleLimit; // For a sector scan (cont = 0), in degrees
    double RightAngleLimit; // For a sector scan (cont = 0), in degrees
    int VelocityOfSound; // In m/s
    int RangeScale; // In meters
    double StepAngleSize; // In degrees: Ultimate Resolution (0.225°), High Resolution (0.45°), Medium Resolution (0.9°), Low Resolution (1.8°)
    // StepAngleSize = (scanWidth/NSteps)
    int NBins; // Number of bins generated after a ping
    int IGain; // Initial Gain of the receiver (in units 0..210 = 0..+80dB = 0..100%)
  };

  void setParams(const SNRPARAMS& params) {this->params = params; buildMessage();}

  void setRange(const double& range_in_m) {params.RangeScale = range_in_m; buildMessage();}
  void setNbins(int nBins) {params.NBins = nBins; buildMessage();}
  void setAngleStep(const double &angle_step) {params.StepAngleSize = angle_step; buildMessage();}
  void setGain(const double &gain) {params.IGain = (int)(100.*gain/210.); buildMessage();}
  void setContinuous(bool continuous) {params.cont = continuous; buildMessage();}
  void setLeftLimit(const double& leftLimit) {params.LeftAngleLimit = leftLimit; buildMessage();}
  void setRightLimit(const double &rightLimit) {params.RightAngleLimit = rightLimit; buildMessage();}

protected:
  SNRPARAMS params;
  void buildMessage();
};

class SeaNetMsg_HeadData : public SeaNetMsg
{
public:
  SeaNetMsg_HeadData() : SeaNetMsg() {};

  double bearing() const { uint16_t bearing_grad = read_ushort_at(40); return 360./6400.*bearing_grad;}
  double ADInterval_s() const { uint16_t ad_int_640ns = read_ushort_at(33); return 640.e-9*ad_int_640ns;}
  double ADInterval_m() const { return 1500.*ADInterval_s();}
  int nBins() const { return read_ushort_at(42); }

  const uint8_t * scanlineData() const { return reinterpret_cast<const uint8_t *>(m_data.data()+44); }
  uint8_t bin(int i) const { return m_data[44+i]; }

  double firstObstacleDist( uint8_t threshold, const double& min_dist, const double& max_dist) const;

};

#endif // SEANETMSG_H
