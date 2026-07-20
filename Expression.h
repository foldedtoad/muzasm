//
//  Expression.h
//  MUZ-Workshop
//
//  Created by Francis Pierot on 11/12/2018.
//  Copyright © 2018 Francis Pierot. All rights reserved.
//

#ifndef Expression_h
#define Expression_h

#include <vector>
#include <map>

#include "TokenType.h"
#include "ExpVector.h"
#include "All-Operators.h"


namespace MUZ {


	/** Class for the expression evaluator. Automatic type conversion can be set in the evaluator: it will replaced a type by another in all internal sub expressions before evaluation by operators. */
	class ExpressionEvaluator
	{
		/** Stack for arguments and operators. */
		ExpVector stack;
		
		/** Specific type conversions. */
		std::map<TokenType, TokenType>typeConvert;
		
		/** Default type conversion. UNKNOWN means no type conversion. */
		TokenType defaultTypeConversion = tokenTypeUNKNOWN;
		
		/** Evaluates a non-parenthesed expression starting at a given token index. Updates the end index depending on the tokens erased and replaced by intermediate results. Tokens before the starting index are untouched.
		 @throw EXPRESSIONLeftOperandMissing
		 */
		ParseToken EvaluateExpression(ExpVector& tokens, int start, int& end) noexcept(false);

		/** Reduces all parentheses sub-expressions to their result. Updates the end index depending on the tokens erased and replaced by intermediate results. Tokens before the starting index are untouched.
		 @throw EXPRESSIONLeftOperandMissing
		 */
		ParseToken ReduceParenthesis(ExpVector& tokens, int start, int& end) noexcept(false);

	public:
		ExpressionEvaluator();

		/** Sets a default type conversion. Use tokenTypeUKNOWN to disable. */
		void SetDefaultConversion(TokenType dest);
		
		/** Sets a type conversion. Use tokenTypeUKNOWN for destination to disable a previous type conversion. */
		void SetConversion(TokenType src, TokenType dest);
		
		/** Clear type conversions. */
		void ClearConversions();
		
		/** Check parenthesis levels are paired.
		 	@return < 0 too much ')', 0 OK, > 0 too much '('
		 */
		int CheckParenthesis(ExpVector& tokens);
		
		/** Evaluates a sub expression starting at a given token until end of tokens or invalid token type.
		 Returns a result, and updates the last token used
		 @throw EXPRESSIONLeftOperandMissing
		 */
		ParseToken Evaluate(ExpVector& tokens, int start, int& end) noexcept(false);
	};
	
} // namespace
#endif /* Expression_hpp */
