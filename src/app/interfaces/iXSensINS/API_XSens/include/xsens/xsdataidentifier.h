#ifndef XSDATAIDENTIFIER_H
#define XSDATAIDENTIFIER_H

//////////////////////////////////////////////////////////////////////////////////////////
/*!	\addtogroup enums Global enumerations
	@{
*/

/*!	\enum XsDataIdentifier
	\brief Defines the data identifiers

	The list of standard data identifiers is shown below. 
	The last positions in the data identifier depends on the configuration of the data. 
	For example 0x4020 is 3D acceleration in single precision float format, 
	where 0x4022 is 3D acceleration in fixed point 16.32 format. 
	
	Refer to the Low Level Communication Protocol for more details.
*/
enum XsDataIdentifier
{
	XDI_None					= 0x0000,
	XDI_TypeMask				= 0xFE00,
	XDI_FullTypeMask			= 0xFFF0,
	XDI_FullMask				= 0xFFFF,
	XDI_FormatMask				= 0x01FF,
	XDI_DataFormatMask			= 0x000F,

	XDI_SubFormatMask			= 0x0003,	//determines, float, fp12.20, fp16.32, double output... (where applicable)
	XDI_SubFormatFloat			= 0x0000,
	XDI_SubFormatFp1220			= 0x0001,
	XDI_SubFormatFp1632			= 0x0002,
	XDI_SubFormatDouble			= 0x0003,

	XDI_TemperatureGroup		= 0x0800,
	XDI_Temperature				= 0x0810,

	XDI_TimestampGroup			= 0x1000,
	XDI_UtcTime					= 0x1010,
	XDI_PacketCounter			= 0x1020,
	XDI_Itow					= 0x1030,
	XDI_GpsAge					= 0x1040,	//!< \deprecated
	XDI_GnssAge					= 0x1040,
	XDI_PressureAge				= 0x1050,
	XDI_SampleTimeFine			= 0x1060,
	XDI_SampleTimeCoarse		= 0x1070,
	XDI_FrameRange				= 0x1080,	// add for MTw (if needed)
	XDI_PacketCounter8			= 0x1090,
	XDI_SampleTime64			= 0x10A0,

	XDI_OrientationGroup		= 0x2000,
	XDI_CoordSysMask			= 0x000C,
	XDI_CoordSysEnu				= 0x0000,
	XDI_CoordSysNed				= 0x0004,
	XDI_CoordSysNwu				= 0x0008,
	XDI_Quaternion				= 0x2010,
	XDI_RotationMatrix			= 0x2020,
	XDI_EulerAngles				= 0x2030,

	XDI_PressureGroup			= 0x3000,
	XDI_BaroPressure			= 0x3010,

	XDI_AccelerationGroup		= 0x4000,
	XDI_DeltaV					= 0x4010,
	XDI_Acceleration			= 0x4020,
	XDI_FreeAcceleration		= 0x4030,

	XDI_PositionGroup			= 0x5000,
	XDI_AltitudeMsl				= 0x5010,
	XDI_AltitudeEllipsoid		= 0x5020,
	XDI_PositionEcef			= 0x5030,
	XDI_LatLon					= 0x5040,

	XDI_SnapshotGroup			= 0xC800,
	XDI_RetransmissionMask		= 0x0001,
	XDI_RetransmissionFlag		= 0x0001,
	XDI_AwindaSnapshot 			= 0xC810,

	XDI_GnssGroup				= 0x7000,
	XDI_GnssPvtData				= 0x7010,
	XDI_GnssSatInfo				= 0x7020,

	XDI_AngularVelocityGroup	= 0x8000,
	XDI_RateOfTurn				= 0x8020,
	XDI_DeltaQ					= 0x8030,

	XDI_GpsGroup				= 0x8800,		//!< \deprecated
	XDI_GpsDop					= 0x8830,		//!< \deprecated
	XDI_GpsSol					= 0x8840,		//!< \deprecated
	XDI_GpsTimeUtc				= 0x8880,		//!< \deprecated
	XDI_GpsSvInfo				= 0x88A0,		//!< \deprecated

	XDI_RawSensorGroup			= 0xA000,
	XDI_RawAccGyrMagTemp		= 0xA010,
	XDI_RawGyroTemp				= 0xA020,
	XDI_RawAcc					= 0xA030,
	XDI_RawGyr					= 0xA040,
	XDI_RawMag					= 0xA050,

	XDI_AnalogInGroup			= 0xB000,
	XDI_AnalogIn1				= 0xB010,
	XDI_AnalogIn2				= 0xB020,

	XDI_MagneticGroup			= 0xC000,
	XDI_MagneticField			= 0xC020,

	XDI_VelocityGroup			= 0xD000,
	XDI_VelocityXYZ				= 0xD010,

	XDI_StatusGroup				= 0xE000,
	XDI_StatusByte				= 0xE010,
	XDI_StatusWord				= 0xE020,
	XDI_Rssi					= 0xE040,
	XDI_DeviceId				= 0xE080,

	XDI_IndicationGroup	        = 0x4800, // 0100.1000 -> bit reverse = 0001.0010 -> type 18
	XDI_TriggerIn1				= 0x4810,
	XDI_TriggerIn2				= 0x4820,
};
/*! @} */

typedef enum XsDataIdentifier XsDataIdentifier;

#define XDI_MAX_FREQUENCY		((uint16_t) 0xFFFF)

#ifdef __cplusplus
inline XsDataIdentifier operator | (XsDataIdentifier a, XsDataIdentifier b)
{
	return (XsDataIdentifier) ((int) a | (int) b);
}

inline XsDataIdentifier operator & (XsDataIdentifier a, XsDataIdentifier b)
{
	return (XsDataIdentifier) ((int) a & (int) b);
}

inline XsDataIdentifier operator ~ (XsDataIdentifier a)
{
	return (XsDataIdentifier) ~((unsigned short)a);
}
#endif

#endif // file guard
