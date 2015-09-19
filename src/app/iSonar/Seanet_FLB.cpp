// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "Seanet.h"
#include "SeanetProcessing.h"

THREAD_PROC_RETURN_VALUE SeanetThread(void* pParam)
{
	SEANET seanet;
	//SEANETDATA seanetdata;
	struct timeval tv;
	double angle = 0;
	unsigned char scanline[MAX_NB_BYTES_SEANET];
	unsigned char auxbuf[128]; // For daisy-chained device.
	int nbauxbytes = 0; // For daisy-chained device.
	double distance = 0; // For daisy-chained echosounder.
	BOOL bConnected = FALSE;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&seanet, 0, sizeof(SEANET));

	for (;;)
	{
		//mSleep(100);

		if (bPauseSeanet)
		{
			if (bConnected)
			{
				printf("Seanet paused.\n");
				bConnected = FALSE;
				DisconnectSeanet(&seanet);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartSeanet)
		{
			if (bConnected)
			{
				printf("Restarting a Seanet.\n");
				bConnected = FALSE;
				DisconnectSeanet(&seanet);
			}
			bRestartSeanet = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectSeanet(&seanet, "Seanet0.txt") == EXIT_SUCCESS)
			{
				bConnected = TRUE;

				memset(&tv, 0, sizeof(tv));
				angle = 0;
				memset(scanline, 0, sizeof(scanline));
				memset(auxbuf, 0, sizeof(auxbuf));
				nbauxbytes = 0;

				EnterCriticalSection(&SeanetConnectingCS);

				AdLow = seanet.ADLow;
				AdSpan = seanet.ADSpan;
				Steps = seanet.Resolution;
				NSteps = seanet.NSteps;
				Hdctrl = seanet.HdCtrl;
				StepAngleSize = seanet.StepAngleSize;
				NBins = seanet.NBins;

				alpha_max_err = seanet.alpha_max_err;
				d_max_err = seanet.d_max_err;
				rangescale = seanet.RangeScale;

				index_scanlines_prev = 0;
				index_scanlines = 0;

				free(tvs); free(angles); free(scanlines);
				tvs = NULL; angles = NULL; scanlines = NULL;
				tvs = (struct timeval*)calloc(seanet.NSteps, sizeof(struct timeval));
				angles = (double*)calloc(seanet.NSteps, sizeof(double));
				scanlines = (unsigned char*)calloc(seanet.NSteps*MAX_NB_BYTES_SEANET, sizeof(unsigned char));
				if ((tvs == NULL)||(angles == NULL)||(scanlines == NULL))
				{
					printf("Unable to allocate Seanet data.\n");
					LeaveCriticalSection(&SeanetConnectingCS);
					break;
				}

				LeaveCriticalSection(&SeanetConnectingCS);

				if (seanet.pfSaveFile != NULL)
				{
					fclose(seanet.pfSaveFile);
					seanet.pfSaveFile = NULL;
				}
				if ((seanet.bSaveRawData)&&(seanet.pfSaveFile == NULL))
				{
					if (strlen(seanet.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", seanet.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "seanet");
					}
					// Remove the extension.
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.csv", szTemp, strtime_fns());
					LeaveCriticalSection(&strtimeCS);
					seanet.pfSaveFile = fopen(szSaveFilePath, "w");
					if (seanet.pfSaveFile == NULL)
					{
						printf("Unable to create Seanet data file.\n");
						break;
					}
					fprintf(seanet.pfSaveFile,
						"SOf,DateTime,Node,Status,Hdctrl,Rangescale,Gain,Slope,AdLow,AdSpan,LeftLim,RightLim,Steps,Bearing,Dbytes,Dbytes of DATA\n"
						);
					fflush(seanet.pfSaveFile);
				}
			}
			else
			{
				bConnected = FALSE;
				mSleep(1000);
			}
		}
		else
		{
			nbauxbytes = 0;
			// Swap commented line to enable/disable Aux device support...
			//if (GetHeadDataSeanet(&seanet, scanline, &angle) == EXIT_SUCCESS)
			if (GetHeadDataAndAuxDataSeanet(&seanet, scanline, &angle, auxbuf, &nbauxbytes) == EXIT_SUCCESS)
			{
				time_t tt = 0;
				struct tm* timeptr = NULL;
				int Hour = 0, Min = 0;
				double Seconds = 0;

				// Time...
				if (gettimeofday(&tv, NULL) != EXIT_SUCCESS)
				{
					tv.tv_sec = 0;
					tv.tv_usec = 0;
				}

				tt = tv.tv_sec;
				timeptr = localtime(&tt);
				if (timeptr != NULL)
				{
					Hour = timeptr->tm_hour;
					Min = timeptr->tm_min;
					Seconds = timeptr->tm_sec+0.000001*tv.tv_usec;
				}

				if (nbauxbytes > 0)
				{
					// Echosounder.
					if (sscanf((char*)auxbuf, "%lfm\r", &distance) == 1)
					{
						EnterCriticalSection(&StateVariablesCS);

						altitude_sea_floor = distance;

						LeaveCriticalSection(&StateVariablesCS);
					}

					if (seanet.bSaveRawData)
					{
						// Should remove the final LF from auxbuf (maybe in HeadDataReplyAndAuxDataSeanet()...)
						// and end-of-line characters might be inconsistent with Seanet log files (especially on Linux)...

						fprintf(seanet.pfSaveFile, "%02d:%02d:%06.3f,%.100s\n", Hour, Min, Seconds, (char*)auxbuf);
					}
				}

				EnterCriticalSection(&SeanetDataCS);

				tvs[index_scanlines] = tv;
				angles[index_scanlines] = angle;
				memcpy(scanlines+index_scanlines*seanet.NBins, scanline, seanet.NBins);
				index_scanlines++;
				index_scanlines = index_scanlines%seanet.NSteps;

				//// For direct waterfall display...
				//memmove(tvs+1, tvs, (seanet.NSteps-1)*sizeof(struct timeval));
				//tvs[0] = tv;
				//memmove(angles+1, angles, (seanet.NSteps-1)*sizeof(double));
				//angles[0] = angle;
				//memmove(scanlines+seanet.NBins, scanlines, (seanet.NSteps-1)*seanet.NBins);
				//memcpy(scanlines, scanline, seanet.NBins);

				LeaveCriticalSection(&SeanetDataCS);

				if (seanet.bSaveRawData)
				{
					char SOf[3+1];
					int Bearing = 0;

					if (seanet.bDST) strcpy(SOf, "DIG"); else strcpy(SOf, "SON");

					Bearing = ((int)(angle/0.05625+3200+6400))%6400; // Angle of the transducer (0..6399 in 1/16 Gradian units, 0.05625 = (1/16)*(9/10)).

					fprintf(seanet.pfSaveFile, "%.3s,%02d:%02d:%06.3f,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
						SOf, Hour, Min, Seconds, SERIAL_PORT_SONAR_NODE_NUMBER_SEANET,
						seanet.HeadStatus, seanet.HeadHdCtrl.i, seanet.HeadRangescale, seanet.HeadIGain,
						seanet.HeadSlope, seanet.HeadADLow, seanet.HeadADSpan, seanet.HeadLeftLim, seanet.HeadRightLim,
						seanet.HeadSteps, Bearing, seanet.Dbytes
						);

					// We should take into account ADLow and ADSpan here?

					if (!seanet.adc8on)
					{
						for (i = 0; i < seanet.Dbytes; i++)
						{
							int byte = ((scanline[2*i]/16)<<4)+scanline[2*i+1]/16;
							fprintf(seanet.pfSaveFile, ",%d", byte);
						}
					}
					else
					{
						for (i = 0; i < seanet.Dbytes; i++)
						{
							fprintf(seanet.pfSaveFile, ",%d", (int)scanline[i]);
						}
					}

					fprintf(seanet.pfSaveFile, "\n");

					fflush(seanet.pfSaveFile);
				}
			}
			else
			{
				printf("Connection to a Seanet lost.\n");
				bConnected = FALSE;
				DisconnectSeanet(&seanet);
				mSleep(100);
			}
		}

		if (bExit) break;
	}

	if (seanet.pfSaveFile != NULL)
	{
		fclose(seanet.pfSaveFile);
		seanet.pfSaveFile = NULL;
	}

	EnterCriticalSection(&SeanetConnectingCS);
	free(scanlines);
	scanlines = NULL;
	free(angles);
	angles = NULL;
	free(tvs);
	tvs = NULL;
	LeaveCriticalSection(&SeanetConnectingCS);

	if (bConnected) DisconnectSeanet(&seanet);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
