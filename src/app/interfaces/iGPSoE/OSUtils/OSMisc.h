/***************************************************************************************************************:')

OSMisc.h

Miscellaneous things.

Fabrice Le Bars
mean() and var() with the help of Guillaume Brosse and Antone Borissov
fgets2() by Luc Jaulin

Created : 2009-01-28

Version status : Tested some parts

***************************************************************************************************************:)*/

// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef OSMISC_H
#define OSMISC_H

#include "OSTime.h"

/*
Debug macros specific to OSMisc.
*/
#ifdef _DEBUG_MESSAGES_OSUTILS
#	define _DEBUG_MESSAGES_OSMISC
#endif // _DEBUG_MESSAGES_OSUTILS

#ifdef _DEBUG_WARNINGS_OSUTILS
#	define _DEBUG_WARNINGS_OSMISC
#endif // _DEBUG_WARNINGS_OSUTILS

#ifdef _DEBUG_ERRORS_OSUTILS
#	define _DEBUG_ERRORS_OSMISC
#endif // _DEBUG_ERRORS_OSUTILS

#ifdef _DEBUG_MESSAGES_OSMISC
#	define PRINT_DEBUG_MESSAGE_OSMISC(params) PRINT_DEBUG_MESSAGE(params)
#else
#	define PRINT_DEBUG_MESSAGE_OSMISC(params)
#endif // _DEBUG_MESSAGES_OSMISC

#ifdef _DEBUG_WARNINGS_OSMISC
#	define PRINT_DEBUG_WARNING_OSMISC(params) PRINT_DEBUG_WARNING(params)
#else
#	define PRINT_DEBUG_WARNING_OSMISC(params)
#endif // _DEBUG_WARNINGS_OSMISC

#ifdef _DEBUG_ERRORS_OSMISC
#	define PRINT_DEBUG_ERROR_OSMISC(params) PRINT_DEBUG_ERROR(params)
#else
#	define PRINT_DEBUG_ERROR_OSMISC(params)
#endif // _DEBUG_ERRORS_OSMISC

#ifdef _WIN32
#else 
#include <termios.h>
#endif // _WIN32

#define MAX_BUF_LEN 256

#define MAX_TIMEOUT_PROMPTGETUSERINPUTTIMEOUT 25500

// Kelvin to Celsius degrees conversions.
#define KELVIN2CELSIUS(temperature) ((temperature)-273.15)
#define CELSIUS2KELVIN(temperature) ((temperature)+273.15)

// Earth radius in m.
#define EARTH_RADIUS 6371000

#define EAST_NORTH_UP_COORDINATE_SYSTEM 0
#define NORTH_EAST_DOWN_COORDINATE_SYSTEM 1
#define NORTH_WEST_UP_COORDINATE_SYSTEM 2

/*
Swap the bits of a given number coded on 8 bits.

uint8 x : (IN) The number to swap.

Return : The number swapped.
*/
inline uint8 SwapBits(uint8 x)
{
	return  
		((x & 0x01) << 7) | ((x & 0x02) << 5) | ((x & 0x04) << 3) | ((x & 0x08) << 1) | 
		((x & 0x10) >> 1) | ((x & 0x20) >> 3) | ((x & 0x40) >> 5) | ((x & 0x80) >> 7);
}

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

/*
Prompt for the user to press any key until a specified timeout.

UINT timeout : (IN) Time to wait to get at least 1 char in ms (with a 
precision of tenths of s, max is MAX_TIMEOUT_PROMPTGETUSERINPUTTIMEOUT).

Return : The first character pressed.
*/
EXTERN_C char PromptForUserInputTimeout(UINT timeout);

/*
Wait for the user to press any key until a specified timeout.

UINT timeout : (IN) Time to wait to get at least 1 char in ms (with a 
precision of tenths of s, max is MAX_TIMEOUT_PROMPTGETUSERINPUTTIMEOUT).

Return : The first character pressed.
*/
EXTERN_C char GetUserInputTimeout(UINT timeout);

/*
Prompt for the user to press any key.

Return : The first character pressed.
*/
EXTERN_C char PromptForUserInput(void);

/*
Wait for the user to press any key.

Return : The first character pressed.
*/
EXTERN_C char GetUserInput(void);

/*
Wait for the user to press any key.

Return : Nothing.
*/
EXTERN_C void WaitForUserInput(void);

/*
Wait for the user to press the ENTER key.

Return : Nothing.
*/
EXTERN_C void WaitForENTER(void);

#ifndef FGETS2_DEFINED
#define FGETS2_DEFINED
/*
Return a line of a file using fgets(), skipping lines that begin with a '%'. 
Return NULL when a line begin with a '$' or when fgets() return NULL.

FILE* file : (IN) Pointer to a file.
char* line : (IN) Storage location for data.
int nbChar : (IN) Maximum number of characters to read.

Return : The line or NULL.
*/
inline char* fgets2(FILE* file, char* line, int nbChar)
{ 
	char* r = NULL;

	do   
	{
		r = fgets(line, nbChar, file);
	} 
	while ((line[0] == '%') && (r != NULL));

	if (line[0] == '$')
	{
		r = NULL;
	}

	return r;
}
#endif // FGETS2_DEFINED

/*
Return a line of a file using fgets(), skipping lines that begin with a '%' 
(Scilab-style comments), a '#' (Linux configuration files-style comments) or 
"//" (C-style comments). 
Return NULL when a line begin with a '$' or when fgets() return NULL, or if 
the maximum number of characters to read is less than 2.

FILE* file : (IN) Pointer to a file.
char* line : (IN) Storage location for data.
int nbChar : (IN) Maximum number of characters to read.

Return : The line or NULL.
*/
inline char* fgets3(FILE* file, char* line, int nbChar)
{ 
	char* r = NULL;

	if (nbChar < 2)
	{
		return NULL;
	}

	do   
	{
		r = fgets(line, nbChar, file);
	} 
	while ((
		(line[0] == '%')||
		(line[0] == '#')||
		((line[0] == '/')&&(line[1] == '/'))
		) && (r != NULL));

	if (line[0] == '$')
	{
		r = NULL;
	}

	return r;
}

inline double sensor_err(double bias_err, double max_rand_err)
{
	return bias_err+max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
}

/*
Compute the square of a value.

double x : (IN) Value.

Return : The square of x.
*/
inline double sqr(double x)
{
	return x*x;
}

/*
Return x/epsilon if x is between -epsilon and epsilon or -1 if x is negative, 
+1 if x is positive.

double x : (IN) Value.
double epsilon : (IN) Threshold.

Return : -1, +1 or x/epsilon.
*/
inline double sign(double x, double epsilon)
{ 
	if (x >= epsilon) 
		return 1;
	else if (x <= -epsilon) 
		return -1;
	else 
		return x/epsilon;
}

/*
Return an angle between -M_PI and M_PI.

double theta : (IN) Value.

Return : The converted angle.
*/
inline double fmod_2PI(double theta)
{
	return fmod(fmod(theta, 2*M_PI)+3*M_PI, 2*M_PI)-M_PI;
}

// A NMEA sentence begins with a '$' and ends with a carriage return/line feed sequence and can 
// be no longer than 80 characters of visible text (plus the line terminators). The data is contained 
// within this single line with data items separated by commas. The data itself is just ascii text. 
// There is a provision for a checksum at the end of each sentence which may or may not be checked by 
// the unit that reads the data. The checksum field consists of a '*' and two hex digits representing 
// an 8 bit exclusive OR of all characters between, but not including, the '$' and '*'.

// Maximum number of characters of a NMEA sentence (excluding the line terminators CR and LF).
#define MAX_NB_BYTES_NMEA_SENTENCE 80

inline char* FindNMEASentence(char sentencebegin[7], char* str)
{
	char* ptr = NULL;
	char* foundstr = NULL;

	ptr = strstr(str, sentencebegin);
	if (!ptr)
	{
		// Could not find the beginning of the sentence.
		return NULL;
	}

	// Save the position of the beginning of the sentence.
	foundstr = ptr;

	// Check if the sentence is complete.
	ptr = strstr(foundstr+strlen(sentencebegin), "\r");
	if (!ptr)
	{
		// The sentence is incomplete.
		return NULL;
	}

	return foundstr;
}

inline char* FindLatestNMEASentence(char sentencebegin[7], char* str)
{
	char* ptr = NULL;
	char* foundstr = NULL;

	ptr = FindNMEASentence(sentencebegin, str);
	while (ptr) 
	{
		// Save the position of the beginning of the sentence.
		foundstr = ptr;

		// Search just after the beginning of the sentence.
		ptr = FindNMEASentence(sentencebegin, foundstr+strlen(sentencebegin));
	}

	return foundstr;
}

inline void ComputeNMEAchecksum(char* sentence, char checksum[4])
{
	int i = 0;
	char res = 0;

	memset(checksum, 0, sizeof(checksum));
	while (sentence[i])
	{
		if (sentence[i] == '$')
		{
			i++;
			continue;
		}
		if (sentence[i] == '*')
		{
			break;
		}
		res ^= sentence[i];
		i++;
	}

	sprintf(checksum, "*%02x", (int)res);
}

/*
Convert GPS data to coordinates in a reference coordinate system.

double lat0 : (IN) GPS latitude of the reference coordinate system origin (in decimal deg).
double long0 : (IN) GPS longitude of the reference coordinate system origin (in decimal deg).
double alt0 : (IN) GPS altitude of the reference coordinate system origin (in m).
double latitude : (IN) GPS latitude (in decimal deg).
double longitude : (IN) GPS longitude (in decimal deg).
double altitude : (IN) GPS altitude (in m).
double* pX : (INOUT) Valid pointer that will receive the x coordinate in the reference coordinate 
system (in m).
double* pY : (INOUT) Valid pointer that will receive the y coordinate in the reference coordinate 
system (in m).
double* pZ : (INOUT) Valid pointer that will receive the z coordinate in the reference coordinate 
system (in m).
int cstype : (IN) Either EAST_NORTH_UP_COORDINATE_SYSTEM, NORTH_EAST_DOWN_COORDINATE_SYSTEM 
or NORTH_WEST_UP_COORDINATE_SYSTEM.

Return : Nothing.
*/
inline void GPS2RefCoordSystem(double lat0, double long0, double alt0,
							   double latitude, double longitude, double altitude,
							   double* pX, double* pY, double* pZ,
							   int cstype)
{
	switch (cstype)
	{
	case NORTH_EAST_DOWN_COORDINATE_SYSTEM:
		*pX = (M_PI/180.0)*EARTH_RADIUS*(latitude-lat0);
		*pY = (M_PI/180.0)*EARTH_RADIUS*(longitude-long0)*cos((M_PI/180.0)*latitude);
		*pZ = alt0-altitude;
		break;
	case NORTH_WEST_UP_COORDINATE_SYSTEM:
		*pX = (M_PI/180.0)*EARTH_RADIUS*(latitude-lat0);
		*pY = (M_PI/180.0)*EARTH_RADIUS*(long0-longitude)*cos((M_PI/180.0)*latitude);
		*pZ = altitude-alt0;
		break;
	default: // EAST_NORTH_UP_COORDINATE_SYSTEM
		*pX = (M_PI/180.0)*EARTH_RADIUS*(longitude-long0)*cos((M_PI/180.0)*latitude);
		*pY = (M_PI/180.0)*EARTH_RADIUS*(latitude-lat0);
		*pZ = altitude-alt0;
		break;
	}
}

/*
Convert coordinates in a reference coordinate system to GPS data.

double lat0 : (IN) GPS latitude of the reference coordinate system origin (in decimal deg).
double long0 : (IN) GPS longitude of the reference coordinate system origin (in decimal deg).
double alt0 : (IN) GPS altitude of the reference coordinate system origin (in m).
double x : (IN) x coordinate in the reference coordinate system (in m).
double y : (IN) y coordinate in the reference coordinate system (in m).
double z : (IN) z coordinate in the reference coordinate system (in m).
double* pLatitude : (INOUT) Valid pointer that will receive the GPS latitude (in decimal deg).
double* pLongitude : (INOUT) Valid pointer that will receive the GPS longitude (in decimal deg).
double* pAltitude : (INOUT) Valid pointer that will receive the GPS altitude (in m).
int cstype : (IN) Either EAST_NORTH_UP_COORDINATE_SYSTEM, NORTH_EAST_DOWN_COORDINATE_SYSTEM 
or NORTH_WEST_UP_COORDINATE_SYSTEM.

Return : Nothing.
*/
inline void RefCoordSystem2GPS(double lat0, double long0, double alt0, 
							   double x, double y, double z,
							   double* pLatitude, double* pLongitude, double* pAltitude,
							   int cstype)
{
	switch (cstype)
	{
	case NORTH_EAST_DOWN_COORDINATE_SYSTEM:
		*pLatitude = (x/(double)EARTH_RADIUS)*(180.0/M_PI)+lat0;
		if ((fabs(*pLatitude-90.0) < DBL_EPSILON)||(fabs(*pLatitude+90.0) < DBL_EPSILON))
		{
			*pLongitude = 0;
		}
		else
		{
			*pLongitude = (y/(double)EARTH_RADIUS)*(180.0/M_PI)/cos((M_PI/180.0)*(*pLatitude))+long0;
		}
		*pAltitude = alt0-z;
		break;
	case NORTH_WEST_UP_COORDINATE_SYSTEM:
		*pLatitude = (x/(double)EARTH_RADIUS)*(180.0/M_PI)+lat0;
		if ((fabs(*pLatitude-90.0) < DBL_EPSILON)||(fabs(*pLatitude+90.0) < DBL_EPSILON))
		{
			*pLongitude = 0;
		}
		else
		{
			*pLongitude = long0-(y/(double)EARTH_RADIUS)*(180.0/M_PI)/cos((M_PI/180.0)*(*pLatitude));
		}
		*pAltitude = z+alt0;
		break;
	default: // EAST_NORTH_UP_COORDINATE_SYSTEM
		*pLatitude = (y/(double)EARTH_RADIUS)*(180.0/M_PI)+lat0;
		if ((fabs(*pLatitude-90.0) < DBL_EPSILON)||(fabs(*pLatitude+90.0) < DBL_EPSILON))
		{
			*pLongitude = 0;
		}
		else
		{
			*pLongitude = (x/(double)EARTH_RADIUS)*(180.0/M_PI)/cos((M_PI/180.0)*(*pLatitude))+long0;
		}
		*pAltitude = z+alt0;
		break;
	}
}

inline void DecDeg2DegDecMin(double decdeg, int* pDeg, double* pDecMin)
{
	*pDeg = (int)decdeg;
	*pDecMin = fabs((decdeg-*pDeg)*60.0);
}

inline void DecDeg2DegMinDecSec(double decdeg, int* pDeg, int* pMin, double* pDecSec)
{
	double decmin;
	DecDeg2DegDecMin(decdeg, pDeg, &decmin);
	*pMin = (int)decmin;
	*pDecSec = (decmin-*pMin)*60.0;
}

inline void DegDecMin2DecDeg(int deg, double decmin, double* pDecDeg)
{
	*pDecDeg = (deg >= 0)?deg+fabs(decmin/60.0):deg-fabs(decmin/60.0);
}

inline void DegMinDecSec2DecDeg(int deg, int min, double decsec, double* pDecDeg)
{
	double decmin = abs(min)+fabs(decsec)/60.0;
	DegDecMin2DecDeg(deg, decmin, pDecDeg);
}

inline void GPSLatitudeDecDeg2DegDecMin(double decdeg, int* pDeg, double* pDecMin, char* pNorthOrSouth)
{
	DecDeg2DegDecMin(decdeg, pDeg, pDecMin);
	*pDeg = abs(*pDeg);
	*pNorthOrSouth = (decdeg >= 0)?'N':'S';
}

inline void GPSLongitudeDecDeg2DegDecMin(double decdeg, int* pDeg, double* pDecMin, char* pEastOrWest)
{
	DecDeg2DegDecMin(decdeg, pDeg, pDecMin);
	*pDeg = abs(*pDeg);
	*pEastOrWest = (decdeg >= 0)?'E':'W';
}

inline void GPSLatitudeDegDecMin2DecDeg(int deg, double decmin, char NorthOrSouth, double* pDecDeg)
{
	DegDecMin2DecDeg(abs(deg), fabs(decmin), pDecDeg);
	*pDecDeg = (NorthOrSouth == 'N')?*pDecDeg:-*pDecDeg;
}

inline void GPSLongitudeDegDecMin2DecDeg(int deg, double decmin, char EastOrWest, double* pDecDeg)
{
	DegDecMin2DecDeg(abs(deg), fabs(decmin), pDecDeg);
	*pDecDeg = (EastOrWest == 'E')?*pDecDeg:-*pDecDeg;
}

inline void GPSLatitudeDecDeg2DegMinDecSec(double val, int* pDeg, int* pMin, double* pDecSec, char* pNorthOrSouth)
{
	DecDeg2DegMinDecSec(val, pDeg, pMin, pDecSec);
	*pDeg = abs(*pDeg);
	*pNorthOrSouth = (val >= 0)?'N':'S';
}

inline void GPSLongitudeDecDeg2DegMinDecSec(double val, int* pDeg, int* pMin, double* pDecSec, char* pEastOrWest)
{
	DecDeg2DegMinDecSec(val, pDeg, pMin, pDecSec);
	*pDeg = abs(*pDeg);
	*pEastOrWest = (val >= 0)?'E':'W';
}

inline void GPSLatitudeDegMinDecSec2DecDeg(int deg, int min, double decsec, char NorthOrSouth, double* pDecDeg)
{
	DegMinDecSec2DecDeg(abs(deg), abs(min), fabs(decsec), pDecDeg);
	*pDecDeg = (NorthOrSouth == 'N')?*pDecDeg:-*pDecDeg;
}

inline void GPSLongitudeDegMinDecSec2DecDeg(int deg, int min, double decsec, char EastOrWest, double* pDecDeg)
{
	DegMinDecSec2DecDeg(abs(deg), abs(min), fabs(decsec), pDecDeg);
	*pDecDeg = (EastOrWest == 'E')?*pDecDeg:-*pDecDeg;
}

/*
Used by HSL2RGB().

double v1 : (IN) 
double v2 : (IN) 
double vH : (IN) 

Return : A value used by HSL2RGB().
*/
inline double Hue_2_RGB(double v1, double v2, double vH)
{
	if (vH < 0) vH += 1;
	if (vH > 1) vH -= 1;
	if (6.0*vH < 1) return (v1+(v2-v1)*6.0*vH);
	if (2.0*vH < 1) return (v2);
	if (3.0*vH < 2) return (v1+(v2-v1)*((2.0/3.0)-vH)*6.0);

	return v1;
}

/*
Convert from HSL to RGB.

double h : (IN) 
double s : (IN) 
double l : (IN) 
UCHAR* r : (OUT) 
UCHAR* g : (OUT) 
UCHAR* b : (OUT) 

Return : Nothing.
*/
inline void HSL2RGB(double h, double s, double l, UCHAR* r, UCHAR* g, UCHAR* b)
{
	double var_1 = 0;
	double var_2 = 0;

	if (s == 0)
	{

		*r = (UCHAR)(l*255.0);
		*g = (UCHAR)(l*255.0);
		*b = (UCHAR)(l*255.0);
	}
	else
	{
		if (l < 0.5) var_2 = l*(1+s);
		else var_2 = (l+s)-(s*l);

		var_1 = 2*l-var_2;

		*r = (UCHAR)(255.0*Hue_2_RGB(var_1,var_2,h + 1.0/3.0));
		*g = (UCHAR)(255.0*Hue_2_RGB(var_1,var_2,h));
		*b = (UCHAR)(255.0*Hue_2_RGB(var_1,var_2,h - 1.0/3.0));
	}
}

/*
Convert from HSL to RGB.

double hue : (IN) Hue between 0..240.
double saturation : (IN) Saturation between 0..240.
double luminance : (IN) Luminance between 0..240.
double* pR : (OUT) Pointer to the red between 0..255.
double* pG : (OUT) Pointer to the green between 0..255.
double* pB : (OUT) Pointer to the blue between 0..255.

Return : Nothing.
*/
inline void HSL2RGB_MSPaint(double hue, double saturation, double luminance, double* pR, double* pG, double* pB)
{
	double var_1 = 0;
	double var_2 = 0;

	// Microsoft Paint limits for H,S,L are 0..240.
	hue = hue/240.0;
	saturation = saturation/240.0;
	luminance = luminance/240.0;

	if (saturation == 0)
	{
		*pR = luminance*255.0;
		*pG = luminance*255.0;
		*pB = luminance*255.0;
	}
	else
	{
		if (luminance < 0.5) var_2 = luminance*(1+saturation); else var_2 = (luminance+saturation)-(saturation*luminance);
		var_1 = 2*luminance-var_2;

		*pR = Hue_2_RGB(var_1, var_2, hue+1.0/3.0)*255.0;
		*pG = Hue_2_RGB(var_1, var_2, hue)*255.0;
		*pB = Hue_2_RGB(var_1, var_2, hue-1.0/3.0)*255.0;
	}
}

/*
Convert from RGB to HSL.

double red : (IN) Red between 0..255.
double green : (IN) Green between 0..255.
double blue : (IN) Blue between 0..255.
double* pH : (OUT) Pointer to the hue between 0..240.
double* pS : (OUT) Pointer to the saturation between 0..240.
double* pL : (OUT) Pointer to the luminance between 0..240.

Return : Nothing.
*/
inline void RGB2HSL_MSPaint(double red, double green, double blue, double* pH, double* pS, double* pL)
{
	double minval = min(red, min(green, blue));
	double maxval = max(red, max(green, blue));
	double mdiff = maxval-minval;
	double msum = maxval+minval;
	double hue = 0.0;
	double saturation = 0.0;
	double luminance = msum/510.0;

	if (maxval != minval)
	{
		double rnorm = (maxval-red)/mdiff;
		double gnorm = (maxval-green)/mdiff;
		double bnorm = (maxval-blue)/mdiff;
		saturation = (luminance <= 0.5)?(mdiff/msum):(mdiff/(510.0-msum));
		if (red == maxval)
			hue = 60.0*(6.0+bnorm-gnorm);
		if (green == maxval)
			hue = 60.0*(2.0+rnorm-bnorm);
		if (blue == maxval)
			hue = 60.0*(4.0+gnorm-rnorm);
		if (hue > 360.0)
			hue -= 360.0;
	}

	// Microsoft Paint limits for H,S,L are 0..240.

	*pH = hue*240.0/360.0; 
	*pS = saturation*240.0; 
	*pL = luminance*240.0;
}

/*
Compute the mean of a table.

float* tab : (IN) Table.
int tab_length : (IN) Number of elements in the table.

Return : The mean.
*/
EXTERN_C float mean(float* tab, int tab_length);

/*
Compute the variance of a table.

float* tab : (IN) Table.
int tab_length : (IN) Number of elements in the table.

Return : The variance.
*/
EXTERN_C float var(float* tab, int tab_length);

/*
Allocate memory for an array of height*width and initialize it to 0.
An element of this array can then be accessed as follows :

allocated_array[i][j]

The memory allocated can then be freed with free_array2()

int height : (IN) Height of the array to allocate.
int width : (IN) Width of the array to allocate.
size_t SizeOfElements : (IN) Length in bytes of each element.

Return : A pointer to the allocated space.
*/
EXTERN_C void* calloc_array2(int height, int width, size_t SizeOfElements);

/*
Allocate memory for an array of height*width*depth and initialize it to 0.
An element of this array can then be accessed as follows :

allocated_array[i][j][k]

The memory allocated can then be freed with free_array3().

int height : (IN) Height of the array to allocate.
int width : (IN) Width of the array to allocate.
int depth : (IN) Depth of the array to allocate.
size_t SizeOfElements : (IN) Length in bytes of each element.

Return : A pointer to the allocated space.
*/
EXTERN_C void* calloc_array3(int height, int width, int depth, size_t SizeOfElements);

/*
Free memory allocated by calloc_array2().

void*** p : (INOUT) Array previously allocated.
int height : (IN) Height of the array.

Return : Nothing.
*/
EXTERN_C void free_array2(void** p, int height); 

/*
Free memory allocated by calloc_array3().

void*** p : (INOUT) array previously allocated.
int height : (IN) Height of the array.
int width : (IN) Width of the array.

Return : Nothing.
*/
EXTERN_C void free_array3(void*** p, int height, int width);

inline void useless_function(int useless_param)
{
	UNREFERENCED_PARAMETER(useless_param);
	printf("This function is not so useless!\n");
}

#endif // OSMISC_H
