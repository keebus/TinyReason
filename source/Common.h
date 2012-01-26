/*******************************************************************************
 * Tiny Reason                                                                 *
 * Copyright (c) 2012, Canio Massimo Tristano <massimo.tristano@gmail.com>     *
 * All rights reserved.                                                        *
 *                                                                             *
 * Redistribution and use in source and binary forms, with or without          *
 * modification, are permitted provided that the following conditions are met: *
 *     * Redistributions of source code must retain the above copyright        *
 *       notice, this list of conditions and the following disclaimer.         *
 *     * Redistributions in binary form must reproduce the above copyright     *
 *       notice, this list of conditions and the following disclaimer in the   *
 *       documentation and/or other materials provided with the distribution.  *
 *     * Neither the name of the <organization> nor the                        *
 *       names of its contributors may be used to endorse or promote products  *
 *       derived from this software without specific prior written permission. *
 *                                                                             *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" *
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   *
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  *
 * ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY      *
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES  *
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;*
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND *
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT  *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF    *
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.           *
 ******************************************************************************/

#pragma once

#include <iostream>
#include <string>
#include <set>
#include <map>
#include <list>
#include <vector>
#include <sstream>
#include <exception>
#include <queue>
#include <fstream>
#include <algorithm>
#include <tr1/memory>
#define shared_ptr std::tr1::shared_ptr

namespace tinyreason
{
typedef unsigned long Symbol;

// Forward decls
class Concept;
class ConceptManager;
class Reasoner;
class SymbolDictionary;
class Model;
class Individual;

// Base classes

class Exception : public std::exception {
public:
	Exception() throw () { }
	Exception(const Exception& other) throw () {
		*this = other;
	}
	Exception(const std::string& message) throw () {
		mMessageBuffer << message;
	}
	virtual ~Exception() throw () { }
	const char* what() const throw () {
		return mMessageBuffer.str().c_str();
	}
	Exception & operator=(const Exception& other) throw () {
		mMessageBuffer.str(other.mMessageBuffer.str());
		return *this;
	}
protected:
	std::stringstream mMessageBuffer;
};
/* Some basic algorithms */
template<class T>
inline void deleteAll(std::list<T>& someList) {
	for (typename std::list<T>::iterator it = someList.begin(); it != someList.end();)
	{
		delete *it;
		it = someList.erase(it);
	}
}
template<class T>
inline void deleteAll(std::vector<T>& someVector) {
	for (typename std::vector<T>::iterator it = someVector.begin(); it != someVector.end(); ++it)
		delete *it;
	someVector.clear();
}
template<class T>
inline void deleteAll(std::set<T>& someVector) {
	for (typename std::set<T>::iterator it = someVector.begin(); it != someVector.end(); ++it)
		delete *it;
	someVector.clear();
}
template<class T, class Q>
inline void deleteAll(std::map<T, Q>& someMap) {
	for (typename std::map<T, Q>::iterator it = someMap.begin(); it != someMap.end(); ++it)
		delete it->second;
	someMap.clear();
}
template <typename T>
inline std::string toString(const T& t) {
	std::stringstream ss;
	ss << t;
	return ss.str();
}
}

