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
#include "SymbolDictionary.h"

namespace salcr
{
class Concept;

class ConceptManager {
public:
	ConceptManager(SymbolDictionary* pSD);
	~ConceptManager();
	const Concept* parse(const std::string& str);
	const Concept* parse(std::istream& source);
	const Concept* makeNegation(const Concept* pConcept);
	void clearConcepts();

private:

	enum TokenType {
		T_INVALID,
		T_EOS,
		T_ELEMENT,
		T_NOT,
		T_AND,
		T_OR,
		T_SOME,
		T_ALL,
		T_ONLY,
		T_IN,
		T_LPAR,
		T_RPAR,
		T_NONE,
		T_IS
	};

	const Concept* getAtomicConcept(const std::string& name);

	const Concept* parseConcept(std::istream& source);
	const Concept* parseSubsumption(std::istream& source);
	const Concept* parseDisjunction(std::istream& source);
	const Concept* parseConjunction(std::istream& source);
	const Concept* parseSimpleConcept(std::istream& source);

	void nextToken(std::istream& source);
	void scanElement(std::istream& source);
	void throwSyntaxException();
	inline void getNextChar(std::istream& source) {
		mCurrChar = source.get();
	}
	inline void addAndGetNextChar(std::istream& source) {
		mTokenString += mCurrChar;
		mCurrChar = source.get();
	}

	typedef std::pair<const Concept*, const Concept*> ConceptPair;
	typedef std::pair<Symbol, const Concept*> SymbolConceptPair;

	typedef std::map< Symbol, const Concept*> SymbolToConceptMap;
	typedef std::map< const Concept*, const Concept*> ConceptToConceptMap;
	typedef std::map< ConceptPair, const Concept*> ConceptPairToConceptMap;
	typedef std::map< SymbolConceptPair, const Concept*> SymbolConceptPairToConceptMap;

	SymbolDictionary* mpSymbolDictionary;

	SymbolToConceptMap mAtomicConcepts;
	ConceptToConceptMap mNegationConcept;
	ConceptPairToConceptMap mSubsumptionConcepts;
	ConceptPairToConceptMap mConjunctionConcepts;
	ConceptPairToConceptMap mDisjunctionConcepts;
	SymbolConceptPairToConceptMap mExistentialConcepts;
	SymbolConceptPairToConceptMap mUniversalConcepts;


	TokenType mTokenType;
	std::string mTokenString;
	char mCurrChar;
};

}

