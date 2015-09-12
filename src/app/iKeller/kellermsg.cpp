#include "kellermsg.h"
#include <cstring>
#include <sstream>
#include <cstdio>

#define strtime_m() 0
#define hDev 0

using namespace std;

KellerMsg::KellerMsg()
{
}

KellerMsg::KellerMsg(const std::string &data)
    : m_data(data)
{
}

KellerMsg::~KellerMsg()
{
}

void KellerMsg::print_hex(int max_bytes) const
{
  for (int k=0; k < min(max_bytes,(int)m_data.size());++k)
    printf("%02x ", (unsigned char)m_data.data()[k]);
  if (max_bytes < (int)m_data.size())
    printf("... ");
  printf("[%dB]", (int)m_data.size());
}


bool KellerMsg::startsWithHeader(const std::string &data)
{
  if (data.empty()) return false;
  if (data[0] != (uint8)0xfa) return false;
  return true;
}

KellerMsg::MessageTypeID KellerMsg::detectMessageType(const std::string &data)
{
  if (data.size() < 2) return msgNoMessageType;
  else return (MessageTypeID) (data[2]);
}
int KellerMsg::numberBytesMissing(const std::string &data, int &full_length)
{
  // If no data, request the size of the smallest message
  if (data.empty())
    return 4;
  // Check presence of header
  // if (data[0] != (uint8)0xfa){
  if ((uint8)data[0] != 0xfa){
    printf("no Header!");
    return mrNotAMessage;
  }
  else if (data.size()<2)
    return 4;
  else if ((uint8)data[1] == 0x30)
  {
    if (data.size()<3)
      return 4 - data.size();
    else if ((uint8)data[2] == 0x04)
      return 4 - data.size() - data.size();
    else if ((uint8)data[2] == 0x05)
      return 10 - data.size();
  }
  else if ((uint8)data[1] == 0x49)
  {
    if ((uint8)data[2] == 0x01)
      return 5 - data.size();
    else if ((uint8)data[2] == 0x41)
      return 10 - data.size();
  }

}
