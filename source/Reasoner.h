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

#pragma once

#include "Common.h"
#include "Concept.h"
#include "SymbolDictionary.h"
#include "ConceptManager.h"

namespace salcr
{

class Reasoner {
public:
	Reasoner(const SymbolDictionary* pSymbolDictionary, const ConceptManager* pConceptManager);
	~Reasoner();
	bool isSatisfiable(const Concept* pConcept, Model* pModel = 0) const;
private:

	struct Node {
		std::set<Symbol> positiveAtomicConcepts;
		std::set<Symbol> negativeAtomicConcepts;
		std::set<const Concept*> complexConcepts;
		std::multimap<Symbol, Node*> roleAccessibilities;

		typedef std::multimap<Symbol, Node*>::iterator RelationMapIterator;
		typedef std::pair<RelationMapIterator, RelationMapIterator> RelationMapRange;

		bool add(const Concept * pConcept);
		bool contains(const Concept * pConcept);
	};
	typedef std::pair<Symbol, Node*> SymbolNodePair;

	class CompletionTree {
	public:
		~CompletionTree();
		Node * createNode() {
			Node* pNode = new Node;
			mNodes.insert(pNode);
			return pNode;
		}
		void toModel(const ConceptManager* pConceptManager, Model* pModel) const;
	private:
		typedef std::set<Node*> NodeSet;
		NodeSet mNodes;
	};

	struct ExpandableConcept {
		CompletionTree* pCompletionTree;
		Node* pNode;
		const Concept* pConcept;
		ExpandableConcept(CompletionTree* pCompletionTree, Node* pNode, const Concept * pConcept) :
		pCompletionTree(pCompletionTree), pNode(pNode), pConcept(pConcept) { }

		/** Heuristic for choosing the next complex concept to expand in an instance of a completion tree */
		struct Compare {
			bool operator()(const ExpandableConcept* pEC1, const ExpandableConcept * pEC2) const;
		};

	};

	enum ExpansionResult {
		EXPANSION_RESULT_NOT_POSSIBLE,
		EXPANSION_RESULT_OK_SKIP,
		EXPANSION_RESULT_OK_REINSERT,
		EXPANSION_RESULT_CLASH
	};

	typedef std::priority_queue<const ExpandableConcept*, std::vector<const ExpandableConcept*>, ExpandableConcept::Compare> ExpandableConceptQueue;

	static ExpansionResult expand(ExpandableConceptQueue& openConcepts, const ExpandableConcept* pExpandableConcept, CompletionTree*& pNewCompletionTree);

	const SymbolDictionary* mpSymbolDictionary;
	const ConceptManager* mpConceptManager;
};

}

