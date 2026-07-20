//
//  Section.cpp
//  MUZ-Workshop
//
//  Created by Francis Pierot on 11/01/2019.
//  Copyright © 2019 Francis Pierot. All rights reserved.
//
//#include "pch.h"
#include "Section.h"


namespace MUZ {

	/** Tests if an address merges to the left of a range.*/
	bool Section::mergesLeft(DWORD address, size_t rangeindex) {
		return (address + 1 == m_ranges[rangeindex].start);
	}
	
	/** Tests if an address merges to the right of a range.*/
	bool Section::mergesRight( DWORD address, size_t rangeindex) {
		return (m_ranges[rangeindex].end + 1 == address);
	}
	
	/** Tests if an address is inside a range. */
	bool Section::isInside(DWORD address, size_t rangeindex) {
		return ((m_ranges[rangeindex].start <= address) && (address <= m_ranges[rangeindex].end));
	}
	
	/** Tests if this section contains an address range in its ranges and returns its index, returns -1 if not. */
	int Section::FindRange(DWORD s, DWORD e) {
		for (size_t i = 0 ; i < m_ranges.size() ; i++) {
			if (m_ranges[i].start <= s && e <= m_ranges[i].end) return (int)i;
		}
		return -1;
	}
	
	/** Enters an address into an xxisting or new address range. Merges two existing ranges when address links them. */
	void Section::SetAddress(DWORD address) {
		
		// All pathes set the current address
		m_curaddress = address;
		
		// try all ranges
		for (size_t i  = 0 ; i < m_ranges.size() ; i++) {
			
			// inside this range?
			if (isInside(address, i)) {
				m_currange = i;
				return;
			}
			// merges to the left of [i]?
			if (mergesLeft(address, i)) {
				// check if it links with the end of another range
				for (size_t j = 0 ; j < m_ranges.size() ; j++) {
					if (i == j) continue;// skip self test
					if (mergesRight(address, j)) {
						// [j].end - a - [i].start
						if (i < j) {
							// merge in range[i] and delete [j]
							m_ranges[i].start = m_ranges[j].end;
							m_ranges.erase(m_ranges.begin() + (int)j);
							m_currange = i;
							return;
						}
						// merge in range[j] and delete [i]
						m_ranges[j].end = m_ranges[i].end;
						m_ranges.erase(m_ranges.begin() + (int)i);
						m_currange = j;
						return;
					}
				}
				// No link between two ranges: simply expand to the left of range[i]
				m_ranges[i].start = address;
				m_currange = i;
				return;
			}
			// merges to the right of [i]?
			if (mergesRight(address, i)) {
				// check if it links with the start of another range
				for (size_t j = 0 ; j < m_ranges.size() ; j++) {
					if (i == j) continue;// skip self test
					if (mergesLeft(address, j)) {
						// [i].end - a - [j].start
						if (i < j) {
							// merge in range[i] and delete [j]
							m_ranges[i].end = m_ranges[j].end;
							m_ranges.erase(m_ranges.begin() + (int)j);
							m_currange = i;
							return;
						}
						// merge in range[j] and delete [i]
						m_ranges[j].start = m_ranges[i].start;
						m_ranges.erase(m_ranges.begin() + (int)i);
						m_currange = j;
						return;
					}
				}
				// No link between two ranges: simply expand to the right of range[i]
				m_ranges[i].end = address;
				m_currange = i;
				return;
			} // merge right(i)
		} // for all ranges (i)
		
		// no range found, create a new one for this address
		AddressRange range;
		range.start = address;
		range.end = address;
		m_ranges.push_back(range);
		m_currange = m_ranges.size() - 1;
	}
	
	/** Returns the lowest starting address of all ranges. */
	DWORD Section::absoluteStart() {
		if (m_ranges.size()==0) return 0;
		DWORD start = ADDRESSMASK;
		for (auto &range : m_ranges) {
			if (range.start < start) start = range.start;
		}
		return start;
	}
	
	/** Returns the highest ending address of all ranges. */
	DWORD Section::absoluteEnd() {
		DWORD end = 0;
		for (auto &range : m_ranges) {
			if (range.end > end) end = range.end;
		}
		return end;
	}
	
	/** Returns starting address for current range. */
	DWORD Section::start() const {
		if (m_ranges.size()==0) return 0;
		return m_ranges[m_currange].start;
	}
	
	/** Returns ending address for current range. */
	DWORD Section::end() const {
		if (m_ranges.size()==0) return 0;
		return m_ranges[m_currange].end;
	}
	
	/** Returns current address for current range. */
	DWORD Section::curaddress() const {
		return m_curaddress;
	}

	/** Returns this section name. */
	std::string Section::name() const {
		return m_name;
	}
	
	/** Returs true if the section must be saved in HEX output. */
	bool Section::save() const {
		return m_saved;
	}
	
	/** Clear status. */
	void Section::Reset() {
		m_currange = INT_MAX;
		m_curaddress = ADDRESSMASK;
		for (size_t i = 0 ; i < m_ranges.size() ; i++) {
			if (m_ranges[i].start < m_curaddress) {
				m_currange = i;
				m_curaddress = m_ranges[i].start;
			}
		}
	}
	
	/** Sets the name, can only be done once. */
	void Section::SetName(std::string name) {
		if (m_name.empty()) {
			m_name = name;
		}
	}
	
	/** Sets the current address. */
	void Section::SetOrg(DWORD address) {
		m_curaddress = address; // do NOT touch m_ranges
	}
	
	/** Sets the SAVE attribute to enable/disable the HEX output. */
	void Section::SetSave(bool yes)
	{
		m_saved = yes;
	}
};
