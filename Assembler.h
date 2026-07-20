//
//  MUZ-Assembler.h
//  MUZ-Workshop
//
//  Created by Francis Pierot on 01/12/2018.
//  Copyright © 2018 Francis Pierot. All rights reserved.
//

#ifndef Assembler_h
#define Assembler_h

#include <stdio.h>
#include <vector>
#include <unordered_map>
#include <list>

#include "Types.h"
#include "Exceptions.h"

#include "ParsingMode.h"
#include "Errors.h"
#include "Label.h"
#include "DefSymbol.h"
#include "Directive.h"
#include "ExpVector.h"
#include "CodeLine.h"

namespace MUZ {
	
	class Assembler
	{
		// list of known processors
		std::list<std::string> knownProcessors;

	public:
		//MARK: - Public structures for API

		/** Public structure for various parts of a listing line, returned by GetListing public function. */
		struct ListingLine {

			/** Flags for parts of the listing. Each flag is set to 1 to enable the corresponding data part in the line.
			 	These flags use a bitfield structure so they take 0 or 1 values only. */
			struct ListingParts {
				int address: 1;	// display address at left most
				int warnaddress:1;	// display address with a warning (wrong value)
				int code:1;		// display bytes of code
				int defsymbol:1;// display a DEFINE symbol value instead of the address/code field
				int line:1;		// display line number
				int source:1;	// display source instruction/directive
				int comment:1;	// display comment
				int message:1;	// display message
				int file:1;		// display own file path
				int include:1;	// display include file path
			} parts = {0,0,0,0,0,0,0,0,0,0};

			/** nullptr or pointer to previous line when contiuing a block of 5+ bytes. */
			//ListingLine* prev = nullptr;

			/** Address or value part. (left most). Enabled by parts.address */
			DWORD address = 0;
			/** Up to 4 bytes of code. Enabled by parts.code */
			std::string codebytes;
			/** Reference to a #DEFINE symbol */
			std::string defsymbol;
			/** File number. Display name if parts.file is 1 */
			size_t file = 0;
			/** Included file number. Displays name if parts.include is 1 */
			int includefile = -1;
			/** Line number. Enabled by parts.line */
			size_t line = 0;
			/** Source code without comment. Enabled by parts.source */
			std::string source;
			/** Comment. Enabled by parts.comment */
			std::string comment;
			/** Warning/Error message reference. */
			int message = -1;
			/** Optional reference to a relevant token number in code line tokens */
			size_t token = 0;
		};

		// Contains a ready to use listing
		struct Listing : public std::vector<ListingLine>
		{
			// contains all lines
		};

		//MARK: - Private management structures
		/** Definition for one source file. */
		struct SourceFile
		{
			bool		included = false; 	// true for #INCLUDEd files
			size_t		parentfile = 0;		// if included, parent file reference
			size_t		parentline = 0;		// if included, line number in parent file
			SourceFile*	parent = nullptr;	// if included, parent source file
			std::string	fileprefix;			// Windows specific prefixes
			std::string	filepath;			// path part of the file
			std::string	filename;			// filename part of the file
			std::vector<CodeLine> lines;	// parsed/assembled content, matches the source file lines
			LabelMap	labels;				// local labels
			
			/** Gets the root parent of this SourceFile. */
			SourceFile* Root();
			
			/** Sets this SourceFile from the given filename and parent. Set parent to NULL to set a main source file. */
			ErrorType Set(std::string file, SourceFile* parent);
		};
		
		// The #INCLUDE directive uses Assembler::AssembleFile private function
		friend class DirectiveINCLUDE;
		// The #INSERTHEX directive uses Assembler::AssembleHexFile private function
		friend class DirectiveINSERTHEX;
		// The #INSERTBIN directive uses Assembler::AssembleBinFile private function
		friend class DirectiveINSERTBIN;

		// Map tables for the instruction set and directives
		InstructionsMap				m_instructions;
		DirectivesMap				m_directives;
		
		// Tables for the assembly

		/** Table for all the #DEFINE symbols. */
		DefSymbolsMap				m_defsymbols;
		/** Table for all the #REQUIRES symbols. */
		DefSymbolsMap				m_reqsymbols;
		/** Table for all the global labels. */
		LabelMap					labels;
		/** Table for each files, [0] is the main file, following are the included files in their inclusion order. */
		std::vector<SourceFile*>	m_files;
		/** Map of all sections. Default names are CODE and DATA for the .CODE and .DATA section. */
		std::unordered_map<std::string, Section*> m_sections;

		// Stack for imbricated #IF conditionnal modes

		/** Stack of each #IF stacked mode. The mode at level 0 is always ROOT and is never popped. */
		ParsingModeStack			m_modes;
		/** Current #IF mode level, this is the size of the m_modes stack. Should be 1 when at root. */
		size_t						m_curlevel = 0; //
		
		/** Current assembly status singleton. */
		struct AssemblyStatus {
			/** Current code or data section. */
			Section*	cursection = nullptr;
			/** Number of current assembly pass. 0 for first pass, 1 for second pass. */
			bool		firstpass  = true;
			/** Number of current file, should always be the last in the m_files array. */
			size_t		curfile		= 0;
			/** Last global label, used as a prefix for local labels starting with '@'. */
			std::string	lastlabel;
			/** Flag to activate debug trace on standard output. */
			bool		trace		= false;
			/** Flag to list all bytes of a line, false to limit to 2 lines of listing. */
			bool 		allcodelisting=false;
			/** Flag to enable listings (directives #LIST ON/OFF and #NOLIST). */
			bool		listing    = true;
			/** flag to terminate assembly (.END directive) */
			bool		finished = false;
		} m_status;
		
		//MARK: - Private Output directory and file names
		/** root output directory for all files */
		std::string					m_outputdir;
		/** file name for the binary output */
		std::string					m_binfilename;
		/** file name for the Intel HEX output */
		std::string 				m_hexfilename;
		/** file name for the listing output */
		std::string					m_listingfilename;
		/** file name for the memory output */
		std::string					m_memoryfilename;
		/** file name for the symbols output */
		std::string					m_symbolsfilename;
		/** file name for errors/warnings log */
		std::string					m_logfilename;
		/** Number of bytes in HEX output */
		ADDRESSTYPE					m_hexbytes = 0x10;

		//MARK: - Private Assembler functions
		/** Assembles a prepared code line. */
		ErrorType AssembleCodeLine(CodeLine& codeline, ErrorList& msg);
		/** Initializes listing file, closes previous if any. */
		FILE* PrepareListing(ErrorList& msg);
		/** Closes the listing file, ignore if the name is "stdout". */
		void CloseListing( FILE* & file );
		/** Generates a listing line for an assembled codeline. */
		void GenerateCodeLineListing(CodeLine& codeline, ErrorList& msg, Listing & listing);
		/** Initializes memory listing file, close previous if any. */
		void GenerateMemoryDump(DATATYPE* memory, Section& section, ErrorList& msg);
		/** Generates Intel HEX output. */
		void GenerateIntelHex(DATATYPE* memory, Section& section, ErrorList& msg);
		/** Generates in-memory file listing from current assembling. */
		void GenerateFileListing(size_t file, ErrorList& msg, Listing & listing);
		/** Generate the sections list in an opened file. */
		void GenerateSectionsList( FILE* file );
		/** Generates the #DEFINE symbols list in an opened file. */
		void GenerateDefSymbolsList( FILE* file );
		/** Generates the #REQUIRES symbols list in an opened file. */
		void GenerateReqSymbolsList( FILE* file );
		/** Generates the .EQU equate symbols list in an opened file. */
		void GenerateEquatesList( FILE* file );
		/** Generates the global labels in an opened file. */
		void GenerateLabelsList( FILE* file );
		/** List tables in an opened file. */
		void SaveTables( FILE* file );
		/** Fills a memory image and lists of sections from an assembled source file. */
		void FillFromFile(size_t file, DATATYPE* memory, Section& section, ErrorList& msg);
		/** Generates warning/error file. */
		void GenerateLog(ErrorList& msg);
		
		//MARK: - Private Sections management
		/** Gets or create a named section. */
		Section* GetSection(std::string name);
		/** Tells if a section exists. */
		bool ExistSection(std::string name);
		
		//MARK: - Private Labels management
		/** Scans a code line for a global or local label, creating it if needed. */
		Label* ScanLabel(CodeLine& codeline, ErrorList& msg);
		/** Creates a label at current address. */
		Label* CreateLabel(std::string name, CodeLine& codeline, ErrorList& msg);
		/** Sets the last global label name. */
		void SetLastLabelName(std::string name);
		/** Returns the last global label name. */
		std::string GetLastLabelName(void);
		
		//MARK: - Private Conditionnal Parsing modes
		/** Tells if current mode is root (no conditionnal running) */
		bool isRoot() ;
		/** Tells mode stack level. */
		size_t modeLevel();
		/** Tells current imbrication level. */
		size_t currentLevel();
		/** Tells the current parsing mode. */
		ParsingMode currentMode();
		/** Increment the imbrication level. */
		void IncLevel();
		/** Decrement the imbrication level. */
		void DecLevel();
		/** Enter a new conditional mode. */
		void EnterMode(ParsingMode p);
		/** Closes one conditionnal mode. The expected mode must be given. */
		bool ExitMode(ParsingMode p);
	
		//MARK: - Private ASM, HEX and binary assembling/including

		/** Assembles a main source file. */
		ErrorType AssembleMainFilePassOne(std::string file, ErrorList& msg);
		ErrorType AssembleMainFilePassTwo(std::string file, ErrorList& msg);
		/** Assembles an included source file. */
		ErrorType AssembleIncludedFilePassOne(std::string file, CodeLine& codeline, ErrorList& msg);
		ErrorType AssembleIncludedFilePassTwo(std::string file, CodeLine& codeline, ErrorList& msg);
		/** Assembles an HEX included file. */
		ErrorType AssembleHexFile(std::string file, CodeLine& codeline, ErrorList& msg);
		/** Assembles a binary included file. */
		ErrorType AssembleBinFile(std::string file, CodeLine& codeline, ErrorList& msg);

	public:
		//MARK: - PUBLIC API

		//MARK: - Constructor and destructor
		/** Initialize directives and instructions. */
		Assembler();
		/** Clean up memory. */
		virtual ~Assembler();
		
		//MARK: - Initializer and setting output files
		/** Reset the assembly. */
		void Reset();
		/** Enable/Disable all bytes listing or 2-lines listing. */
		void EnableFullListing(bool yes);
		/** Enable/Disable trace on standard output. */
		void EnableTrace(bool yes);
		/** Sets the directory where MUZ places the output files and listings. */
		void SetOutputDirectory(std::string directory);
		/** Sets the listing filename. */
		void SetListingFilename(std::string filename);
		/** Sets the binary filename. */
		void SetBinaryFilename(std::string filename);
		/** Sets the IntelHex filename. */
		void SetIntelHexFilename(std::string filename);
		/** Sets the number of bytes in hex output lines */
		void SetHexBytes(ADDRESSTYPE nbbytes);
		/** Sets the Memory listing filename. */
		void SetMemoryFilename(std::string filename);
		/** Sets the Symbols filename. */
		void SetSymbolsFilename(std::string filename);
		/** Sets the erros/warnings log filename. */
		void SetLogFilename(std::string filename);
		/** Gets the full listing from current assembling. */
		Listing GetListing(ErrorList& msg);
		/** Terminates assembly at next line */
		void Terminate();

		/** Known Processor check **/
		bool isKnownProcessor(std::string name);

		/** Save a memory listing to a text file. Use "stdout" to print on standard output. */
		void SaveListing( Listing & listing, std::string file, ErrorList& msg);
		/** Save a memory listing to an opened file. */
		ErrorType SaveListing( Listing & listing, FILE* file, ErrorList& msg);

		/** Enable/Disable the listings. */
		void EnableListing(bool yes);
		/** Known listing enabled status. */
		bool isListingEnabled();
		
		//MARK: - Sections and current address management
		/** sets current section to code .*/
		void SetCodeSection(std::string name = "");
		/** sets current section to data .*/
		void SetDataSection(std::string name = "", bool save = false);
		/** Sets the current assembling address. */
		void SetAddress(ADDRESSTYPE address);
		/** Advances the current assembling address. */
		void AdvanceAddress( ADDRESSTYPE advance );
		/** Returns the current address. */
		DWORD GetAddress();
		/** Returns the current section. */
		Section* GetSection();
		/** Finds a section from an address range. */
		Section* FindSection(DWORD a, DWORD e);

		
		//MARK: - Pass 1 / 2 setting
		/** Sets first or second pass. */
		void SetFirstPass(bool yes);
		/** Checks if running first pass. */
		bool IsFirstPass();

		//MARK: - Assembling an individual code line or a main file
		/** Assemble a single line and return a codeline */
		CodeLine AssembleLine(std::string sourceline, ErrorList& msg);
		/** Assembles a main source file. */
		ErrorType AssembleFile(std::string file, ErrorList& msg);
		/** Get the name of a file from its index. */
		std::string GetFileName(size_t index);

		//MARK: - Interface to instructions, labels, directives, symbols
		
		/** Sets the instruction and operand set. */
		void SetInstructions(std::string name);
		
		/** Tries to find a directive by name. Must include the '#' or '.' prefix. */
		Directive* GetDirective(std::string name);
		/** Try to find a n instruction in the instruction set. */
		Instruction* GetInstruction(std::string name);
		/** Try to find a named label in assembled labels. */
		Label* GetLabel(std::string name);
		/** Create a #DEFINE symbol, optionaly with a given string value. */
		DefSymbol* CreateDefSymbol(std::string name, std::string value);
		/** Delete a #DEFINE symbol. */
		bool DeleteDefSymbol(std::string name);
		/** Check if a symbol is #DEFINEd.*/
		bool ExistDefSymbol(std::string name);
		/** Try to replace a symbol from the #DEFINE table. */
		bool ReplaceDefSymbol(ParseToken& token);
		/** try to replace a symbol from the Label table */
		bool ReplaceLabel(std::string& source);
		/** Create a #REQUIRES symbol. */
		DefSymbol* CreateReqSymbol(std::string name);
		/** Delete a #REQUIRES symbol. */
		bool DeleteReqSymbol(std::string name);
		/** Check if a symbol is #REQUIREd.*/
		bool ExistReqSymbol(std::string name);

		/** Interface to files and lines. */
		CodeLine* GetCodeLine(size_t file, size_t line);

	};
}
#endif /* Assembler_h */
