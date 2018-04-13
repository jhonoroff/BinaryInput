// BinaryInput.cpp : Defines the entry point for the console application.
//
//	Main application
//
//  Developed and compiled in Visual Studio 2012 as a console application
//
//  STL is used throughout
//  C++ 11 std::shared_ptr is used - could have used boost::shared_ptr instead.
//  input.dat is hard coded, but could add file name as a program argument. 
//  Also, I could add cmds [-e -p] where -e = execute instruction and -p = print instruction as arguments as well.
//  This function (client/consumer of IInstruction) only knows that the first segment of each instruction has the 
//		same struction that specifies which instruction and the segment (DWord) count.
//  
// Basic Process:
//  Get a reference to the factory's baseElements
//  Read from input file directly into the factory's elementsBuffer (baseElements)
//  Get an IInstruction ptr to a specific instruction created in the InstuctionFactory
//  Get the size of the remaining instruction data from the IInstruction ptr and allocate a read buffer
//  Read the remaining instruction data (segments) into buffer
//  Init the IInstruction from buffer
//  Call the IInstruction's Print method 

// Update 2/24/2014 - The following issues have been resolved.
// Issues:
//  While(!file.eof() continues once after the last instruction and prints garbage 
//  Sting elemens are not null terminated and are allocated on DWord bounderies 
//  
// ToDo: 
//  The appliction control and file reading should be put into a class structure!
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <tchar.h>

#include <iostream>
#include <fstream>
#include "Instruction.h"

using namespace std;


int _tmain(int argc, _TCHAR* argv[])
{
 	ifstream file("input.dat", ios::in|ios::binary);
	if (file.is_open())
	{
		bool				bKeepGoing = true;			
		InstructionFactory	instFact;
		// Get a pointer to use for the read buffer and is only valid while instFact exists.
		char* buf = instFact.GetElementsBuffer();


		cout << " input.dat contains the following Instructions:" << endl << endl;
		while (bKeepGoing)
		{
			// Read (4 Bytes) from file directly into the Factory's baseElements
			file.read(buf, sizeof(baseElements)); 
			if (!file.eof())
			{
				// Create an IInstruction std::shared_ptr and initialize it to the shared_ptr returned by the factory
				// This would allow us to use instruction outside of this scope or pass it to other 
				// functions or objects without worrying about leaking memory.
				shared_ptr<IInstruction> instruction(instFact.CreateInstruction());

				// Allocate a raw buffer for the remaining instruction elements
				char* buffer = new char[instruction->GetElementsSize()];
				// Read from  file into buffer
				file.read(buffer, instruction->GetElementsSize());
				if (!file.eof())
				{
					instruction->Init(buffer);
					instruction->Print();
				}
				else
				{
					bKeepGoing = false;
				}
				if(buffer)
				{
					delete buffer;
					buffer = 0;
				}
			}
			else
			{
				bKeepGoing = false;
			}
		}

		file.close();
	}
	else 
	{
		cout << "Unable to open file";
	}
	return 0;
};



