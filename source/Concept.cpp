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

#include "Concept.h"

using namespace std;

namespace salcr
{

const Concept* Concept::getTopConcept()
{
	static Concept top((Symbol) 1);
	return &top;
}

const Concept* Concept::getBottomConcept()
{
	static Concept bottom((Symbol) 0);
	return &bottom;
}

Concept::Concept(Symbol symbol) :
mType(TYPE_ATOMIC),
mSymbol(symbol) { }

Concept::Concept(const Concept* pConcept) :
mType(TYPE_NEGATION),
mpConcept(pConcept) { }

Concept::Concept(Type type, const Concept* pConcept1, const Concept* pConcept2) :
mType(type),
mpConcept1(pConcept1),
mpConcept2(pConcept2) { }

Concept::Concept(Type type, Symbol role, const Concept* pQualificationConcept) :
mType(type),
mRole(role),
mpQualificationConcept(pQualificationConcept) { }

Concept::~Concept() { }

std::string Concept::toString(const SymbolDictionary& sd) const
{
	switch (mType)
	{
		case TYPE_ATOMIC:
			return sd.toName(mSymbol);
		case TYPE_NEGATION:
			return "not " + mpConcept->toString(sd);
		case TYPE_CONJUNCTION:
			return "(" + mpConcept1->toString(sd) + " and " + mpConcept2->toString(sd) + ")";
		case TYPE_DISJUNCTION:
			return "(" + mpConcept1->toString(sd) + " or " + mpConcept2->toString(sd) + ")";
		case TYPE_SUBSUMPTION:
			return "(" + mpConcept1->toString(sd) + " is " + mpConcept2->toString(sd) + ")";
		case TYPE_EXISTENTIAL:
			return sd.toName(mRole) + " some " + mpConcept2->toString(sd);
		case TYPE_UNIVERSAL:
			return sd.toName(mRole) + " only " + mpConcept2->toString(sd);
		default:
			return "INVALID CONCEPT";
	}
}

}

