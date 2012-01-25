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
#include "Concept.h"
#include "SymbolDictionary.h"
#include "ConceptManager.h"

namespace tinyreason
{

class Reasoner {
public:
	Reasoner(const SymbolDictionary* pSymbolDictionary, const ConceptManager* pConceptManager);
	~Reasoner();
	void setOntology(const std::vector<const Concept*>& ontology);
	bool isSatisfiable(const Concept* pConcept, Model* pModel = 0, bool verbose = false) const;
	bool isSatisfiable(const std::vector<const Concept*>& concepts, Model* pModel = 0, bool verbose = false) const;
private:

	class Logger;
	class Node;
	class CompletionTree;
	class ExpandableConcept;

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
		bool add(const Concept * pConcept, const Logger* pLogger, const CompletionTree * pLoggingCT);
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
		CompletionTree(const Logger* pLogger);
		~CompletionTree();
		Node * createNode(Node* pParent);
		void addExpandableConcept(const ExpandableConcept* pExpandableConcept);
		ExpansionResult expand(const std::vector<const Concept*>& ontology, CompletionTree*& pNewCompletionTree);
		std::pair<CompletionTree*, Node*> duplicate(const Node* pNode) const;
		void toModel(const ConceptManager* pConceptManager, Model* pModel) const;
	private:
		const Logger* mpLogger;
		typedef std::set<Node*> NodeSet;
		NodeSet mNodes;
		std::vector<const ExpandableConcept*> mExpandableConceptQueue;
	};

	class Logger {
	public:
		Logger(std::ostream& outStream, const SymbolDictionary* pSymbolDictionary);
		void log(const std::string& message) const;
		void log(const CompletionTree* pCP, const std::string& message) const;
		void log(const CompletionTree* pCP, const Node* pNode, const std::string& message) const;
		void log(const CompletionTree* pCP, const Node* pNode, const Concept* pConcept, const std::string& message) const;
		std::string getNodeStrID(const CompletionTree* pCP, const Node* pNode) const;
	private:
		typedef std::map<const CompletionTree*, std::pair<size_t, size_t > > CompletionTreeInfoMap;
		typedef std::map<std::pair<const CompletionTree*, const Node*>, size_t> CompletionTreeNodeInfoMap;
		std::pair<size_t, size_t> getCPInfo(const CompletionTree* pCP) const;
		size_t getNodeID(const CompletionTree* pCP, const Node* pNode) const;
		std::ostream& mOutStream;
		const SymbolDictionary* mpSymbolDictionary;
		mutable size_t mCPCounter;
		mutable CompletionTreeInfoMap mCPtoIDInfo;
		mutable CompletionTreeNodeInfoMap mCPNodeToID;
	};

	const SymbolDictionary* mpSymbolDictionary;
	const ConceptManager* mpConceptManager;
	std::vector<const Concept*> mOntology;
};

}

