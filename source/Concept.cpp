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

#include "Concept.h"

using namespace std;

namespace tinyreason
{

const Concept* Concept::getTopConcept()
{
	static Concept top(true, (Symbol) 1);
	return &top;
}

const Concept* Concept::getBottomConcept()
{
	static Concept bottom(true, (Symbol) 0);
	return &bottom;
}

Concept::Concept(bool positive, Symbol symbol) :
mType(positive ? TYPE_POSITIVE_ATOMIC : TYPE_NEGATIVE_ATOMIC),
mSymbol(symbol) { }

Concept::Concept(Type type, const Concept* pConcept1, const Concept* pConcept2) :
mType(type),
mpConcept1(pConcept1),
mpConcept2(pConcept2) { }

Concept::Concept(Type type, Symbol role, const Concept* pQualificationConcept) :
mType(type),
mRole(role),
mpQualificationConcept(pQualificationConcept) { }

Concept::~Concept() { }

bool Concept::isExpandable() const
{
	if (mType == TYPE_POSITIVE_ATOMIC || mType == TYPE_NEGATIVE_ATOMIC)
		return false;
	return true;
}

bool Concept::isExpansionDeterministic() const
{
	if (mType == TYPE_DISJUNCTION)
		return false;
	return true;
}

std::string Concept::toString(const SymbolDictionary& sd) const
{
	switch (mType)
	{
		case TYPE_POSITIVE_ATOMIC:
			return sd.toName(mSymbol);
		case TYPE_NEGATIVE_ATOMIC:
			return "not " + sd.toName(mSymbol);
		case TYPE_CONJUNCTION:
			return "(" + mpConcept1->toString(sd) + " and " + mpConcept2->toString(sd) + ")";
		case TYPE_DISJUNCTION:
			return "(" + mpConcept1->toString(sd) + " or " + mpConcept2->toString(sd) + ")";
		case TYPE_EXISTENTIAL_RESTRICTION:
			return sd.toName(mRole) + " some" + (mpConcept2 == getTopConcept() ? "thing" : " " + mpConcept2->toString(sd));
		case TYPE_UNIVERSAL_RESTRICTION:
			return sd.toName(mRole) + " only " + mpConcept2->toString(sd);
		default:
			return "INVALID CONCEPT";
	}
}

}

