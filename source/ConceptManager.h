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
#include "SymbolDictionary.h"

namespace tinyreason
{
class Concept;

class ConceptManager {
public:
	ConceptManager(SymbolDictionary* pSD);
	~ConceptManager();
	const Concept* parseConcept(const std::string& str) const;
	const Concept* parseConcept(std::istream& source) const;

	void parseAssertions(const std::string& str, std::vector<const Concept*>& concepts, std::vector<Symbol>& transitiveRoles) const;
	void parseAssertions(std::istream& source, std::vector<const Concept*>& concepts, std::vector<Symbol>& transitiveRoles) const;

	const Concept* makeNegation(const Concept* pConcept) const;
	const Concept* getAtomicConcept(bool isPositive, Symbol symbol) const;
	void clearCache() const;
private:

	enum TokenType {
		T_INVALID,
		T_EOS,
		T_ELEMENT,
		T_NOT,
		T_AND,
		T_OR,
		T_SOME,
		T_THING,
		T_ONLY,
		T_IN,
		T_LPAR,
		T_RPAR,
		T_NOTHING,
		T_IS,
		T_ISA,
		T_SEMICOLON,
		T_TRANS,
	};

	void parseAssertionList(std::istream& source, std::vector<const Concept*>& concepts, std::vector<Symbol>& transitiveRoles) const;
	void parseTransitiveRoleAssertion(std::istream& source, std::vector<Symbol>& transitiveRoles) const;
	const Concept* parseSingleComplexConcept(std::istream& source) const;

	const Concept* parseEquivalence(std::istream& source) const;
	const Concept* parseSubsumption(std::istream& source) const;
	const Concept* parseDisjunction(std::istream& source) const;
	const Concept* parseConjunction(std::istream& source) const;
	const Concept* parseSimpleConcept(std::istream& source) const;

	void nextToken(std::istream& source) const;
	void scanElement(std::istream& source) const;
	void throwSyntaxException() const;
	inline void getNextChar(std::istream& source) const {
		mCurrChar = source.get();
	}
	inline void addAndGetNextChar(std::istream& source) const {
		mTokenString += mCurrChar;
		mCurrChar = source.get();
	}

	typedef std::pair<const Concept*, const Concept*> ConceptPair;
	typedef std::pair<Symbol, const Concept*> SymbolConceptPair;

	typedef std::map< Symbol, const Concept*> SymbolToConceptMap;
	typedef std::map< ConceptPair, const Concept*> ConceptPairToConceptMap;
	typedef std::map< SymbolConceptPair, const Concept*> SymbolConceptPairToConceptMap;

	SymbolDictionary* mpSymbolDictionary;

	mutable SymbolToConceptMap mPositiveAtomicConcepts;
	mutable SymbolToConceptMap mNegativeAtomicConcepts;
	mutable ConceptPairToConceptMap mConjunctionConcepts;
	mutable ConceptPairToConceptMap mDisjunctionConcepts;
	mutable SymbolConceptPairToConceptMap mExistentialConcepts;
	mutable SymbolConceptPairToConceptMap mUniversalConcepts;

	mutable TokenType mTokenType;
	mutable std::string mTokenString;
	mutable char mCurrChar;
};

}

