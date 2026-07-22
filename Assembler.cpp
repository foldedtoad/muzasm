//
//  MUZ-Assembler.cpp
//  MUZ-Workshop
//
//  Created by Francis Pierot on 01/12/2018.
//  Copyright © 2018 Francis Pierot. All rights reserved.
//

//#include "pch.h"

#include <sys/stat.h>
#include <sys/types.h>

#include "FileUtils.h"
#include "Section.h"
#include "Parser.h"
#include "All-Directives.h"
#include "Z180-Instructions.h"
#include <list>
#include <algorithm>
#include <unistd.h>

using MUZ::BYTE;
using MUZ::ADDRESSTYPE;
using std::vector;
using std::list;
using std::string;

#ifdef _WIN32
// Windows macro conflicts with ErrorList::GetMessage()
#undef GetMessage
#endif

namespace MUZ {

	// Prototypes to avoid warnings
	string buildCodes(vector<BYTE> code, size_t firstcode, size_t nbcodes);
	Assembler::ListingLine buildOneListingLineStructure(DWORD address, vector<DATATYPE> code, size_t firstcode, size_t nbcodes, Label* label, std::string defsymbol, size_t file, size_t line, string source, int message);
	Assembler::ListingLine buildOneListingLineStructure(size_t file, size_t line, string source, ErrorList& msg);
	Assembler::Listing buildListingLineStructures(CodeLine& codeline, ErrorList& msg, bool all );

	//MARK: - Listing helper functions
	
	// Build listing line helper
	// places the various components aligned on template
	//
	// 0044: 00 00 00 00   0234              .DW  0,0            ;0x0044  Not used
	// ADDR: XX XX XX XX   NNNN  <source>
	// 012345678901234567890123456
	//
	// ADDR: at pos 0                       : 4 bytes hexa address, blank for non assembled
	// XX    at pos 6/7 9/10 12/13 15/16    : up to 4 bytes of code, can be continued on next line for long data directives (.DB etc)
	// NNNNN at pos 20-23                   : line number with 4 digits, left 0-padded
	// Source at pos 26                     : source text
	
	/** Returns the 12 characters part of byte codes. */
	string buildCodes(vector<BYTE> code, size_t firstcode, size_t nbcodes)
	{
		// 0 codes: all spaces
		if ((nbcodes == 0) || (code.size() == 0)) return spaces(12);
		// 1 to 4 codes
		string result;
		size_t lastindex = std::min<size_t>(firstcode + nbcodes - 1, code.size() - 1);
		// (0,4) -> codes 0 to 3
		// (4,4) -> codes 4 to 7
		// (0,2) -> codes 0 to 1 then "   "
		// (4,3) -> codes 4 to 6 then ".. "
		for (size_t index = (size_t)firstcode ; index <= firstcode + 3 ; index++) {
			if (index <= lastindex) {
				result += data_to_hex(code[index]) + " "; 	// normal code display
			} else if (index < code.size()) {
				result += "...";									// show code presence but not value
			} else {
				result += spaces(3);							// no more code available, all space
			}
		} // 4*3 characters added
		return result;
	}
	
	/** Builds one listing line from a code index.
	 @param address the starting address to put first
	 @param code the vector of code containing all code
	 @param firstcode the index of the first code to display
	 @param nbcodes the number of codes to display, from 0 to 4 maximum
	 @param label a label containing a value to display instead of 'address', or nullptr to ignore
	 @param file the file number of the code source line, -1 to ignore
	 @param line the line number of the code source line, -1 to ignore
	 @param source the source code to display after address, code and line number
	 @return the string containing the listing line
	 */
	Assembler::ListingLine buildOneListingLineStructure(DWORD address, vector<DATATYPE> code, size_t firstcode, size_t nbcodes, Label* label, std::string defsymbol, size_t file, size_t line, string source, int message)
	{
		Assembler::ListingLine ll;

		ll.file = file;
		ll.line = line;

		// Is there any code?
		if (code.empty()) {
			// no code: any #DEFINE symbol?
			if (!defsymbol.empty()) {
				ll.defsymbol = defsymbol;
				ll.parts.defsymbol = -1;
			}
			// no code: any label on this line?
			else if ((label != nullptr) && label->isAt(file, line)) {
				ll.address = label->AddressFrom(address);
				ll.parts.address = -1;
			}
		} else {
			ll.address = address;
			ll.parts.address = -1;
		}
		if (message >= 0) {
			ll.parts.warnaddress = -1;
		}

		string instr = source;
		string comment = "";
		/* -- align last comments on the right, commented-out for now: this breaks commented-out aligned code.
		// separate source and comment
		string instr, comment;
		size_t semicommapos = source.find(";");
		if (semicommapos != string::npos) {
			if (semicommapos == 0) {
				// first character is ';', try to find another one
				semicommapos = source.substr(1).rfind(";");
				if (semicommapos != std::string::npos) {
					// comment at first pos, then another one
					instr = source.substr(0, semicommapos+1); // until second ';'
					strtrimright(instr);
					comment = source.substr(semicommapos+1);	// from second ';'
				} else {
					// only one comment at first pos
					comment = source;		// full comment
				}
			} else {
				// one comment somewhere
				instr = source.substr(0, semicommapos);
				strtrimright(instr);
				comment = source.substr( semicommapos);
			}
		} else {
			// no comment
			instr = source;
		}
		*/

		// for no-code and for first line of code, include line number source and comment
		if (code.empty() || (firstcode == 0)) {
			ll.line = line;
			ll.parts.line = -1;
			ll.source = instr;
			ll.parts.source = instr.empty() ? 0 : -1;
			ll.comment = comment;
			ll.parts.comment = comment.empty() ? 0 : -1;
		}
		// for all lines of code, include byte codes
		if (! code.empty()) {
			ll.codebytes = buildCodes(code, firstcode, nbcodes);
			ll.parts.code = -1;
		}

		return ll;
	}

	/** Builds one listing line for a non assembled line
	 @param file the file number of the code source line, -1 to ignore
	 @param line the line number of the code source line, -1 to ignore
	 @param source the source code to display after address, code and line number
	 @return the string containing the listing line
	 */
	Assembler::ListingLine buildOneListingLineStructure(size_t file, size_t line, string source, ErrorList& )
	{
		Assembler::ListingLine ll;
		ll.file = file;
		ll.line = line;
		ll.parts.line = -1;
		ll.source = source;
		ll.parts.source = source.empty() ? 0 : 1;
		return ll;
	}

	/** Build one or more lines of listing for the codes given.
	 @param codeline the assembled code line containing code to list.
	 @param all true to list all bytes , false to limit to 2 lines of listing, with a "..." ellipsis for code not listed
	 @return a vector of strings to display/list in order
	 */
	Assembler::Listing buildListingLineStructures(CodeLine& codeline, ErrorList& , bool all )
	{
		Assembler::Listing result;
		if (!codeline.listing) return result;

		vector<DATATYPE>& code = codeline.code;
		size_t codesize = code.size();

		// first line complete with 0 to 4 bytes of code
		result.push_back(buildOneListingLineStructure(codeline.address, code, 0, std::min<size_t>(4, codesize), codeline.label, codeline.defsymbol, codeline.file, codeline.line, codeline.source, codeline.message));

		// second line depend on the number of codes
		if (codesize >= 5 && codesize <= 8) {
			// second line with 1 to 4 bytes of code
			result.push_back(buildOneListingLineStructure(codeline.address + 4, code, 4, codesize - 4, nullptr, "", 0, 0, "", -1));
		} else if (codeline.code.size() >= 9) {
			// rest of listing
			if (all) {
				// each packet of 4 code bytes
				for (DWORD start = 4 ; start < codeline.code.size() ; start += 4) {
					result.push_back(buildOneListingLineStructure(codeline.address + start, code, start, 4, nullptr, "", 0, 0, "", -1));
				}
			} else {
				// one line only with 1 to 3 bytes of code bytes 4 to 7, then "..." if there's more
				result.push_back(buildOneListingLineStructure(codeline.address + 4, code, 4, 3, nullptr, "", 0, 0, "", -1));
			}
		}
		return result;
	}

	//MARK: - Assembler::SourceFile structure
	
	/** Gets the root up the whole parent SourceFile tree. Should return the structure for the main source file. */
	Assembler::SourceFile* Assembler::SourceFile::Root()
	{
		SourceFile* root = this->parent;
		while (root && root->parent)
			root = root->parent;
		return root;
	}
	
	/** Lets a SourceFile set itself from a given file path and a parent specification. */
	ErrorType Assembler::SourceFile::Set(std::string file, SourceFile *parentsourcefile) {
		// included files may be relative to the path of their parent, check this
		included = (parentsourcefile != nullptr);
		if (included) {
			bool absolute = false;
#ifdef WIN32
			// Separate any possible prefix on Windows
			// UNC long names on Windows
			if (file.substr(0,4) == "\\\\?\\") {
				fileprefix = "\\\\?\\";
				file = file.substr(4); // cut UNC prefix
			}
			// Windows drive unit letter followed by a ':'
			if ((file[1] == ':') && ((file[0] >= 'a' && file[0] <= 'z') || (file[0] >= 'A' && file[0] <= 'Z'))) {
				fileprefix += file.substr(0,2);
				file = file.substr(2); // cut drive letter
			}
			// Windows server pathes starting with \\ are absolute and not prefixed
#else
			// UNIX-like pathes (Linux, MacOS) start with / or ~ when absolute
#endif

			// split into non-prefixed absolute or relative path and name
			splitpath(file, filepath, filename);
			parent = parentsourcefile;			// Immediate parent file (the one with a #INCLUDE/#INSERTBIN/#INSERTHEX)
			SourceFile* root = parent->Root();	// Main source file (the very first assembled file)
			if (filepath.size() == 0) {
				// no path 1: try to find in parent directory (parent relative)
				if (ExistFile(parent->filepath + NORMAL_DIR_SEPARATOR + filename)) {
					filepath = parent->filepath;
				} else {
					// no path 2: try to find file in root (main source relative)
					if (ExistFile(root->filepath + NORMAL_DIR_SEPARATOR + filename)) {
						filepath = root->filepath;
					} else {
						// not found, will generate an error
						filepath = parent->filepath;
					}
				}
			} else {
				// path not empty: is it an absolute path?
				absolute = (filepath[0] == NORMAL_DIR_SEPARATOR) || (filepath[0] == ALTERNATE_ROOTDIR);
				// if absolute, keep it this way
				if (!absolute) {
					// not absolute: try relative to parent
					if (ExistFile(parent->filepath + NORMAL_DIR_SEPARATOR + filepath + NORMAL_DIR_SEPARATOR + filename)) {
						filepath = parent->filepath + NORMAL_DIR_SEPARATOR + filepath;
					} else {
						if (ExistFile(root->filepath + NORMAL_DIR_SEPARATOR + filepath + NORMAL_DIR_SEPARATOR + filename)) {
							filepath = root->filepath + NORMAL_DIR_SEPARATOR + filepath;
						} else {
							// not found, will generate an error
							filepath = parent->filepath;
						}
					}
				}
			}
		} else {
			// main file, simply cut into parts
			splitpath(file, filepath, filename);
		}
		
		return errorTypeOK;
	}
	
	
	//MARK: - Private Assembler functions

	/** Assemble a prepared code line. The code line must have its file and source set, and the assembler will
	 fill the rest. Notice that running conditionnal directive conditions can make the line to be unassembled
	 and ignored. In this case, the "assembled" flag is not set.
	 */
	ErrorType Assembler::AssembleCodeLine(CodeLine& codeline, ErrorList& msg)
	{
		// cut the source line into a vector of tokens
		Parser parser(*this); // give reference of the assembbler to the parser
		parser.Split(codeline,msg);
		
		// Handle conditionnal assembling for IF/ELSE/ENDIF directives
		ParsingMode curmode = m_modes.top();
		if (parser.Test(hasIF)) {
			if ((curmode == parsingModeSKIPTOEND) || (curmode == parsingModeSKIPTOELSE)) {
				EnterMode(parsingModeSKIPTOEND);
				return errorTypeFALSE;
			}
			// Compute the result of the IF directive
			ErrorType result = parser.LastDirective(codeline, msg);
			if (result == errorTypeFATAL) return result; // breaks assembly
			EnterMode( (result == errorTypeOK) || (result == errorTypeTRUE) ? parsingModeDOTOELSE : parsingModeSKIPTOELSE);
			// Create label if there is one
			ScanLabel(codeline, msg);
			return errorTypeOK;
		}
		if (parser.Test(hasELSE)) {
			switch (curmode) {
					
				case parsingModeROOT:
					if (! IsFirstPass()) msg.ForceWarning(errorElseNoIf, codeline);
					return errorTypeFALSE;
				case parsingModeSKIPTOEND:
					return errorTypeFALSE; // ignore
				case parsingModeDOTOEND:
					if (! IsFirstPass()) msg.ForceWarning(errorElseNoIf, codeline);
					return errorTypeFALSE;
					
				case parsingModeSKIPTOELSE:
					ExitMode(curmode);
					EnterMode(parsingModeDOTOEND);
					break;
					
				case parsingModeDOTOELSE:
					ExitMode(curmode);
					EnterMode(parsingModeSKIPTOEND);
					ScanLabel(codeline, msg);
					break;
			}
			return errorTypeOK;
		}
		else if (parser.Test(hasENDIF)) {
			if (curmode == parsingModeROOT) {
				if (! IsFirstPass()) msg.ForceWarning(errorEndifNoIf, codeline);
				return errorTypeFALSE;
			}
			ExitMode(curmode);
			ScanLabel(codeline, msg);
			return errorTypeOK;
		}
		
		// IF/ELSE/ENDIF already returned, here we handle conditionnal skipping modes
		if ((curmode == parsingModeSKIPTOEND) || (curmode == parsingModeSKIPTOELSE)) {
			return errorTypeFALSE; // ignore everything in SKIP modes
		}
		
		// If we reach here, all conditions have been managed
		
		// Scan for a possible label
		Label* label = ScanLabel(codeline, msg);
		
		// If there is a label it is assigned the current address, on pass 1 only.
		// Ignore equates, they are set by EQU directive
		if (label && IsFirstPass() && !label->equate) {
			label->SetAddress(GetAddress());
		}
		
		// Handle the including directives (#INCLUDE, #INSERTHEX, #INSERTBIN) as well as '.' directives
		// Skip label if any
		for (codeline.curtoken = (label != nullptr) ? 1 : 0 ; codeline.curtoken < codeline.tokens.size() ; codeline.curtoken++) {
			ParseToken& token = codeline.tokens[codeline.curtoken];
			if (token.type == tokenTypeDIRECTIVE) {
				Directive* directive = GetDirective(token.source);
				if (!directive) {
					return msg.Error(errorUnknownDirective, codeline);
				}
				// Let the directive do further parsing and symbols resolving
				ErrorType result = directive->Parse(*this, parser, codeline, label, msg);
				// replace last code line label
				codeline.label = label;
				return result;

				// anything else can be an instruction
			} else if (token.type == tokenTypeLETTERS) {
				
				// USELESS? ignore if followed by a colon or a directive, next token will handle it
				if (codeline.curtoken + 1 < codeline.tokens.size()) {
					ParseToken& nexttoken = codeline.tokens[codeline.curtoken + 1];
					if (nexttoken.type == tokenTypeCOLON) continue;
					if (nexttoken.type == tokenTypeDIRECTIVE) continue;
				}
				// Should be an instruction
				// If no .PROC directive has been used, the assembler adopts the Z80 instruction set
				if (m_instructions.empty()) SetInstructions("Z80");
				Instruction* instruction = GetInstruction(token.source);
				if (instruction == nullptr) {
					return msg.Error(errorUknownInstruction, codeline);
				}
				// resolve any symbols, and prepare the token index for assembling
				/*vector<size_t> unsolved = */parser.ResolveSymbols((int)codeline.curtoken + 1, false);
				codeline.ResetInstruction( codeline.curtoken );
				codeline.as = this;
				// let the instruction do the work and/or signal errors/warnings
				instruction->Assemble(codeline, msg);
				break;
			} else {
				// assume it is something to convert, like HEXNUMBER
				parser.ResolveSymbolAt((int)codeline.curtoken, false);
			}
		}
		// replace last code line label
		codeline.label = label;
		return errorTypeOK;
	}

	/** Initializes listing file, close previous if any.
	 	Returns the standard output if "output" is the name for the listing file.
	 */
	FILE* Assembler::PrepareListing(ErrorList& msg)
	{
		// cross reference log messages to codelines
		msg.Close(*this);

		// create listing file
		if ( ! m_outputdir.empty() && ! m_listingfilename.empty()) {
			if (m_listingfilename == "stdout") return stdout;
			string filename = m_outputdir + NORMAL_DIR_SEPARATOR + m_listingfilename;
			return fopen(filename.c_str(), "w");
		}
		return nullptr;
	}

	/** Closes the listing file, ignore if the name is "stdout". */
	void Assembler::CloseListing( FILE* & file )
	{
		if (m_listingfilename != "stdout") {
			fclose(file);
			file = nullptr;
		}
	}

	/** Generate listing for an assembled code line.
	 @param codeline the assembled code line containing code to list
	 @param msg the error and warning list returned by assembler
	 */
	void Assembler::GenerateCodeLineListing(CodeLine& codeline, ErrorList& msg, Listing & listing)
	{
		if (codeline.listing) {
			// Full address/code details for assembled lines, simple line source for non assembled
			if (codeline.assembled == errorTypeOK) {
				Listing lines = buildListingLineStructures(codeline, msg, m_status.allcodelisting);
				for (auto & line: lines) listing.push_back(line);
			} else {
				listing.push_back(buildOneListingLineStructure(codeline.file, codeline.line, codeline.source, msg));
			}
		}
		// always add warning/errors
		if (codeline.message >= 0) {
			ListingLine line;
			line.file = codeline.file;
			line.parts.message = -1;
			line.message = codeline.message;
			line.token = codeline.curtoken;
			listing.push_back(line);
		}
	}

	/** Initializes memory listing file, close previous if any.
	 */
	void Assembler::GenerateMemoryDump(DATATYPE* memory, Section& section, ErrorList& mergingMsg)
	{
		FILE* memoryfile = nullptr;
		if ( ! m_outputdir.empty() && ! m_memoryfilename.empty()) {
			string filename = m_outputdir + NORMAL_DIR_SEPARATOR + m_memoryfilename;
			memoryfile = fopen(filename.c_str(), "w");
		}
		if (memoryfile == nullptr) {
			CodeLine codeline;
			mergingMsg.AboutFile(errorWritingListing, codeline, m_memoryfilename);
			return;
		}
		// output listing
		// format:
		// 0080:  01 C3 54 01  C3 54 01 C3  54 01 C3 54  01 C3 54 01   ..T..T..T..T..T.
		// partial zone with unwritten bytes:
		// 3E20:  0A 00 .. ..  .. .. .. ..  .. .. .. ..  .. .. .. ..   ................
		
		// list sections
		if (m_sections.size()) {
			std::string s = "\nSections:\n";
			fprintf(memoryfile, "%s", s.c_str());
			for (auto onesection: m_sections) {
				s = string("\t") + onesection.first + ":";
				if (onesection.second->m_ranges.size() == 0) {
					s += "<empty>";
				} else {
					// sort ranges by starting addresses
					std::sort(onesection.second->m_ranges.begin(), onesection.second->m_ranges.end(), []( AddressRange const& a, AddressRange const& b) {
						return a.start < b.start;
					});
					for (auto range: onesection.second->m_ranges) {
						s += " [" + address_to_base(range.start, 16, 4) + "-" + address_to_base(range.end, 16, 4) + "]";
					}
				}
				fprintf(memoryfile, "%s\n", s.c_str());
			}
			fprintf(memoryfile, "%s\n", "");
		}
		
		for (auto &range: section.m_ranges) {
			
			string line = "[" + address_to_base(range.start, 16, 4) + "-" + address_to_base(range.end, 16, 4) + "]:";
			Section* namedsection = FindSection(range.start, range.end);
			if (namedsection) {
				line = line + namedsection->name();
			}
			fprintf(memoryfile, "%s\n", line.c_str());
			
			ADDRESSTYPE startdumpaddress = (ADDRESSTYPE)(ADDRESSMASK & ((range.start >> 4) << 4)); // zero last 4 bits so we dump 16 bytes
			ADDRESSTYPE enddumpaddress = (ADDRESSTYPE)(ADDRESSMASK & ((range.end >> 4) << 4));
			for (ADDRESSTYPE dumpaddress = startdumpaddress ; dumpaddress <= enddumpaddress ; dumpaddress += 16) {
				// Address
				line = address_to_base(dumpaddress, 16, 4) + ":" + spaces(2);
				// 16-bytes hex dump
				for (ADDRESSTYPE address = dumpaddress ; address < dumpaddress + 16 ; address += 1) {
					if (address < range.start || address > range.end)
						line = line + "  " + " ";
					else
						line = line + data_to_hex(memory[address]) + " ";
					// 4 bytes separator
					if (address % 4 == 3)
						line = line + " ";
				}
				line = line + " ";
				// 16-bytes ASCII dump
				for (ADDRESSTYPE address = dumpaddress ; address < dumpaddress + 16 ; address += 1) {
					if (address < range.start || address > range.end) {
						line = line + " ";
					} else {
						char c = (char)(memory[address] & 0xFF);
						if (c < 32 || c > 127) c = '.';
						line = line + c;
					}
				}
				// write this line
				fprintf(memoryfile, "%s\n", line.c_str());
			} // next dumpaddress
			fprintf(memoryfile, "%s\n", "");
		} // next zone
		fclose(memoryfile);
	}

	/** Generate Intel HEX output. */
	void Assembler::GenerateIntelHex(DATATYPE* memory, Section& section, ErrorList& msg)
	{
		CodeLine codeline;
		FILE* hexfile = nullptr;
		if ( ! m_outputdir.empty() && ! m_hexfilename.empty()) {
			string filename = m_outputdir + NORMAL_DIR_SEPARATOR + m_hexfilename;
			hexfile = fopen(filename.c_str(), "w");
		}
		if (hexfile == nullptr) {
			msg.AboutFile(errorWritingListing, codeline, m_hexfilename);
			return;
		}
		// output listing
		// format:
		// :1019A0004CC101A0FF4DC101B0FF4EC10144FF4F2A
		// ':'
		// '10'   - nbbytes, generally 16 (10), but up to 255 (FF)
		// '19A0' - address
		// '00'   - Intel record type = 0
		// '4CC101A0FF4DC101B0FF4EC10144FF4F' - up to nbbytes data
		// '2A' - control byte
		string line;
		DWORD nbbytes;
		Section* asmsection;
		
		// for each address range in the parameter section
		for (auto &range: section.m_ranges) {
			
			// for each address in the range
			for (DWORD dumpaddress = range.start ; dumpaddress <= range.end ; dumpaddress += 16) {
				
				// prepare up to <m_hexbytes>
				nbbytes = m_hexbytes;
				if (dumpaddress + m_hexbytes - 1 > range.end) {
					nbbytes = range.end - dumpaddress + 1;
				}
				// get the assembled section it comes from
				asmsection = FindSection(dumpaddress, dumpaddress + nbbytes - 1);
				if (!asmsection) {
					// This can NOT happen!
					msg.Error(errorMUZNoSection, codeline);
				} else if ( ! asmsection->save() ) {
					// Comes from a non saved .data section, don't output
					continue;
				}
				// code comes from a code or saved-data section
				line = ":";
				line = line + data_to_hex((DATATYPE)(DATAMASK & nbbytes)) + address_to_base(dumpaddress, 16, 4) + "00"; // nbbytes, address, record type
				DWORD sum = nbbytes + (dumpaddress >> 8) + (dumpaddress & 0xFF); // start control sum with bytes after ':'
				for (DWORD address = dumpaddress ; address < dumpaddress + nbbytes ; address += 1) {
					line = line + data_to_hex(memory[address]);
					sum = sum + memory[address];
				}
				// compute 2's complement and keep LSB
				sum = ((~sum) + 1) & 0xFF;
				line = line + data_to_hex((DATATYPE)(DATAMASK & sum));
				// write this line
				fprintf(hexfile, "%s\n", line.c_str());
			} // next dumpaddress
		} // next zone
		// write end record
		fprintf(hexfile, "%s\n", ":00000001FF");
		fclose(hexfile);
	}

	/** Generate in-memory listing from current assembling. */
	void Assembler::GenerateFileListing(size_t file, ErrorList& msg, Listing & listing)
	{
		// get shortcut to the file to list
		if (file < 0 || file >= m_files.size()) return;
		SourceFile* sourcefile = m_files.at(file);

		// work variable
		ListingLine fileline;

		// add the file path to listing
		fileline.parts.file = -1;
		fileline.file = file;
		listing.push_back(fileline);

		// List all lines and included files
		for (auto & codeline : sourcefile->lines) {

			// record listing lines for this code line
			GenerateCodeLineListing(codeline, msg, listing);

			// For included files, do the listing with a recursive call
			if (codeline.includefile > codeline.file) {
				GenerateFileListing(codeline.includefile, msg, listing);
				// list current parent file to show that include if finished
				listing.push_back(fileline);
			}
		}
	}

	/** Generates the sections list in an opened file. */
	void Assembler::GenerateSectionsList( FILE* file )
	{
		// list sections
		if (m_sections.size()) {
			std::string s = "\nSections:\n------------------------------------------------------------------------------------\n";
			fprintf(file, "%s", s.c_str());
			if (m_status.trace) printf("%s", s.c_str());
			for (auto section: m_sections) {
				s = string("\t") + section.first + ":";
				if (section.second->m_ranges.size() == 0) {
					s += "<empty>";
				} else {
					// sort ranges by starting addresses
					std::sort(section.second->m_ranges.begin(), section.second->m_ranges.end(), []( AddressRange const& a, AddressRange const& b) {
						return a.start < b.start;
					});
					for (auto range: section.second->m_ranges) {
						s += " [" + address_to_base(range.start, 16, 4) + "-" + address_to_base(range.end, 16, 4) + "]";
					}
				}
				fprintf(file, "%s\n", s.c_str());
			}
		}
	}

	/** Generates the #DEFINE symbols list in an opened file. */
	void Assembler::GenerateDefSymbolsList( FILE* file )
	{
		// list DEFINEs
		if (m_defsymbols.size()) {
			std::string s = "\nDefines:\n------------------------------------------------------------------------------------\n";
			fprintf(file, "%s", s.c_str());
			if (m_status.trace) printf("%s", s.c_str());
			std::map<string,DefSymbol*> sortedSymbols;
			for (auto symbol: m_defsymbols) {
				sortedSymbols[symbol.first] = symbol.second;
			}
			for (auto defsymbol: sortedSymbols) {
				string sleft;
				sleft = defsymbol.first.substr(0,29);
				sleft += spaces(30 - (int)sleft.length());
				sleft += " :" + defsymbol.second->value;
				s = string("\t") + sleft + "\n";
				fprintf(file, "%s", s.c_str());
				if (m_status.trace) printf("%s", s.c_str());
			}
		}
	}

	/** Generates the #REQUIRES symbols list in an opened file. */
	void Assembler::GenerateReqSymbolsList( FILE* file )
	{
		// list DEFINEs
		if (m_reqsymbols.size()) {
			std::string s = "\nRequires:\n------------------------------------------------------------------------------------\n";
			fprintf(file, "%s", s.c_str());
			if (m_status.trace) printf("%s", s.c_str());
			std::map<string, DefSymbol*> sortedSymbols;
			for (auto symbol: m_reqsymbols) {
				sortedSymbols[symbol.first] = symbol.second;
			}
			for (auto reqsymbol: sortedSymbols) {
				string sleft;
				sleft = reqsymbol.first.substr(0,29);
				sleft += spaces(30 - (int)sleft.length());
				sleft += " :" + reqsymbol.second->value;
				s = string("\t") + sleft + "\n";
				fprintf(file, "%s", s.c_str());
				if (m_status.trace) printf("%s", s.c_str());
			}
		}
	}

	/** Generates the .EQU equate symbols list in an opened file. */
	void Assembler::GenerateEquatesList( FILE* file )
	{
		// build sorted map of equates
		std::map<string,Label*> sortedEquates;		// map, ordered by name
		for (auto label: labels) {
			if ( label.second->equate) {
				sortedEquates[label.first] = label.second;
			}
		}

		// list equates
		if (sortedEquates.size()) {
			std::string s = "\nEquates:\n------------------------------------------------------------------------------------\n";
			fprintf(file, "%s", s.c_str());
			if (m_status.trace) printf("%s", s.c_str());
			for (auto equate: sortedEquates) {
				string sleft;
				sleft = equate.first.substr(0,29);
				sleft += spaces(30 - (int)sleft.length());
				sleft += " :" + address_to_base(equate.second->addresses[0], 16, 4);
				s = string("\t") + sleft + "\n";
				fprintf(file, "%s", s.c_str());
				if (m_status.trace) printf("%s", s.c_str());
			}
		}
	}

	/** Generates the labels list in an opened file. */
	void Assembler::GenerateLabelsList( FILE* file )
	{
		// build sorted maps of labels
		std::map<string,ADDRESSTYPE> nameSortedLabels;	//map ordered by name
		std::map<ADDRESSTYPE, string> addressSortedLabels;	// map ordered by address
		for (auto label: labels) {
			if ( ! label.second->equate) {
				nameSortedLabels[label.first] = (ADDRESSTYPE)label.second->addresses[0];
				addressSortedLabels[(ADDRESSTYPE)label.second->addresses[0]] = label.first;
			}
		}
		// list global labels sorted by value then name
		if (nameSortedLabels.size() || addressSortedLabels.size()) {
			std::string s = "\nGlobal labels:\n--- By Name -------------------------------|---By Address --------------------------\n";
			fprintf(file, "%s", s.c_str());
			if (m_status.trace) printf("%s", s.c_str());
			bool nameFinished = false, addressFinished = false;
			auto iterName = nameSortedLabels.begin();
			auto iterAddress = addressSortedLabels.begin();
			if (iterName == nameSortedLabels.end()) nameFinished = true;
			if (iterAddress == addressSortedLabels.end()) addressFinished = true;
			do {
				string sleft, sright;
				if (nameFinished) {
					sleft = spaces(30) + "  " + spaces(4);
				} else {
					sleft = iterName->first.substr(0,29);
					sleft += spaces(30 - (int)sleft.length());
					sleft += " :" + address_to_base(iterName->second, 16, 4);
					iterName++;
					if (iterName == nameSortedLabels.end()) nameFinished = true;
				}
				if (!addressFinished) {
					sright = address_to_base(iterAddress->first, 16, 4) + ": ";
					sright += iterAddress->second.substr(0,30);
					iterAddress++;
					if (iterAddress == addressSortedLabels.end()) addressFinished = true;
				}
				s = string("\t") + sleft + spaces(3) + "|" + spaces(3) + sright + "\n";
				fprintf(file, "%s", s.c_str());
				if (m_status.trace) printf("%s", s.c_str());

			} while (!nameFinished || !addressFinished);
		}
	}

	/** List tables in an opened file. */
	void Assembler::SaveTables( FILE* file )
	{
		GenerateSectionsList( file );
		GenerateDefSymbolsList( file );
		GenerateReqSymbolsList( file );
		GenerateEquatesList( file );
		GenerateLabelsList( file );
	}


	/** Fill a memory image and list of sections from an assembled source file. */
	void Assembler::FillFromFile(size_t file, DATATYPE* memory, Section& section, ErrorList& msg)
	{
		SourceFile* sourcefile = m_files.at(file);
		for (auto & codeline : sourcefile->lines) {
			
			// #include lines will recursively call this function
			if (codeline.includefile > codeline.file) {
				FillFromFile(codeline.includefile, memory, section, msg);
			} else if (codeline.code.size() > 0) {
				// copy this line code in memory image
				DWORD address = codeline.address;
				for (auto c: codeline.code) {
					memory[address] = c;
					section.SetAddress(address);
					address += 1;
				}
			}
		}

		// master file?
		if (file == 0) {
			// sort ranges by starting addresses
			std::sort(section.m_ranges.begin(), section.m_ranges.end(), []( AddressRange const& a, AddressRange const& b) {
				return a.start < b.start;
			});
		}
	}

	/** Generate warning/error file. */
	void Assembler::GenerateLog(ErrorList& msg)
	{
		// cross reference log messages to codelines
		msg.Close(*this);
		// create file
		FILE* logfile = nullptr;
		if ( ! m_outputdir.empty() && ! m_logfilename.empty()) {
			string filename = m_outputdir + NORMAL_DIR_SEPARATOR + m_logfilename;
			logfile = fopen(filename.c_str(), "w");
		}
		if (logfile == nullptr) {
			return;
		}

		// dump warnings?
		string mainfile = m_files[0]->fileprefix + m_files[0]->filepath + NORMAL_DIR_SEPARATOR + m_files[0]->filename;
		fprintf(logfile, "%s\n", mainfile.c_str());
		if (m_status.trace) printf("%s\n", mainfile.c_str());

		// count warnings and errors
		int nbWarnings = 0;
		int nbErrors = 0;
		int nbFatals = 0;
		for (MUZ::ErrorMessage& m : msg) {
			if (m.type == MUZ::errorTypeWARNING) nbWarnings++;
			if (m.type == MUZ::errorTypeERROR) nbErrors++;
			if (m.type == MUZ::errorTypeFATAL) nbFatals ++;
		}

		if (nbWarnings == 0) {
			fprintf(logfile, "%s\n", "No Warnings");
			if (m_status.trace) printf("%s\n", "No Warnings");
		} else {
			fprintf(logfile, "%s\n", "WARNINGS:");
			if (m_status.trace) printf("%s\n", "WARNINGS:");
			for (MUZ::ErrorMessage& m : msg) {
				if (m.type == MUZ::errorTypeWARNING) {
					CodeLine* codeline = GetCodeLine(m.file, m.line);
					if (m.token >= 0 && m.token < codeline->tokens.size()) {
						fprintf(logfile, "\t%5d: W%04d: '%s': %s\n", (int)m.line, m.kind, codeline->tokens[m.token].source.c_str(), msg.GetMessage(m.kind).c_str());
					} else {
						fprintf(logfile, "\t%5d: W%04d: %s\n", (int)m.line, m.kind, msg.GetMessage(m.kind).c_str());
					}
					if (m_status.trace) printf("%s(%d): %s\n", GetFileName(m.file).c_str(), (int)m.line, msg.GetMessage(m.kind).c_str());
				}
			}
		}

		if (nbErrors+nbFatals == 0) {
			fprintf(logfile, "%s\n", "No Errors");
			if (m_status.trace) printf("%s\n", "No Errors");
		} else {
			fprintf(logfile, "%s\n", "ERRORS:");
			if (m_status.trace) printf("%s\n", "ERRORS:");
			for (MUZ::ErrorMessage& m : msg) {
				if (m.type != MUZ::errorTypeERROR && m.type != MUZ::errorTypeFATAL)
					continue;
				std::string prefix = "";
				if (m.type == MUZ::errorTypeFATAL) prefix = "(FATAL) ";
				CodeLine* codeline = GetCodeLine(m.file, m.line);
				if (m.token >= 0 && m.token < codeline->tokens.size()) {
					fprintf(logfile, "\t%5d: E%04d: '%s': %s\n", (int)m.line, m.kind, codeline->tokens[m.token].source.c_str(), (prefix + msg.GetMessage(m.kind)).c_str());
				} else {
					fprintf(logfile, "\t%5d: E%04d: %s\n", (int)m.line, m.kind, (prefix + msg.GetMessage(m.kind)).c_str());
				}
				if (m_status.trace) printf("%s(%d): %s\n", GetFileName(m.file).c_str(), (int)m.line, (prefix + msg.GetMessage(m.kind)).c_str());
			}
		}
		fclose(logfile);
	}

	//MARK: - Private Sections management
	
	Section* Assembler::GetSection(std::string name)
	{
		if (m_sections.count(name) == 0) {
			Section* section = new Section;
			m_sections[name] = section;
		}
		return m_sections[name];
	}
	
	/** Tells if a section exists. */
	bool Assembler::ExistSection(std::string name)
	{
		return (m_sections.count(name) != 0);
	}
	
	//MARK: - Private Labels management

	/** Creates a label if the code line starts with a label.  A Label can be followed by ':' or by a directive. */
	Label* Assembler::ScanLabel(CodeLine& codeline, ErrorList& msg)
	{
		const int nbtokens = (const int)codeline.tokens.size();
		// empty line
		if (nbtokens == 0) {
			return nullptr;
		}
		// <letters> ?
		string labelName = codeline.tokens[0].source;
		TokenType tokenType = codeline.tokens[0].type;
		if (nbtokens == 1) {
			if (tokenType == tokenTypeLETTERS) {
				// <directive> ? (ex: .CODE)
				if (GetDirective(labelName)) return nullptr;
				// <instruction> ? (ex: RET)
				if (GetInstruction(labelName)) return nullptr;
				// set as last global label name if not local and create label
				SetLastLabelName(labelName) ;
				return CreateLabel(labelName, codeline, msg);
			}
			// error, only one token and unknown case
			if (tokenType != tokenTypeCOMMENT && tokenType != tokenTypeDIRECTIVE) {
				msg.Error(errorUnknownSyntax, codeline);
			}
			return nullptr;
		}
		// <letters> <token> ... ?
		if (tokenType == tokenTypeLETTERS) {
			// <letters> <:> ... ?
			if (codeline.tokens[1].type == tokenTypeCOLON) {
				// neutralize colon for parser
				codeline.tokens[1].type = tokenTypeIGNORE;
				// store as last global label if not followed by .EQU
				if ((nbtokens <= 2) || (!DirectiveEQU::Identify(codeline.tokens[2].source) && !DirectiveSET::Identify(codeline.tokens[2].source))) {
					SetLastLabelName(labelName) ;
				}
				return CreateLabel(labelName, codeline, msg);
			}
			// <letters> <directive> ?
			if (GetDirective(codeline.tokens[1].source)) {
				// store as last global label unless it is an .EQU or local label
				if (!DirectiveEQU::Identify(codeline.tokens[2].source) && !DirectiveSET::Identify(codeline.tokens[2].source)) {
					SetLastLabelName(labelName) ;
				}
				return CreateLabel(labelName, codeline, msg);
			}
			// <letters> <instruction> ?
			if (GetInstruction(codeline.tokens[1].source)) {
				// store as last global label
				SetLastLabelName(labelName) ;
				return CreateLabel(labelName, codeline, msg);
			}
		}
		return nullptr;
	}

	/** Create a label at current address. If the label name starts with a '@', a local label is created for current file and is prefixed with the last
	 global label name, else the label is global. */
	Label* Assembler::CreateLabel(std::string name, CodeLine& codeline, ErrorList& msg)
	{
		if (name.empty()) return nullptr;
		Label* label = GetLabel(name);
		// Existing label?
		if (label) {
			// Different line?
			if (! label->isAt(codeline.file, codeline.line)) {
				// Signal a warning, only for pass 2
				msg.Warning(errorLabelExists, codeline, 2);
			}
			// Set new file and line, only on pass 1
			if (codeline.as->IsFirstPass()) {
				label->SetFileLine(codeline.file, codeline.line);
			}
			// finished
			return label;
		}
		// Non existing label, create for this file/line
		label = new Label();
		if (!label) throw OutOfMemoryException();
		if (name[0] == '@') {
			// Local label in current file
			string fullname = GetLastLabelName() + name;
			m_files[m_status.curfile]->labels[fullname] = label;
			label->multiple = true;
		} else {
			// Global label
			labels[name] = label;
			label->multiple = false;
		}
		label->SetFileLine(codeline.file, codeline.line);
		return label;
	}

	/** Sets the last global label name. Does nothing if the name is empty or starts with a '@'. */
	void Assembler::SetLastLabelName(std::string name)
	{
		if (name.length() > 0 && name[0] != '@')
			m_status.lastlabel = name;
	}
	
	/** Returns the last global label name. */
	std::string Assembler::GetLastLabelName(void)
	{
		return m_status.lastlabel;
	}
	
	//MARK: - Private Conditionnal Parsing modes
	
	/** Tells if current mode is root (no conditionnal running) */
	bool Assembler::isRoot()
	{
		return m_modes.isRoot();
	}
	
	/** Tells mode stack level. */
	size_t Assembler::modeLevel()
	{
		return m_modes.size();
	}
	
	/** Tells current imbrication level. */
	size_t Assembler::currentLevel()
	{
		return m_curlevel;
	}
	
	/** Tells the current parsing mode. */
	ParsingMode Assembler::currentMode()
	{
		return m_modes.top();
	}
	
	/** Increment the imbrication level. */
	void Assembler::IncLevel()
	{
		m_curlevel += 1;
	}
	
	/** Decrement the imbrication level. */
	void Assembler::DecLevel()
	{
		m_curlevel -= 1;
	}
	
	/** Enter a new conditional mode. */
	void Assembler::EnterMode(ParsingMode p)
	{
		m_modes.push(p);
		m_curlevel += 1;
	}
	
	/** Closes one conditionnal mode. The expected mode must be given. */
	bool Assembler::ExitMode(ParsingMode p)
	{
		if (p == m_modes.top()) {
			m_modes.pop();
			m_curlevel -= 1;
			return true;
		}
		return false;
	}
	
	//MARK: - Private ASM, HEX and binary files including
	
	/** Assembles a source file as a main file.
	 
	 The first pass is for labels generation:
	 - doesn't store the assembled codelines
	 - doesn't generate listing
	 - follows directives like .ORG or #DEFINE or #INCLUDE
	 - updates current address so labels can be created correctly
	 
	 The second pass is for code generation.
	 
	 A source file name is split into three parts:
	 - a prefix, which may include UNC "\\?\" and/or "<unit>:" on Windows, can be empty for relative pathes
	 - an absolute path starting with a / or \, can be empty for relative pathes
	 - a filename.ext part, cannot be empty
	 
	 Absolute pathes for included files are prepended with their parent file path.
	 
	 @param file the file path to the source to assemble, can be relative to parent file path if included
	 @param msg the stack of error and warnings returned by the assembler
	 
	 @return errorTypeOK if assembly was correctly done, other value if not
	 
	 @throw MUZ::OutOfMemoryException
	 */
	ErrorType Assembler::AssembleMainFilePassOne(string file, ErrorList& msg)
	{
		// basic security
		if (file.size() < 1) return errorTypeFATAL;

		// clear sections
		for (auto & section: m_sections) {
			delete section.second;
		}
		m_status.cursection = nullptr;
		
		size_t filenum = m_files.size();
		BYTE* buffer = nullptr;
		int linesize = 0;
		Label* lastLabel = nullptr;
		FILE* f = nullptr;
		// Prepare the path and name for file
		SourceFile* sourcefile = new SourceFile;
		if (sourcefile == nullptr) throw MUZ::OutOfMemoryException();
		if (errorTypeOK != sourcefile->Set(file, nullptr)) {
			goto FatalNonOpening;
		}
		
		// try to open the source file		
		file = sourcefile->fileprefix + sourcefile->filepath + NORMAL_DIR_SEPARATOR + sourcefile->filename;
		f = fopen(file.c_str(), "r");
		if (!f) {			
			goto FatalNonOpening;
		}
		
		// Store this file definition
		m_files.push_back(sourcefile);
		m_status.curfile = filenum;
		
		// now explore the file line by line (until .END directive at most)
		while (fgetline(&buffer, &linesize, f) && (! m_status.finished)) {
			
			// debug
			if (m_status.trace) printf("%04X: [%4d] %s\n", GetAddress(),(int)sourcefile->lines.size()  + 1, buffer);
			
			// prepare the codeline to assemble
			CodeLine cl;
			cl.address = GetAddress();
			cl.section = GetSection();
			cl.assembled = errorTypeFALSE;
			cl.as = this;
			cl.file = filenum;
			//cl.offset = offset;
			//cl.size = linesize;
			cl.source = string((char*)buffer);
			cl.line = sourcefile->lines.size()  + 1;
			cl.label = lastLabel;	// send previous label so a possible .EQU directive will change its value
			cl.listing = m_status.listing; // enable or disable listing
			// Assemble this line, will include another file if #INCLUDE is met
			cl.as = this;
			cl.assembled = AssembleCodeLine(cl, msg);
			if (cl.assembled == errorTypeOK) {
				cl.address = GetAddress();// useless?
				cl.section = GetSection();
				lastLabel = cl.label;
			}
			// Store assembly result
			sourcefile->lines.push_back(cl);
			// Exit if fatal error
			if (cl.assembled == errorTypeFATAL) {
				fclose(f);
				free(buffer);
				return errorTypeFATAL;
			}
			// update current address and file position
			//offset = ftell(f);
			AdvanceAddress((ADDRESSTYPE)cl.code.size());
		}
		
		// close main source and release IO buffer
		fclose(f);
		free(buffer);

		return errorTypeOK;

	FatalNonOpening:
		if (sourcefile) delete sourcefile;
		CodeLine codeline;
		return msg.Fatal(errorOpeningSource, codeline, file);
	}

	ErrorType Assembler::AssembleMainFilePassTwo(string file, ErrorList& msg)
	{
		
		// reset sections
		for (auto & section: m_sections) {
			section.second->Reset();
		}
		m_status.cursection = nullptr;
		m_status.finished = false;

		// Execute pass 2
		CodeLine codeline;
		codeline.includefile = 0;
		codeline.file = 0;
		codeline.as = this;
		return AssembleIncludedFilePassTwo(file, codeline, msg);
	}
	
	ErrorType Assembler::AssembleIncludedFilePassOne(string file, CodeLine& codeline, ErrorList& msg)
	{
		// basic security
		if (file.size() < 2) return errorTypeFATAL;
		if (codeline.file >= m_files.size()) return errorTypeFATAL;
		
		// Prepare the path and name for file
		SourceFile* sourcefile = new SourceFile;
		if (sourcefile == nullptr) throw MUZ::OutOfMemoryException();
		if (errorTypeOK != sourcefile->Set(file, (codeline.file >= 0) ? m_files[codeline.file] : nullptr)) {
			delete sourcefile;
			return msg.Fatal(errorOpeningSource, codeline, file);
		}
		
		// try to open the source file
		file = sourcefile->fileprefix + sourcefile->filepath + NORMAL_DIR_SEPARATOR + sourcefile->filename;
		FILE* f = fopen(file.c_str(), "r");
		if (!f) {
			delete sourcefile;
			return msg.Fatal(errorOpeningSource, codeline, file);
		}
		
		// Store this file definition
		size_t filenum = m_files.size();
		m_files.push_back(sourcefile);
		m_status.curfile = filenum;
		
		sourcefile->parentfile = codeline.file;
		sourcefile->parentline = codeline.line;
		sourcefile->included = (sourcefile->parentfile >= 0);
		
		// now explore the file line by line
		BYTE* buffer = nullptr;
		int linesize = 0;
			Label* lastLabel = nullptr;
		while (fgetline(&buffer, &linesize, f) && (! m_status.finished)) {
			
			// debug
			if (m_status.trace) printf("%04X: [%4d] %s\n", GetAddress(),(int)sourcefile->lines.size()  + 1, buffer);
			
			// prepare the codeline to assemble
			CodeLine cl;
			cl.address = GetAddress();
			cl.section = GetSection();
			cl.assembled = errorTypeFALSE;
			cl.file = filenum;
			cl.source = string((char*)buffer);
			cl.line = sourcefile->lines.size() + 1;
			cl.label = lastLabel;	// send previous label so a possible .EQU directive will change its value
			// Assemble this line, will include another file if #INCLUDE is met
			cl.as = this;
			cl.assembled = AssembleCodeLine(cl, msg);
			if (cl.assembled == errorTypeOK) {
				cl.address = GetAddress();// act up possible ORG
				cl.section = GetSection();
				lastLabel = cl.label;
			}
			// Store assembly result
			sourcefile->lines.push_back(cl);
			// Exit if fatal error
			if (cl.assembled == errorTypeFATAL) {
				fclose(f);
				free(buffer);
				return errorTypeFATAL;
			}
			// update current address
			AdvanceAddress((ADDRESSTYPE)cl.code.size());
		}
		
		// close main source and release IO buffer
		fclose(f);
		free(buffer);
		return errorTypeOK;
	}
	
	ErrorType Assembler::AssembleIncludedFilePassTwo(string file, CodeLine& codeline, ErrorList& msg)
	{
		// basic security
		if (file.size() < 2) return errorTypeFATAL;
		if (codeline.file >= m_files.size()) return errorTypeFATAL;
		if (codeline.includefile >= m_files.size()) return errorTypeFATAL;

		SourceFile* sourcefile = m_files.at(codeline.includefile);
		m_status.curfile = codeline.includefile;
		// now reassemble line by line
		for (CodeLine& cl : sourcefile->lines) {
			cl.code.clear();
			cl.as = this;
			cl.assembled = AssembleCodeLine(cl, msg);
			if (cl.assembled == errorTypeOK) {
				cl.address = GetAddress();
				cl.section = GetSection();
			}
			AdvanceAddress((ADDRESSTYPE)cl.code.size()) ;
		}
		return errorTypeOK;
	}
	
	/** Assembled an HEX included file.
	 @param file the file path for the HEX file to include
	 @param msg the list of message and warnings returned by the assembler
	 @return true if the file was included
	 */
	ErrorType Assembler::AssembleHexFile(std::string file, CodeLine& codeline, ErrorList& msg)
	{
		// basic security
		if (file.size() < 2) return errorTypeFATAL;
		if (codeline.file >= m_files.size()) return errorTypeFATAL;
		
		// first pass?
		if (m_status.firstpass) {
			
			// Prepare the path and name for file
			SourceFile* sourcefile = new SourceFile;
			if (sourcefile == nullptr) throw MUZ::OutOfMemoryException();
			if (errorTypeOK != sourcefile->Set(file, codeline.file >= 0 ? m_files[codeline.file] : nullptr)) {
				delete sourcefile;
				return msg.Fatal(errorOpeningSource, codeline, file);
			}
			
			// try to open the source file
			file = sourcefile->fileprefix + sourcefile->filepath + NORMAL_DIR_SEPARATOR + sourcefile->filename;
			FILE* f = fopen(file.c_str(), "r");
			if (!f) {
				delete sourcefile;
				return msg.Fatal(errorOpeningSource, codeline, file);
			}
			
			// Store this file definition
			size_t filenum = m_files.size();
			m_files.push_back(sourcefile);
			m_status.curfile = filenum;
			sourcefile->parentfile = codeline.file;
			sourcefile->parentline = codeline.line;
			sourcefile->included = (sourcefile->parentfile >= 0);
			
			// now explore the file line by line
			BYTE* buffer = nullptr;
			int linesize = 0;
			//long offset = ftell(f);
			Label* lastLabel = nullptr;
			BYTE* binbuffer = (BYTE*)calloc(1024,1);// should be suffficient, lines can only store 256 bytes definitions
			while (fgetline(&buffer, &linesize, f)) {
				
				// debug
				if (m_status.trace) printf("%04X: [%4d] %s\n", GetAddress(),(int)sourcefile->lines.size()  + 1, buffer);
				// translate the hex line into .DB source line
				MUZ::ADDRESSSIZETYPE nbbytes = hexNbBytes(buffer);
				if (nbbytes) {
					hexStore(buffer, binbuffer);
					string source="\t.DB ";
					int b = 0;
					for ( ; b < (int)nbbytes ; b++) {
						source += "$" + data_to_hex(binbuffer[b]) + ",";
					}

					// prepare the codeline to assemble
					CodeLine cl;
					cl.address = GetAddress();
					cl.section = GetSection();
					cl.assembled = errorTypeFALSE;
					cl.file = filenum;
					cl.source = source;
					cl.line = sourcefile->lines.size()  + 1;
					cl.label = lastLabel;	// send previous label so a possible .EQU directive will change its value
					// Assemble this line, will include another file if #INCLUDE is met
					cl.as = this;
					cl.assembled = AssembleCodeLine(cl, msg);
					if (cl.assembled) {
						cl.address = GetAddress();// useless?
						cl.section = GetSection();
						lastLabel = cl.label;
					}
					// Store assembly result
					sourcefile->lines.push_back(cl);
					// update current address and file position
					AdvanceAddress((ADDRESSTYPE)cl.code.size());
				}
			}
			
			// close main source and release IO buffer
			fclose(f);
			free(buffer);
			free(binbuffer);
		} else {
			return AssembleIncludedFilePassTwo(file, codeline, msg);
		}
		return errorTypeOK;
	}
	
	/** Assembled an BIN included file.
	 */
	ErrorType Assembler::AssembleBinFile(std::string file, CodeLine& codeline, ErrorList& msg)
	{
		// basic security
		if (file.size() < 2) return errorTypeFATAL;
		if (codeline.file >= m_files.size()) return errorTypeFATAL;
		
		// first pass?
		if (m_status.firstpass) {
			
			// Prepare the path and name for file
			SourceFile* sourcefile = new SourceFile;
			if (sourcefile == nullptr) throw MUZ::OutOfMemoryException();
			if (errorTypeOK != sourcefile->Set(file, codeline.file >= 0 ? m_files[codeline.file] : nullptr)) {
				delete sourcefile;
				return msg.Fatal(errorOpeningSource, codeline, file);
			}
			
			// try to open the source file
			file = sourcefile->fileprefix + sourcefile->filepath + NORMAL_DIR_SEPARATOR + sourcefile->filename;
			FILE* f = fopen(file.c_str(), "r");
			if (!f) {
				delete sourcefile;
				return msg.Fatal(errorOpeningSource, codeline, file);
			}
			
			// Store this file definition
			size_t filenum = m_files.size();
			m_files.push_back(sourcefile);
			m_status.curfile = filenum;
			sourcefile->parentfile = codeline.file;
			sourcefile->parentline = codeline.line;
			sourcefile->included = (sourcefile->parentfile >= 0);
			
			// now explore the file up to 16 bytes at a time
			BYTE* binbuffer = (BYTE*)calloc(16,1);
			int nbbytes = (int)fread(binbuffer, 1, 16, f);
			while (nbbytes > 0)   {
				
				// translate the bytes into .DB source line
				string source="\t.DB ";
				int b = 0;
				for ( ; b < nbbytes ; b++) {
					source += "$" + data_to_hex(binbuffer[b]) + ",";
				}
				
				// prepare the codeline to assemble
				CodeLine cl;
				cl.address = GetAddress();
				cl.section = GetSection();
				cl.assembled = errorTypeFALSE;
				cl.file = filenum;
				//cl.offset = ftell(f);;
				//cl.size = nbbytes;
				cl.source = source;
				cl.line = sourcefile->lines.size()  + 1;
				cl.label = nullptr;
				// Assemble this line, will include another file if #INCLUDE is met
				cl.as = this;
				cl.assembled = AssembleCodeLine(cl, msg);
				if (cl.assembled) {
					cl.address = GetAddress();// useless?
					cl.section = GetSection();
				}
				// Store assembly result
				sourcefile->lines.push_back(cl);
				// mark section and advance address to next position for code
				AdvanceAddress((ADDRESSTYPE)cl.code.size());
				// advance in binary file
				nbbytes = (int)fread(binbuffer, 1, 16, f);
			}
			
			// close main source and release IO buffer
			fclose(f);
			free(binbuffer);
		} else {
			return AssembleIncludedFilePassTwo(file, codeline, msg);
		}
		return errorTypeOK;
	}
	
	//MARK: - PUBLIC API
	
	//MARK: - Constructor and destructor
	
	Assembler::Assembler()
	{
		Reset();

		// enter known processors here
		knownProcessors.push_back("Z80");
		knownProcessors.push_back("Z180");

		// Preprocessor directives
		m_directives["DEFINE"] = new DirectiveDEFINE();
		m_directives["UNDEF"] = new DirectiveUNDEFINE();
		m_directives["IF"] = new DirectiveIF();
		m_directives["COND"] = new DirectiveIF();
		m_directives["IFDEF"] = new DirectiveIFDEF();
		m_directives["ELSE"] = new DirectiveELSE();
		m_directives["IFNDEF"] = new DirectiveIFNDEF();
		m_directives["ENDIF"] = new DirectiveENDIF();
		m_directives["ENDC"] = new DirectiveENDIF();
		m_directives["INCLUDE"] = new DirectiveINCLUDE();
		m_directives["INSERTHEX"] = new DirectiveINSERTHEX();
		m_directives["INSERTBIN"] = new DirectiveINSERTBIN();
		m_directives["NOLIST"] = new DirectiveLISTOFF();
		m_directives["LIST"] = new DirectiveLIST();
		m_directives["REQUIRES"] = new DirectiveREQUIRES();
		m_directives["IFREQUIRED"] = new DirectiveIFREQUIRED();

		// Assembler directives
		m_directives["PROC"] = new DirectivePROC();
		m_directives["ORG"] = new DirectiveORG();
		m_directives["DATA"] = new DirectiveDATA();
		m_directives["CODE"] = new DirectiveCODE();
		m_directives["END"] = new DirectiveEND();
		m_directives["EQU"] = new DirectiveEQU();
		m_directives["SET"] = new DirectiveSET();
		m_directives["BYTE"] = new DirectiveBYTE();
		m_directives["DB"] = new DirectiveBYTE();
		m_directives["DEFB"] = new DirectiveBYTE();
		m_directives["WORD"] = new DirectiveWORD();
		m_directives["DW"] = new DirectiveWORD();
		m_directives["DEFW"] = new DirectiveWORD();
		m_directives["SPACE"] = new DirectiveSPACE();
		m_directives["DS"] = new DirectiveSPACE();
		m_directives["DEFS"] = new DirectiveSPACE();
		m_directives["HEXBYTES"] = new DirectiveHEXBYTES();
	}
	
	Assembler::~Assembler()
	{
		for (auto &i : m_instructions) {
			delete i.second;
		}
		for (auto &d : m_directives) {
			delete d.second;
		}
		for (auto &d : m_defsymbols) {
			delete d.second;
		}
		for (auto &d : m_reqsymbols) {
			delete d.second;
		}
		for (auto &d : labels) {
			delete d.second;
		}
		for (auto &f : m_files) {
			delete f;
		}
		for (auto &s : m_sections) {
			delete s.second;
		}
	}
	
	/** Sets the instruction and operand set. */
	void Assembler::SetInstructions(std::string name)
	{
		if (name=="Z80") {
			m_instructions.clear();
			m_instructions["LD"] = new Z80::InstructionLD();
			m_instructions["PUSH"] = new Z80::InstructionPUSH();
			m_instructions["POP"] = new Z80::InstructionPOP();
			m_instructions["EXX"] = new Z80::InstructionEXX();
			m_instructions["EX"] = new Z80::InstructionEX();
			m_instructions["LDI"] = new Z80::InstructionLDI();
			m_instructions["LDIR"] = new Z80::InstructionLDIR();
			m_instructions["LDD"] = new Z80::InstructionLDD();
			m_instructions["LDDR"] = new Z80::InstructionLDDR();
			m_instructions["CPI"] = new Z80::InstructionCPI();
			m_instructions["CPIR"] = new Z80::InstructionCPIR();
			m_instructions["CPD"] = new Z80::InstructionCPD();
			m_instructions["CPDR"] = new Z80::InstructionCPDR();
			m_instructions["ADD"] = new Z80::InstructionADD();
			m_instructions["ADC"] = new Z80::InstructionADC();
			m_instructions["SUB"] = new Z80::InstructionSUB();
			m_instructions["SBC"] = new Z80::InstructionSBC();
			m_instructions["AND"] = new Z80::InstructionAND();
			m_instructions["OR"] = new Z80::InstructionOR();
			m_instructions["XOR"] = new Z80::InstructionXOR();
			m_instructions["CP"] = new Z80::InstructionCP();
			m_instructions["INC"] = new Z80::InstructionINC();
			m_instructions["DEC"] = new Z80::InstructionDEC();
			m_instructions["DAA"] = new Z80::InstructionDAA();
			m_instructions["CPL"] = new Z80::InstructionCPL();
			m_instructions["NEG"] = new Z80::InstructionNEG();
			m_instructions["CCF"] = new Z80::InstructionCCF();
			m_instructions["SCF"] = new Z80::InstructionSCF();
			m_instructions["NOP"] = new Z80::InstructionNOP();
			m_instructions["HALT"] = new Z80::InstructionHALT();
			m_instructions["DI"] = new Z80::InstructionDI();
			m_instructions["EI"] = new Z80::InstructionEI();
			m_instructions["IM"] = new Z80::InstructionIM();
			m_instructions["RLCA"] = new Z80::InstructionRLCA();
			m_instructions["RLA"] = new Z80::InstructionRLA();
			m_instructions["RRCA"] = new Z80::InstructionRRCA();
			m_instructions["RRA"] = new Z80::InstructionRRA();
			m_instructions["RLC"] = new Z80::InstructionRLC();
			m_instructions["RL"] = new Z80::InstructionRL();
			m_instructions["RRC"] = new Z80::InstructionRRC();
			m_instructions["RR"] = new Z80::InstructionRR();
			m_instructions["SLA"] = new Z80::InstructionSLA();
			m_instructions["SLL"] = new Z80::InstructionSLL();//undoc
			m_instructions["SRA"] = new Z80::InstructionSRA();
			m_instructions["SRL"] = new Z80::InstructionSRL();
			m_instructions["RLD"] = new Z80::InstructionRLD();
			m_instructions["RRD"] = new Z80::InstructionRRD();
			m_instructions["BIT"] = new Z80::InstructionBIT();
			m_instructions["SET"] = new Z80::InstructionSET();
			m_instructions["RES"] = new Z80::InstructionRES();
			m_instructions["JP"] = new Z80::InstructionJP();
			m_instructions["JR"] = new Z80::InstructionJR();
			m_instructions["DJNZ"] = new Z80::InstructionDJNZ();
			m_instructions["CALL"] = new Z80::InstructionCALL();
			m_instructions["RET"] = new Z80::InstructionRET();
			m_instructions["RETI"] = new Z80::InstructionRETI();
			m_instructions["RETN"] = new Z80::InstructionRETN();
			m_instructions["RST"] = new Z80::InstructionRST();
			m_instructions["IN"] = new Z80::InstructionIN();
			m_instructions["INI"] = new Z80::InstructionINI();
			m_instructions["INIR"] = new Z80::InstructionINIR();
			m_instructions["IND"] = new Z80::InstructionIND();
			m_instructions["INDR"] = new Z80::InstructionINDR();
			m_instructions["OUT"] = new Z80::InstructionOUT();
			m_instructions["OUTI"] = new Z80::InstructionOUTI();
			m_instructions["OTIR"] = new Z80::InstructionOTIR();
			m_instructions["OUTD"] = new Z80::InstructionOUTD();
			m_instructions["OTDR"] = new Z80::InstructionOTDR();
		} else if (name=="Z180") {
			// Z-80 compatible, shorter states
			m_instructions.clear();
			m_instructions["LD"] = new Z180::InstructionLD();
			m_instructions["PUSH"] = new Z180::InstructionPUSH();
			m_instructions["POP"] = new Z180::InstructionPOP();
			m_instructions["EXX"] = new Z180::InstructionEXX();
			m_instructions["EX"] = new Z180::InstructionEX();
			m_instructions["LDI"] = new Z180::InstructionLDI();
			m_instructions["LDIR"] = new Z180::InstructionLDIR();
			m_instructions["LDD"] = new Z180::InstructionLDD();
			m_instructions["LDDR"] = new Z180::InstructionLDDR();
			m_instructions["CPI"] = new Z180::InstructionCPI();
			m_instructions["CPIR"] = new Z180::InstructionCPIR();
			m_instructions["CPD"] = new Z180::InstructionCPD();
			m_instructions["CPDR"] = new Z180::InstructionCPDR();
			m_instructions["ADD"] = new Z180::InstructionADD();
			m_instructions["ADC"] = new Z180::InstructionADC();
			m_instructions["SUB"] = new Z180::InstructionSUB();
			m_instructions["SBC"] = new Z180::InstructionSBC();
			m_instructions["AND"] = new Z180::InstructionAND();
			m_instructions["OR"] = new Z180::InstructionOR();
			m_instructions["XOR"] = new Z180::InstructionXOR();
			m_instructions["CP"] = new Z180::InstructionCP();
			m_instructions["INC"] = new Z180::InstructionINC();
			m_instructions["DEC"] = new Z180::InstructionDEC();
			m_instructions["DAA"] = new Z180::InstructionDAA();
			m_instructions["CPL"] = new Z180::InstructionCPL();
			m_instructions["NEG"] = new Z180::InstructionNEG();
			m_instructions["CCF"] = new Z180::InstructionCCF();
			m_instructions["SCF"] = new Z180::InstructionSCF();
			m_instructions["NOP"] = new Z180::InstructionNOP();
			m_instructions["HALT"] = new Z180::InstructionHALT();
			m_instructions["DI"] = new Z180::InstructionDI();
			m_instructions["EI"] = new Z180::InstructionEI();
			m_instructions["IM"] = new Z180::InstructionIM();
			m_instructions["RLCA"] = new Z180::InstructionRLCA();
			m_instructions["RLA"] = new Z180::InstructionRLA();
			m_instructions["RRCA"] = new Z180::InstructionRRCA();
			m_instructions["RRA"] = new Z180::InstructionRRA();
			m_instructions["RLC"] = new Z180::InstructionRLC();
			m_instructions["RL"] = new Z180::InstructionRL();
			m_instructions["RRC"] = new Z180::InstructionRRC();
			m_instructions["RR"] = new Z180::InstructionRR();
			m_instructions["SLA"] = new Z180::InstructionSLA();
			m_instructions["SLL"] = new Z180::InstructionSLL();//undoc
			m_instructions["SRA"] = new Z180::InstructionSRA();
			m_instructions["SRL"] = new Z180::InstructionSRL();
			m_instructions["RLD"] = new Z180::InstructionRLD();
			m_instructions["RRD"] = new Z180::InstructionRRD();
			m_instructions["BIT"] = new Z180::InstructionBIT();
			m_instructions["SET"] = new Z180::InstructionSET();
			m_instructions["RES"] = new Z180::InstructionRES();
			m_instructions["JP"] = new Z180::InstructionJP();
			m_instructions["JR"] = new Z180::InstructionJR();
			m_instructions["DJNZ"] = new Z180::InstructionDJNZ();
			m_instructions["CALL"] = new Z180::InstructionCALL();
			m_instructions["RET"] = new Z180::InstructionRET();
			m_instructions["RETI"] = new Z180::InstructionRETI();
			m_instructions["RETN"] = new Z180::InstructionRETN();
			m_instructions["RST"] = new Z180::InstructionRST();
			m_instructions["IN"] = new Z180::InstructionIN();
			m_instructions["INI"] = new Z180::InstructionINI();
			m_instructions["INIR"] = new Z180::InstructionINIR();
			m_instructions["IND"] = new Z180::InstructionIND();
			m_instructions["INDR"] = new Z180::InstructionINDR();
			m_instructions["OUT"] = new Z180::InstructionOUT();
			m_instructions["OUTI"] = new Z180::InstructionOUTI();
			m_instructions["OTIR"] = new Z180::InstructionOTIR();
			m_instructions["OUTD"] = new Z180::InstructionOUTD();
			m_instructions["OTDR"] = new Z180::InstructionOTDR();
			// Z-180 specifics
			m_instructions["MLT"] = new Z180::InstructionMLT();
			m_instructions["MULT"] = new Z180::InstructionMLT();
			m_instructions["OTIM"] = new Z180::InstructionOTIM();
			m_instructions["OTIMR"] = new Z180::InstructionOTIMR();
			m_instructions["OTDM"] = new Z180::InstructionOTDM();
			m_instructions["OTDMR"] = new Z180::InstructionOTDMR();
			m_instructions["IN0"] = new Z180::InstructionIN0();
			m_instructions["OUT0"] = new Z180::InstructionOUT0();
			m_instructions["SLP"] = new Z180::InstructionSLP();
			m_instructions["TST"] = new Z180::InstructionTST();
			m_instructions["TSTIO"] = new Z180::InstructionTSTIO();

		}
	}

	//MARK: - Initializer and setting output files
	
	/** Resets the assembler. */
	void Assembler::Reset() {
		m_defsymbols.clear();
		m_reqsymbols.clear();
		labels.clear();
		m_files.clear();
		for (auto &section : m_sections) delete section.second;
		m_sections.clear();
		m_status.cursection = nullptr;
		m_status.finished = false;
		m_modes = ParsingModeStack();// resets
	}
	
	/** Enable/Disable all bytes listing or 2-lines listing. */
	void Assembler::EnableFullListing(bool yes)
	{
		m_status.allcodelisting = yes;
	}
	
	/** Enable/Disable trace on standard output. */
	void Assembler::EnableTrace(bool yes)
	{
		m_status.trace = yes;
	}

	/** Sets the directory where MUZ places the output files and listings. */
	void Assembler::SetOutputDirectory(std::string directory)
	{
		m_outputdir = directory;
		if (!ExistDir(m_outputdir)) {
			mkdir(m_outputdir.c_str(), 0777);
		}
	}
	
	/** Sets the listing filename. */
	void Assembler::SetListingFilename(std::string filename)
	{
		m_listingfilename = filename;
	}
	
	/** Sets the binary filename. */
	void Assembler::SetBinaryFilename(std::string filename)
	{
		m_binfilename = filename;
	}
	
	/** Sets the IntelHex filename. */
	void Assembler::SetIntelHexFilename(std::string filename)
	{
		m_hexfilename = filename;
	}
	
	/** Sets the number of bytes in hex output lines */
	void Assembler::SetHexBytes(ADDRESSTYPE nbbytes)
	{
		m_hexbytes = nbbytes;
	}

	/** Sets the Memory listing filename. */
	void Assembler::SetMemoryFilename(std::string filename)
	{
		m_memoryfilename = filename;
	}
	
	/** Sets the Symbols filename. */
	void Assembler::SetSymbolsFilename(std::string filename)
	{
		m_symbolsfilename = filename;
	}

	/** Sets the erros/warnings log filename. */
	void Assembler::SetLogFilename(std::string filename)
	{
		m_logfilename = filename;
	}

	/** Gets the full listing from current assembling. */
	Assembler::Listing Assembler::GetListing(ErrorList& msg)
	{
		// cross reference log messages to codelines
		msg.Close(*this);

		Listing result;
		GenerateFileListing(0, msg, result);
		return result;
	}

	/** Enable/Disable the listings. */
	void Assembler::EnableListing(bool yes)
	{
		m_status.listing = yes;
	}

	/** Known listing enabled status. */
	bool Assembler::isListingEnabled()
	{
		return m_status.listing;
	}

	/** Terminates assembly at next line */
	void Assembler::Terminate()
	{
		m_status.finished = true;
	}


	//MARK: - Sections and current address management
	
	/** Sets current section to code section.*/
	void Assembler::SetCodeSection(std::string name)
	{
		std::string fullname = "CODE" + (name.empty() ? "" : "@"+name);
		bool justcreated = !ExistSection(fullname);
		m_status.cursection = GetSection(fullname);
		if  (justcreated) {
			m_status.cursection->SetSave(true);
			m_status.cursection->SetName(fullname);
		}
	}
	
	/** Sets current section to data section. The attributes (save, name) are only set if the section is set for the first time. */
	void Assembler::SetDataSection(std::string name, bool save)
	{
		std::string fullname = "DATA" + (name.empty() ? "" : "@"+name);
		bool justcreated = !ExistSection(fullname);
		m_status.cursection = GetSection(fullname);
		if  (justcreated) {
			m_status.cursection->SetSave(save);
			m_status.cursection->SetName(fullname);
		}
	}
	
	/** Sets the current assembling address. */
	void Assembler::SetAddress(ADDRESSTYPE address)
	{
		if (m_status.cursection == nullptr) SetCodeSection();
		m_status.cursection->SetAddress( address );
	}
	
	/** Advance the current assembling address. Marks the section for addresses from current to just before the new position. */
	void Assembler::AdvanceAddress( ADDRESSTYPE advance )
	{
		if (advance) {
			// touch current address to just before current address
			DWORD start = GetAddress();
			for (size_t i = 0 ; i < advance ; i++)
				 SetAddress((ADDRESSTYPE)(ADDRESSMASK & (start + i)));
			// set new current address
			m_status.cursection->m_curaddress = start + advance;
		}
	}
	
	/** Returns the current address in current section. */
	DWORD Assembler::GetAddress()
	{
		if (m_status.cursection == nullptr) SetCodeSection();
		return m_status.cursection->curaddress();
	}


/** Returns the current section. */
	Section* Assembler::GetSection()
	{
		return m_status.cursection;
	}
	
	/** Finds a section from an address range. */
	Section* Assembler::FindSection(DWORD s, DWORD e)
	{
		for (auto& section: m_sections) {
			int range = section.second->FindRange(s, e);
			if (range >= 0)
				return section.second;
		}
		return nullptr;
	}

	//MARK: - Pass 1 / 2 setting
	
	/** Set first or second pass. */
	void Assembler::SetFirstPass(bool yes)
	{
		m_status.firstpass = yes;
	}
	/** Check if running first pass. */
	bool Assembler::IsFirstPass()
	{
		return m_status.firstpass;
	}
	
	//MARK: - Assembling an individual code line or a main file

	/** Assemble a single line into a codeline,
	 */
	CodeLine Assembler::AssembleLine(std::string sourceline, ErrorList& msg)
	{
		CodeLine codeline;
		codeline.address = 0;
		codeline.assembled = errorTypeFALSE;
		codeline.file = 0;
		//codeline.offset = 0;
		//codeline.size = 0;
		codeline.source = sourceline;
		codeline.as = this;
		codeline.assembled = AssembleCodeLine(codeline, msg);
		return codeline;
	}

	/** Assembles a main file.
	 
	 @param file the file path to the source to assemble, can be relative to parent file path if included
	 @param msg the stack of error and warnings returned by the assembler
	 
	 @return errorTypeOK if assembly was correctly done
	 */
	ErrorType Assembler::AssembleFile(string file, ErrorList& msg)
	{
		SetFirstPass(true);
		msg.Clear();							// clear warnings
		if (m_status.trace)	printf("Pass 1: %s\n", file.c_str());
		ErrorType result = errorTypeFALSE;
		try {
			result = AssembleMainFilePassOne(file, msg);
			if (result == errorTypeOK) {
				SetFirstPass(false);
				if (m_status.trace) printf("Pass 2: %s\n", file.c_str());
				result = AssembleMainFilePassTwo(file, msg);				
			}

			// output listings anyway
			Listing listing = GetListing(msg);
			SaveListing(listing, m_listingfilename, msg);

			if (result != errorTypeFATAL) {
				// first build memory image and a section with all the written address ranges
				DATATYPE* memory = (DATATYPE*)calloc(MEMMAXSIZE, 1);
				Section section;
				ErrorList mergingMsg;
				FillFromFile(0, memory, section, mergingMsg); // this handles recursive calls for included files

				// dump in memory listing
				GenerateMemoryDump(memory, section, mergingMsg);

				// Output Intel Hex format
				GenerateIntelHex(memory, section, mergingMsg);

				// clean memory work image
				free(memory);
			}

		} catch (std::exception& e) {
			perror(e.what());
		};

		// Output errors and warnings
		GenerateLog(msg);
		return result;
	}

	/** Get the name of a file from its index. */
	std::string Assembler::GetFileName(size_t index)
	{
		if (index < 0 || index >= m_files.size()) return "";
		return m_files[index]->filepath + "/" + m_files[index]->filename;
	}
	
	//MARK: - Interface to instructions, labels, directives, symbols
	
	/** Try to find a directive in the # and . directives array. */
	Directive* Assembler::GetDirective(std::string name)
	{
		name = std::to_upper(name);
		if (m_directives.count(name))
			return m_directives[name];
		return nullptr;
	}
	
	/** Try to find an instruction in the instruction set.  */
	Instruction* Assembler::GetInstruction(std::string name)
	{
		name = std::to_upper(name);
		if (m_instructions.count(name))
			return m_instructions[name];
		return nullptr;
	}
	
	/** Try to find a named label in local or global labels. */
	Label* Assembler::GetLabel(std::string name)
	{
		if (name.empty()) return nullptr;
		if (name[0] == '@') {
			string fullname = GetLastLabelName() + name;
			if (m_files[m_status.curfile]->labels.count(fullname)) {
				return m_files[m_status.curfile]->labels[fullname];
			}
		} else {
			if (labels.count(name)) {
				return labels[name];
			}
		}
		return nullptr;
	}
	
	/** Create a #DEFINE symbol, optionaly with a given string value.
	 	Sending an empty value will define the symbol as a virtually true value in a DefSymbol,
	 	while non empty values are stored as strings in a DefSymbol.
	 */
	DefSymbol* Assembler::CreateDefSymbol(std::string name, std::string value)
	{
		// Check if the name exists
		DefSymbol* defsymbol = nullptr;
		if (m_defsymbols.count(name)) {
			defsymbol = m_defsymbols[name];
		}
		if (!defsymbol) defsymbol = new DefSymbol();
		if (!defsymbol) throw OutOfMemoryException();
		// check if the value is empty and if so, define the symbol as an empty but true defsymbol
		if (value.empty()) {
			defsymbol->value.clear();
			defsymbol->singledefine = true;
		} else {
			defsymbol->value = value;
			defsymbol->singledefine = false;
		}
		// do the assignment
		m_defsymbols[name] = defsymbol;
		return defsymbol;
	}
	
	/** Delete a #DEFINE symbol. */
	bool Assembler::DeleteDefSymbol(std::string name)
	{
		if (m_defsymbols.count(name)) {
			m_defsymbols.erase(name);
			return true;
		}
		return false;
	}
	
	/** Check if a symbol is #DEFINEd.*/
	bool Assembler::ExistDefSymbol(std::string name)
	{
		if (m_defsymbols.count(name))
			return true;
		return false;
	}
	
	/** Create a #REQUIRES symbol
	 */
	DefSymbol* Assembler::CreateReqSymbol(std::string name)
	{
		// Check if the name exists
		DefSymbol* reqsymbol = nullptr;
		if (m_reqsymbols.count(name)) {
			reqsymbol = m_reqsymbols[name];
		}
		if (!reqsymbol) reqsymbol = new DefSymbol();
		if (!reqsymbol) throw OutOfMemoryException();
		reqsymbol->value.clear();
		reqsymbol->singledefine = true;
		// do the assignment
		m_reqsymbols[name] = reqsymbol;
		return reqsymbol;
	}
	
	/** Delete a #REQUIRES symbol. */
	bool Assembler::DeleteReqSymbol(std::string name)
	{
		if (m_reqsymbols.count(name)) {
			m_reqsymbols.erase(name);
			return true;
		}
		return false;
	}
	
	/** Check if a symbol is #REQUIREd.*/
	bool Assembler::ExistReqSymbol(std::string name)
	{
		if (m_reqsymbols.count(name))
			return true;
		return false;
	}


	/** Try to replace a symbol from the #DEFINE table.
	 Symbols associated to nothing are considered as a boolean true, while the other symbols are replaced by their string value.
	 @param token [IN/OUT] the token to check, if its value exists as a symbol it will be replaced by the symbol value if it has a value or by a boolean set to true if the symbol is empty
	 @return true if the symbol has been found and the token value replaced, false is not
	 */
	bool Assembler::ReplaceDefSymbol(ParseToken& token)
	{
		if (m_defsymbols.count(token.source)) {
			auto defsymbol = m_defsymbols[token.source];
			if (defsymbol) {
				if (defsymbol->singledefine) {
					token.source = "t";
					token.type = tokenTypeBOOL;
				} else {
					token.source = defsymbol->value;
					token.type = tokenTypeSTRING;
				}
			}
			return true;
		}
		return false; // not found
	}
	

	/** Try to replace a symbol from the Label table. Returns closest address if this is a local label with more than one address. Returns false if the label
	 doesn't exist or has no address yet. */
	bool Assembler::ReplaceLabel(std::string& source)
	{
		// local label?
		if (source.empty()) return false;
		Label* label = GetLabel(source);
		if (label && !label->empty()) {
			source = std::to_string(label->AddressFrom(GetAddress())); // std::to_string() extension at the top of this file
			return true;
		}
		return false;
	}
	
	/** Interface to files and lines. */
	CodeLine* Assembler::GetCodeLine(size_t file, size_t line)
	{
		// files go from 0 to N-1
		if (file < 0 || file >= m_files.size()) return nullptr;
		SourceFile* sourcefile = m_files.at(file);
		// lines go from 1 to N
		if (line < 1 || line > sourcefile->lines.size()) return nullptr;
		return &sourcefile->lines.at(line - 1);
	}

	/** Known Processor check **/
	bool Assembler::isKnownProcessor(std::string name)
	{
		if (std::find(knownProcessors.begin(), knownProcessors.end(), name)==knownProcessors.end()) {
			return false;
		}
		return true;
	}

	//MARK: - Listing in memory

	/** Save a memory listing to a text file. Use "stdout" to print on standard output. */
	void Assembler::SaveListing( Listing & listing, std::string filename, ErrorList& msg)
	{
		m_listingfilename = filename;
		FILE* file = PrepareListing(msg);
		if (file) {
			if (SaveListing(listing,file,msg) == errorTypeOK) {
				SaveTables(file);
			} else {

				perror("fopen failed? ");
			}
			CloseListing(file);
		}
	}

	/** Save a memory listing to a text file. Use "stdout" to just print on standard output. */
	ErrorType Assembler::SaveListing( Listing & listing, FILE* file, ErrorList& msg )
	{
		FILE* output = (file == nullptr ? stdout : file);
		int leftpartsize = 22;

		// when this flag is set, finish listing by writing only the line.parts.file lines
		ErrorType error = errorTypeOK;
		for (auto & line : listing) {

			SourceFile* sourcefile = m_files[line.file];
			string s;
			// display a file path?
			if (line.parts.file ) {
				string mainfile = sourcefile->fileprefix + sourcefile->filepath + NORMAL_DIR_SEPARATOR + sourcefile->filename;
				// condense if following a fatal error
				if (error == errorTypeFATAL) {
					s = spaces(leftpartsize) + mainfile + "\n";
				} else {
					s ="\n" + spaces(leftpartsize) + mainfile + "\n\n";
				}
				fprintf(output, "%s", s.c_str());
			} else if ((error != errorTypeFATAL) && line.parts.message) {
				// display a warning or error
				ErrorMessage & m =  msg.at((size_t)line.message);
				CodeLine& codeline = sourcefile->lines.at(m.line - 1);
				string prefix = spaces(leftpartsize);
				if (m.type == MUZ::errorTypeWARNING) {
					prefix += "      Warning W";
				} else if (m.type == MUZ::errorTypeERROR) {
					prefix += "      Error E";
				} else if (m.type == MUZ::errorTypeFATAL) {
					prefix += "      FATAL F";
				}
				if (m.token >= 0 && m.token < codeline.tokens.size()) {
					fprintf(output, "%s%04d: '%s': %s\n", prefix.c_str(), m.kind, codeline.tokens[m.token].source.c_str(), msg.GetMessage(m.kind).c_str());
				} else {
					fprintf(output, "%s%04d: %s\n", prefix.c_str(), m.kind, msg.GetMessage(m.kind).c_str());
				}
				// carry the error type
				error = m.type;
			} else if (error != errorTypeFATAL) {
				if ( ! line.defsymbol.empty()) {
					if (ExistDefSymbol(line.defsymbol)) {
						DefSymbol* defsymbol = m_defsymbols[line.defsymbol];
						if (defsymbol->singledefine) {
							s = spaces(leftpartsize-2) + "* ";
						} else {
							s = string("*") + defsymbol->value.substr(0,(size_t)leftpartsize-3);
							if (s.length() < (size_t)leftpartsize-2) s += spaces(leftpartsize-2 - (int)s.length());
							s += "* ";
						}
					} else if (ExistReqSymbol(line.defsymbol)) {
						s = spaces(leftpartsize-2) + "* ";
					}
				} else {
					if (line.parts.address) {
						int digits = 4;
						if (line.address > 0xFFFF) {
							digits = 8;
						}
						s = spaces(8-digits) + address_to_base(line.address, 16, digits);

						if (line.parts.warnaddress) {
							s += "? ";
						} else {
							s += ": ";
						}
					} else {
						if (line.parts.warnaddress) {
							s += "??" + spaces(8);
						} else {
							s = spaces(10);
						}
					}
					if (line.parts.code) {
						s += line.codebytes;
					}
					s += spaces(leftpartsize - (int)s.length());
				}
				if (line.parts.line) {
					string linenum = std::to_string(line.line);
					while (linenum.length() < 4) {
						linenum = "0" + linenum;
					}
					s += linenum + spaces(2);
				} else {
					s += spaces(6);
				}
				if (line.parts.source) {
					s += line.source;
				}
				if (line.parts.comment) {
					s += line.comment;
				}
				fprintf(output, "%s\n", s.c_str());
			}

		}
		return error;

	}



} // namespace MUZ

