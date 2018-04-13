//////////////////////////////////////////////////////////////////////
// Instruction.h : Defines the interface:   IInstruction.
//							   class:	    BaseInstruction
//							   class:		File_Header
//							   class:		File_Header
//							   class:		Save_Memory_To_File
//							   class:		Write_Memory
//							   class:		InstructionFactory
//
//  Usually and especially in production code, I would define each class in a seperate header file
//  For this small program, the interface IInstruction is not really necessary.  The client/main app 
//		could just get and use a pointer to the BaseInstruction.  I created and use IInstruction to 
//		demonstrate my understanding.
//  STL is used throughout 
//  C++ 11 std::shared_ptr is used - could have used boost::shared_ptr instead.
//	I refer to the fields as elements and DWord blocks as segments 
////////////////////////////////////////////////////////////////////////
#pragma once

#include <stdint.h>
#include <memory>
#include <string>


const int32_t SEGMENT_SIZE = 4;

typedef struct
{	
	uint16_t dwCount;
	uint16_t opCode;
} baseElements;


// IInstruction - Instruction Interface
struct IInstruction
{
	virtual void Init(char * inElements) = 0;
	virtual uint32_t GetElementsSize() = 0;
	virtual void Execute() = 0;
	virtual void Print() = 0;
};

// BaseInstruction
// Derives from IInstruction
// All the specific instructions derive from this class
// The consumer/main app could use this class!
class BaseInstruction : public IInstruction
{
public:
	BaseInstruction(baseElements baseElm, int32_t segCount);
	virtual ~BaseInstruction(){};

	virtual char* GetElementsBuffer();

	// Implement IInstruction 
	virtual void Init(char* buffer){}; // Not Emplemented in base
	virtual uint32_t GetElementsSize();
	virtual void Execute(){} // Not Emplemented
	virtual void Print();

protected:
	baseElements m_baseElements;

	// number of fixed segments (DWords)
	const int32_t m_fixedSegmentCount;  
};

class File_Header : public BaseInstruction
{
public:
	File_Header(baseElements baseElm);
	~File_Header(){};

	// Implement IInstruction 
	void Init(char * buffer);
	void Print();
	
private:
	struct elements
	{
		uint32_t year:12, day:5, month:4, reserved:3, stepping:4, product:4; 
	} m_elements;

};


class Save_Memory_To_File : public BaseInstruction
{
public:

	Save_Memory_To_File(baseElements baseElm);
	~Save_Memory_To_File(){}

	// Implement IInstruction 
	void Init(char* buffer);
	void Print();

private:
	struct 
	{
		uint32_t memHandle;
		std::string fileName;

	} m_elements;

};


class Write_Memory : public BaseInstruction
{
public: 
	Write_Memory(baseElements baseElm);
	~Write_Memory();

	// Implement IInstruction 
	void Init(char * buffer);
	void Print();

private:
	struct elements
	{
		uint32_t	memAddress;
		union   // Not the best, but allows us to refer to the segment by either a single uint32_t(attributes) or the individual elements.
		{
			struct
			{
				uint32_t	dataElmSize:2, dataCount:30;
			};
			uint32_t	attributes;
		};
		uint32_t*	data;
	} m_elements;

	uint32_t* testptr;
};

// InstructionFactory
// This context class knows the structure of the baseElements and which
// instruction to instantiate.  Based on the OpCode CreateInstruction() 
// instantiates a specific instuction as a shard_ptr of IInstructon. 
class InstructionFactory
{
public:
	InstructionFactory(){};
	~InstructionFactory(){};

	char* GetElementsBuffer();
	std::shared_ptr<IInstruction> CreateInstruction();
	
	baseElements m_elements;
};