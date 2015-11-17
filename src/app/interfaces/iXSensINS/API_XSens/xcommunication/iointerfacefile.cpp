#include "iointerfacefile.h"

#include <errno.h>
#ifndef _WIN32
#	include <unistd.h>		// close
#	include <sys/ioctl.h>	// ioctl
#	include <fcntl.h>		// open, O_RDWR
#	include <string.h>		// strcpy
#	include <sys/param.h>
#	include <sys/stat.h>
#	include <stdarg.h>
#else
#	include <winbase.h>
#	include <sys/stat.h>
#   include <io.h>
#endif

#include <xsens/xsdeviceid.h>

#ifndef _CRT_SECURE_NO_DEPRECATE
#	define _CRT_SECURE_NO_DEPRECATE
#	ifdef _WIN32
#		pragma warning(disable:4996)
#	endif
#endif

// maybe log to nothing at this level
#ifdef LOG_CMT1
#	include "xslog.h"
#	define XDA1LOG_OBSOLETE		XSENSLOG
#else
#	define XDA1LOG_OBSOLETE(...)	(void)0
#endif

//////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// IoInterfaceFile  /////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

/*! Default constructor, initializes all members to their default values.
*/
IoInterfaceFile::IoInterfaceFile()
{
	m_readPos = 0;
	m_writePos = 0;
	m_lastResult = XRV_OK;
	m_reading = true;
	m_fileSize = 0;
	m_readOnly = false;
	m_handle = NULL;
}

/*! Destructor
*/
IoInterfaceFile::~IoInterfaceFile()
{
	try {
		closeFile();	//lint !e534
	} catch(...)
	{}
}

/*! \brief Write data to the end of the file.
	\details The function writes the given data to the file at the end. The current write
	position is also moved to the end of the file.
	\param bdata The byte data to append to the file
	\returns XRV_OK if the write was successful
*/
XsResultValue IoInterfaceFile::appendData(const XsByteArray& bdata)
{
	if (!m_handle)
		return m_lastResult = XRV_NOFILEOPEN;
	if (m_readOnly)
		return m_lastResult = XRV_READONLY;
	if (!bdata.size())
		return m_lastResult = XRV_OK;

	if (m_reading || m_writePos != m_fileSize)
	{
		m_reading = false;
		m_handle->seek_r(0); //lint !e534
	}
	size_t bytesWritten = m_handle->write(bdata.data(), 1, bdata.size());
	(void)bytesWritten;
	m_writePos = m_handle->tell();
	m_fileSize = m_writePos;

	return (m_lastResult = XRV_OK);
}

//////////////////////////////////////////////////////////////////////////////////////////
/*! \brief Close the file, overrides IoInterface::close().
	\returns XRV_OK if the file was closed successfully
*/
XsResultValue IoInterfaceFile::close (void)
{
	return closeFile();
}

/*! \brief Close the file.
	\returns XRV_OK if the file was closed successfully
*/
XsResultValue IoInterfaceFile::closeFile(void)
{
	if (m_handle)
	{
		m_handle->flush();
		m_handle->close();

		delete m_handle;
		m_handle = NULL;
	}

	m_readPos = 0;
	m_writePos = 0;
	m_reading = true;
	m_fileSize = 0;
	m_readOnly = false;

	return m_lastResult = XRV_OK;
}

/*! \brief Close the file and delete it.
	\returns XRV_OK if the file was closed and deleted successfully
*/
XsResultValue IoInterfaceFile::closeAndDelete(void)
{
	if (m_handle)
	{
		m_handle->flush();
		m_handle->close();

		if (m_readOnly)
			m_lastResult = XRV_READONLY;
		else
		{
			m_lastResult = XsFile::erase(m_filename);
		}

		delete m_handle;
		m_handle = NULL;
	}
	else
		m_lastResult = XRV_NOFILEOPEN;

	m_readPos = 0;
	m_writePos = 0;
	m_reading = true;
	m_fileSize = 0;
	m_readOnly = false;

	return m_lastResult;
}

/*! \brief Create an empty file
	\param filename The desired (path+)name of the file
	\returns XRV_OK if the file was created successfully
*/
XsResultValue IoInterfaceFile::create(const XsString& filename)
{
	if (m_handle)
		return m_lastResult = XRV_ALREADYOPEN;

	m_handle = new XsFile();
	m_lastResult = m_handle->create(filename, false);
	if (m_lastResult != XRV_OK)
	{
		delete m_handle;
		m_handle = NULL;
		return m_lastResult;
	}

	m_lastResult = XsFile::fullPath(filename, m_filename);

	if (m_lastResult != XRV_OK)
	{
		m_handle->close();
		XsFile::erase(m_filename);
		delete m_handle;
		m_handle = NULL;

		return m_lastResult = XRV_INVALIDPARAM;
	}

	m_readPos = 0;
	m_writePos = 0;
	m_fileSize = 0;
	m_reading = true;
	m_readOnly = false;
	return m_lastResult = XRV_OK;
}

/*! \brief Delete the given data from the file.
	\details The function erases the given data from the file at the given write position. This
	operation may take a while to complete, but is faster than insertData.

	The write position is not changed and the read position	is checked for validity upon function exit.
	\param start The offset of the first byte to delete
	\param length The total number of bytes to delete
	\returns XRV_OK if the data was deleted successfully
*/
XsResultValue IoInterfaceFile::deleteData (XsFilePos start, XsSize length)
{
	if (!m_handle)
		return m_lastResult = XRV_NOFILEOPEN;
	if (m_readOnly)
		return m_lastResult = XRV_READONLY;

	gotoWrite();

	XsFilePos wPos = start;
	XsFilePos rPos = wPos + length;

	size_t read1;
	XsFilePos endPos = (start + (XsFilePos) length);
	if (endPos < m_fileSize)
	{
		XsFilePos remaining = m_fileSize - endPos;
		char buffer[512];

		// copy data
		m_handle->seek(rPos);

		while (remaining > 0)
		{
			if (remaining >= 512)
				read1 = m_handle->read(buffer,1,512);
			else
				read1 = m_handle->read(buffer,1,(size_t) remaining);

			remaining -= read1;
			rPos += read1;

			// write block to the correct position
			m_handle->seek(wPos);
			wPos += m_handle->write(buffer, 1, read1);
			m_handle->seek(rPos);
		}
		m_fileSize -= length;
	}
	else
	{
		m_fileSize = start;
	}

	XsResultValue truncateResult = m_handle->truncate((int32_t) m_fileSize);

	m_writePos = start;
	m_handle->seek(wPos);

	return m_lastResult = truncateResult;
}

/*! \brief Find a string of bytes in the file
	\details The function searches from the current read position until the given \c needle is
	found. If the needle is not found, XsResultValue::NOT_FOUND is returned. The function
	will update the seek position to the first character of the found needle.
	\param needleV	The byte string to find.
	\param pos	The position where \a needleV was found. This will point to the first character
				of the found \a needleV.
	\returns XRV_OK if the data was found, XRV_ENDOFFILE if it wasn't found
*/
XsResultValue IoInterfaceFile::find(const XsByteArray& needleV, XsFilePos& pos)
{
	if (!m_handle)
		return m_lastResult = XRV_NOFILEOPEN;

	XsSize needleLength = needleV.size();

	pos = 0;
	if (needleLength == 0)
		return m_lastResult = XRV_OK;

	const char* needle = (const char*) needleV.data();

	gotoRead();

	char buffer[512];
	uint32_t bufferPos, needlePos = 0;
	size_t readBytes;
	if (m_readPos & 0x1FF)										// read a block of data
		readBytes = m_handle->read(buffer, 1, (512-((size_t) m_readPos & 0x1FF)));
	else
		readBytes = m_handle->read(buffer, 1, 512);		// read a block of data

	while (readBytes > 0)
	{
		m_readPos += readBytes;
		bufferPos = 0;

		while (bufferPos < readBytes && needlePos < needleLength)
		{
			if (buffer[bufferPos] == needle[needlePos])
			{
				// found a byte
				++needlePos;
			}
			else
			{
				if (needlePos > 0)
					needlePos = 0;
				else
				if (buffer[bufferPos] == needle[0])
				{
					// found a byte
					needlePos = 1;
				}
			}
			++bufferPos;
		}
		if (needlePos < needleLength)
			readBytes = m_handle->read(buffer, 1, 512);	// read next block
		else
		{
			m_readPos = m_readPos + bufferPos - readBytes - needleLength; // or without needleLength
			pos = m_readPos; // - needleLength;
			m_handle->seek(m_readPos);
			return m_lastResult = XRV_OK;
		}
	}
	return m_lastResult = XRV_ENDOFFILE;
}

/*! \brief Return the size of the file.
	\returns The size of the file.
*/
XsFilePos IoInterfaceFile::getFileSize(void) const
{
	return m_fileSize;
}

/*! \brief Return the creation date of the file
	\returns The creation date of the file
*/
XsTimeStamp IoInterfaceFile::getFileDate(void) const
{
#ifdef _WIN32
	struct _stat stats;
	if (_wstat(m_filename.toStdWString().c_str(), &stats) == 0)
#else
	struct stat stats;
	if (stat(m_filename.c_str(), &stats) == 0)
#endif
	{
		XsTimeStamp t = XsTimeStamp( (int64_t)stats.st_mtime * 1000);
		return t;
	}
	return XsTimeStamp();
}

/*! \copydoc IoInterface::flushData */
XsResultValue IoInterfaceFile::flushData (void)
{
	m_handle->flush();

	return m_lastResult = XRV_OK;
}

/*! \brief Retrieve the filename that was last successfully opened.

	\param filename	The XsString which will contain the filename.
	\returns XRV_OK
*/
XsResultValue IoInterfaceFile::getName(XsString& filename) const
{
	filename = m_filename;
	return m_lastResult = XRV_OK;
}

/*! \brief Return the filename that was last successfully opened.
*/
XsString IoInterfaceFile::getFileName() const
{
	return m_filename;
}

//! \brief Change from writing to reading mode
void IoInterfaceFile::gotoRead(void)
{
	if (m_reading)
		return;

	m_handle->flush();
	m_handle->seek(m_readPos);
	m_reading = true;
}

//! \brief Change from reading to writing mode
void IoInterfaceFile::gotoWrite(void)
{
	if (!m_reading)
		return;

	m_handle->flush();
	m_handle->seek(m_writePos);
	m_reading = false;
}

/*! \brief Insert the given data into the file.
	\details The function writes the given data to the file at the current write position. This
	operation may take a while to complete.

	The write position is placed at the end of the inserted data.
	\param start The offset in the file to write the first byte
	\param data The data to insert in the file
	\returns XRV_OK if the data was inserted successfully
*/
XsResultValue IoInterfaceFile::insertData (XsFilePos start, const XsByteArray& data)
{
	if (!m_handle)
		return m_lastResult = XRV_NOFILEOPEN;
	if (m_readOnly)
		return m_lastResult = XRV_READONLY;

	gotoWrite();

	XsSize length = data.size();
	XsFilePos rPos = start;
	XsFilePos wPos = rPos + length;

	size_t read1, read2;
	XsFilePos remaining = m_fileSize - start;
	size_t bsize = (length > 512)?length:512;
	char* bufferRoot = (char*) malloc(bsize*2);
	if (!bufferRoot)
		return XRV_OUTOFMEMORY;
	char* buffer1 = bufferRoot;
	char* buffer2 = bufferRoot+bsize;
	char* btemp;

	// copy data
	m_handle->seek(rPos);

	if (data.size() == 0)
		return m_lastResult = XRV_OK;

	if (remaining >= (XsFilePos) bsize)
		read1 = m_handle->read(buffer1, 1, bsize);
	else
		read1 = m_handle->read(buffer1, 1, (size_t) remaining);

	remaining -= read1;
	rPos += read1;

	while(remaining > 0)
	{
		// move data to correct buffer
		read2 = read1;
		btemp = buffer1; buffer1 = buffer2; buffer2 = btemp;

		// read next block
		if (remaining >= (XsFilePos) bsize)
			read1 = m_handle->read(buffer1, 1, bsize);
		else
			read1 = m_handle->read(buffer1, 1, (size_t) remaining);

		remaining -= read1;
		rPos += read1;

		// write block to the correct position
		m_handle->seek(wPos);
		wPos += m_handle->write(buffer2, 1, read2);
		m_handle->seek(rPos);
	}

	m_handle->seek(wPos);
	wPos += m_handle->write(buffer1, 1, read1);

	m_handle->seek(start);
	m_writePos = start + m_handle->write(data.data(), 1, length);
	m_fileSize += length;

	free(bufferRoot);
	return m_lastResult = XRV_OK;
}

/*! \brief Open a file.
	\param filename The name of the file to open
	\param createNew When true, the file will be created if it doesn't exist yet
	\param readOnly When true, the file will be marked as read only for %IoInterfaceFile,
			preventing accidental writes to the file.
	\returns XRV_OK if the file was opened successfully
	\sa createFile
*/
XsResultValue IoInterfaceFile::open(const XsString& filename, bool createNew, bool readOnly)
{
	if (m_handle)
		return m_lastResult = XRV_ALREADYOPEN;

	m_handle = new XsFile();

	//! \test does this work for non-existing files? Or do we need a check and create?
	m_readOnly = readOnly;
	XsResultValue localResult = m_handle->open(filename, readOnly);

	if (localResult != XRV_OK)
	{
		if (createNew)
		{
			localResult = m_handle->create(filename, false);
		}
		else
		{
			// final attempt: open it forced readonly
			localResult = m_handle->open(filename, false);
			m_readOnly = true;
		}
	}
	if (localResult != XRV_OK)
	{
		delete m_handle;
		m_handle = NULL;
		return m_lastResult = XRV_INPUTCANNOTBEOPENED;	// overrides a possible OUTPUTCANNOTBEOPENED
	}

	bool fail = false;
#ifdef _WIN32
	wchar_t fullpath[XS_MAX_FILENAME_LENGTH];
	if (_wfullpath(fullpath,filename.toStdWString().c_str(),XS_MAX_FILENAME_LENGTH) == NULL)
		fail = true;
#else
	// use the same trick again.
	char fullpath[XS_MAX_FILENAME_LENGTH*2];
	if (realpath(filename.c_str(), fullpath) == NULL)
		fail = true;
#endif
	m_filename = XsString(fullpath);

	if (fail)
	{
		m_handle->close();
		delete m_handle;
		m_handle = NULL;
		return m_lastResult = XRV_INVALIDPARAM;
	}

	m_readPos = 0;
	m_writePos = 0;
	m_reading = true;
	m_handle->seek_r(0);
	m_fileSize = m_handle->tell();
	m_handle->seek(0);
	return (m_lastResult = XRV_OK);
}

/*! \copydoc IoInterface::readData
	\note This function reads exactly the number of bytes as requested from the file unless the end
	of file boundary is encountered.
*/
XsResultValue IoInterfaceFile::readData(XsSize maxLength, XsByteArray& data)
{
	if (!m_handle)
		return m_lastResult = XRV_NOFILEOPEN;

	if (maxLength == 0)
	{
		data.clear();
		return m_lastResult = XRV_OK;
	}

	XsSize length;

	gotoRead();
	data.setSize(maxLength);

	length = m_handle->read(data.data(), 1, maxLength);
	if (m_handle->eof())
	{
		data.clear();
		return (m_lastResult = XRV_ENDOFFILE);
	}

	m_readPos += length;
	if (length < maxLength)
		data.pop_back(maxLength - length);
	return m_lastResult = XRV_OK;
}

/*! \brief Read data from the file and put it into the data buffer.

	This function reads upp to the number of bytes as requested from the file.
	The function will also stop if the given terminator character is encountered.
	The terminator is included in the output buffer.
	\param maxLength	The amount of data that will be read.
	\param terminator	A character that will end the read operation if encountered.
	\param bdata		A buffer that will store the read data.
	\returns XRV_OK if the data was read successfully
*/
XsResultValue IoInterfaceFile::readTerminatedData(XsSize maxLength, unsigned char terminator, XsByteArray& bdata)
{
	if (!m_handle)
		return m_lastResult = XRV_NOFILEOPEN;

	if (maxLength == 0)
	{
		bdata.clear();
		return m_lastResult = XRV_OK;
	}

	bdata.setSize(maxLength);
	char *data = (char *) bdata.data();

	XsSize length;
	int32_t readChar;

	gotoRead();

	length = 0;
	readChar = (uint32_t) m_handle->getc();

	while (!m_handle->eof() && !m_handle->error())
	{
		data[length] = (char) readChar;
		++length;
		++m_readPos;

		if (length >= maxLength)
			return m_lastResult = XRV_OK;
		if ((unsigned char) readChar == terminator)
		{
			bdata.pop_back(maxLength - length);
			return m_lastResult = XRV_OK;
		}
	}
	bdata.pop_back(maxLength - length);
	return m_lastResult = XRV_ENDOFFILE;
}

/*! \brief Set the new absolute read position
	\details The read position is checked against the filesize first.
	\param pos The new read position
	\returns XRV_OK if the read position was updated successfully
*/
XsResultValue IoInterfaceFile::setReadPosition (XsFilePos pos)
{
	if (!m_handle)
		return m_lastResult = XRV_NOFILEOPEN;

	if (m_readPos != pos)
	{
		m_readPos = pos;
		if (m_reading)
			m_handle->seek(m_readPos);
	}

	return m_lastResult = XRV_OK;
}

/*! \brief Set the new absolute write position
	\details The write position is checked against the filesize first.
	\param pos The new write position
	\returns XRV_OK if the write position was updated successfully
*/
XsResultValue IoInterfaceFile::setWritePosition(XsFilePos pos)
{
	if (!m_handle)
		return m_lastResult = XRV_NOFILEOPEN;
	if (m_readOnly)
		return m_lastResult = XRV_READONLY;

	if (pos == -1)
	{
		if (m_reading)
			m_reading = false;
		m_handle->seek_r(0);
		m_writePos = m_handle->tell();
	}
	else
	{
		if (m_writePos != pos)
		{
			m_writePos = pos;
			if (!m_reading)
				m_handle->seek(m_writePos);
		}
	}

	return m_lastResult = XRV_OK;
}

/*! \copydoc IoInterface::writeData
	\note The function writes the given data to the file at the current write position.
*/
XsResultValue IoInterfaceFile::writeData(const XsByteArray& data, XsSize *written)
{
	if (!m_handle)
		return m_lastResult = XRV_NOFILEOPEN;
	if (m_readOnly)
		return m_lastResult = XRV_READONLY;

	size_t length = data.size();
	if (length == 0)
		return m_lastResult = XRV_OK;

	gotoWrite();
	size_t writeRes = m_handle->write(data.data(), 1, length);
	if (writeRes == (size_t)EOF || writeRes < length)
	{
		int32_t err = (int32_t)errno;
		switch (err)
		{
		case 0:			break;
		case ENOSPC:	return m_lastResult = XRV_INSUFFICIENTSPACE;
		case ENOMEM:	return m_lastResult = XRV_OUTOFMEMORY;
		default:		return m_lastResult = XRV_ERROR;
		}
	}
	m_writePos += writeRes;
	if (written)
		*written = (uint32_t)writeRes;

	if (m_writePos > m_fileSize)
		m_fileSize = m_writePos;

	return m_lastResult = XRV_OK;
}

/*! \brief Return the current read position.
	\returns The current read position.
*/
XsFilePos IoInterfaceFile::getReadPosition(void) const
{
	return m_readPos;
}

/*! \brief Return the current write position.
	\returns The current write position.
*/
XsFilePos IoInterfaceFile::getWritePosition(void) const
{
	return m_writePos;
}

/*! \brief Return the result code of the last operation.
	\returns The result code of the last operation.
*/
XsResultValue IoInterfaceFile::getLastResult(void) const
{
	return m_lastResult;
}

/*! \brief Return whether the file is open or not.
	\returns true if the file is open
*/
bool IoInterfaceFile::isOpen(void) const
{
	return m_handle != NULL;
}

/*! \brief Return whether the file is readonly or not.
	\returns true if the file is readonly
*/
bool IoInterfaceFile::isReadOnly(void) const
{
	return !isOpen() || m_readOnly;
}

