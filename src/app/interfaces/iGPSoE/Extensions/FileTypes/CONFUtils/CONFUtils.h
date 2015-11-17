/***************************************************************************************************************:')

CONFUtils.h

CONF files to store programs parameters.

Fabrice Le Bars

Created : 2009-06-28

Version status : Not finished

***************************************************************************************************************:)*/

#ifndef CONFUTILS_H
#define CONFUTILS_H

#include "OSCore.h"

int fread_value_conf(char** value, char* key, FILE* fp);
int LoadFromCONFFile(char* filename, char** value, char* key);

#endif // CONFUTILS_H




