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

#include "Common.h"
#include "Reasoner.h"
#include "Model.h"

using namespace std;
using namespace tinyreason;

/*
 * 
 */
int main(int argc, char** argv)
{
	try
	{
		if (argc < 4)
		{
			cout << "Usage: saltr <options|-> (<ontology-file-name|-) <concepts-to-test-satisfiability>\n\noptions:\n\tv: verbose, shows a log of the precedure of decision;\n\tp: shows parsed concepts (both from ontology and user concept);" << endl;
			return -1;
		}

		bool verbose = false, showParsedResult = false;
		string stroptions(argv[1]);
		for (size_t i = 0; i < stroptions.size(); ++i)
		{
			switch (stroptions[i])
			{
				case '-':
					break;
				case 'v': // Verbose
					verbose = true;
					break;
				case 'p': // Shows parse result
					showParsedResult = true;
					break;
			}
		}

		SymbolDictionary sd;
		ConceptManager cp(&sd);
		Reasoner r(&sd, &cp);

		if (argv[2][0] != '-')
		{
			vector<const Concept*> ontologyConcepts;
			ifstream f(argv[2]);
			cp.parseConcepts(f, ontologyConcepts);
			r.setOntology(ontologyConcepts);
			if (showParsedResult)
			{
				cout << "Ontology concepts (optimized and normalized):" << endl;
				for (size_t i = 0; i < ontologyConcepts.size(); ++i)
					cout << "\t* " << ontologyConcepts[i]->toString(sd) << endl;
			}
		}

		string conceptString = "";
		for (int i = 3; i < argc; ++i)
			conceptString += string(argv[i]) + " ";

		vector<const Concept*> concepts;
		if (!conceptString.empty())
		{
			cp.parseConcepts(conceptString, concepts);
			if (showParsedResult)
			{
				cout << "Parsed concepts: " << endl;
				for (size_t i = 0; i < concepts.size(); ++i)
					cout << "\t* " << concepts[i]->toString(sd) << endl;
			}
		}

		Model example;
		if (r.isSatisfiable(concepts, &example, verbose))
		{
			ofstream outFile("example.dot");
			example.dumpToDOTFile(sd, outFile);
			cout << "RESULT: Conjunction of concepts is satisfiable! (example dumped to example.dot)" << endl;

		} else
			cout << "RESULT: Conjunction of concepts is not satisfiable!" << endl;

		return 0;

	} catch (Exception& e)
	{
		cout << e.what() << endl;
		cout.flush();
		return -1;
	}

}

