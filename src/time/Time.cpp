/*
 * \file: Time.cpp
 * \brief: Created by hushouguo at Jul 06 2017 20:46:41
 */

#include "bundle.h"

BEGIN_NAMESPACE_BUNDLE {
	Time::Time() {
		this->now();
	}

	Time::Time(u64 milliseconds) {
		this->_tv.tv_sec = milliseconds / 1000;
		this->_tv.tv_usec = (milliseconds % 1000) * 1000;
	}

	Time::Time(u64 secondPart, u64 millisecondPart) {
		this->_tv.tv_sec = secondPart;
		this->_tv.tv_usec = millisecondPart * 1000;
	}

	Time::Time(const Time& t) {
		this->_tv.tv_sec = t.secondPart();
		this->_tv.tv_usec = t.microsecondsPart();
	}
	
	void Time::now() {
		gettimeofday(&this->_tv, nullptr);
 	}

	void Time::operator = (u64 milliseconds) {
		this->_tv.tv_sec = milliseconds / 1000;
		this->_tv.tv_usec = (milliseconds % 1000) * 1000;
 	}
	
	void Time::operator = (const Time& t) {
		this->_tv.tv_sec = t.secondPart();
		this->_tv.tv_usec = t.microsecondsPart();
	}
	
	u64  Time::operator + (u64 milliseconds) {
		return this->milliseconds() + milliseconds;
	}
	
	u64  Time::operator + (const Time& t) {
		return this->milliseconds() + t.milliseconds();
	}
	
	u64  Time::operator - (u64 milliseconds) {
		return this->milliseconds() >= milliseconds ? this->milliseconds() - milliseconds : 0;
	}
	
	u64  Time::operator - (const Time& t) {
		return this->milliseconds() >= t.milliseconds() ? this->milliseconds() - t.milliseconds() : 0;
	}
	
	bool Time::operator > (const Time& t) {
		return this->milliseconds() > t.milliseconds();
	}
	
	bool Time::operator < (const Time& t) {
		return this->milliseconds() < t.milliseconds();
	}

	INITIALIZE_INSTANCE(Time);
}

