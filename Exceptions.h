/*
 * Exceptions.h
 *
 *  Created on: Nov 26, 2018
 *      Author: Francis Piérot
 */

#ifndef SRC_MUZ_EXCEPTIONS_H_
#define SRC_MUZ_EXCEPTIONS_H_

#include <exception>



namespace MUZ {

	// COMPUTER
	
	// A memory address has been used which does not fall in available RAM and ROM address ranges
	class MemoryRangeException : public std::exception {
		virtual const char* what() const noexcept { return "out of memory range address"; }
	};

	// A memory module has been used which has no RAM or ROM content
	class MemoryUnassignedException : public std::exception {
		virtual const char* what() const noexcept { return "unassigned memory module"; }
	};

	// Memory could not be allocated
	class OutOfMemoryException : public std::exception {
		virtual const char* what() const noexcept { return "out of memory"; }
	};

	// A file was not found
	class NoFileException: public std::exception {
		virtual const char* what() const noexcept{ return "file not found"; }
	};

	// A port address has been used which has no assigned port module
	class UnassignedPortException: public std::exception {
		virtual const char* what() const noexcept{ return "unassigned port address"; }
	};
	
	// A port address has been used which has a null assigned port module - THIS IS A BUG
	class BUGNullAssignedPortException: public std::exception {
		virtual const char* what() const noexcept{ return "BUG: NULL PORT assigned"; }
	};

	// The paging port has not been initialized with a computer - THIS IS A BUG
	class BUGNoComputerException: public std::exception {
		virtual const char* what() const noexcept{ return "BUG: no computer assigned"; }
	};
	
}; // namespace MUZ

#endif /* SRC_MUZ_EXCEPTIONS_H_ */
