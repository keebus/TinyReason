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

#include "Common.h"

namespace salcr
{

class SymbolDictionary {
public:
	SymbolDictionary();
	virtual ~SymbolDictionary();
	bool isDefined(const std::string& name) const;
	Symbol get(const std::string& name);
	Symbol toSymbol(const std::string& name) const;
	const std::string & toName(Symbol symbol) const;
private:
	typedef std::map<Symbol, std::string> SymbolToNameMap;
	typedef std::map<std::string, Symbol> NameToSymbolMap;
	Symbol mNextFreeSymbol;
	SymbolToNameMap mSymbolToNameMap;
	NameToSymbolMap mNameToSymbolMap;
};

}

