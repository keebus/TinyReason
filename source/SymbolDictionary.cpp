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

#include "SymbolDictionary.h"

using namespace std;

namespace tinyreason
{

SymbolDictionary::SymbolDictionary() :
mNextFreeSymbol(0)
{
	// Define TOP and BOTTOM strings
	get("nothing");
	get("anything");
}

SymbolDictionary::~SymbolDictionary() { }

bool SymbolDictionary::isDefined(const std::string& name) const
{
	return mNameToSymbolMap.find(name) != mNameToSymbolMap.end();
}

Symbol SymbolDictionary::get(const std::string& name)
{
	NameToSymbolMap::const_iterator it = mNameToSymbolMap.find(name);
	if (it == mNameToSymbolMap.end())
	{
		mNameToSymbolMap[name] = mNextFreeSymbol;
		mSymbolToNameMap[mNextFreeSymbol] = name;
		return mNextFreeSymbol++;
	} else
		return it->second;
}

Symbol SymbolDictionary::toSymbol(const std::string& name) const
{
	NameToSymbolMap::const_iterator it = mNameToSymbolMap.find(name);
	if (it == mNameToSymbolMap.end())
		throw Exception("Undefined symbol \"" + name + "\".");
	else
		return it->second;
}

const std::string& SymbolDictionary::toName(Symbol symbol) const
{
	SymbolToNameMap::const_iterator it = mSymbolToNameMap.find(symbol);
	if (it == mSymbolToNameMap.end())
		throw Exception("Undefined symbol.");
	else
		return it->second;
}

}

