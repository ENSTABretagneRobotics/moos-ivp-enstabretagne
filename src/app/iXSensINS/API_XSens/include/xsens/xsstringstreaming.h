#ifndef XSSTRINGSTREAMING_H
#define XSSTRINGSTREAMING_H

#ifdef __cplusplus
#include "xsstring.h"
#include "xsvector.h"
#include "xsmatrix.h"
#include "xsquaternion.h"

#ifndef XSENS_NO_STL
#include <ostream>
namespace std {

template<typename _CharT, typename _Traits>
basic_ostream<_CharT, _Traits>& operator<<(basic_ostream<_CharT, _Traits>& o, XsVector const& xv)
{
	o << "V<" << xv.size() << ">(";
	for (XsSize i = 0; i < xv.size() - 1; i++)
		o << xv[i] << ", ";
	return (o << xv[xv.size() - 1] << ")");
}

template<typename _CharT, typename _Traits>
basic_ostream<_CharT, _Traits>& operator<<(basic_ostream<_CharT, _Traits>& o, XsMatrix const& xm)
{
	o << "M<" << xm.rows() << "," << xm.cols() << ">(";
	for (XsSize r = 0; r < xm.rows(); ++r)
	{
		for (XsSize c = 0; c < xm.cols() - 1; ++c)
			o << xm[r][c] << ", ";
		o << xm[r][xm.cols()-1];
		if (r < xm.rows() - 1)
			o << "\n";
	}
	o << ")";
	return o;
}

template<typename _CharT, typename _Traits>
basic_ostream<_CharT, _Traits>& operator<<(basic_ostream<_CharT, _Traits>& o, XsQuaternion const& xq)
{
	o << "Q(";
	for (int i = 0; i < 3; i++)
		o << xq[i] << ", ";
	return (o << xq[3] << ")");
}

}
#endif

inline XsString& operator<<(XsString& o, XsSize const& v)
{
	char buffer[32];	// 2e64 = 1.8e19 so this should be enough
	sprintf(buffer, "%" PRINTF_SIZET_MODIFIER "d", v);
	o << XsString(buffer);
	return o;
}

inline XsString& operator<<(XsString& o, XsReal const& v)
{
	char buffer[64];
	sprintf(buffer, "%g", v);
	o << XsString(buffer);
	return o;
}

inline XsString& operator<<(XsString& o, XsVector const& xv)
{
	o << "V<" << xv.size() << ">(";
	for (XsSize i = 0; i < xv.size() - 1; i++)
		o << xv[i] << ", ";
	return (o << xv[xv.size() - 1] << ")");
}

inline XsString& operator<<(XsString& o, XsMatrix const& xm)
{
	o << "M<" << xm.rows() << "," << xm.cols() << ">(";
	for (XsSize r = 0; r < xm.rows(); ++r)
	{
		if (xm.cols())
		{
			if (r > 0)
				o << "\t";
			for (XsSize c = 0; c < xm.cols() - 1; ++c)
				o << xm[r][c] << ", ";
			o << xm[r][xm.cols()-1];
			if (r < xm.rows() - 1)
				o << "\n";
		}
	}
	o << ")";
	return o;
}

inline XsString& operator<<(XsString& o, XsQuaternion const& xq)
{
	o << "Q(";
	for (int i = 0; i < 3; i++)
		o << xq[i] << ", ";
	return (o << xq[3] << ")");
}

#endif

#endif // file guard
