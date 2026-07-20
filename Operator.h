//
//  Operator.h
//  MUZ-Workshop
//
//  Created by Francis Pierot on 12/12/2018.
//  Copyright © 2018 Francis Pierot. All rights reserved.
//

#ifndef Operator_h
#define Operator_h

#include "ParseToken.h"

namespace MUZ {
	
	/** This structure has an Exec() function which let a token execute its operations on 1 or 2 other tokens, returning a token as a result. */
	struct Operator : public ParseToken
	{
		virtual ~Operator() {}
		/** Execute the operation with the given arguments and return result. */
		virtual ParseToken Exec(ParseToken& arg1, ParseToken& arg2);
	};
	
	/** global, neutral NO-OPERATION operator. */
	extern Operator& nop;

}

#endif /* Operator_h */
