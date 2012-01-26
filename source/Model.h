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

#include "Common.h"

namespace tinyreason
{

class Individual {
public:
	Individual(size_t id) : mID(id) { }
	const std::set<const Concept*>& getConcepts() {
		return mConcepts;
	}
	void addConcept(const Concept* pConcept) {
		mConcepts.insert(pConcept);
	}
	void addRoleAccessibility(Symbol role, const Individual* pIndividual);
	void dumpToString(const SymbolDictionary& symbolDictionary, std::ostream& outStream, bool showComplexConcepts = false) const;
	void dumpToDOT(const SymbolDictionary& symbolDictionary, std::ostream& outStream, bool showComplexConcepts = false) const;
private:
	typedef std::multimap<Symbol, const Individual*> RoleAccessibilityMap;
	size_t mID;
	std::set<const Concept*> mConcepts;
	RoleAccessibilityMap mRoleAccessibilities;
};

class Model {
public:
	Model();
	~Model();
	Individual* createIndividual();
	void clear();
	void dumpToString(const SymbolDictionary& symbolDictionary, std::ostream& outStream, bool showComplexConcepts = false) const;
	void dumpToDOT(const SymbolDictionary& symbolDictionary, std::ostream& outStream, bool showComplexConcepts = false) const;
private:
	size_t mFreeIndividualID;
	std::vector<Individual*> mIndividuals;
};


}

