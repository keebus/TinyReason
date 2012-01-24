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

#include "ConceptManager.h"
#include "Concept.h"

using namespace std;

namespace salcr
{

template<class T, class Q>
inline void deleteAll(std::map<T, Q>& someMap)
{
	for (typename std::map<T, Q>::iterator it = someMap.begin(); it != someMap.end(); ++it)
		delete it->second;
	someMap.clear();
}

ConceptManager::ConceptManager(SymbolDictionary* pSD) :
mpSymbolDictionary(pSD) { }

ConceptManager::~ConceptManager() { }

const Concept* ConceptManager::parse(const std::string& str)
{
	istringstream source(str);
	return parse(source);
}

const Concept* ConceptManager::parse(std::istream& source)
{
	getNextChar(source);
	mTokenType = T_EOS;
	mTokenString = "";
	nextToken(source);

	const Concept* pConcept = parseConcept(source);

	if (mTokenType != T_EOS)
		throwSyntaxException();

	return pConcept;
}

const Concept* ConceptManager::makeNegation(const Concept* pConcept)
{
	if (pConcept->getType() == Concept::TYPE_NEGATION)
		return pConcept->getConcept();
	else
	{
		ConceptToConceptMap::iterator it = mNegationConcept.find(pConcept);
		if (it == mNegationConcept.end())
			it = mNegationConcept.insert(it, ConceptToConceptMap::value_type(pConcept, new Concept(pConcept)));
		return it->second;
	}
}

void ConceptManager::clearConcepts()
{
	deleteAll(mAtomicConcepts);
	deleteAll(mNegationConcept);
	deleteAll(mConjunctionConcepts);
	deleteAll(mDisjunctionConcepts);
	deleteAll(mExistentialConcepts);
	deleteAll(mUniversalConcepts);
}


////////////////////////////////////////////////////////////////////////////////

const Concept* ConceptManager::parseConcept(std::istream& source)
{
	return parseSubsumption(source);
}

const Concept* ConceptManager::parseSubsumption(std::istream& source)
{
	const Concept* pC1 = parseDisjunction(source);
	if (mTokenType == T_IS)
	{
		nextToken(source);
		const Concept * pC2 = parseDisjunction(source);
		ConceptPair p(pC1, pC2);
		ConceptPairToConceptMap::iterator it = mSubsumptionConcepts.find(p);
		if (it == mSubsumptionConcepts.end())
			it = mSubsumptionConcepts.insert(it, std::pair<ConceptPair, const Concept*>(p, new Concept(Concept::TYPE_SUBSUMPTION, pC1, pC2)));
		return it->second;
	}
	return pC1;
}

const Concept* ConceptManager::parseDisjunction(std::istream& source)
{
	const Concept* pC1 = parseConjunction(source);
	if (mTokenType == T_OR)
	{
		nextToken(source);
		const Concept * pC2 = parseConjunction(source);
		ConceptPair p(pC1, pC2);
		ConceptPairToConceptMap::iterator it = mDisjunctionConcepts.find(p);
		if (it == mDisjunctionConcepts.end())
		{
			it = mDisjunctionConcepts.find(ConceptPair(pC2, pC1));
			if (it == mDisjunctionConcepts.end())
				it = mDisjunctionConcepts.insert(it, std::pair<ConceptPair, const Concept*>(p, new Concept(Concept::TYPE_DISJUNCTION, pC1, pC2)));
		}
		return it->second;
	}
	return pC1;
}

const Concept* ConceptManager::parseConjunction(std::istream& source)
{
	const Concept* pC1 = parseSimpleConcept(source);
	if (mTokenType == T_AND)
	{
		nextToken(source);
		const Concept * pC2 = parseConjunction(source);
		ConceptPair p(pC1, pC2);
		ConceptPairToConceptMap::iterator it = mConjunctionConcepts.find(p);
		if (it == mConjunctionConcepts.end())
		{
			it = mConjunctionConcepts.find(ConceptPair(pC2, pC1));

			if (it == mConjunctionConcepts.end())
				it = mConjunctionConcepts.insert(it, ConceptPairToConceptMap::value_type(p, new Concept(Concept::TYPE_CONJUNCTION, pC1, pC2)));
		}
		return it->second;
	}
	return pC1;
}

const Concept* ConceptManager::parseSimpleConcept(std::istream& source)
{
	switch (mTokenType)
	{
		case T_ALL:
			nextToken(source);
			return Concept::getTopConcept();
		case T_NONE:
			nextToken(source);
			return Concept::getBottomConcept();

		case T_ELEMENT: // Atomic element
		{
			Symbol s = mpSymbolDictionary->get(mTokenString);
			nextToken(source);
			if (mTokenType == T_SOME)
			{
				nextToken(source);
				const Concept* pConcept = parseSimpleConcept(source);
				SymbolConceptPairToConceptMap::iterator it = mExistentialConcepts.find(SymbolConceptPair(s, pConcept));
				if (it == mExistentialConcepts.end())
					it = mExistentialConcepts.insert(it, SymbolConceptPairToConceptMap::value_type(
					SymbolConceptPair(s, pConcept), new Concept(Concept::TYPE_EXISTENTIAL, s, pConcept)));
				return it->second;
			} else if (mTokenType == T_ONLY)
			{
				nextToken(source);
				const Concept* pConcept = parseSimpleConcept(source);
				SymbolConceptPairToConceptMap::iterator it = mUniversalConcepts.find(SymbolConceptPair(s, pConcept));
				if (it == mUniversalConcepts.end())
					it = mUniversalConcepts.insert(it, SymbolConceptPairToConceptMap::value_type(
					SymbolConceptPair(s, pConcept), new Concept(Concept::TYPE_UNIVERSAL, s, pConcept)));
				return it->second;
			} else
			{
				SymbolToConceptMap::iterator it = mAtomicConcepts.find(s);
				if (it == mAtomicConcepts.end())
					it = mAtomicConcepts.insert(it, SymbolToConceptMap::value_type(s, new Concept(s)));
				return it->second;
			}
		}
		case T_NOT: // Negation
			nextToken(source);
			return makeNegation(parseSimpleConcept(source));


		case T_LPAR: // sub concept
		{
			nextToken(source);
			const Concept* pConcept = parseConcept(source);
			if (mTokenType != T_RPAR)
				throwSyntaxException();
			nextToken(source);

			return pConcept;
		}
		default:
			throwSyntaxException();
	}
	return 0;
}

void ConceptManager::nextToken(std::istream& source)
{
	mTokenString = "";

	while (source.good())
	{
		switch (mCurrChar)
		{
			case 0:
			case -1:
			case ' ':
			case '\t':
			case '\n':
				break;

			case ';':
				mTokenType = T_AND;
				addAndGetNextChar(source);
				return;

			case '(':
				mTokenType = T_LPAR;
				addAndGetNextChar(source);
				return;

			case ')':
				mTokenType = T_RPAR;
				addAndGetNextChar(source);
				return;
			case 'a':
			{
				addAndGetNextChar(source);
				if (mCurrChar == 'n')
				{
					addAndGetNextChar(source);
					if (mCurrChar == 'd')
					{
						mTokenType = T_AND;
						addAndGetNextChar(source);
						scanElement(source);
						return;
					}
				} else if (mCurrChar == 'l')
				{
					addAndGetNextChar(source);
					if (mCurrChar == 'l')
					{
						addAndGetNextChar(source);
						mTokenType = T_ALL;
						scanElement(source);
						return;

					}
				}
				mTokenType = T_ELEMENT;
				scanElement(source);
				return;
			}
			case 's':
			{
				addAndGetNextChar(source);
				if (mCurrChar == 'o')
				{
					addAndGetNextChar(source);
					if (mCurrChar == 'm')
					{
						addAndGetNextChar(source);
						if (mCurrChar == 'e')
						{
							addAndGetNextChar(source);
							mTokenType = T_SOME;
							scanElement(source);
							return;
						}
					}
				}
				mTokenType = T_ELEMENT;
				scanElement(source);
				return;
			}

			case 'o':
			{
				addAndGetNextChar(source);
				if (mCurrChar == 'r')
				{
					addAndGetNextChar(source);
					mTokenType = T_OR;
					scanElement(source);
					return;
				} else if (mCurrChar == 'n')
				{
					addAndGetNextChar(source);
					if (mCurrChar == 'l')
					{
						addAndGetNextChar(source);
						if (mCurrChar == 'y')
						{
							addAndGetNextChar(source);
							mTokenType = T_ONLY;
							scanElement(source);
							return;
						}
					}
				}
				mTokenType = T_ELEMENT;
				scanElement(source);
				return;
			}

			case 'n':
			{
				addAndGetNextChar(source);
				if (mCurrChar == 'o')
				{
					addAndGetNextChar(source);
					if (mCurrChar == 't')
					{
						addAndGetNextChar(source);
						mTokenType = T_NOT;
						scanElement(source);
						return;
					} else if (mCurrChar == 'n')
					{
						addAndGetNextChar(source);
						if (mCurrChar == 'e')
						{
							addAndGetNextChar(source);
							mTokenType = T_NONE;
							scanElement(source);
							return;
						}
					}
				}
				mTokenType = T_ELEMENT;
				scanElement(source);
				return;
			}

			case 'i':
			{
				addAndGetNextChar(source);
				if (mCurrChar == 'n')
				{
					addAndGetNextChar(source);
					mTokenType = T_IN;
					scanElement(source);
					return;
				} else if (mCurrChar == 's')
				{
					addAndGetNextChar(source);
					mTokenType = T_IS;
					scanElement(source);
					return;
				}
				mTokenType = T_ELEMENT;
				scanElement(source);
				return;
			}

			default:
				mTokenType = T_INVALID;
				scanElement(source);
				if (mTokenType != T_ELEMENT)
					throw Exception("Invalid character: " + string(&mCurrChar, 1));
				return;
		}
		getNextChar(source);
	}
	mTokenType = T_EOS;
}

void ConceptManager::scanElement(std::istream& source)
{
	if (!((mCurrChar >= 'a' && mCurrChar <= 'z') || (mCurrChar >= 'A' && mCurrChar <= 'Z') || mCurrChar == '_' || (mCurrChar >= '0' && mCurrChar <= '9')))
		return;
	do
	{
		addAndGetNextChar(source);
	} while ((mCurrChar >= 'a' && mCurrChar <= 'z') || (mCurrChar >= 'A' && mCurrChar <= 'Z') || mCurrChar == '_' || (mCurrChar >= '0' && mCurrChar <= '9'));
	mTokenType = T_ELEMENT;

	return;
}

void ConceptManager::throwSyntaxException()
{
	throw Exception("Invalid concept (token \"" + mTokenString + "\" found)");
}

}

