/*
 * system-output-value-inl.h
 *
 *  Created on: Sep 13, 2009
 *      Author: dc
 */


#include "../../../detail/stl_utils.h"


namespace barrett {
namespace systems {


inline System::AbstractOutput::AbstractValue::AbstractValue(System* parentSys) :
			parentSystem(parentSys)
{
	parentSystem->outputValues.push_back(this);
}

inline System::AbstractOutput::AbstractValue::~AbstractValue()
{
	if (parentSystem != NULL) {
		replaceWithNull(parentSystem->outputValues, this);
//		std::replace(parentSystem->outputValues.begin(), parentSystem->outputValues.end(),
//				const_cast<AbstractValue*>(this),
//				static_cast<AbstractValue*>(NULL));
	}
}


template<typename T>
inline System::Output<T>::Value::~Value()
{
	delete value;  // delete NULL does nothing
}

template<typename T>
inline void System::Output<T>::Value::setValue(const T& newValue) {
	if (value == NULL) {
		value = new T(newValue);
	} else {
		(*value) = newValue;
	}

	undelegate();
}

template<typename T>
inline void System::Output<T>::Value::setValueUndefined() {
	if (value != NULL) {
		delete value;
		value = NULL;
	}

	undelegate();
}

template<typename T>
inline void
System::Output<T>::Value::delegateTo(Output<T>& delegateOutput)
{
	undelegate();
	delegate = &(delegateOutput.value);
	delegate->parentOutput.delegators.push_back(&parentOutput);
}

template<typename T>
inline void System::Output<T>::Value::undelegate()
{
	if (delegate != NULL) {
		delegate->parentOutput.delegators.remove(&parentOutput);
		delegate = NULL;
	}
}


template<typename T>
inline void System::Output<T>::Value::updateValue()
{
	// TODO(dc): test!
	if (parentSystem != NULL) {
		parentSystem->update();
	}
}


}
}
