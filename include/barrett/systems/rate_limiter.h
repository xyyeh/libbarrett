/*
	Copyright 2012 Barrett Technology <support@barrett.com>

	This file is part of libbarrett.

	This version of libbarrett is free software: you can redistribute it
	and/or modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of the
	License, or (at your option) any later version.

	This version of libbarrett is distributed in the hope that it will be
	useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along
	with this version of libbarrett.  If not, see
	<http://www.gnu.org/licenses/>.

	Further, non-binding information about licensing is available at:
	<http://wiki.barrett.com/libbarrett/wiki/LicenseNotes>
*/

/*
 * rate_limiter.h
 *
 *  Created on: Apr 12, 2012
 *      Author: dc
 */

#ifndef BARRETT_SYSTEMS_RATE_LIMITER_H_
#define BARRETT_SYSTEMS_RATE_LIMITER_H_


#include <string>

#include <Eigen/Core>

#include <barrett/detail/ca_macro.h>
#include <barrett/math/traits.h>
#include <barrett/systems/abstract/single_io.h>


namespace barrett {
namespace systems {


template<typename T, typename MathTraits = math::Traits<T> >
class RateLimiter : public SingleIO<T,T> {
protected:
	typedef MathTraits MT;

public:
	RateLimiter(const T& limit = T(0.0), const std::string& sysName = "RateLimiter") :
		SingleIO<T,T>(sysName), T_s(0.0), limit(), maxDelta(), data(), delta()
	{
		setLimit(limit);
		getSamplePeriodFromEM();
	}
	~RateLimiter() {
		this->mandatoryCleanUp();
	}

	void setLimit(const T& newLimit);

	void setCurVal(const T& newPos);

protected:
	double T_s;
	T limit, maxDelta;
	T data;

	T delta;
	virtual void operate();

	virtual void onExecutionManagerChanged() {
		SingleIO<T,T>::onExecutionManagerChanged();  // First, call super
		getSamplePeriodFromEM();
	}
	void getSamplePeriodFromEM();

private:
	DISALLOW_COPY_AND_ASSIGN(RateLimiter);

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW_IF(MT::RequiresAlignment)
};


}
}


// include template definitions
#include <barrett/systems/detail/rate_limiter-inl.h>


#endif /* BARRETT_SYSTEMS_RATE_LIMITER_H_ */
