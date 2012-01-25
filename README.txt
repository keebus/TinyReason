*******************************************************************************
*                                  TinyReason                                 *
* A very tiny Description Logic Reasoner for the S logic (ALC + trans roles). *
*                   Copyright (R) 2012 Canio Massimo Tristano                 *
*                             Politecnico di Milano                           *
*******************************************************************************

License
=======
	This software is licensed under the BSD-3 license. You will find a copy of
	this license in the LICENSE.txt file contained in this source distribution.

Compiling
=========
	To compile the source code you will require GNU Make and GNU GCC (any
	version	will do fine); on Windows, MinGW is thus required.
		To compile for Windows type
			make WIN32=1
		To compile for any other platform (Linux, OSX) simply type
			make

	You could even use build.bat script file to compile for Windows or build.sh
	to compile for the other platforms.

	The source code has no dependecies (apart of stdlibc++) and can be compiled
	with any compiler, but you will have to make your own makefile for that.

DL Syntax
=========
	* "anything" stands for the top concept, while "nothing" stands for the
		bottom concept.
	* Any string like (a-zA-Z_0-9)+ is an Atomic Concept thus a Concept.
	* If A is a concept, then "not A" is a concept.
	* If A and B are concepts, then "A and B", "A or B" are concepts.
	* If A and B are concepts, then "A isa B" is a concept that means "B subsumes
		A" or "if A then B" and "A is B" is a concept that means "A subsumes B
		and B subsumes A" or "A if and only if B".
	* "R something" where R is the name of a role form (a-zA-Z_0-9)+ stands for
		"<there exists>R.T" in common DL syntax where T is the top concept.
	* If C is a concept and R is the name of a role, "R some C" means
		"<there exists>R.C" in common DL syntax.
	* If C is a concept and R is the name of a role, "R only C" means
		"<for all>R.C" in common DL syntax.
	* If A is a concept, then "(A)" is a concept.
	
	Examples:
		Man and Woman isa nothing; 		(or simply "not(Man and Woman)")
		Person is Man or Woman;
		Woman is Person and Female;
		Man is Person and Male;
		Parent is hasChild something;
		Mother is Woman and Parent;
		
Usage
=====
	The tc executable must be used giving the following arguments:
		tc (options|-) (ontology_file|-) <concept to evaluate>
	
	The following options are supported:
		v: verbose, will force the Reasoner to print out the log of all the
			operations done.
		p: parser output, will print the result of the parsing of the ontology
			concepts and the given user concept.
		-: no option (mandatory if you specify no option).
	
	The ontology file is optional. It must contain a list of concepts separated
	by a semicolon ';'. To use no ontology, pass '-' as argument.
	
	The concept to evaluate is mandatory. The Reasoner will try to satisfy the
	specified concept within the specified ontology.

Reasoning Services
==================
	* Concept satisfiability: pass any option and '-' as ontology argument then
	specify the concept to evaluate. The reasoner will answer whether it is
	satisfiable or not. If so, it will dump a model example into the file
	"example.dot" in Graphviz DOT format.
	
	* Ontology coherence test: pass any option and specify the ontology file,
	then use "anything" (top concept) as concept to evaluate. If the concept is
	satisfiable, the ontology is coherent.
	
	* Concept satisfiable in ontology: pass any option, specify the ontology
	and the concept to test. The reasoner will return the answer.
	
	* Subsumption test: if you want to know if a concept is subsumes another
	pass any option, the ontology file to use and a concept in the form 
	"not(Concept1 isa Concept2)". If this concept is not satisfiable, then
	Concept2 subsumes Concept1 within the given ontology.

APPENDIX
========
	To convert the DOT file to a PNG image file you can use the following
	command (granted you have Graphviz installed on your machine):
		cat example.dot | dot -Tpng > example.png