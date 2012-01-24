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

#include "SymbolDictionary.h"

using namespace std;

namespace salcr
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

