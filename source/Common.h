/*******************************************************************************
 * ALC Reasoner                                                                *
 * Copyright (c) 2012 Canio Massimo Tristano <massimo.tristano@gmail.com>      *
 *                                                                             *
 * This software is provided 'as-is', without any express or implied           *
 * warranty. In no event will the authors be held liable for any damages       *
 * arising from the use of this software.                                      *
 *	                                                                            *
 * Permission is granted to anyone to use this software for any purpose,       *
 * including commercial applications, and to alter it and redistribute it      *
 * freely, subject to the following restrictions:                              *
 *                                                                             *
 *  1. The origin of this software must not be misrepresented; you must not    *
 *  claim that you wrote the original software. If you use this software       *
 *  in a product, an acknowledgment in the product documentation would be      *
 *  appreciated but is not required.                                           *
 *                                                                             *
 *  2. Altered source versions must be plainly marked as such, and must not be *
 *  misrepresented as being the original software.                             *
 *                                                                             *
 * 3. This notice may not be removed or altered from any source                *
 * distribution.                                                               *
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

namespace salcr
{
typedef unsigned long Symbol;

// Forward decls
class Concept;
class ConceptManager;
class Reasoner;
class SymbolDictionary;
class Model;
class Instance;

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

