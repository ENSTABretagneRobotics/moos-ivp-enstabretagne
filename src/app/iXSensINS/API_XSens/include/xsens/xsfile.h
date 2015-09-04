#ifndef XSFILE_H
#define XSFILE_H

#include "xstypesconfig.h"
#include "pstdint.h"
#include <stdio.h>
#include "xsresultvalue.h"
#include "xsfilepos.h"
#include "xsstring.h"

#ifdef _MSC_VER
#	define XS_MAX_FILENAME_LENGTH	512
#else
#	define XS_MAX_FILENAME_LENGTH	PATH_MAX
#endif

#ifdef __cplusplus
extern "C" {
#else
#define XSFILE_INITIALIZER { 0 }
#endif

struct XsFile;

XSTYPES_DLL_API void XsFile_destruct(struct XsFile* thisPtr);

XSTYPES_DLL_API XsResultValue XsFile_create(struct XsFile *thisPtr, const struct XsString* filename, int writeOnly);
XSTYPES_DLL_API XsResultValue XsFile_createText(struct XsFile *thisPtr, const struct XsString* filename, int writeOnly);
XSTYPES_DLL_API XsResultValue XsFile_open(struct XsFile *thisPtr, const struct XsString* filename, int readOnly);
XSTYPES_DLL_API XsResultValue XsFile_openText(struct XsFile *thisPtr, const struct XsString* filename, int readOnly);
XSTYPES_DLL_API XsResultValue XsFile_reopen(struct XsFile *thisPtr, const struct XsString* filename, const struct XsString* mode);
XSTYPES_DLL_API int XsFile_isOpen(struct XsFile *thisPtr);
XSTYPES_DLL_API XsResultValue XsFile_close(struct XsFile *thisPtr);

XSTYPES_DLL_API int XsFile_exists(const struct XsString* filename);

XSTYPES_DLL_API XsResultValue XsFile_flush(struct XsFile *thisPtr);
XSTYPES_DLL_API XsResultValue XsFile_truncate(struct XsFile *thisPtr, int32_t fileSize);
XSTYPES_DLL_API XsResultValue XsFile_erase(const struct XsString* filename);

XSTYPES_DLL_API XsSize XsFile_read(struct XsFile *thisPtr, void *destination, XsSize size, XsSize count);
XSTYPES_DLL_API XsSize XsFile_write(struct XsFile *thisPtr, const void *source, XsSize size, XsSize count);
XSTYPES_DLL_API int XsFile_getc(struct XsFile *thisPtr);
XSTYPES_DLL_API XsResultValue XsFile_putc(struct XsFile *thisPtr, int character);
XSTYPES_DLL_API char* XsFile_gets(struct XsFile *thisPtr, char *str, int num);
XSTYPES_DLL_API XsResultValue XsFile_puts(struct XsFile *thisPtr, const char *str);

XSTYPES_DLL_API XsResultValue XsFile_seek(struct XsFile* thisPtr, XsFilePos offset);
XSTYPES_DLL_API XsResultValue XsFile_seek_r(struct XsFile* thisPtr, XsFilePos offset);
XSTYPES_DLL_API XsFilePos XsFile_tell(struct XsFile *thisPtr);

XSTYPES_DLL_API int XsFile_eof(struct XsFile *thisPtr);
XSTYPES_DLL_API XsResultValue XsFile_error(struct XsFile *thisPtr);
XSTYPES_DLL_API XsResultValue XsFile_fullPath(const struct XsString* filename, struct XsString* fullPath);

XSTYPES_DLL_API XsResultValue XsFile_getline(struct XsFile *thisPtr, struct XsString *line);

#ifdef __cplusplus
} // extern "C"
#endif

struct XsFile {
#ifdef __cplusplus
	/*! \brief Default constructor, creates an empty file object */
	explicit inline XsFile(void)
		: m_handle(NULL)
	{
		// Silence -Wunused-private-field
		// This field is used in the C back-end, but
		// some compilers may not notice this
		(void)m_handle;
	}

	/*! \brief \copybrief XsFile_destruct */
	inline ~XsFile()
	{
		XsFile_destruct(this);
	}

	/*! \brief \copybrief XsFile_create */
	inline XsResultValue create(const XsString &filename, bool writeOnly)
	{
		return XsFile_create(this, &filename, writeOnly ? 1 : 0);
	}

	/*! \brief \copybrief XsFile_createText */
	inline XsResultValue createText(const XsString &filename, bool writeOnly)
	{
		return XsFile_createText(this, &filename, writeOnly ? 1 : 0);
	}

	/*! \brief \copybrief XsFile_open */
	inline XsResultValue open(const XsString &fileName, bool readOnly)
	{
		return XsFile_open(this, &fileName, readOnly ? 1 : 0);
	}

	/*! \brief \copybrief XsFile_openText */
	inline XsResultValue openText(const XsString &fileName, bool readOnly)
	{
		return XsFile_openText(this, &fileName, readOnly ? 1 : 0);
	}

	/*! \brief \copybrief XsFile_openText */
	inline XsResultValue reopen(const XsString &fileName, const XsString &mode)
	{
		return XsFile_reopen(this, &fileName, &mode);
	}

	/*! \brief \copybrief XsFile_isOpen */
	inline bool isOpen()
	{
		return (XsFile_isOpen(this) == 0);
	}

	/*! \brief \copybrief XsFile_exists */
	static inline bool exists(const XsString &fileName)
	{
		return (XsFile_exists(&fileName) == 0);
	}

	/*! \brief \copybrief XsFile_close */
	inline XsResultValue close()
	{
		return XsFile_close(this);
	}

	/*! \brief \copybrief XsFile_flush */
	inline XsResultValue flush()
	{
		return XsFile_flush(this);
	}

	/*! \brief \copybrief XsFile_truncate */
	inline XsResultValue truncate(int32_t fileSize)
	{
		return XsFile_truncate(this, fileSize);
	}

	/*! \brief \copybrief XsFile_erase */
	static XsResultValue erase(const XsString& filename)
	{
		return XsFile_erase(&filename);
	}

	/*! \brief \copybrief XsFile_read */
	inline XsSize read(void *destination, XsSize size, XsSize count)
	{
		return XsFile_read(this, destination, size, count);
	}

	/*! \brief \copybrief XsFile_write */
	inline XsSize write(const void *source, XsSize size, XsSize count)
	{
		return XsFile_write(this, source, size, count);
	}

	/*! \brief \copybrief XsFile_getc */
	inline int getc()
	{
		return XsFile_getc(this);
	}

	/*! \brief \copybrief XsFile_putc */
	inline XsResultValue putc(int character)
	{
		return XsFile_putc(this, character);
	}

	/*! \brief \copybrief XsFile_gets */
	inline char* gets(char *destination, int maxCount)
	{
		return XsFile_gets(this, destination, maxCount);
	}

	/*! \brief \copybrief XsFile_puts */
	inline XsResultValue puts(const char *source)
	{
		return XsFile_puts(this, source);
	}

	/*! \brief \copybrief XsFile_seek */
	inline XsResultValue seek(XsFilePos offset)
	{
		return XsFile_seek(this, offset);
	}

	/*! \brief \copybrief XsFile_seek_r */
	inline XsResultValue seek_r(XsFilePos offset)
	{
		return XsFile_seek_r(this, offset);
	}

	/*! \brief \copybrief XsFile_tell */
	inline XsFilePos tell()
	{
		return XsFile_tell(this);
	}

	/*! \brief \copybrief XsFile_eof */
	inline bool eof()
	{
		return (0!=XsFile_eof(this));
	}

	/*! \brief \copybrief XsFile_error */
	inline XsResultValue error()
	{
		return XsFile_error(this);
	}

	/*! \brief \copybrief XsFile_fullPath */
	static XsResultValue fullPath(const XsString &filename, XsString &fullPath)
	{
		return XsFile_fullPath(&filename, &fullPath);
	}

	/*! \brief \copybrief XsFile_getline */
	inline XsResultValue getline(XsString& line)
	{
		return XsFile_getline(this, &line);
	}

	/*! \brief \copybrief XsFile_getline */
	inline XsResultValue getline(std::string& line)
	{
		XsString tmp;
		XsResultValue rv = XsFile_getline(this, &tmp);
		if (rv == XRV_OK)
			line = tmp.toStdString();
		return rv;
	}
private:
#endif
	FILE* m_handle;
};

typedef struct XsFile XsFile;

#endif // file guard
