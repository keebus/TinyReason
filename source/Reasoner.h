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
	void setOntology(const std::vector<const Concept*>& ontology);
	bool isSatisfiable(const Concept* pConcept, Model* pModel = 0) const;
	bool isSatisfiable(const std::vector<const Concept*>& concepts, Model* pModel = 0) const;
private:

	struct Node {
		typedef std::multimap<Symbol, Node*> RoleAccessibilityMap;
		typedef RoleAccessibilityMap::iterator RelationMapIterator;
		typedef std::pair<RelationMapIterator, RelationMapIterator> RelationMapRange;

		const Node* pParentNode;

		std::set<Symbol> positiveAtomicConcepts;
		std::set<Symbol> negativeAtomicConcepts;
		std::set<const Concept*> complexConcepts;
		std::multimap<Symbol, Node*> roleAccessibilities;
		const Node* pBlockingNode;
		size_t mID;
		// When a new node is created, it automatically is blocked by its parent
		// because its (empty) label is contained within its parent.
		Node(const Node * pParent) : pParentNode(pParent), pBlockingNode(pParent) { }
		bool isBlocked() const {
			return pBlockingNode;
		}
		bool add(const Concept * pConcept);
		bool contains(const Concept * pConcept) const;
		bool containsConceptsOf(const Node * pNode) const;
	};
	typedef std::pair<Symbol, Node*> SymbolNodePair;

	struct ExpandableConcept {
		Node* pNode;
		const Concept* pConcept;
		ExpandableConcept(Node* pNode, const Concept * pConcept) :
		pNode(pNode), pConcept(pConcept) { }

		/** Heuristic for choosing the next complex concept to expand in an instance of a completion tree */
		struct Compare {
			bool operator()(const ExpandableConcept* pEC1, const ExpandableConcept * pEC2) const;
		};
	};

	enum ExpansionResult {
		EXPANSION_RESULT_NOT_POSSIBLE,
		EXPANSION_RESULT_OK,
		EXPANSION_RESULT_CLASH
	};

	class CompletionTree {
	public:
		CompletionTree() {
			std::cout << "New Completion Tree created: " << this << std::endl;
		}
		~CompletionTree();
		Node * createNode(Node* pParent);
		void addExpandableConcept(const ExpandableConcept* pExpandableConcept);
		ExpansionResult expand(const std::vector<const Concept*>& ontology, CompletionTree*& pNewCompletionTree);
		std::pair<CompletionTree*, Node*> duplicate(const Node* pNode) const;
		void toModel(const ConceptManager* pConceptManager, Model* pModel) const;
	private:
		typedef std::set<Node*> NodeSet;
		NodeSet mNodes;
		std::vector<const ExpandableConcept*> mExpandableConceptQueue;
	};

	class Logger {
		void log(const CompletionTree* pCP, const std::string& message) const;
	private:
		std::map<const CompletionTree* pCP, size_t> mCPtoID;
	};

	const SymbolDictionary* mpSymbolDictionary;
	const ConceptManager* mpConceptManager;
	std::vector<const Concept*> mOntology;
};

}

