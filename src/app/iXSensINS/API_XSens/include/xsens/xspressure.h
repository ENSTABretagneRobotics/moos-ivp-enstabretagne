#ifndef XSPRESSURE_H
#define XSPRESSURE_H

#include "pstdint.h"

/*! \brief Pressure data.
	\details Contains the pressure data and the pressure age
*/
struct XsPressure {
#ifdef __cplusplus
	/*! \brief Create an XsPressure
		\param pressure the pressure
		\param age the pressure age
	*/
	explicit XsPressure(double pressure = 0, uint8_t age = 0) :
		m_pressure(pressure),
		m_pressureAge(age)
	{
	}

	/*! \brief Create a new XsPressure as copy from \a other
		\param other the pressure carrier to copy from
	*/
	inline XsPressure(XsPressure const& other) :
		m_pressure(other.m_pressure),
		m_pressureAge(other.m_pressureAge)
	{
	}

	/*! \brief Copy the data from \a other
		\param other the pressure carrier to copy from
		\return this
	*/
	inline XsPressure const & operator=(XsPressure const& other)
	{
		m_pressure = other.m_pressure;
		m_pressureAge = other.m_pressureAge;
		return *this;
	}

	/*! \brief Return true if this is equal to \a other
		\param other the pressure carrier to compare against
		\return true if both XsPressures are equal
	*/
	inline bool operator==(XsPressure const& other) const
	{
		return other.m_pressure == m_pressure && other.m_pressureAge == m_pressureAge;
	}
#endif
	double		m_pressure;		//!< Pressure in Pascal
	uint8_t		m_pressureAge;	//!< Age of pressure data in samples
};
typedef struct XsPressure XsPressure;

#endif // file guard
