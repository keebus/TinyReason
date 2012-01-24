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
		string concept = "";
		for (int i = 1; i < argc; ++i)
			concept += string(argv[i]) + " ";

		SymbolDictionary sd;
		ConceptManager cp(&sd);
		const Concept* pConcept = cp.parse(concept);
		cout << "Parsed: " << pConcept->toString(sd) << endl;

		Reasoner r(&sd, &cp);
		Model example;
		if (r.isSatisfiable(pConcept, &example))
		{
			cout << "Concept satisfiable!" << endl;
			ofstream outFile("example.dot");
			example.dumpToDOTFile(sd, outFile);
			cout << "Example dumped to example.dot" << endl;
		} else
			cout << "Concept not satisfiable!" << endl;

		return 0;

	} catch (Exception& e)
	{
		cout << e.what() << endl;
		cout.flush();
		return -1;
	}

}

