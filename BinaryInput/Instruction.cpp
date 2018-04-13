//////////////////////////////////////////////////////////////////////
// Instruction.cpp : contains the immplementation of the 
//							   class:	    BaseInstruction
//							   class:		File_Header
//							   class:		File_Header
//							   class:		Save_Memory_To_File
//							   class:		Write_Memory
//							   class:		InstructionFactory
//
//  Usually and especially in production code, I would put the code for each class in a seperate file
//  STL is used throughout
//	I refer to the fields as elements and DWord blocks as segments 
///////////////////////////////////////////////////////////////////////////////


#include "Instruction.h"
#include <iostream>
#include <sstream> 

using namespace std;


// Quick and dirty look up tables!
static const char* ProductEnumStrings[] = { "Bananas", 
											"Orange", 
											"Grapefruit",
											"Watermelon",
											"Tangerine",
											"Kiwi",
											"Papaya",
											"Mango" };

static const char* SteppingEnumStrings[] = { "a", 
											 "b", 
											 "c",
											 "d",
											 "e",
											 "f",
											 "g",
											 "h" };

static const char* DataSizeString[] = { "BYTE", 
										"WORD", 
										"DWORD",
										"QWORD" };


/////////////////////////////////////////////////////////////////////////////////////////////////////
// BaseInstruction

BaseInstruction::BaseInstruction(baseElements baseElm, int32_t segCount) 
	: m_baseElements(baseElm), m_fixedSegmentCount(segCount)
{}

// This function returns the size of the instruction minus the baseElements
uint32_t BaseInstruction::GetElementsSize() 
{ 
	return (m_baseElements.dwCount*SEGMENT_SIZE)-sizeof(baseElements); 
}

char* BaseInstruction::GetElementsBuffer()
{
	return reinterpret_cast<char*>(&m_baseElements); 
}


void BaseInstruction::Print()
{
	cout << " OpCode: " << "0x" << hex << m_baseElements.opCode << endl;
	cout << " DWord Count: " << m_baseElements.dwCount << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// File_Header

File_Header::File_Header(baseElements baseElm) 
	: BaseInstruction(baseElm, 2)			// 2 = fixedSegmentCount for File_Header
{};

void File_Header::Init(char* buffer)
{
	m_elements = *( reinterpret_cast<elements*>(buffer) );
}


void File_Header::Print()
{
	cout << "FILE_HEADER Elements:" << endl;
	// Call the bass class print
	BaseInstruction::Print();
	cout << " Product: " << m_elements.product << " = " << ProductEnumStrings[m_elements.product] << endl;
	cout << " Stepping: " << m_elements.stepping << " = " << SteppingEnumStrings[m_elements.stepping] << endl;
	cout << " Date: " << dec << m_elements.month << "/" << m_elements.day << "/" << m_elements.year << endl << endl;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Save_Memory_To_File


Save_Memory_To_File::Save_Memory_To_File(baseElements baseElm)
	: BaseInstruction(baseElm, 2)				// 2 = fixedSegmentCount for Save_Memory_To_File
{}

// Init sets m_elements 
// param: buffer is raw data that is assumed to match the size and order of m_elements
void Save_Memory_To_File::Init(char* buffer)
{
	m_elements.memHandle = *reinterpret_cast<uint32_t*>(buffer);    
	buffer += SEGMENT_SIZE;
	int32_t strLen = GetElementsSize() - sizeof(m_elements.memHandle);	// number of chars to copy 
	m_elements.fileName.assign(buffer, strLen);
}


void Save_Memory_To_File::Print()
{
	cout << "SAVE_MEMORY_TO_FILE Elements:" << endl;
	// Call the bass class print
	BaseInstruction::Print();
	cout << " Memory Handle: " << "0x" << hex << m_elements.memHandle << endl;
	cout << " File Name: " << m_elements.fileName << endl << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Write_Memory

Write_Memory::Write_Memory(baseElements baseElm)
	: BaseInstruction(baseElm, 3)				// 3 = fixedSegmentCount for Write_Memory)
{
	m_elements.memAddress = 0;
	m_elements.attributes = 0;
	m_elements.data = 0;
	m_elements.data = new uint32_t[(m_baseElements.dwCount - m_fixedSegmentCount)*SEGMENT_SIZE];
}

Write_Memory::~Write_Memory()
{
	if(m_elements.data)
		delete[] m_elements.data;
};


// Init sets m_elements 
// param: buffer is raw data that is assumed to match the size and order of m_elements
void Write_Memory::Init(char* buffer)
{
	m_elements.memAddress = *( reinterpret_cast<uint32_t*>(buffer) );
	buffer += SEGMENT_SIZE;
	
	// m_elements.attributes repesents dataElmSize and dataCount as a single uint32_t defined by the union 
	m_elements.attributes = *(reinterpret_cast<uint32_t*>(buffer));
	buffer += SEGMENT_SIZE;

	// dereference to force a copy of the data and not just the ptr
	*m_elements.data = *(reinterpret_cast<uint32_t*>(buffer));
}

void Write_Memory::Print()
{
	cout << "WRITE_MEMORY Elements:" << endl;
	// Call the bass class print
	BaseInstruction::Print();
	cout << " Memory Address: " << "0x" << hex << m_elements.memAddress << endl;
	cout << " Data Count: " << m_elements.dataCount << endl;

	std::ostringstream	dataOutput;

	// This switch and multiple loops is not pretty and the code can be optomized and reduced!!! 
	switch(m_elements.dataElmSize)
	{
		case 0:
		{
			uint8_t* data = reinterpret_cast<uint8_t *>(m_elements.data);
			for(int i = 0; i < m_elements.dataCount; i++)
			{
				dataOutput << (*data) << ", ";
				data++;
			}
		}
		break;

		case 1:
		{
			uint16_t* data = reinterpret_cast<uint16_t *>(m_elements.data);		
			for(int i = 0; i < m_elements.dataCount; i++)
			{
				dataOutput << (*data) << ", ";
				data++;
			}
		}
		break;

		case 2:
		{
			uint32_t* data = reinterpret_cast<uint32_t *>(m_elements.data);
			for(int i = 0; i < m_elements.dataCount; i++)
			{
				dataOutput << (*data) << ", ";
				data++;
			}
		}
		break;

		case 3:
		{
			uint64_t* data = reinterpret_cast<uint64_t *>(m_elements.data);
			for(int i = 0; i < m_elements.dataCount; i++)
			{
				dataOutput << (*data) << ", ";
				data++;
			}
		}
		break;
	}

	cout << " Element Size: " << m_elements.dataElmSize << " = " << DataSizeString[m_elements.dataElmSize] << endl;

	cout << " Data: " << dataOutput.str() << endl << endl;
 }

/////////////////////////////////////////////////////////////////////////////////////////////////////
// InstructionFactory

// Returns a reference to the factory's baseElements so the caller can read data dirently into it.
char* InstructionFactory::GetElementsBuffer()
{
	return reinterpret_cast<char*>(&m_elements); 
}


shared_ptr<IInstruction> InstructionFactory::CreateInstruction()
{
	shared_ptr<IInstruction> inst;

	if(m_elements.opCode == 0x5eae)
	{
		inst = shared_ptr<IInstruction>(new File_Header(m_elements));
	}
	else if(m_elements.opCode == 0x13b9)
	{
		inst = shared_ptr<IInstruction>(new Save_Memory_To_File(m_elements));
	}
	else if(m_elements.opCode == 0x1143)
	{
		inst = shared_ptr<IInstruction>(new Write_Memory(m_elements));
	}
	return inst;
}


