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
			cout << "Usage: saltr <options|-> (<Tbox-file-name|-) <concepts-to-test-satisfiability>\n\noptions:\n\te: prints the structure of the example model (if found) that satisfies the concept;\n\tv: verbose, shows a log of the precedure of decision;\n\tp: shows parsed concepts (both from tbox and user concept);\n\tc: prints complex concepts in the output model too;\n\tD: dumps the example model (if found) into a GraphViz DOT file \'example.dot\';" << endl;
			return -1;
		}

		bool printExampleModelStructure = false,
			verbose = false,
			showParsedResult = false,
			showComplexConcepts = false,
			dumpToDOT = false;
		string stroptions(argv[1]);
		for (size_t i = 0; i < stroptions.size(); ++i)
		{
			switch (stroptions[i])
			{
				case '-':
					break;
				case 'e': // Example model structure on stdout
					printExampleModelStructure = true;
					break;
				case 'v': // Verbose
					verbose = true;
					break;
				case 'p': // Shows parse result
					showParsedResult = true;
					break;
				case 'c': // Shows complex concepts
					showComplexConcepts = true;
					break;
				case 'D':
					dumpToDOT = true;
					break;
			}
		}

		SymbolDictionary sd;
		ConceptManager cp(&sd);
		Reasoner r(&sd, &cp);

		vector<Symbol> transitiveRoles;

		if (argv[2][0] != '-')
		{
			vector<const Concept*> tboxConcepts;
			ifstream f(argv[2]);
			cp.parseAssertions(f, tboxConcepts, transitiveRoles);
			r.setTboxConcepts(tboxConcepts);
			if (showParsedResult)
			{
				cout << "Tbox concepts (optimized and normalized):" << endl;
				for (size_t i = 0; i < tboxConcepts.size(); ++i)
					cout << "\t* " << tboxConcepts[i]->toString(sd) << endl;
			}
		}

		string conceptString = "";
		for (int i = 3; i < argc; ++i)
			conceptString += string(argv[i]) + " ";

		vector<const Concept*> concepts;
		if (!conceptString.empty())
		{
			cp.parseAssertions(conceptString, concepts, transitiveRoles);
			if (showParsedResult)
			{
				cout << "Parsed concepts: " << endl;
				for (size_t i = 0; i < concepts.size(); ++i)
					cout << "\t* " << concepts[i]->toString(sd) << endl;
			}
		}

		r.setTransitiveRoles(transitiveRoles);
		if (showParsedResult)
		{
			if (!transitiveRoles.empty())
			{
				cout << "Transitive roles:" << endl << "\t";
				for (size_t i = 0; i < transitiveRoles.size(); ++i)
					cout << sd.toName(transitiveRoles[i]) << ((i == transitiveRoles.size() - 1) ? "\n" : " ,");
			} else
				cout << "NO transitive roles." << endl;
		}

		Model example;
		if (r.isSatisfiable(concepts, &example, verbose))
		{
			cout << "RESULT: Conjunction of concepts is satisfiable!" << endl;
			if (printExampleModelStructure)
			{
				cout << "Example model: " << endl;
				example.dumpToString(sd, std::cout, showComplexConcepts);
			}
			if (dumpToDOT)
			{
				ofstream outFile("example.dot");
				example.dumpToDOT(sd, outFile, showComplexConcepts);
				cout << "Example model dumped DOT file dumped to example.dot." << endl;
			}

		} else
			cout << "RESULT: Conjunction of concepts is NOT satisfiable!" << endl;

		return 0;

	} catch (Exception& e)
	{
		cout << e.what() << endl;
		cout.flush();
		return -1;
	}

}

