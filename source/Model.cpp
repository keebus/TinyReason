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

#include "Model.h"
#include "Concept.h"

using namespace std;

namespace tinyreason
{

void Instance::dumpToDOTFile(const SymbolDictionary& symbolDictionary, std::ostream& outStream, bool showComplexConcepts) const
{
	outStream << (size_t)this << "[label=\"";
	for (std::set<const Concept*>::const_iterator it = mConcepts.begin(); it != mConcepts.end(); ++it)
		if (showComplexConcepts or (*it)->isAtomic())
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

void Model::dumpToDOTFile(const SymbolDictionary& symbolDictionary, std::ostream& outStream, bool showComplexConcepts) const
{
	outStream << "digraph {rankdir=TB;node[shape=record];";

	for (size_t i = 0; i < mInstances.size(); ++i)
		mInstances[i]->dumpToDOTFile(symbolDictionary, outStream, showComplexConcepts);

	outStream << "}";
}

}

