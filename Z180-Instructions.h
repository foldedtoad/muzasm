//
//  Z180-Instructions.h
//  muzlib
//
//  Created by Francis Pierot on 14/01/2020.
//  Copyright © 2020 Francis Pierot. All rights reserved.
//

#ifndef Z180_Instructions_h
#define Z180_Instructions_h

#include "Z80-Instructions.h"

namespace MUZ {
namespace Z180 {

	//MARK: Z-80 instructions, same encoding but different timings
	class InstructionLD : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionPUSH : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionPOP : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionEXX : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionEX : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionLDI : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionLDIR : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionLDD : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionLDDR : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionCPI : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionCPIR : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionCPD : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionCPDR : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionADD : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionADC : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionSUB : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionSBC : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionAND : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionOR : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionXOR : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionCP : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionINC : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionDEC : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionDAA : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionCPL : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionNEG : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionCCF : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionSCF : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionNOP : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionHALT : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionDI : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionEI : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionIM : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionRLCA : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionRLA : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionRRCA : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionRRA : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionRLC : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionRL : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionRRC : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionRR : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionSLA : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionSLL : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionSRA : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionSRL : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionRLD : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionRRD : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionBIT : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionSET : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionRES : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionJP : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionJR : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionDJNZ : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionCALL : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionRET : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionRETI : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionRETN : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionRST : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionIN : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionINI : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionINIR : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionIND : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionINDR : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionOUT : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionOUTI : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionOTIR : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionOUTD : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};

	class InstructionOTDR : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};


	//MARK: Z-180 new instructions

	class InstructionTST : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};
	class InstructionTSTIO : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};
	class InstructionSLP : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};
	class InstructionOUT0 : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};
	class InstructionIN0 : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};
	class InstructionOTDM : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};
	class InstructionOTIM : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};
	class InstructionOTDMR : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};
	class InstructionOTIMR : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};
	class InstructionMLT : public Instruction {
	public:
		virtual bool Assemble(CodeLine& codeline, ErrorList& msg);
	};
} // namespace Z180
} // namespace MUZ
#endif /* Z180_Instructions_h */
