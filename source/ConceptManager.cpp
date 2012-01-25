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

#include "ConceptManager.h"
#include "Concept.h"

using namespace std;

namespace tinyreason
{

ConceptManager::ConceptManager(SymbolDictionary* pSD) :
mpSymbolDictionary(pSD) { }

ConceptManager::~ConceptManager() { }

const Concept* ConceptManager::parseConcept(const std::string& str) const
{
	istringstream source(str);
	return parseConcept(source);
}

const Concept* ConceptManager::parseConcept(std::istream& source) const
{
	getNextChar(source);
	mTokenType = T_EOS;
	nextToken(source);

	const Concept* pConcept = parseSingleComplexConcept(source);

	if (mTokenType != T_EOS)
		throwSyntaxException();

	return pConcept;
}

void ConceptManager::parseAssertions(const std::string& str, std::vector<const Concept*>& concepts, std::vector<Symbol>& transitiveRoles) const
{
	istringstream source(str);
	return parseAssertions(source, concepts, transitiveRoles);
}

void ConceptManager::parseAssertions(std::istream& source, std::vector<const Concept*>& concepts, std::vector<Symbol>& transitiveRoles) const
{
	getNextChar(source);
	mTokenType = T_EOS;
	nextToken(source);

	parseAssertionList(source, concepts, transitiveRoles);

	if (mTokenType != T_EOS)
		throwSyntaxException();
}

const Concept* ConceptManager::makeNegation(const Concept* pConcept) const
{
	if (pConcept == 0)
		return 0;

	if (pConcept == Concept::getTopConcept())
		return Concept::getBottomConcept();
	else if (pConcept == Concept::getBottomConcept())
		return Concept::getTopConcept();

	// neither top nor bottom

	switch (pConcept->getType())
	{
		case Concept::TYPE_POSITIVE_ATOMIC:
		{
			SymbolToConceptMap::iterator it = mNegativeAtomicConcepts.find(pConcept->getSymbol());
			if (it == mNegativeAtomicConcepts.end())
				it = mNegativeAtomicConcepts.insert(it, SymbolToConceptMap::value_type(pConcept->getSymbol(), new Concept(false, pConcept->getSymbol())));
			return it->second;

		}
		case Concept::TYPE_NEGATIVE_ATOMIC:
		{
			SymbolToConceptMap::iterator it = mPositiveAtomicConcepts.find(pConcept->getSymbol());
			if (it == mPositiveAtomicConcepts.end())
				it = mPositiveAtomicConcepts.insert(it, SymbolToConceptMap::value_type(pConcept->getSymbol(), new Concept(true, pConcept->getSymbol())));
			return it->second;
		}
		case Concept::TYPE_CONJUNCTION:
		{
			ConceptPair cp(makeNegation(pConcept->getConcept1()), makeNegation(pConcept->getConcept2()));
			ConceptPairToConceptMap::iterator it = mDisjunctionConcepts.find(cp);
			if (it == mDisjunctionConcepts.end())
			{
				ConceptPair swappedCP(cp.second, cp.first);
				it = mDisjunctionConcepts.find(swappedCP);
				if (it == mDisjunctionConcepts.end())
					it = mDisjunctionConcepts.insert(it, ConceptPairToConceptMap::value_type(cp, new Concept(Concept::TYPE_DISJUNCTION, cp.first, cp.second)));
			}
			return it->second;
		}
		case Concept::TYPE_DISJUNCTION:
		{
			ConceptPair cp(makeNegation(pConcept->getConcept1()), makeNegation(pConcept->getConcept2()));
			ConceptPairToConceptMap::iterator it = mConjunctionConcepts.find(cp);
			if (it == mConjunctionConcepts.end())
			{
				ConceptPair swappedCP(cp.second, cp.first);
				it = mConjunctionConcepts.find(swappedCP);
				if (it == mConjunctionConcepts.end())
					it = mConjunctionConcepts.insert(it, ConceptPairToConceptMap::value_type(cp, new Concept(Concept::TYPE_CONJUNCTION, cp.first, cp.second)));
			}
			return it->second;
		}
		case Concept::TYPE_EXISTENTIAL_RESTRICTION:
		{
			SymbolConceptPair scp(pConcept->getRole(), makeNegation(pConcept->getQualificationConcept()));
			SymbolConceptPairToConceptMap::iterator it = mUniversalConcepts.find(scp);
			if (it == mUniversalConcepts.end())
				it = mUniversalConcepts.insert(it, SymbolConceptPairToConceptMap::value_type(scp, new Concept(Concept::TYPE_UNIVERSAL_RESTRICTION, scp.first, scp.second)));
			return it->second;
		}
		case Concept::TYPE_UNIVERSAL_RESTRICTION:
		{
			SymbolConceptPair scp(pConcept->getRole(), makeNegation(pConcept->getQualificationConcept()));
			SymbolConceptPairToConceptMap::iterator it = mExistentialConcepts.find(scp);
			if (it == mExistentialConcepts.end())
				it = mExistentialConcepts.insert(it, SymbolConceptPairToConceptMap::value_type(scp, new Concept(Concept::TYPE_EXISTENTIAL_RESTRICTION, scp.first, scp.second)));
			return it->second;
		}
		default:
			throw Exception("Invalid concept received while making negation.");
	}
	return 0;
}

const Concept* ConceptManager::getAtomicConcept(bool isPositive, Symbol symbol) const
{
	SymbolToConceptMap* pSymbolToConceptMap;

	if (isPositive)
		pSymbolToConceptMap = &mPositiveAtomicConcepts;
	else
		pSymbolToConceptMap = &mNegativeAtomicConcepts;

	SymbolToConceptMap::iterator it = pSymbolToConceptMap->find(symbol);
	if (it == pSymbolToConceptMap->end())
		it = pSymbolToConceptMap->insert(it, SymbolToConceptMap::value_type(symbol, new Concept(isPositive, symbol)));
	return it->second;
}

void ConceptManager::clearCache() const
{
	deleteAll(mPositiveAtomicConcepts);
	deleteAll(mNegativeAtomicConcepts);
	deleteAll(mConjunctionConcepts);
	deleteAll(mDisjunctionConcepts);
	deleteAll(mExistentialConcepts);
	deleteAll(mUniversalConcepts);
}

////////////////////////////////////////////////////////////////////////////////

void ConceptManager::parseAssertionList(std::istream& source, vector<const Concept*>& concepts, std::vector<Symbol>& transitiveRoles) const
{
	do
	{
		while (mTokenType == T_SEMICOLON)
			nextToken(source);

		if (mTokenType == T_EOS)
			return;

		if (mTokenType == T_TRANS) // it's a transitive role assertion
			parseTransitiveRoleAssertion(source, transitiveRoles);
		else
			concepts.push_back(parseSingleComplexConcept(source));
	} while (true);
}

void ConceptManager::parseTransitiveRoleAssertion(std::istream& source, std::vector<Symbol>& transitiveRoles) const
{
	if (mTokenType != T_TRANS)
		throwSyntaxException();
	nextToken(source);
	do
	{
		if (mTokenType != T_ELEMENT)
			throwSyntaxException();
		transitiveRoles.push_back(mpSymbolDictionary->get(mTokenString));
		nextToken(source);
		if (mTokenType == T_SEMICOLON)
		{
			nextToken(source);
			break;
		}
	} while (true);
}

const Concept* ConceptManager::parseSingleComplexConcept(std::istream& source) const
{
	return parseEquivalence(source);
}

const Concept* ConceptManager::parseEquivalence(std::istream& source) const
{
	const Concept* pC1 = parseSubsumption(source);
	if (mTokenType == T_IS)
	{
		nextToken(source);
		const Concept* pC2 = parseEquivalence(source);
		// Simplifications
		if (pC1 == Concept::getTopConcept())
			return pC2;
		if (pC2 == Concept::getTopConcept())
			return pC1;
		const Concept* pNegatedC1 = makeNegation(pC1);
		const Concept* pNegatedC2 = makeNegation(pC2);
		if (pC1 == Concept::getBottomConcept())
			return pNegatedC1;
		if (pC2 == Concept::getBottomConcept())
			return pNegatedC2;

		// Ok can't simplify. This is a disjunction of conjunctions (both true or both false)
		const Concept* pBothTrueConcept, *pBothFalseConcept;

		ConceptPair btcp(pC1, pC2);
		ConceptPairToConceptMap::iterator it = mConjunctionConcepts.find(btcp);
		if (it == mConjunctionConcepts.end())
		{
			it = mConjunctionConcepts.find(ConceptPair(btcp.second, btcp.first));
			if (it == mConjunctionConcepts.end())
				it = mConjunctionConcepts.insert(it, ConceptPairToConceptMap::value_type(btcp, new Concept(Concept::TYPE_CONJUNCTION, btcp.first, btcp.second)));
		}
		pBothTrueConcept = it->second;

		ConceptPair bfcp(pNegatedC1, pNegatedC2);
		it = mConjunctionConcepts.find(bfcp);
		if (it == mConjunctionConcepts.end())
		{
			it = mConjunctionConcepts.find(ConceptPair(bfcp.second, bfcp.first));
			if (it == mConjunctionConcepts.end())
				it = mConjunctionConcepts.insert(it, ConceptPairToConceptMap::value_type(bfcp, new Concept(Concept::TYPE_CONJUNCTION, bfcp.first, bfcp.second)));
		}
		pBothFalseConcept = it->second;

		// Then make cojunction
		ConceptPair conjCP(pBothTrueConcept, pBothFalseConcept);
		it = mDisjunctionConcepts.find(conjCP);
		if (it == mDisjunctionConcepts.end())
		{
			it = mDisjunctionConcepts.find(ConceptPair(conjCP.second, conjCP.first));
			if (it == mDisjunctionConcepts.end())
				it = mDisjunctionConcepts.insert(it, ConceptPairToConceptMap::value_type(conjCP, new Concept(Concept::TYPE_DISJUNCTION, conjCP.first, conjCP.second)));
		}
		return it->second;
	}
	return pC1;
}

const Concept* ConceptManager::parseSubsumption(std::istream& source) const
{
	const Concept* pC1 = parseDisjunction(source);
	if (mTokenType == T_ISA)
	{
		nextToken(source);
		const Concept* pC2 = parseSubsumption(source);
		const Concept* pNegatedC1 = makeNegation(pC1);
		// Simplifications
		if (pNegatedC1 == Concept::getTopConcept() || pC2 == Concept::getTopConcept())
			return Concept::getTopConcept();
		if (pNegatedC1 == Concept::getBottomConcept())
			return pC2;
		if (pC2 == Concept::getBottomConcept())
			return pNegatedC1;

		ConceptPair cp(pNegatedC1, pC2);
		ConceptPairToConceptMap::iterator it = mDisjunctionConcepts.find(cp);
		if (it == mDisjunctionConcepts.end())
		{
			it = mDisjunctionConcepts.find(ConceptPair(cp.second, cp.first));
			if (it == mDisjunctionConcepts.end())
				it = mDisjunctionConcepts.insert(it, ConceptPairToConceptMap::value_type(cp, new Concept(Concept::TYPE_DISJUNCTION, cp.first, cp.second)));
		}
		return it->second;
	}
	return pC1;
}

const Concept* ConceptManager::parseDisjunction(std::istream& source) const
{
	const Concept* pC1 = parseConjunction(source);
	if (mTokenType == T_OR)
	{
		nextToken(source);
		const Concept * pC2 = parseDisjunction(source);

		// Simplifications
		if (pC1 == Concept::getTopConcept() || pC2 == Concept::getTopConcept())
			return Concept::getTopConcept();
		if (pC1 == Concept::getBottomConcept())
			return pC2;
		if (pC2 == Concept::getBottomConcept())
			return pC1;

		ConceptPair cp(pC1, pC2);
		ConceptPairToConceptMap::iterator it = mDisjunctionConcepts.find(cp);
		if (it == mDisjunctionConcepts.end())
		{
			it = mDisjunctionConcepts.find(ConceptPair(cp.second, cp.first));
			if (it == mDisjunctionConcepts.end())
				it = mDisjunctionConcepts.insert(it, ConceptPairToConceptMap::value_type(cp, new Concept(Concept::TYPE_DISJUNCTION, cp.first, cp.second)));
		}
		return it->second;
	}
	return pC1;
}

const Concept* ConceptManager::parseConjunction(std::istream& source) const
{
	const Concept* pC1 = parseSimpleConcept(source);
	if (mTokenType == T_AND)
	{
		nextToken(source);
		const Concept * pC2 = parseConjunction(source);

		// Simplifications
		if (pC1 == Concept::getBottomConcept() || pC2 == Concept::getBottomConcept())
			return Concept::getBottomConcept();
		if (pC1 == Concept::getTopConcept())
			return pC2;
		if (pC2 == Concept::getTopConcept())
			return pC1;

		ConceptPair cp(pC1, pC2);
		ConceptPairToConceptMap::iterator it = mConjunctionConcepts.find(cp);
		if (it == mConjunctionConcepts.end())
		{
			it = mConjunctionConcepts.find(ConceptPair(pC2, pC1));

			if (it == mConjunctionConcepts.end())
				it = mConjunctionConcepts.insert(it, ConceptPairToConceptMap::value_type(cp, new Concept(Concept::TYPE_CONJUNCTION, pC1, pC2)));
		}
		return it->second;
	}
	return pC1;
}

const Concept* ConceptManager::parseSimpleConcept(std::istream& source) const
{
	switch (mTokenType)
	{
		case T_ANYTHING:
			nextToken(source);
			return Concept::getTopConcept();

		case T_NOTHING:
			nextToken(source);
			return Concept::getBottomConcept();

		case T_ELEMENT:
		{
			Symbol s = mpSymbolDictionary->get(mTokenString);
			nextToken(source);
			if (mTokenType == T_SOME || mTokenType == T_SOMETHING)
			{
				const Concept* pConcept;
				if (mTokenType == T_SOMETHING)
				{
					nextToken(source);
					pConcept = Concept::getTopConcept();
				} else
				{
					nextToken(source);
					pConcept = parseSimpleConcept(source);
				}
				SymbolConceptPairToConceptMap::iterator it = mExistentialConcepts.find(SymbolConceptPair(s, pConcept));
				if (it == mExistentialConcepts.end())
					it = mExistentialConcepts.insert(it, SymbolConceptPairToConceptMap::value_type(
					SymbolConceptPair(s, pConcept), new Concept(Concept::TYPE_EXISTENTIAL_RESTRICTION, s, pConcept)));
				return it->second;

			} else if (mTokenType == T_ONLY)
			{
				nextToken(source);
				const Concept* pConcept = parseSimpleConcept(source);
				SymbolConceptPairToConceptMap::iterator it = mUniversalConcepts.find(SymbolConceptPair(s, pConcept));
				if (it == mUniversalConcepts.end())
					it = mUniversalConcepts.insert(it, SymbolConceptPairToConceptMap::value_type(
					SymbolConceptPair(s, pConcept), new Concept(Concept::TYPE_UNIVERSAL_RESTRICTION, s, pConcept)));
				return it->second;
			} else
				return getAtomicConcept(true, s);
		}
		case T_NOT: // Negation
			nextToken(source);
			return makeNegation(parseSimpleConcept(source));

		case T_LPAR: // sub concept
		{
			nextToken(source);
			const Concept* pConcept = parseSingleComplexConcept(source);
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

void ConceptManager::nextToken(std::istream & source) const
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
				mTokenType = T_SEMICOLON;
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
					} else if (mCurrChar == 'y')
					{
						addAndGetNextChar(source);
						if (mCurrChar == 't')
						{
							addAndGetNextChar(source);
							if (mCurrChar == 'h')
							{
								addAndGetNextChar(source);
								if (mCurrChar == 'i')
								{
									addAndGetNextChar(source);
									if (mCurrChar == 'n')
									{
										addAndGetNextChar(source);
										if (mCurrChar == 'g')
										{
											addAndGetNextChar(source);
											mTokenType = T_ANYTHING;
											scanElement(source);
											return;

										}
									}
								}
							}
						}
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
							if (mCurrChar == 't')
							{
								addAndGetNextChar(source);
								if (mCurrChar == 'h')
								{
									addAndGetNextChar(source);
									if (mCurrChar == 'i')
									{
										addAndGetNextChar(source);
										if (mCurrChar == 'n')
										{
											addAndGetNextChar(source);
											if (mCurrChar == 'g')
											{
												addAndGetNextChar(source);
												mTokenType = T_SOMETHING;
												scanElement(source);
												return;
											}
										}
									}
								}
							} else
							{
								mTokenType = T_SOME;
								scanElement(source);
								return;
							}
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
						if (mCurrChar == 'h')
						{
							addAndGetNextChar(source);
							if (mCurrChar == 'i')
							{
								addAndGetNextChar(source);
								if (mCurrChar == 'n')
								{
									addAndGetNextChar(source);
									if (mCurrChar == 'g')
									{
										addAndGetNextChar(source);
										mTokenType = T_NOTHING;
										scanElement(source);
										return;
									}
								}
							}
						} else
						{
							mTokenType = T_NOT;
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
					if (mCurrChar == 'a')
					{
						addAndGetNextChar(source);
						mTokenType = T_ISA;
						scanElement(source);
						return;
					} else
					{
						mTokenType = T_IS;
						scanElement(source);
						return;
					}
				}
				mTokenType = T_ELEMENT;
				scanElement(source);
				return;
			}

			case 't':
			{
				addAndGetNextChar(source);
				if (mCurrChar == 'r')
				{
					addAndGetNextChar(source);
					if (mCurrChar == 'a')
					{
						addAndGetNextChar(source);
						if (mCurrChar == 'n')
						{
							addAndGetNextChar(source);
							if (mCurrChar == 's')
							{
								addAndGetNextChar(source);
								mTokenType = T_TRANS;
								scanElement(source);
								return;
							}
						}
					}
				}
				mTokenType = T_ELEMENT;
				scanElement(source);
				return;
			}

			default:
				mTokenType = T_INVALID;
				scanElement(source);
				if (mTokenType != T_ELEMENT)
					throw Exception("Invalid concept string (invalid character: \'" + string(&mCurrChar, 1) + "\')");
				return;
		}
		getNextChar(source);
	}
	mTokenType = T_EOS;
}

void ConceptManager::scanElement(std::istream & source) const
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

void ConceptManager::throwSyntaxException() const
{
	throw Exception("Invalid concept string (token \"" + mTokenString + "\" found)");
}

}

