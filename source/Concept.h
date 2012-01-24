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

class Concept {
public:

	enum Type {
		TYPE_POSITIVE_ATOMIC,
		TYPE_NEGATIVE_ATOMIC,
		TYPE_CONJUNCTION,
		TYPE_DISJUNCTION,
		TYPE_EXISTENTIAL,
		TYPE_UNIVERSAL,
	};

	static const Concept* getTopConcept();
	static const Concept* getBottomConcept();

	Concept(bool positive, Symbol symbol);
	Concept(Type type, const Concept* pConcept1, const Concept* pConcept2);
	Concept(Type type, Symbol role, const Concept* pQualificationConcept);
	~Concept();
	Symbol getSymbol() const {
		return mSymbol;
	}
	Type getType() const {
		return mType;
	}
	const Concept* getConcept1() const {
		return mpConcept1;
	}
	const Concept* getConcept2() const {
		return mpConcept2;
	}
	const Concept* getQualificationConcept() const {
		return mpQualificationConcept;
	}
	Symbol getRole() const {
		return mRole;
	}
	bool isExpandable() const;
	bool isExpansionDeterministic() const;
	std::string toString(const SymbolDictionary& sd) const;
private:
	Type mType;

	union {
		// For atomic concepts
		Symbol mSymbol;

		// For disj and conj concepts
		struct {
			const Concept* mpConcept1;
			const Concept* mpConcept2;
		};

		// For role restriction concepts
		struct {
			Symbol mRole;
			const Concept* mpQualificationConcept;
		};
	};
};



}

