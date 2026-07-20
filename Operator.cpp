//
//  Operator.cpp
//  MUZ-Workshop
//
//  Created by Francis Pierot on 12/12/2018.
//  Copyright © 2018 Francis Pierot. All rights reserved.
//
//#include "pch.h"
#include "Operator.h"

namespace MUZ {
	
	Operator &nop = *new Operator;
	
	/** Execute the operation with the given arguments and return result. Argument type must have been checked first. */
	ParseToken Operator::Exec(ParseToken& , ParseToken& )
	{
		return nop;
	}
}
