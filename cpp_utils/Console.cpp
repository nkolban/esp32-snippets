/*
 * Console.cpp
 *
 *  Created on: Jun 15, 2018
 *      Author: kolban
 */

/**
 * Example:
 * Argtable argtable;
 * argtable.addString("myparam", "l", "list", "Get Listings");
 * argtable.parse(argc, argv);
 * ArgTableEntry_String* pStr = (ArgTableEntry_String*)argTable.get("myparam");
 * pStr->getValue();
 */
#include "Console.h"

/**
 * Argtable instance constructor.
 */
ArgTable::ArgTable() {
	m_argtable = nullptr;
	m_argEnd   = nullptr;
} // ArgTable#ArgTable


/**
 * Argtable instance destructor.
 */
ArgTable::~ArgTable() {
	freeArgtable(); // Release any resources associated with the argtable.
} // ArgTable#~ArgTable


/**
 * Build the ArgTable that will be used for parsing.
 */
/* private */ void ArgTable::build() {
	/*
	 * The m_argTableEntries is a std::list that contains the argtable entries in the form of a std::pair.  We
	 * allocate storage for the ArgTable and then populate it.  The last entry of the argtable must be an end marker.
	 */
	int size = m_argTableEntries.size();
	m_argtable = new void*[size + 1];
	int i = 0;
	for (auto it = m_argTableEntries.begin(); it != m_argTableEntries.end(); ++it) {
		m_argtable[i] = it->second->getEntry();
		i++;
	}
	m_argEnd = arg_end(10);
	m_argtable[i] = m_argEnd;
} // ArgTable#build


ArgTableEntry_Date ArgTable::addDate(std::string name, std::string shortopts, std::string longopts, std::string glossary) {
	ArgTableEntry_Date* pDate = new ArgTableEntry_Date(shortopts, longopts, glossary);
	m_argTableEntries.push_back(std::make_pair(name, pDate));
	return *pDate;
} // ArgTable#addDate


ArgTableEntry_Double ArgTable::addDouble(std::string name, std::string shortopts, std::string longopts, std::string glossary) {
	ArgTableEntry_Double* pDouble = new ArgTableEntry_Double(shortopts, longopts, glossary);
	m_argTableEntries.push_back(std::make_pair(name, pDouble));
	return *pDouble;
} // ArgTable#addDouble


ArgTableEntry_File ArgTable::addFile(std::string name, std::string shortopts, std::string longopts, std::string glossary) {
	ArgTableEntry_File* pFile = new ArgTableEntry_File(shortopts, longopts, glossary);
	m_argTableEntries.push_back(std::make_pair(name, pFile));
	return *pFile;
} // ArgTable#addFile


ArgTableEntry_Int ArgTable::addInt(std::string name, std::string shortopts, std::string longopts, std::string glossary) {
	ArgTableEntry_Int* pInt = new ArgTableEntry_Int(shortopts, longopts, glossary);
	m_argTableEntries.push_back(std::make_pair(name, pInt));
	return *pInt;
} // ArgTable#addInt


ArgTableEntry_Lit ArgTable::addLit(std::string name, std::string shortopts, std::string longopts, std::string glossary) {
	ArgTableEntry_Lit* pLit = new ArgTableEntry_Lit(shortopts, longopts, glossary);
	m_argTableEntries.push_back(std::make_pair(name, pLit));
	return *pLit;
} // ArgTable#addLit


ArgTableEntry_String ArgTable::addString(std::string name, std::string shortopts, std::string longopts, std::string glossary, int min, int max) {
	ArgTableEntry_String* pStr = new ArgTableEntry_String(shortopts, longopts, glossary, min, max);
	m_argTableEntries.push_back(std::make_pair(name, pStr));
	return *pStr;
} // ArgTable#addString


/**
 * Parse the input and output parameters against this argtable.
 * @param argc A count of the number of parameters.
 * @param argv An array of string parameters.
 * @return The number of errors detected.
 */
int ArgTable::parse(int argc, char* argv[]) {
	if (m_argtable == nullptr) {   // If we don't have an argtable, build it.
		build();
	}
	int nErrors = arg_parse(argc, argv, m_argtable);
	return nErrors;
} // ArgTable#parse


/**
 * Print any errors associated with the parsing.
 */
void ArgTable::printErrors(FILE* fp, std::string progName) {
	if (m_argEnd != nullptr) {
		arg_print_errors(fp, m_argEnd, progName.c_str());
	}
} // ArgTable#printErrors


/**
 * Release the argtable data.
 */
/* private */void ArgTable::freeArgtable() {
	if (m_argtable != nullptr) {
		arg_free(m_argtable);
		m_argtable = nullptr;
		m_argEnd   = nullptr;
	}
} // ArgTable#freeArgtable


ArgTableEntry_Date::ArgTableEntry_Date(std::string shortopts, std::string longopts, std::string glossary) {
	m_argDate = arg_daten(shortopts.c_str(), longopts.c_str(), "", "", 0, 1, glossary.c_str());
	m_type   = ArgType_t::DATE;
} // ArgTableEntry_Date#ArgTableEntry_Date


int ArgTableEntry_Date::getCount() {
	return m_argDate->count;
} // ArgTableEntry_Date#getCount


ArgTableEntry_Double::ArgTableEntry_Double(std::string shortopts, std::string longopts, std::string glossary) {
	m_argDbl = arg_dbln(shortopts.c_str(), longopts.c_str(), "", 0, 1, glossary.c_str());
	m_type   = ArgType_t::DBL;
}


int ArgTableEntry_Double::getCount() {
	return m_argDbl->count;
}


ArgTableEntry_File::ArgTableEntry_File(std::string shortopts, std::string longopts, std::string glossary) {
	m_argFile = arg_filen(shortopts.c_str(), longopts.c_str(), "", 0, 1, glossary.c_str());
	m_type = ArgType_t::FILE;
}


int ArgTableEntry_File::getCount() {
	return m_argFile->count;
}


ArgTableEntry_Int::ArgTableEntry_Int(std::string shortopts, std::string longopts, std::string glossary) {
	m_argInt = arg_intn(shortopts.c_str(), longopts.c_str(), "", 0, 1, glossary.c_str());
	m_type   = ArgType_t::INT;
}


int ArgTableEntry_Int::getCount() {
	return m_argInt->count;
}


ArgTableEntry_Lit::ArgTableEntry_Lit(std::string shortopts, std::string longopts, std::string glossary) {
	m_argLit = arg_litn(shortopts.c_str(), longopts.c_str(), 0, 1, glossary.c_str());
	m_type   = ArgType_t::LIT;
}


int ArgTableEntry_Lit::getCount() {
	return m_argLit->count;
}


int ArgTableEntry_Regex::getCount() {
	return m_argRex->count;
}


ArgTableEntry_String::ArgTableEntry_String(std::string shortopts, std::string longopts, std::string glossary, int min, int max) {
	m_argStr = arg_strn(shortopts.c_str(), longopts.c_str(), "", min, max, glossary.c_str());
	m_type   = ArgType_t::STR;
}


int ArgTableEntry_String::getCount() {
	return m_argStr->count;
}


Console::Console() {
}


Console::~Console() {
}

