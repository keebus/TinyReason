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
#include "Model.h"

using namespace std;

namespace salcr
{

Reasoner::Reasoner(const SymbolDictionary* pSymbolDictionary, const ConceptManager* pConceptManager) :
mpSymbolDictionary(pSymbolDictionary),
mpConceptManager(pConceptManager) { }

Reasoner::~Reasoner() { }

void Reasoner::setOntology(const std::vector<const Concept*>& ontology)
{
	mOntology = ontology;
}

bool Reasoner::isSatisfiable(const Concept* pConcept, Model* pModel) const
{
	vector<const Concept*> singleton;
	singleton.push_back(pConcept);
	return isSatisfiable(singleton, pModel);
}

bool Reasoner::isSatisfiable(const std::vector<const Concept*>& concepts, Model* pModel) const
{
	CompletionTree* pCompletionTree = new CompletionTree;
	Node* pNode = pCompletionTree->createNode(0);

	// Make a queue containing expandable concepts
	cout << "Adding ontology concepts to first instance." << endl;
	for (size_t i = 0; i < mOntology.size(); ++i)
		if (pNode->add(mOntology[i]))
			pCompletionTree->addExpandableConcept(new ExpandableConcept(pNode, mOntology[i]));
	cout << "Adding testing user concept to first instance." << endl;
	for (size_t i = 0; i < concepts.size(); ++i)
		if (pNode->add(concepts[i]))
			pCompletionTree->addExpandableConcept(new ExpandableConcept(pNode, concepts[i]));

	list<CompletionTree*> completionTrees;
	completionTrees.push_front(pCompletionTree);

	// Then... go!	
	bool foundCompleteCompletionTree = false;
	do
	{
		// Take the first available Completion Tree in the open list
		pCompletionTree = completionTrees.front();
		cout << "Expanding in Completion Tree " << pCompletionTree << endl;
		// Let the completion tree expand
		CompletionTree* pNewCompletionTree = 0;
		ExpansionResult result = pCompletionTree->expand(mOntology, pNewCompletionTree);
		// If the expansion algorithm created a new completion tree, add it to our active set
		if (pNewCompletionTree)
			completionTrees.push_back(pNewCompletionTree);
		switch (result)
		{
			case EXPANSION_RESULT_NOT_POSSIBLE:
				// Yes! We found a model as this completion tree cannot be further expanded.
				cout << "Expansion could not be possible, model found!" << endl;
				foundCompleteCompletionTree = true;
				break;

			case EXPANSION_RESULT_OK:
				// Ok we expandend the complex concept but we didnt find a clash.
				cout << "Expansion ok." << endl;
				break;

			case EXPANSION_RESULT_CLASH:
				// Clash found in a node of current completion tree.
				// Delete the incoherent completion tree from our set processing
				// trees.
				cout << "Clash found in completion tree " << pCompletionTree << "!" << endl;
				delete pCompletionTree;
				completionTrees.pop_front();
				break;
		}
	} while (!completionTrees.empty() && !foundCompleteCompletionTree);

	// Now check results
	if (completionTrees.empty())
	{
		// All completion trees are closed, the concept is not satisfiable.
		return false;
	} else
	{
		// The concept is satisfiable as there is one complete completion tree alive.
		if (pModel)
		{
			// Convert a completion tree into a model
			CompletionTree* pExampleCompletionTree = completionTrees.front();
			pExampleCompletionTree->toModel(mpConceptManager, pModel);
			pExampleCompletionTree = 0;
		}
		// Cleanup memory
		for (list<CompletionTree*>::iterator it = completionTrees.begin(); it != completionTrees.end(); ++it)
			delete *it;

		return true;
	}

}
////////////////////////////////////////////////////////////////////////////////

bool Reasoner::Node::add(const Concept * pConcept)
{
	// If I'm trying to add TOP, skip it and say "we already have it"
	if (pConcept == Concept::getTopConcept())
		return false;
	bool result;
	switch (pConcept->getType())
	{
		case Concept::TYPE_POSITIVE_ATOMIC:
			result = positiveAtomicConcepts.insert(pConcept->getSymbol()).second;
			break;
		case Concept::TYPE_NEGATIVE_ATOMIC:
			result = negativeAtomicConcepts.insert(pConcept->getSymbol()).second;
			break;

		default:
			result = complexConcepts.insert(pConcept).second;
			break;
	}
	// If we did add a new concept to this node, we must chack that whether the blocking
	// node contains it anyway
	if (result && pBlockingNode)
	{
		if (!pBlockingNode->contains(pConcept))
		{
			// Our blocking node does NOT contain the newly added concept thus we
			// must look for another node whose set of concepts contains this ones'
			pBlockingNode = pBlockingNode->pParentNode;
			while (pBlockingNode != 0)
			{
				if (!pBlockingNode->containsConceptsOf(this))
					pBlockingNode = pBlockingNode->pParentNode;
			}
			// Here if pBlockingNode != 0 we found one and this node is still blocked
			// else this node will now be free.
		}
	}
	return result;
}

bool Reasoner::Node::contains(const Concept * pConcept) const
{
	if (pConcept == Concept::getTopConcept())
		return true;
	else if (pConcept == Concept::getBottomConcept())
		return false;
	switch (pConcept->getType())
	{
		case Concept::TYPE_POSITIVE_ATOMIC:
			return positiveAtomicConcepts.find(pConcept->getSymbol()) != positiveAtomicConcepts.end();
		case Concept::TYPE_NEGATIVE_ATOMIC:
			return negativeAtomicConcepts.find(pConcept->getSymbol()) != negativeAtomicConcepts.end();

		default:
			return complexConcepts.find(pConcept) != complexConcepts.end();
	}
}

bool Reasoner::Node::containsConceptsOf(const Node* pNode) const
{
	for (std::set<Symbol>::const_iterator it = pNode->positiveAtomicConcepts.begin(); it != pNode->positiveAtomicConcepts.end(); ++it)
		if (positiveAtomicConcepts.find(*it) == positiveAtomicConcepts.end())
			return false;
	for (std::set<Symbol>::const_iterator it = pNode->negativeAtomicConcepts.begin(); it != pNode->negativeAtomicConcepts.end(); ++it)
		if (negativeAtomicConcepts.find(*it) == negativeAtomicConcepts.end())
			return false;
	for (std::set<const Concept*>::const_iterator it = pNode->complexConcepts.begin(); it != pNode->complexConcepts.end(); ++it)
		if (complexConcepts.find(*it) == complexConcepts.end())
			return false;
	return true;
}


////////////////////////////////////////////////////////////////////////////////

Reasoner::CompletionTree::~CompletionTree()
{
	deleteAll(mNodes);
	deleteAll(mExpandableConceptQueue);
}

Reasoner::Node* Reasoner::CompletionTree::createNode(Node* pParent)
{
	Node* pNode = new Node(pParent);
	cout << "New node " << pNode << " created from Completion Tree " << this << "." << endl;
	mNodes.insert(pNode);
	return pNode;
}

void Reasoner::CompletionTree::addExpandableConcept(const ExpandableConcept* pExpandableConcept)
{
	mExpandableConceptQueue.push_back(pExpandableConcept);
	push_heap(mExpandableConceptQueue.begin(), mExpandableConceptQueue.end(), ExpandableConcept::Compare());
}

Reasoner::ExpansionResult Reasoner::CompletionTree::expand(const std::vector<const Concept*>& ontology, CompletionTree*& pNewCompletionTree)
{
	list<const ExpandableConcept*> insertionList;
	ExpansionResult result = EXPANSION_RESULT_NOT_POSSIBLE;
	bool skipThisExpandableConcept = false;
	const ExpandableConcept* pEC;

	while (result == EXPANSION_RESULT_NOT_POSSIBLE && !mExpandableConceptQueue.empty())
	{
		// Pop the most promising expandable concept
		pop_heap(mExpandableConceptQueue.begin(), mExpandableConceptQueue.end(), ExpandableConcept::Compare());
		pEC = mExpandableConceptQueue.back();
		mExpandableConceptQueue.pop_back();

		// If we're expanding a "bottom" concept, that means inconsistency
		if (pEC->pConcept == Concept::getBottomConcept())
			result = EXPANSION_RESULT_CLASH;
		else if (pEC->pNode->isBlocked())
		{
			// We cannot expand in a blocked node, carry on.
			insertionList.push_back(pEC);
			cout << "Node " << pEC->pNode << " is blocked thus concept skipped." << endl;
		} else
		{
			switch (pEC->pConcept->getType())
			{
				case Concept::TYPE_POSITIVE_ATOMIC:
					// If there's a negative atomic argument in node with the same concept, clash!
					if (pEC->pNode->negativeAtomicConcepts.find(pEC->pConcept->getSymbol()) != pEC->pNode->negativeAtomicConcepts.end())
						result = EXPANSION_RESULT_CLASH;
					else
					{
						result = EXPANSION_RESULT_OK;
						skipThisExpandableConcept = true;
					}
					break;

				case Concept::TYPE_NEGATIVE_ATOMIC:
					// If there's a positive atomic argument in node with the same concept, clash!
					if (pEC->pNode->positiveAtomicConcepts.find(pEC->pConcept->getSymbol()) != pEC->pNode->positiveAtomicConcepts.end())
						result = EXPANSION_RESULT_CLASH;
					else
					{
						result = EXPANSION_RESULT_OK;
						skipThisExpandableConcept = true;
					}
					break;

				case Concept::TYPE_CONJUNCTION:
					// Add both subconcepts in node, simple enough! :)
					if (pEC->pNode->add(pEC->pConcept->getConcept1()))
						insertionList.push_back(new ExpandableConcept(pEC->pNode, pEC->pConcept->getConcept1()));
					if (pEC->pNode->add(pEC->pConcept->getConcept2()))
						insertionList.push_back(new ExpandableConcept(pEC->pNode, pEC->pConcept->getConcept2()));
					result = EXPANSION_RESULT_OK;
					skipThisExpandableConcept = true;
					break;

				case Concept::TYPE_DISJUNCTION:
				{
					// We now need to duplicate the incoming completion tree.
					// This will clone the completion tree returning the new completion tree and the corresponding node to the one given.
					std::pair<CompletionTree*, Node*> dupresult = duplicate(pEC->pNode);
					pNewCompletionTree = dupresult.first;
					// Now add the first concept of the disjunction to the actual completion tree
					if (pEC->pNode->add(pEC->pConcept->getConcept1()))
						insertionList.push_back(new ExpandableConcept(pEC->pNode, pEC->pConcept->getConcept1()));
					// then add the second concept of the disjunction to the new completion tree
					if (dupresult.second->add(pEC->pConcept->getConcept2()))
						pNewCompletionTree->addExpandableConcept(new ExpandableConcept(dupresult.second, pEC->pConcept->getConcept2()));
					result = EXPANSION_RESULT_OK;
					skipThisExpandableConcept = true;
					break;
				}

				case Concept::TYPE_EXISTENTIAL_RESTRICTION:
				{
					Symbol role = pEC->pConcept->getRole();
					const Concept* pQualificationConcept = pEC->pConcept->getQualificationConcept();
					// First get the range of nodes reachable by this one through thic concept role
					Node::RelationMapRange range = pEC->pNode->roleAccessibilities.equal_range(role);
					bool conceptFound = false;
					for (Node::RelationMapIterator it = range.first; it != range.second && !conceptFound; ++it)
						conceptFound = it->second->contains(pQualificationConcept);
					if (!conceptFound)
					{
						// Then create a new world that contains the qualification concept
						Node* pNode = createNode(pEC->pNode);
						// Add all ontology concepts to it
						for (size_t i = 0; i < ontology.size(); ++i)
							if (pNode->add(ontology[i]))
								insertionList.push_back(new ExpandableConcept(pNode, ontology[i]));
						pEC->pNode->roleAccessibilities.insert(SymbolNodePair(role, pNode));
						if (pNode->add(pQualificationConcept))
							insertionList.push_back(new ExpandableConcept(pNode, pQualificationConcept));
					}
					result = EXPANSION_RESULT_OK;
					skipThisExpandableConcept = true; //existential concept always consumed
					break;
				}

				case Concept::TYPE_UNIVERSAL_RESTRICTION:
				{
					Symbol role = pEC->pConcept->getRole();
					const Concept* pQualificationConcept = pEC->pConcept->getQualificationConcept();
					// First get the range of nodes reachable by this one through thic concept role
					Node::RelationMapRange range = pEC->pNode->roleAccessibilities.equal_range(role);
					bool conceptNotFound = false;
					for (Node::RelationMapIterator it = range.first; it != range.second; ++it)
					{
						if (it->second->add(pQualificationConcept))
						{
							insertionList.push_back(new ExpandableConcept(it->second, pQualificationConcept));
							conceptNotFound = true;
						}
					}
					if (conceptNotFound)
					{
						result = EXPANSION_RESULT_OK;
						skipThisExpandableConcept = false;
					} else
						result = EXPANSION_RESULT_NOT_POSSIBLE;
					break;
				}

				default:
					throw Exception("Invalid concept found during expansion.");
			}
		}
	}

	if (skipThisExpandableConcept)
		delete pEC;
	else
		insertionList.push_back(pEC); // Reinsert it in the list

	// Now insert back all temporarily removed or newly inserted expandable concepts
	while (!insertionList.empty())
	{
		mExpandableConceptQueue.push_back(insertionList.back());
		insertionList.pop_back();
	}
	// Restore the heap structure
	make_heap(mExpandableConceptQueue.begin(), mExpandableConceptQueue.end(), ExpandableConcept::Compare());
	return result;
}

std::pair<Reasoner::CompletionTree*, Reasoner::Node*> Reasoner::CompletionTree::duplicate(const Node* pNode) const
{
	CompletionTree* pCompletionTree = new CompletionTree;
	Node* pCorrespondingNode;
	map<const Node*, Node*> nodeToNodeMap;
	nodeToNodeMap[0] = 0;
	for (NodeSet::const_iterator it = mNodes.begin(); it != mNodes.end(); ++it)
	{
		Node* pNewNode = new Node(**it);
		// Map the old one to the new one
		nodeToNodeMap[*it] = pNewNode;
		pCompletionTree->mNodes.insert(pNewNode);

		if (*it == pNode)
			pCorrespondingNode = pNewNode;
	}
	// Adjust role accessibilities and adjust all parent and blockingNode pointers so that they point to the new nodes.
	for (NodeSet::const_iterator it = pCompletionTree->mNodes.begin(); it != pCompletionTree->mNodes.end(); ++it)
	{
		for (Node::RoleAccessibilityMap::iterator rit = (*it)->roleAccessibilities.begin(); rit != (*it)->roleAccessibilities.end(); ++rit)
			rit->second = nodeToNodeMap[rit->second];

		(*it)->pBlockingNode = nodeToNodeMap[(*it)->pBlockingNode];
		(*it)->pParentNode = nodeToNodeMap[(*it)->pParentNode];
	}
	// Finally duplicate the expandable list into the new Completion Tree
	for (size_t i = 0; i < mExpandableConceptQueue.size(); ++i)
		pCompletionTree->mExpandableConceptQueue.push_back(new ExpandableConcept(
		nodeToNodeMap[mExpandableConceptQueue[i]->pNode],
		mExpandableConceptQueue[i]->pConcept
		));
	// Make the heap structure
	make_heap(pCompletionTree->mExpandableConceptQueue.begin(), pCompletionTree->mExpandableConceptQueue.end(), ExpandableConcept::Compare());
	return pair<Reasoner::CompletionTree*, Reasoner::Node*>(pCompletionTree, pCorrespondingNode);
}

void Reasoner::CompletionTree::toModel(const ConceptManager* pConceptManager, Model* pModel) const
{
	pModel->clear();
	map<Node*, Instance*> nodeToInstance;
	// First create all instaces
	for (NodeSet::const_iterator it = mNodes.begin(); it != mNodes.end(); ++it)
	{
		Node* pNode = *it;
		if (!pNode->isBlocked())
		{
			Instance * pInstance = pModel->createInstance();
			nodeToInstance[pNode] = pInstance;
			for (set<Symbol>::const_iterator it2 = pNode->positiveAtomicConcepts.begin(); it2 != pNode->positiveAtomicConcepts.end(); ++it2)
				pInstance->addConcept(pConceptManager->getAtomicConcept(true, *it2));
			for (set<Symbol>::const_iterator it2 = pNode->negativeAtomicConcepts.begin(); it2 != pNode->negativeAtomicConcepts.end(); ++it2)
				pInstance->addConcept(pConceptManager->getAtomicConcept(false, *it2));
			for (set<const Concept*>::const_iterator it2 = pNode->complexConcepts.begin(); it2 != pNode->complexConcepts.end(); ++it2)
				pInstance->addConcept(*it2);
		}
	}

	// Then add role accessibilities
	for (NodeSet::const_iterator it = mNodes.begin(); it != mNodes.end(); ++it)
	{
		Node* pNode = *it;
		if (!pNode->isBlocked())
		{
			Instance * pInstance = nodeToInstance[pNode];

			for (multimap<Symbol, Node*>::const_iterator it2 = pNode->roleAccessibilities.begin(); it2 != pNode->roleAccessibilities.end(); ++it2)
			{
				if (it2->second->pBlockingNode != pNode)
					pInstance->addRoleAccessibility(it2->first, nodeToInstance[it2->second]);
				else
					pInstance->addRoleAccessibility(it2->first, pInstance);
			}
		}
	}
}

bool Reasoner::ExpandableConcept::Compare::operator ()(const ExpandableConcept* pEC1, const ExpandableConcept * pEC2) const
{
	// Return false to say that pEC1 is BETTER than pEC2

	// Sort by determinism
	if (pEC1->pConcept->isExpansionDeterministic() && !pEC2->pConcept->isExpansionDeterministic())
		return false;

	// Ok pEC2 is not nondeterministic, let's check the concepts
	// Prefer atomic concepts
	if (pEC1->pConcept->isAtomic() || pEC2->pConcept->isAtomic())
	{
		if (pEC1->pConcept->isAtomic())
			return false;
		else
			return true;
	}

	// Ok none of them is atomic, we'll prefer conjunctions then
	if (pEC1->pConcept->getType() == Concept::TYPE_CONJUNCTION || pEC2->pConcept->getType() == Concept::TYPE_CONJUNCTION)
	{
		if (pEC1->pConcept->getType() == Concept::TYPE_CONJUNCTION)
			return false;
		else
			return true;
	}

	// Alright, then choose for universal role restrictions
	if (pEC1->pConcept->getType() == Concept::TYPE_UNIVERSAL_RESTRICTION || pEC2->pConcept->getType() == Concept::TYPE_UNIVERSAL_RESTRICTION)
	{
		if (pEC1->pConcept->getType() == Concept::TYPE_UNIVERSAL_RESTRICTION)
			return false;
		else
			return true;
	}

	// Ok, let's hope it is an existential restriction
	if (pEC1->pConcept->getType() == Concept::TYPE_EXISTENTIAL_RESTRICTION || pEC2->pConcept->getType() == Concept::TYPE_EXISTENTIAL_RESTRICTION)
	{
		if (pEC1->pConcept->getType() == Concept::TYPE_EXISTENTIAL_RESTRICTION)
			return false;

		else
			return true;
	}

	// No way, they're both conjunctions... just say anything (for now).	
	return true;
}


}

