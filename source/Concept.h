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

class Concept {
public:

	enum Type {
		TYPE_POSITIVE_ATOMIC,
		TYPE_NEGATIVE_ATOMIC,
		TYPE_CONJUNCTION,
		TYPE_DISJUNCTION,
		TYPE_EXISTENTIAL_RESTRICTION,
		TYPE_UNIVERSAL_RESTRICTION,
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
	bool isAtomic() const {
		return mType == TYPE_POSITIVE_ATOMIC || mType == TYPE_NEGATIVE_ATOMIC;
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

