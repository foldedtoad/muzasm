//
//  All-Directives.h
//  MUZ-Workshop
//
//  Created by Francis Pierot on 03/12/2018.
//  Copyright © 2018 Francis Pierot. All rights reserved.
//

#ifndef All_Directives_h
#define All_Directives_h

#include "Errors.h"
#include "Directive.h"
#include "CodeLine.h"
#include "Assembler.h"
#include "StrUtils.h"

namespace MUZ {


	/** Returns true if current token is a comma, and go next token. Doesn't change current token if no comma found.
	 @param codeline the parsed code line, with curtoken set to the start token for analysis
	 @return true if a comma has been found
	 */
	bool GetComma(struct CodeLine& codeline) ;
	
	/** Returns true if the tokens array has at least the resquested number of tokens available starting at curtoken.
	 @param codeline the parsed code line, with curtoken set to the start token for analysis
	 @return true if there are enough tokens left after current token
	 */
	bool EnoughTokensLeft(struct CodeLine& codeline, int number) ;
	

	// Preprocessor directives (#xxx)
	
	/** Handles #DEFINE directive. */
	class DirectiveDEFINE : public Directive {
		virtual ErrorType Parse(class Assembler& as, class Parser& parser, CodeLine& codeline, class Label* label, ErrorList& msg);
	public:
		/** Returns true if the given string qualifies for an #DEFINE directive. */
		static bool Identify( std::string source );
	};
	/** Handles #UNDEF directive. */
	class DirectiveUNDEFINE : public Directive {
		virtual ErrorType Parse(class Assembler& as, class Parser& parser, CodeLine& codeline, class Label* label, ErrorList& msg);
	};
	/** Handles #IF. */
	class DirectiveIF : public Directive {
		virtual ErrorType Parse(class Assembler& as, class Parser& parser, CodeLine& codeline, class Label* label, ErrorList& msg);
	public:
		/** Returns true if the given string qualifies for an #IF directive. */
		static bool Identify( std::string source );
	};
	/** Handles #IFDEF. */
	class DirectiveIFDEF : public Directive {
		virtual ErrorType Parse(class Assembler& as, class Parser& parser, CodeLine& codeline, class Label* label, ErrorList& msg);
	public:
		/** Returns true if the given string qualifies for an #IFDEF directive. */
		static bool Identify( std::string source );
	};
	/** Handles #IFNDEF. */
	class DirectiveIFNDEF : public Directive {
		virtual ErrorType Parse(class Assembler& as, class Parser& parser, CodeLine& codeline, class Label* label, ErrorList& msg);
	public:
		/** Returns true if the given string qualifies for an #IFNDEF directive. */
		static bool Identify( std::string source );
	};
	/** #ELSE - this is managed by Assembler when it handles the conditionnal modes. */
	class DirectiveELSE : public Directive {
	public:
		/** Returns true if the given string qualifies for an #ELSE directive. */
		static bool Identify( std::string source );
	};
	/** #ENDIF - this is managed by Assembler when it handles the conditionnal modes. */
	class DirectiveENDIF : public Directive {
	public:
		/** Returns true if the given string qualifies for an #ENDIF directive. */
		static bool Identify( std::string source );
	};
	/** #INCLUDE */
	class DirectiveINCLUDE : public Directive {
		virtual ErrorType Parse(class Assembler& as, class Parser& parser, CodeLine& codeline, class Label* label, ErrorList& msg);
	};
	/** #INSERTHEX */
	class DirectiveINSERTHEX : public Directive {
		virtual ErrorType Parse(class Assembler& as, class Parser& parser, CodeLine& codeline, class Label* label, ErrorList& msg);
	};
	/** #INSERTBIN */
	class DirectiveINSERTBIN : public Directive {
		virtual ErrorType Parse(class Assembler& as, class Parser& parser, CodeLine& codeline, class Label* label, ErrorList& msg);
	};
	/** #NOLIST */
	class DirectiveLISTOFF : public Directive {
		virtual ErrorType Parse(class Assembler& as, class Parser& parser, CodeLine& codeline, class Label* label, ErrorList& msg);
	};
	/** #LIST */
	class DirectiveLIST : public Directive {
		virtual ErrorType Parse(class Assembler& as, class Parser& parser, CodeLine& codeline, class Label* label, ErrorList& msg);
	};

	/** Small Computer Workshop 2019-09-07 and LCD alphanumeric sample compatibility */
	
	/** Handles #REQUIRES directive. */
	class DirectiveREQUIRES : public Directive {
		virtual ErrorType Parse(class Assembler& as, class Parser& parser, CodeLine& codeline, class Label* label, ErrorList& msg);
	public:
		/** Returns true if the given string qualifies for an #DEFINE directive. */
		static bool Identify(std::string source);
	};
	/** Handles #IFREQUIRED. */
	class DirectiveIFREQUIRED : public Directive {
		virtual ErrorType Parse(class Assembler& as, class Parser& parser, CodeLine& codeline, class Label* label, ErrorList& msg);
	public:
		/** Returns true if the given string qualifies for an #REQUIRES directive. */
		static bool Identify(std::string source);
	};


	// Assembler directives (.xxx)
	
	
	/** The .PROC drective only checks that its followed by Z80. */
	class DirectivePROC : public Directive {
		virtual ErrorType Parse(class Assembler& as, class Parser& parser, CodeLine& codeline, class Label* label, ErrorList& msg);
	};
	/** .CODE */
	class DirectiveCODE : public Directive {
		virtual ErrorType Parse(class Assembler& as, class Parser& parser, CodeLine& codeline, class Label* label, ErrorList& msg);
	};
	/** .DATA */
	class DirectiveDATA : public Directive {
		virtual ErrorType Parse(class Assembler& as, class Parser& parser, CodeLine& codeline, class Label* label, ErrorList& msg);
	};
	/** .ORG */
	class DirectiveORG : public Directive {
		virtual ErrorType Parse(class Assembler& as, class Parser& parser, CodeLine& codeline, class Label* label, ErrorList& msg);
	};
	/** Handles .EQU directive. */
	class DirectiveEQU : public Directive {
		virtual ErrorType Parse(class Assembler& as, class Parser& parser, CodeLine& codeline, class Label* label, ErrorList& msg);
	public:
		static bool Identify( std::string source );
	};
	/** Handles .SET directive. */
	class DirectiveSET : public Directive {
		virtual ErrorType Parse(class Assembler& as, class Parser& parser, CodeLine& codeline, class Label* label, ErrorList& msg);
	public:
		static bool Identify( std::string source );
	};
	/** .DB */
	class DirectiveBYTE : public Directive {
		virtual ErrorType Parse(class Assembler& as, class Parser& parser, CodeLine& codeline, class Label* label, ErrorList& msg);
	};
	/** .DW */
	class DirectiveWORD : public Directive {
		virtual ErrorType Parse(class Assembler& as, class Parser& parser, CodeLine& codeline, class Label* label, ErrorList& msg);
	};
	/** .DS */
	class DirectiveSPACE : public Directive {
		virtual ErrorType Parse(class Assembler& as, class Parser& parser, CodeLine& codeline, class Label* label, ErrorList& msg);
	};

	/** .HEXBYTES */
	class DirectiveHEXBYTES: public Directive {
		virtual ErrorType Parse(class Assembler& as, class Parser& parser, CodeLine& codeline, class Label* label, ErrorList& msg);
	};
	/** .END */
	class DirectiveEND : public Directive {
		virtual ErrorType Parse(class Assembler& as, class Parser& parser, CodeLine& codeline, class Label* label, ErrorList& msg);
	};
}
#endif /* All_Directives_h */
