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

#include "Reasoner.h"
#include "Model.h"

using namespace std;

namespace tinyreason
{

Reasoner::Reasoner(const SymbolDictionary* pSymbolDictionary, const ConceptManager* pConceptManager) :
mpSymbolDictionary(pSymbolDictionary),
mpConceptManager(pConceptManager) { }

Reasoner::~Reasoner() { }

void Reasoner::setTboxConcepts(const std::vector<const Concept*>& tbox)
{
	mTbox = tbox;
}

void Reasoner::setTransitiveRole(Symbol role)
{
	mTransitiveRolesSet.insert(role);
}

void Reasoner::setTransitiveRoles(const std::vector<Symbol>& transitiveRoles)
{
	for (size_t i = 0; i < transitiveRoles.size(); ++i)
		mTransitiveRolesSet.insert(transitiveRoles[i]);
}

bool Reasoner::isSatisfiable(const Concept* pConcept, Model* pModel, bool verbose) const
{
	vector<const Concept*> singleton;
	singleton.push_back(pConcept);
	return isSatisfiable(singleton, pModel);
}

bool Reasoner::isSatisfiable(const std::vector<const Concept*>& concepts, Model* pModel, bool verbose) const
{
	auto_ptr< Logger> apLogger(new Logger(std::cout, mpSymbolDictionary));
	const Logger * pLogger = 0;
	if (verbose)
		pLogger = apLogger.get();

	mCompletionTreeIDCounter = 1;
	CompletionTree* pCompletionTree = new CompletionTree(this, pLogger);
	Node* pNode = pCompletionTree->createNode(0);

	// Make a queue containing expandable concepts
	if (!mTbox.empty() && pLogger)
		pLogger->log("Adding Tbox concepts into first node label.");
	for (size_t i = 0; i < mTbox.size(); ++i)
		if (pNode->addConcept(mTbox[i], pLogger, pCompletionTree))
			pCompletionTree->addExpandableConcept(new ExpandableConcept(pNode, mTbox[i]));
	if (pLogger)
		pLogger->log("Adding testing user concept into the first node label.");
	for (size_t i = 0; i < concepts.size(); ++i)
		if (pNode->addConcept(concepts[i], pLogger, pCompletionTree))
			pCompletionTree->addExpandableConcept(new ExpandableConcept(pNode, concepts[i]));

	std::vector<CompletionTree*> completionTrees;
	completionTrees.push_back(pCompletionTree);

	// Then... go!	
	bool foundCompleteCompletionTree = false;
	size_t completeTreeCount = 0;
	do
	{
		// Take the first available Completion Tree in the open list
		pCompletionTree = completionTrees.front();
		if (pLogger)
			pLogger->log("Completion Tree " + toString(pCompletionTree->getID()) + " chosen to be expanded.");
		// Let the completion tree expand
		CompletionTree* pNewCompletionTree = 0;
		ExpansionResult result = pCompletionTree->expand(pNewCompletionTree);
		// If the expansion algorithm created a new completion tree, add it to our active set
		if (pNewCompletionTree)
		{
			completionTrees.push_back(pNewCompletionTree);
			push_heap(completionTrees.begin(), completionTrees.end(), CompletionTree::ComparePtrs());
		}
		switch (result)
		{
			case EXPANSION_RESULT_NOT_POSSIBLE:
				// Yes! We found a model as this completion tree cannot be further expanded.
				if (pLogger)
					pLogger->log(pCompletionTree, "expansion not be possible, model found!");
				foundCompleteCompletionTree = true;
				break;

			case EXPANSION_RESULT_OK:
				// Ok we expandend the complex concept but we didnt find a clash.
				if (pLogger)
					pLogger->log(pCompletionTree, "expansion ok!");
				break;

			case EXPANSION_RESULT_CLASH:
				// Clash found in a node of current completion tree.
				// Delete the incoherent completion tree from our set processing
				// trees.
				if (pLogger)
					pLogger->log(pCompletionTree, "clash found!");
				delete pCompletionTree;
				pop_heap(completionTrees.begin(), completionTrees.end(), CompletionTree::ComparePtrs());
				completionTrees.pop_back();
				++completeTreeCount;
				break;
		}
	} while (!completionTrees.empty() && !foundCompleteCompletionTree);

	cout << "Number of complete trees: " + toString(completeTreeCount) + ". Number of incomplete trees: " + toString(completionTrees.size()) +
	   ". (total " + toString(completeTreeCount + completionTrees.size()) + ").\n";

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
		for (vector<CompletionTree*>::iterator it = completionTrees.begin(); it != completionTrees.end(); ++it)
			delete *it;

		return true;
	}

}
////////////////////////////////////////////////////////////////////////////////

bool Reasoner::Node::addConcept(const Concept * pConcept, const Logger* pLogger, const CompletionTree * pLoggingCT)
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
	if (result)
		++totalConceptCount;
	if (result && pLogger)
		pLogger->log(pLoggingCT, this, pConcept, "added to this node.");
	else if (!result && pLogger)
		pLogger->log(pLoggingCT, this, pConcept, "skipped as already present in this node.");

	// If we did add a new concept to this node, we must chack that whether the blocking
	// node contains it anyway
	if (result && pBlockingNode)
	{
		if (!pBlockingNode->contains(pConcept))
		{
			if (pLogger)
				pLogger->log(pLoggingCT, this, pConcept, " not in present in blocking node (node " + toString(pBlockingNode->ID) + ") label therefore this node can be no more blocked by it.");
			// Our blocking node does NOT contain the newly added concept thus we
			// must look for another node whose set of concepts contains this ones'
			pBlockingNode = pBlockingNode->pParentNode;
			while (pBlockingNode != 0)
			{
				if (pBlockingNode->containsConceptsOf(this))
					break; // Found!
				pBlockingNode = pBlockingNode->pParentNode;
			}
			// Here if pBlockingNode != 0 we found one and this node is still blocked
			// else this node will now be free.
			if (pLogger)
			{
				if (pBlockingNode == 0)
					pLogger->log(pLoggingCT, this, "no ancestor node can block this node. Node is now free.");
				else
				{
					pLogger->log(pLoggingCT, this, "is now blocked by node" + toString(pBlockingNode->ID) + ".");
				}
			}
		}
	}
	return result;
}

void Reasoner::Node::addRoleAccessibility(Symbol role, Node* pToOtherNode)
{
	// NOTE: This implementation assumes that a role accessibility is made always
	// to new nodes.
	roleAccessibilities.insert(SymbolNodePair(role, pToOtherNode));
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

bool Reasoner::ExpandableConcept::Compare::operator ()(const ExpandableConcept* pEC1, const ExpandableConcept * pEC2) const
{
	// Return false to say that pEC1 is BETTER than pEC2

	// Expand non blocked nodes first.
	if (pEC1->pNode->isBlocked() && !pEC2->pNode->isBlocked())
		return true;
	else if (pEC2->pNode->isBlocked() && !pEC1->pNode->isBlocked())
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

////////////////////////////////////////////////////////////////////////////////

bool Reasoner::CompletionTree::ComparePtrs::operator ()(const CompletionTree* pCT1, const CompletionTree* pCT2) const
{
	//	// True means CT1 is worse than CT2
	// Compare score, higher is worse
	if (pCT1->mScore < pCT2->mScore)
		return false;
	else if (pCT1->mScore > pCT2->mScore)
		return true;
	else
	{
		// Then if same, compare the #concept over #nodes ratio
		float conceptCount1 = pCT1->getConceptCount();
		float conceptCount2 = pCT2->getConceptCount();
		return conceptCount1 / pCT1->mNodes.size() < conceptCount2 / pCT2->mNodes.size();
	}
}

Reasoner::CompletionTree::CompletionTree(const Reasoner* pReasoner, const Logger* pLogger) :
mpReasoner(pReasoner), mID(pReasoner->mCompletionTreeIDCounter++), mpLogger(pLogger)
{
	if (mpLogger)
		mpLogger->log("Completion Tree " + toString(mID) + " created.");
}

Reasoner::CompletionTree::~CompletionTree()
{

	deleteAll(mNodes);
	deleteAll(mExpandableConceptQueue);
}

size_t Reasoner::CompletionTree::getConceptCount() const
{
	size_t c = 0;
	for (std::set<Node*>::const_iterator it = mNodes.begin(); it != mNodes.end(); ++it)
		c += (*it)->totalConceptCount;
	return c;
}

Reasoner::Node* Reasoner::CompletionTree::createNode(Node* pParent)
{
	Node* pNode = new Node(mNodes.size() + 1, pParent);
	mNodes.insert(pNode);

	if (mpLogger)
		mpLogger->log("Node " + toString(mID) + "." + toString(pNode->ID) + " created.");

	return pNode;
}

void Reasoner::CompletionTree::addExpandableConcept(const ExpandableConcept* pExpandableConcept)
{
	mExpandableConceptQueue.push_back(pExpandableConcept);
	push_heap(mExpandableConceptQueue.begin(), mExpandableConceptQueue.end(), ExpandableConcept::Compare());
	// Update score
	mScore += getConceptScore(pExpandableConcept->pConcept);
}

Reasoner::ExpansionResult Reasoner::CompletionTree::expand(CompletionTree*& pNewCompletionTree)
{
	list<const ExpandableConcept*> insertionList;
	ExpansionResult result = EXPANSION_RESULT_NOT_POSSIBLE;
	bool skipThisExpandableConcept = false;
	const ExpandableConcept* pEC = 0;

	if (mpLogger && mExpandableConceptQueue.empty())
		mpLogger->log(this, "no more expandable concepts...");

	while (result == EXPANSION_RESULT_NOT_POSSIBLE && !mExpandableConceptQueue.empty())
	{
		// Pop the most promising expandable concept
		pop_heap(mExpandableConceptQueue.begin(), mExpandableConceptQueue.end(), ExpandableConcept::Compare());
		pEC = mExpandableConceptQueue.back();
		mExpandableConceptQueue.pop_back();
		mScore -= getConceptScore(pEC->pConcept);

		skipThisExpandableConcept = true; //by default

		if (mpLogger)
			mpLogger->log(this, pEC->pNode, pEC->pConcept, "chosen to be expanded.");

		// If we're expanding a "bottom" concept, that means inconsistency
		if (pEC->pConcept == Concept::getBottomConcept())
		{
			if (mpLogger)
				mpLogger->log(this, pEC->pNode, pEC->pConcept, "concept is bottom, automatic clash.");
			result = EXPANSION_RESULT_CLASH;
		} else if (pEC->pNode->isBlocked())
		{
			// We cannot expand in a blocked node, carry on.
			if (mpLogger)
				mpLogger->log(this, pEC->pNode, "node is blocked, thus concept is skipped.");
			skipThisExpandableConcept = false;
		} else
		{
			switch (pEC->pConcept->getType())
			{
				case Concept::TYPE_POSITIVE_ATOMIC:
					// If there's a negative atomic argument in node with the same concept, clash!
					if (pEC->pNode->negativeAtomicConcepts.find(pEC->pConcept->getSymbol()) != pEC->pNode->negativeAtomicConcepts.end())
						result = EXPANSION_RESULT_CLASH;
					else
						result = EXPANSION_RESULT_OK;
					break;

				case Concept::TYPE_NEGATIVE_ATOMIC:
					// If there's a positive atomic argument in node with the same concept, clash!
					if (pEC->pNode->positiveAtomicConcepts.find(pEC->pConcept->getSymbol()) != pEC->pNode->positiveAtomicConcepts.end())
						result = EXPANSION_RESULT_CLASH;
					else
						result = EXPANSION_RESULT_OK;
					break;

				case Concept::TYPE_CONJUNCTION:
					if (mpLogger)
						mpLogger->log(this, pEC->pNode, pEC->pConcept, "adding subconcepts to the same node.");
					// Add both subconcepts in node, simple enough! :)
					if (pEC->pNode->addConcept(pEC->pConcept->getConcept1(), mpLogger, this))
						insertionList.push_back(new ExpandableConcept(pEC->pNode, pEC->pConcept->getConcept1()));
					if (pEC->pNode->addConcept(pEC->pConcept->getConcept2(), mpLogger, this))
						insertionList.push_back(new ExpandableConcept(pEC->pNode, pEC->pConcept->getConcept2()));
					result = EXPANSION_RESULT_OK;
					break;

				case Concept::TYPE_DISJUNCTION:
				{
					if (mpLogger)
						mpLogger->log(this, pEC->pNode, pEC->pConcept, "adding the first subconcept into this Completion Tree, the second one into its duplication.");
					// We now need to duplicate the incoming completion tree.
					// This will clone the completion tree returning the new completion tree and the corresponding node to the one given.
					std::pair<CompletionTree*, Node*> dupresult = duplicate(pEC->pNode, insertionList);
					pNewCompletionTree = dupresult.first;
					// Now add the first concept of the disjunction to the actual completion tree
					if (pEC->pNode->addConcept(pEC->pConcept->getConcept1(), mpLogger, this))
						insertionList.push_back(new ExpandableConcept(pEC->pNode, pEC->pConcept->getConcept1()));
					// then add the second concept of the disjunction to the new completion tree
					if (dupresult.second->addConcept(pEC->pConcept->getConcept2(), mpLogger, dupresult.first))
						pNewCompletionTree->addExpandableConcept(new ExpandableConcept(dupresult.second, pEC->pConcept->getConcept2()));
					result = EXPANSION_RESULT_OK;
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
					{
						conceptFound = it->second->contains(pQualificationConcept);
						if (mpLogger && conceptFound)
							mpLogger->log(this, it->second, "qualification concept \"" + pQualificationConcept->toString(*mpReasoner->mpSymbolDictionary) + "\" found, no new node created.");
					}
					if (!conceptFound)
					{
						// Then create a new world that contains the qualification concept
						Node* pNode = createNode(pEC->pNode);
						// Add all tbox concepts to it
						for (size_t i = 0; i < mpReasoner->getTboxConcepts().size(); ++i)
							if (pNode->addConcept(mpReasoner->getTboxConcepts()[i], mpLogger, this))
								insertionList.push_back(new ExpandableConcept(pNode, mpReasoner->getTboxConcepts()[i]));
						// Make other node accessible from this one through this role
						pEC->pNode->addRoleAccessibility(role, pNode);
						if (pNode->addConcept(pQualificationConcept, mpLogger, this))
							insertionList.push_back(new ExpandableConcept(pNode, pQualificationConcept));

						if (mpLogger)
							mpLogger->log(this, pNode, "adding qualification concept \"" + pQualificationConcept->toString(*mpReasoner->mpSymbolDictionary) + "\" to this new node.");
					}
					result = EXPANSION_RESULT_OK;
					break;
				}

				case Concept::TYPE_UNIVERSAL_RESTRICTION:
				{
					Symbol role = pEC->pConcept->getRole();
					const Concept* pQualificationConcept = pEC->pConcept->getQualificationConcept();
					// First get the range of nodes reachable by this one through this concept role
					Node::RelationMapRange range = pEC->pNode->roleAccessibilities.equal_range(role);
					for (Node::RelationMapIterator it = range.first; it != range.second; ++it)
					{
						if (it->second->addConcept(pQualificationConcept, mpLogger, this))
						{
							insertionList.push_back(new ExpandableConcept(it->second, pQualificationConcept));
							result = EXPANSION_RESULT_OK;
							if (mpLogger)
								mpLogger->log(this, it->second, "adding qualification concept \"" + pQualificationConcept->toString(*mpReasoner->mpSymbolDictionary) + "\" to this existing node.");
						}
						// This applies ONLY if this role is transitive.
						if (mpReasoner->isTransitive(role))
						{
							if (it->second->addConcept(pEC->pConcept, mpLogger, this))
							{
								insertionList.push_back(new ExpandableConcept(it->second, pEC->pConcept));
								result = EXPANSION_RESULT_OK;
								if (mpLogger)
									mpLogger->log(this, it->second, pQualificationConcept, "copying the whole concept to this existing node for transitivity.");
							}
						}
					}
					if (result == EXPANSION_RESULT_NOT_POSSIBLE)
					{
						if (mpLogger)
							mpLogger->log(this, pEC->pNode, pEC->pConcept, "impossible to expand as no reachable node does not contain qualification concept or whole concept already present in all reachable nodes (transitivity).");
					}
					skipThisExpandableConcept = false; // never ever totally remove an universal restriction
					break;
				}
				default:
					throw Exception("Invalid concept found during expansion.");
			}
		}

		if (result != EXPANSION_RESULT_NOT_POSSIBLE && skipThisExpandableConcept)
			delete pEC;
		else
			insertionList.push_back(pEC); // Reinsert it in the list
	}

	// Now insert back all temporarily removed or newly inserted expandable concepts
	while (!insertionList.empty())
	{
		mExpandableConceptQueue.push_back(insertionList.back());
		insertionList.pop_back();
		// Add back the score
		mScore += getConceptScore(pEC->pConcept);
	}
	// Restore the heap structure
	make_heap(mExpandableConceptQueue.begin(), mExpandableConceptQueue.end(), ExpandableConcept::Compare());

	return result;
}

std::pair<Reasoner::CompletionTree*, Reasoner::Node*> Reasoner::CompletionTree::duplicate(const Node* pNode, const std::list<const ExpandableConcept*>& insertionList) const
{
	CompletionTree* pCompletionTree = new CompletionTree(mpReasoner, mpLogger);
	Node* pCorrespondingNode = 0;
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
	// Add to be inserted ECs
	for (list<const ExpandableConcept*>::const_iterator it = insertionList.begin(); it != insertionList.end(); ++it)
		pCompletionTree->mExpandableConceptQueue.push_back(new ExpandableConcept(
	   nodeToNodeMap[(*it)->pNode],
	   (*it)->pConcept
	   ));
	// Make the heap structure
	make_heap(pCompletionTree->mExpandableConceptQueue.begin(), pCompletionTree->mExpandableConceptQueue.end(), ExpandableConcept::Compare());

	return pair<Reasoner::CompletionTree*, Reasoner::Node*>(pCompletionTree, pCorrespondingNode);
}

void Reasoner::CompletionTree::toModel(const ConceptManager* pConceptManager, Model* pModel) const
{
	pModel->clear();
	map<const Node*, Individual*> nodeToIndividual;
	// First create all instaces
	for (NodeSet::const_iterator it = mNodes.begin(); it != mNodes.end(); ++it)
	{
		Node* pNode = *it;
		if (!pNode->isBlocked())
		{
			Individual * pIndividual = pModel->createIndividual(pNode->ID);
			nodeToIndividual[pNode] = pIndividual;
			for (set<Symbol>::const_iterator it2 = pNode->positiveAtomicConcepts.begin(); it2 != pNode->positiveAtomicConcepts.end(); ++it2)
				pIndividual->addConcept(pConceptManager->getAtomicConcept(true, *it2));
			for (set<Symbol>::const_iterator it2 = pNode->negativeAtomicConcepts.begin(); it2 != pNode->negativeAtomicConcepts.end(); ++it2)
				pIndividual->addConcept(pConceptManager->getAtomicConcept(false, *it2));
			for (set<const Concept*>::const_iterator it2 = pNode->complexConcepts.begin(); it2 != pNode->complexConcepts.end(); ++it2)
				pIndividual->addConcept(*it2);
		}
	}

	// Then add role accessibilities
	for (NodeSet::const_iterator it = mNodes.begin(); it != mNodes.end(); ++it)
	{
		Node* pNode = *it;
		if (!pNode->isBlocked())
		{
			Individual * pIndividual = nodeToIndividual[pNode];

			for (multimap<Symbol, Node*>::const_iterator it2 = pNode->roleAccessibilities.begin(); it2 != pNode->roleAccessibilities.end(); ++it2)
			{
				if (it2->second->isBlocked())
					pIndividual->addRoleAccessibility(it2->first, nodeToIndividual[it2->second->pBlockingNode]);
				else
					pIndividual->addRoleAccessibility(it2->first, nodeToIndividual[it2->second]);
			}
		}
	}
}

size_t Reasoner::CompletionTree::getConceptScore(const Concept * pConcept)
{
	switch (pConcept->getType())
	{
		case Concept::TYPE_POSITIVE_ATOMIC:
		case Concept::TYPE_NEGATIVE_ATOMIC:
			return 1;
		case Concept::TYPE_CONJUNCTION:
		case Concept::TYPE_UNIVERSAL_RESTRICTION:
			return 5;
		case Concept::TYPE_EXISTENTIAL_RESTRICTION:
			return 10;
		case Concept::TYPE_DISJUNCTION:
			return 20;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

Reasoner::Logger::Logger(std::ostream& outStream, const SymbolDictionary* pSymbolDictionary) :
mOutStream(outStream), mpSymbolDictionary(pSymbolDictionary) { }

void Reasoner::Logger::log(const std::string& message) const
{
	mOutStream << "> " << message << endl;
}

void Reasoner::Logger::log(const CompletionTree* pCP, const std::string& message) const
{
	mOutStream << "> " << "In Completion Tree " << pCP->getID() << ": " << message << endl;
}

void Reasoner::Logger::log(const CompletionTree* pCP, const Node* pNode, const std::string& message) const
{
	mOutStream << "> " << "In node " << pCP->getID() << "." << pNode->ID << ": " << message << endl;
}

void Reasoner::Logger::log(const CompletionTree* pCP, const Node* pNode, const Concept* pConcept, const std::string& message) const
{
	mOutStream << "> " << "In node " << pCP->getID() << "." << pNode->ID << ": concept \"" << pConcept->toString(*mpSymbolDictionary) << "\" " << message << endl;
}

}

