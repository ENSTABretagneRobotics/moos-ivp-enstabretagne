
#ifndef Modem_UTILS
#define Modem_UTILS

using namespace std;

bool invalidChar (char c)
{
  return !(c>=0 && c <128);
}
void stripUnicode(string & str)
{
  str.erase(remove_if(str.begin(),str.end(), invalidChar), str.end());
}
void stripCRLF(string & mystring)
{
  mystring.erase( std::remove(mystring.begin(), mystring.end(), '\r'), mystring.end() );
  mystring.erase( std::remove(mystring.begin(), mystring.end(), '\n'), mystring.end() );
}

void stripCRLF7F(string & mystring)
{
  mystring.erase( std::remove(mystring.begin(), mystring.end(), '\r'), mystring.end() );
  mystring.erase( std::remove(mystring.begin(), mystring.end(), '\n'), mystring.end() );
  mystring.erase( std::remove(mystring.begin(), mystring.end(), 0x7F), mystring.end() );
}

#endif