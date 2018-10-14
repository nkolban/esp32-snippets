/*
 * Console.h
 *
 *  Created on: Jun 15, 2018
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_CONSOLE_H_
#define COMPONENTS_CPP_UTILS_CONSOLE_H_

#include <string>
#include <stdio.h>
#include <list>
#include <argtable3/argtable3.h>

class Console {
public:
	Console();
	virtual ~Console();
};


enum class ArgType_t { LIT, INT, DBL, STR, REX, FILE, DATE };


class ArgTableEntry_Generic {
protected:
	ArgType_t m_type;
public:
	virtual int getCount();
	bool hasValue() {
		return getCount() > 0;
	}
	virtual void* getEntry() = 0;
};


class ArgTableEntry_Lit : public ArgTableEntry_Generic {
private:
	struct arg_lit* m_argLit;
public:
	int getCount();
	ArgTableEntry_Lit(std::string shortopts, std::string longopts, std::string glossary);
	void* getEntry() {
		return m_argLit;
	}
};

class ArgTableEntry_Int : public ArgTableEntry_Generic {
private:
	struct arg_int* m_argInt;
public:
	ArgTableEntry_Int(std::string shortopts, std::string longopts, std::string glossary);
	int getCount();
	int getValue(int index = 0);
	void* getEntry() {
		return m_argInt;
	}
};


class ArgTableEntry_Double : public ArgTableEntry_Generic {
private:
	struct arg_dbl* m_argDbl;
public:
	ArgTableEntry_Double(std::string shortopts, std::string longopts, std::string glossary);
	int getCount();
	double getValue(int index = 0);
	void* getEntry() {
		return m_argDbl;
	}
};


class ArgTableEntry_String : public ArgTableEntry_Generic {
private:
	struct arg_str* m_argStr;
public:
	ArgTableEntry_String(std::string shortopts, std::string longopts, std::string glossary, int min, int max);
	int getCount();
	std::string getValue(int index = 0);
	void* getEntry() {
		return m_argStr;
	}
};


class ArgTableEntry_Regex : public ArgTableEntry_Generic {
private:
	struct arg_rex* m_argRex;
public:
	int getCount();
	std::string getValue(int index = 0);
	void* getEntry() {
		return m_argRex;
	}
};


class ArgTableEntry_File : public ArgTableEntry_Generic {
private:
	struct arg_file* m_argFile;
public:
	ArgTableEntry_File(std::string shortopts, std::string longopts, std::string glossary);
	int getCount();
	std::string getFilename(int index = 0);
	std::string getBasename(int index = 0);
	std::string getExtension(int index = 0);
	void* getEntry() {
		return m_argFile;
	}
};


class ArgTableEntry_Date : public ArgTableEntry_Generic {
private:
	struct arg_date* m_argDate;
public:
	ArgTableEntry_Date(std::string shortopts, std::string longopts, std::string glossary);
	int getCount();
	struct tm* getValue(int index = 0);
	void* getEntry() {
		return m_argDate;
	}
};


class ArgTable {
private:
	void** m_argtable;
	struct arg_end* m_argEnd;
	std::list<std::pair<std::string, ArgTableEntry_Generic*>> m_argTableEntries;
	void build();
	void freeArgtable();

public:
	ArgTable();
	~ArgTable();
	ArgTableEntry_Date   addDate(std::string name,   std::string shortopts, std::string longopts, std::string glossary);
	ArgTableEntry_Double addDouble(std::string name, std::string shortopts, std::string longopts, std::string glossary);
	ArgTableEntry_File   addFile(std::string name,   std::string shortopts, std::string longopts, std::string glossary);
	ArgTableEntry_Int    addInt(std::string name,    std::string shortopts, std::string longopts, std::string glossary);
	ArgTableEntry_Lit    addLit(std::string name,    std::string shortopts, std::string longopts, std::string glossary);
	ArgTableEntry_String addString(std::string name, std::string shortopts, std::string longopts, std::string glossary, int min, int max);
	int parse(int argc, char* argv[]);
	void printErrors(FILE* fp, std::string progName="");
};

#endif /* COMPONENTS_CPP_UTILS_CONSOLE_H_ */
