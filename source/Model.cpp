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

#include "Model.h"
#include "Concept.h"

using namespace std;

namespace salcr
{

void Instance::dumpToDOTFile(const SymbolDictionary& symbolDictionary, std::ostream& outStream) const
{
	outStream << (size_t)this << "[label=\"";
	for (std::set<const Concept*>::const_iterator it = mConcepts.begin(); it != mConcepts.end(); ++it)
		outStream << (*it)->toString(symbolDictionary) << "\\n";
	outStream << "\"];";

	for (std::multimap<Symbol, const Instance*>::const_iterator it = mRoleAccessibilities.begin(); it != mRoleAccessibilities.end(); ++it)
		outStream << (size_t)this << " -> " << (size_t) it->second << "[label=\"" << symbolDictionary.toName(it->first) << "\"];";
}

////////////////////////////////////////////////////////////////////////////////

Model::Model() { }

Model::~Model()
{
	deleteAll(mInstances);
}

Instance* Model::createInstance()
{
	Instance* pInstance = new Instance;
	mInstances.push_back(pInstance);
	return pInstance;
}

void Model::clear()
{
	deleteAll(mInstances);
}

void Model::dumpToDOTFile(const SymbolDictionary& symbolDictionary, std::ostream& outStream) const
{
	outStream << "digraph {rankdir=TB;node[shape=record];";

	for (size_t i = 0; i < mInstances.size(); ++i)
		mInstances[i]->dumpToDOTFile(symbolDictionary, outStream);

	outStream << "}";
}

}

