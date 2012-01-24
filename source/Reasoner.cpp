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

#include "Reasoner.h"

using namespace std;

namespace salcr
{

Reasoner::Reasoner(const SymbolDictionary* pSymbolDictionary, ConceptManager* pConceptManager) :
mpSymbolDictionary(pSymbolDictionary),
mpConceptManager(pConceptManager) { }

Reasoner::~Reasoner() { }

bool Reasoner::isSatisfiable(const Concept* pConcept) const
{
	CompletionTree* pCompletionTree = new CompletionTree;
	Node* pNode = pCompletionTree->createNode();
	pNode->complexConcepts.insert(pConcept);

	set<CompletionTree*> completionTrees;
	completionTrees.insert(pCompletionTree);



	return true;
}

////////////////////////////////////////////////////////////////////////////////

bool Reasoner::ExpandableConcept::Compare::operator ()(const ExpandableConcept* pEC1, const ExpandableConcept* pEC2) const
{
	if (pEC1->pConcept->isExpansionDeterministic())
		return true;
	else
		return false;
}

}

