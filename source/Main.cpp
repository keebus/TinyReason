/*******************************************************************************
 * ALC Reasoner                                                                *
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

#include "Common.h"
#include "Reasoner.h"
#include "Model.h"

using namespace std;
using namespace salcr;

/*
 * 
 */
int main(int argc, char** argv)
{
	try
	{
		if (argc < 3)
		{
			cout << "Usage: saltr (<ontology-file-name|-) <concepts-to-test-satisfiability>";
			return -1;
		}

		SymbolDictionary sd;
		ConceptManager cp(&sd);
		Reasoner r(&sd, &cp);
		
		if (argv[1][0] != '-')
		{
			vector<const Concept*> ontologyConcepts;
			ifstream f(argv[1]);
			cp.parseConcepts(f, ontologyConcepts);
			r.setOntology(ontologyConcepts);
			cout << "Ontology concepts (optimized and normalized):" << endl;
			for (size_t i = 0; i < ontologyConcepts.size(); ++i)
				cout << "\t* " << ontologyConcepts[i]->toString(sd) << endl;
		}

		string conceptString = "";
		for (int i = 2; i < argc; ++i)
			conceptString += string(argv[i]) + " ";

		vector<const Concept*> concepts;
		cp.parseConcepts(conceptString, concepts);
		cout << "Parsed concepts: " << endl;
		for (size_t i = 0; i < concepts.size(); ++i)
			cout << "\t* " << concepts[i]->toString(sd) << endl;


		Model example;
		if (r.isSatisfiable(concepts, &example))
		{
			cout << "Conjunction of concepts is satisfiable!" << endl;
			ofstream outFile("example.dot");
			example.dumpToDOTFile(sd, outFile);
			cout << "Example dumped to example.dot" << endl;
		} else
			cout << "Conjunction of concepts is not satisfiable!" << endl;

		return 0;

	} catch (Exception& e)
	{
		cout << e.what() << endl;
		cout.flush();
		return -1;
	}

}

