/*******************************************************************************
 * Simple ALC Reasoner                                                         *
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

class Instance {
public:
	const std::set<const Concept*>& getConcepts() {
		return mConcepts;
	}
	void addConcept(const Concept* pConcept) {
		mConcepts.insert(pConcept);
	}
	void addRoleAccessibility(Symbol role, const Instance* pInstance) {
		mRoleAccessibilities.insert(std::pair<Symbol, const Instance*> (role, pInstance));
	}
	void dumpToDOTFile(const SymbolDictionary& symbolDictionary, std::ostream& outStream) const;
private:
	std::set<const Concept*> mConcepts;
	std::multimap<Symbol, const Instance*> mRoleAccessibilities;
};

class Model {
public:
	Model();
	~Model();
	Instance* createInstance();
	void clear();
	void dumpToDOTFile(const SymbolDictionary& symbolDictionary, std::ostream& outStream) const;
private:
	std::vector<Instance*> mInstances;
};


}

