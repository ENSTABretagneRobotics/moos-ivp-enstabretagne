#include <xsens/xsportinfoarray.h>
#include <xsens/xsdatapacket.h>
#include <xsens/xstime.h>
#include <xcommunication/legacydatapacket.h>
#include <xcommunication/int_xsdatapacket.h>
#include <xcommunication/enumerateusbdevices.h>

#include "deviceclass.h"

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <string>

#ifdef __GNUC__
#include "conio.h" // for non ANSI _kbhit() and _getch()
#else
#include <conio.h>
#endif

int main(int argc, char* argv[])
{
	DeviceClass device;

	try
	{
		//**************************** OPEN DEVICE *************************************//
		XsPortInfo mtPort("/dev/ttyUSB2", XsBaud::numericToRate(152000));

		// Open the port with the detected device
		std::cout << "Opening port..." << std::endl;
		if (!device.openPort(mtPort))
			throw std::runtime_error("Could not open port. Aborting.");

		//**************************** CONFIGURE THE DEVICE *****************************//
		// Put the device in configuration mode
		std::cout << "Putting device into configuration mode..." << std::endl;
		if (!device.gotoConfig()) // Put the device into configuration mode before configuring the device
		{
			throw std::runtime_error("Could not put device into configuration mode. Aborting.");
		}

		try
		{
			// Print information about detected MTi / MTx / MTmk4 device
			//std::cout << "Device: " << device.getProductCode().toStdString() << " opened." << std::endl;

			// Configure the device. Note the differences between MTix and MTmk4
			std::cout << "Configuring the device..." << std::endl;
			XsOutputMode outputMode = XOM_Orientation; // output orientation data
			XsOutputSettings outputSettings = XOS_OrientationMode_Quaternion; // output orientation data as quaternion

			// set the device configuration
			if (!device.setDeviceMode(outputMode, outputSettings))
			{
				throw std::runtime_error("Could not configure MT device. Aborting.");
			}


			//**************************** MEASURE MODE *********************************//

			// Put the device in measurement mode
			std::cout << "Putting device into measurement mode..." << std::endl;
			if (!device.gotoMeasurement())
			{
				throw std::runtime_error("Could not put device into measurement mode. Aborting.");
			}

			std::cout << "\nMain loop (press any key to quit)" << std::endl;
			std::cout << std::string(79, '-') << std::endl;

			XsByteArray data;
			XsMessageArray msgs;
			while (!_kbhit())
			{
				device.readDataToBuffer(data);
				device.processBufferedData(data, msgs);
				for (XsMessageArray::iterator it = msgs.begin(); it != msgs.end(); ++it)
				{
					// Retrieve a packet
					XsDataPacket packet;
					if ((*it).getMessageId() == XMID_MtData) {
						LegacyDataPacket lpacket(1, false);
						lpacket.setMessage((*it));
						lpacket.setXbusSystem(false, false);
						lpacket.setDeviceId(mtPort.deviceId(), 0);
						lpacket.setDataFormat(XOM_Orientation, XOS_OrientationMode_Quaternion,0);	//lint !e534
						XsDataPacket_assignFromXsLegacyDataPacket(&packet, &lpacket, 0);
					}
					else if ((*it).getMessageId() == XMID_MtData2) {
						packet.setMessage((*it));
						packet.setDeviceId(mtPort.deviceId());
					}

					// Convert packet to euler
					XsEuler euler = packet.orientationEuler();
					std::cout << ",Roll:" << std::setw(7) << std::fixed << std::setprecision(2) << euler.m_roll
							  << ",Pitch:" << std::setw(7) << std::fixed << std::setprecision(2) << euler.m_pitch
							  << ",Yaw:" << std::setw(7) << std::fixed << std::setprecision(2) << euler.m_yaw
					;

					std::cout << std::flush;
				}
				msgs.clear();
				XsTime::msleep(0);
			}
			_getch();
			std::cout << "\n" << std::string(79, '-') << "\n";
			std::cout << std::endl;
		}
		catch (std::runtime_error const & error)
		{
			std::cout << error.what() << std::endl;
		}
		catch (...)
		{
			std::cout << "An unknown fatal error has occured. Aborting." << std::endl;
		}

		// Close port
		std::cout << "Closing port..." << std::endl;
		device.close();
	}
	catch (std::runtime_error const & error)
	{
		std::cout << error.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "An unknown fatal error has occured. Aborting." << std::endl;
	}

	std::cout << "Successful exit." << std::endl;

	std::cout << "Press [ENTER] to continue." << std::endl; std::cin.get();

	return 0;
}