#pragma once

#include <algorithm>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <variant>
#include <vector>

#include "common.h"
#include "enumstring.h"


constexpr int line_buffer_length = 72;
constexpr int identifier_length = 10;
constexpr int string_literal_length = 10;
constexpr int integer_digit_length = 10;

// xx.yyEzz
constexpr int real_base_length = 5;
constexpr int real_decimal_length = 5;
constexpr int real_exponent_length = 2;

using ProgramLine = std::string_view;

// std::string Str_ProgramLine (const ProgramLine &line);
// ProgramLine Sub_ProgramLine (const ProgramLine &line, int indexesToCopy);
// ProgramLine Sub_ProgramLine (const ProgramLine &line, int firstIndex, int indexesToCopy);


// std::ostream &operator<< (std::ostream &os, const ProgramLine &t);


enum class TT
{
	PROG,
	ID,
	P_O,
	P_C,
	SEMIC,
	DOT,
	VAR,
	COLON,
	ARRAY,
	B_O,
	B_C,
	NUM,
	OF,
	STD_T,
	INT,
	REAL,
	PROC,
	BEGIN,
	END,
	CALL,
	COMMA,
	RELOP,
	ADDOP,
	A_OP,
	MULOP,
	NOT,
	SIGN,
	IF,
	THEN,
	ELSE,
	WHILE,
	DO,
	DOT_DOT,
	END_FILE,
	LEXERR,
};

std::string operator+ (const std::string &out, TT tt);

struct NoAttrib
{
	friend std::ostream &operator<< (std::ostream &os, const NoAttrib &t) { return os << "(NULL)"; }
};

struct SymbolType
{
	int loc = -1;
	SymbolType (int loc) : loc (loc) {}

	friend std::ostream &operator<< (std::ostream &os, const SymbolType &t)
	{
		return os << t.loc << " (index in symbol table)";
	}
};

enum class AddOpEnum
{
	// plus,  //+
	// minus, //-
	t_or // or
};

enum class MulOpEnum
{
	mul,  //*
	div,  // div or /
	mod,  // mod
	t_and // and
};

enum class SignOpEnum
{
	plus, //+
	minus //-
};

enum class RelOpEnum
{
	equal,                //=
	not_equal,            //<>
	less_than,            //<
	less_than_or_equal,   //<=
	greater_than,         //>
	greater_than_or_equal //>=
};

enum class StandardTypeEnum
{
	integer,
	real
};

struct NumType
{
	std::variant<int, float> val;
	NumType (int a) : val (a) {}
	NumType (float a) : val (a) {}

	friend std::ostream &operator<< (std::ostream &os, const NumType &t)
	{
		if (t.val.index () == 0) return os << std::get<0> (t.val) << "(INT)";
		if (t.val.index () == 1) return os << std::get<1> (t.val) << "(FLOAT)";
		return os;
	}
};

// struct IntType
//{
//	int val;
//	IntType (int a) : val (a) {}
//
//	friend std::ostream &operator<< (std::ostream &os, const IntType &t)
//	{
//		return os << t.val << "(INT)";
//	}
//};
//
// struct FloatType
//{
//	float val;
//	FloatType (float a) : val (a) {}
//
//	friend std::ostream &operator<< (std::ostream &os, const FloatType &t)
//	{
//		return os << t.val << "(FLOAT)";
//	}
//};

struct StringLiteral
{
	int loc = -1;
	StringLiteral (int loc) : loc (loc) {}

	friend std::ostream &operator<< (std::ostream &os, const StringLiteral &t)
	{
		return os << t.loc << "(index in literal table)";
	}
};

enum class LexerErrorEnum
{
	Unrecognized_Symbol,
	Id_TooLong,
	StrLit_TooLong,
	StrLit_NotTerminated,
	Int_InvalidNumericLiteral,
	Int_TooLong,
	Int_LeadingZero,
	SReal_InvalidNumericLiteral,
	SReal1_LeadingZero,
	SReal1_TooLong,
	SReal2_TooLong,
	SReal2_TooShort,
	LReal_InvalidNumericLiteral,
	LReal1_LeadingZero,
	LReal2_TrailingZero,
	LReal3_LeadingZero,
	LReal1_TooLong,
	LReal2_TooLong,
	LReal2_TooShort,
	LReal3_TooLong,
	LReal3_TooShort,
	CommentContains2ndLeftCurlyBrace
};

struct LexerError
{
	LexerErrorEnum type;
	std::string errorData;
	LexerError (LexerErrorEnum type, std::string errorData) : type (type), errorData (errorData) {}
	LexerError (LexerErrorEnum type, std::string_view errorData)
	: type (type), errorData (std::string (errorData))
	{
	}

	friend std::ostream &operator<< (std::ostream &os, const LexerError &t)
	{
		return os << enumToString (t.type) << " " << t.errorData;
	}
};

using TokenAttribute =
std::variant<NoAttrib, AddOpEnum, MulOpEnum, SignOpEnum, RelOpEnum, StandardTypeEnum, NumType, SymbolType, StringLiteral, LexerError>;

std::ostream &operator<< (std::ostream &os, const TokenAttribute &t);

struct TokenInfo
{
	TT type;
	TokenAttribute attrib;
	int line_location = -1;
	int column_location = -1;

	TokenInfo (TT type, TokenAttribute attrib) : type (type), attrib (attrib) {}

	TokenInfo (TT type, TokenAttribute attrib, int line, int column)
	: type (type), attrib (attrib), line_location (line), column_location (column)
	{
	}

	friend std::ostream &operator<< (std::ostream &os, const TokenInfo &t)
	{
		return os << enumToString (t.type) << '\t' << t.attrib;
	}
};

struct ReservedWord
{
	std::string word;
	TT type;
	TokenAttribute attrib;

	ReservedWord (std::string word, TT type, TokenAttribute attrib = NoAttrib{})
	: word (word), type (type), attrib (attrib){};

	bool operator== (const ReservedWord &other) const
	{
		return (word == other.word && type == other.type);
	}

	bool operator== (const std::string &s) const { return (word == s); }
	bool operator== (const std::string_view &s) const { return (word == s); }

	TokenInfo GetToken () const { return TokenInfo (type, attrib); }
};

namespace std
{
template <> struct hash<ReservedWord>
{
	size_t operator() (const ReservedWord &w) const { return hash<std::string> () (w.word); }
};
} // namespace std

using ReservedWordList = std::unordered_set<ReservedWord>;


struct LexerMachineReturn
{
	using OptionalToken = std::optional<TokenInfo>;

	int chars_to_eat = 0;
	OptionalToken content;

	LexerMachineReturn (int chars_to_eat) : chars_to_eat (chars_to_eat){};

	LexerMachineReturn (int chars_to_eat, TokenInfo token)
	: chars_to_eat (chars_to_eat), content (token)
	{
	}
};

struct LexerContext
{
	LexerContext (CompilationContext &compContext) : compContext (compContext) {}
	bool isInComment = false;
	bool isInLiteral = false;
	CompilationContext &compContext;
};

using LexerMachineFuncSig =
std::function<std::optional<LexerMachineReturn> (LexerContext &context, ProgramLine &line)>;

struct LexerMachine
{
	std::string name;
	int precedence = 10;
	LexerMachineFuncSig machine;

	LexerMachine (std::string name, int precedence, LexerMachineFuncSig machine)
	: name (name), precedence (precedence), machine (machine){};
};

class Lexer;
class TokenStream
{
	public:
	TokenStream (Lexer &lexer, CompilationContext &compilationContext, CodeSource &sourceCode);

	TokenInfo Current () const;
	TokenInfo Advance ();

	private:
	int index = 0;
	std::vector<TokenInfo> tokens;
	CompilationContext &compilationContext;
	CodeSource &sourceCode;
	Lexer &lexer;
};

class Lexer
{
	public:
	Lexer (Logger &logger) : logger (logger)
	{
		ReadReservedWordsFile ();
		CreateMachines ();
	}

	// void LoadReservedWords (ReservedWordList list) { reservedWords = list; };
	void CreateMachines ();

	void AddMachine (LexerMachine &&machine);

	std::vector<TokenInfo> GetTokens (CodeSource &sourceCode, CompilationContext &context);

	void TokenFilePrinter (int line_num, std::string_view lexeme, LexerMachineReturn::OptionalToken content);


	std::optional<TokenInfo> CheckReseredWords (std::string_view s);
	std::optional<ReservedWord> GetReservedWord (std::string s);

	void ReadReservedWordsFile ();

	std::vector<LexerMachine> machines;
	ReservedWordList reservedWords;
	Logger &logger;
};


std::ostream &operator<< (std::ostream &os, const TokenAttribute &t);
