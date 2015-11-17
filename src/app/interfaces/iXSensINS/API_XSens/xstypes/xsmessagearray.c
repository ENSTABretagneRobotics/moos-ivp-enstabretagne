#include "xsmessagearray.h"
#include "xsmessage.h"

/*! \struct XsMessageArray
	\brief A list of XsMessage values
	\sa XsArray
*/

//! \brief Descriptor for XsMessageArray
XsArrayDescriptor const g_xsMessageArrayDescriptor = {
	//lint --e{64} ignore exact type mismatches here
	sizeof(XsMessage),
	XSEXPCASTITEMSWAP XsMessage_swap,
	XSEXPCASTITEMMAKE XsMessage_construct,
	XSEXPCASTITEMCOPY XsMessage_copyConstruct,
	XSEXPCASTITEMMAKE XsMessage_destruct,
	XSEXPCASTITEMCOPY XsMessage_copy,
	XSEXPCASTITEMCOMP XsMessage_compare,
	0
};

/*! \copydoc XsArray_construct
	\note Specialization for XsStringArray
*/
void XsMessageArray_construct(XsMessageArray* thisPtr, XsSize count, XsMessage const* src)
{
	XsArray_construct(thisPtr, &g_xsMessageArrayDescriptor, count, src);
}
