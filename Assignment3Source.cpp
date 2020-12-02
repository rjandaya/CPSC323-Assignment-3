#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <vector>
#include <algorithm>
#include <iomanip>

const std::vector<std::string> keywords = { "int", "float", "return", "function", "while", "if", "fi", "put" , "boolean" , "get" , "true" , "false" };
const std::vector<std::string> seps = { "[", "]", "(", ")", "{", "}", ";", ":"  , "$$" , ",", "$" };
const std::vector<std::string> ops = { "=", "!", "<", ">", "-", "+", "*", "/" , "<=", ">=", "!=", "-=", "+=", "==", "*=" , "/="};

bool display = false;
int line = 1;

class record {
private:
	std::string token, lexeme, type;
public:
	std::string getToken() { return this->token; }
	std::string getLexeme() { return this->lexeme; }
	std::string getType() { return this->type; }

	void setLexeme(std::string s) { this->lexeme = s; }
	void setType(std::string s) { this->type = s; }
	void setToken(std::string s) { this->token = s; }
};

class symbol {
private:
	record sym;
	std::string address;
public:
	record getSym() { return this->sym; }
	std::string getAddress() { return this->address; }

	void setSym(record s) { this->sym = s; }
	void setAddress(int s) { this->address = std::to_string(s); }

	symbol(record sym, int address) { 
		setSym(sym);
		setAddress(address);
	}
};

std::vector<symbol> sym_Table;
int Memory_address = 5000;

void undecla_Error() {
	std::cerr << "Error: undeclared Identifier used in line " << line << ".\n";
	// 
	exit(2);
}

void redecla_Error() {
	std::cerr << "Error: Identifier illegally redeclared in line " << line << ".\n";
	// 
	exit(2);
}


std::string get_address(std::string sym) {
	std::vector<symbol>::iterator i;
	i = sym_Table.begin();
	while (!sym_Table.empty() && i != sym_Table.end()) {
		if (i->getSym().getLexeme() == sym) { return i->getAddress(); }
		i++;
	}
	undecla_Error(); 

}
record get_sym(std::string sym) {
	std::vector<symbol>::iterator i;
	i = sym_Table.begin();
	while (!sym_Table.empty() && i != sym_Table.end()) {
		if (i->getSym().getLexeme() == sym) { return i->getSym(); }
		i++;
	}
	undecla_Error();
}

record get_sym_by_address(std::string sym) {
	std::vector<symbol>::iterator i;
	i = sym_Table.begin();
	while (!sym_Table.empty() && i != sym_Table.end()) {
		if (i->getAddress() == sym) { return i->getSym(); }
		i++;
	}
	undecla_Error();
}

void make_Sym(record sym) {
	std::vector<symbol>::iterator i;
	i = sym_Table.begin();
	while (!sym_Table.empty() && i != sym_Table.end()) {
		if (i->getSym().getLexeme() == sym.getLexeme()) { redecla_Error(); }
		i++;
	}
	sym_Table.push_back(symbol(sym,Memory_address));
	Memory_address++;
}

void print_Symbols(std::ofstream& out) {
	out << "Identifier\t" << "MemoryLocation\tType\n";
	std::vector<symbol>::iterator i = sym_Table.begin();
	while (i != sym_Table.end()) {
		out << i->getSym().getLexeme() << "\t\t" << i->getAddress() << "\t\t" << i->getSym().getToken() << "\n";
		i++;
	}
}

// ----------------------------------------------------------------------------------

class instruction {
private:
	std::string op, oprnd;
	int address;
public:
	std::string getOp() { return this->op; }
	std::string getOprnd() { return this->oprnd; }
	int getAddress() { return this->address; }

	void setOprnd(std::string s) { this->oprnd = s; }
	void setOp(std::string s) { this->op = s; }
	void setAddress(int s) { this->address = s; }

	instruction(int i, std::string s, std::string t) {
		setOprnd(t);
		setOp(s);
		setAddress(i);
	}
};

int instr_address = 1;
std::vector<instruction> Instr_table;

void gen_instr(std::string op, std::string oprnd)
/* instr_address shows the current instruction address is global */
{
	Instr_table.push_back(instruction(instr_address, op, oprnd));
	instr_address++;
};

void print_Instr(std::ofstream& out) {
	out << "\n";
	std::vector<instruction>::iterator i = Instr_table.begin();
	while (i != Instr_table.end()) {
		out << i->getAddress() << "\t" << i->getOp() << "\t" << i->getOprnd() << "\n";
		i++;
	}
}

std::vector<record> arith_Table;

void arith_Error() {
	std::cerr << "Error: Illegal arithmetic operation in line " << line << ".\n";
	// 
	exit(2);
}

void arith_Check() {
	std::string save = arith_Table.back().getLexeme();
	if (arith_Table.back().getLexeme() == "identifier") {
		arith_Table.pop_back();
		if (get_sym_by_address(save).getType() == "boolean" || get_sym_by_address(arith_Table.back().getLexeme()).getType() == "boolean")
			arith_Error();
		if (get_sym_by_address(save).getType() != get_sym_by_address(arith_Table.back().getLexeme()).getType())
			arith_Error();
	}
	else if (arith_Table.back().getLexeme() == "int") {
		if (get_sym_by_address(save).getType() != "int")
			arith_Error();
	}
}

// ----------------------------------------------------------------------------------
std::vector<int> jumpStack;

void push_jumpStack(int address) {
	jumpStack.push_back(address);
}

int pop_jumpStack() {
	int save = jumpStack.back();
	jumpStack.pop_back();
	return save;
}

void back_patch(int jump_addr)
{
	int addr = pop_jumpStack();
	Instr_table[addr].setOprnd(std::to_string(jump_addr));
}

// ----------------------------------------------------------------------------------

bool FSM(std::string& state, char input, std::string& lexeme) {
	std::string c;
	if (state != "comments")
		c.push_back(input);
	if (state == "start") {
		if (isalpha(input)) { state = "identifier"; }
		else if (isdigit(input)) { state = "int"; }
		else if (std::find(ops.begin(), ops.end(), c) != ops.end()) { state = "operator"; }
		else if (std::find(seps.begin(), seps.end(), c) != seps.end()) { state = "separator"; }
		else if (input == EOF) {
			state = "fileend";
			return true;
		}
	}

	else if (state == "identifier" && !isalnum(input) && input != '_') {
		return true;
	}
	else if (state == "int") {
		if (!isdigit(input)) {
			return true;
		}
	}

	else if (state != "comments" && lexeme == "/*") {
		state = "comments";
		lexeme = "";
	}
	else if (state == "operator" && std::find(ops.begin(), ops.end(), lexeme + c) == ops.end()) {
		if (lexeme == "<" || lexeme == "=" || lexeme == "!" || lexeme == "<" || lexeme == ">" || lexeme == "-" || lexeme == "+" || lexeme == "*" || lexeme == "/") {}
		return true;
	}
	else if (state == "separator" && std::find(seps.begin(), seps.end(), lexeme + c) == seps.end()) {
		return true;
	}
	return false;
}



record callLexer(std::ofstream& out,std::ifstream& source) {
	std::string state = "start", lexeme = "";
	int done = 0;
	char c;
	while (done != 1) {
		c = source.get();

		if (FSM(state, c, lexeme) == true) {
			done = 1;
			source.unget();
		}

		if (state != "comments" && lexeme == "/" ) {
			if ((source.get()) == '*') {
				state = "comments";
				lexeme = "";
				done = 0;
			}
			else source.unget();
		}
		
		else if (state == "comments" && c == '*' && (source.get()) == '/') {
			state = "start";
			lexeme = "";
			c = source.get();
		}

		if (done == 1) {
			record latest;
			if (state == "identifier" && std::find(keywords.begin(), keywords.end(), lexeme) != keywords.end()) { 
				state = "keyword"; 
				
			}
			
			latest.setLexeme(lexeme);
			latest.setToken(state);
			


			if (latest.getToken() != "fileend" && display) out << std::left << std::setw(10) << "Token:" << latest.getToken() << "\t:\t" << std::setw(10) << "Lexeme:" << latest.getLexeme() << "\n";
			return latest;
		}
		else if (!isspace(c) && state != "comments" && done != 1) { lexeme.push_back(c); }
		else if (c == '\n') { line += 1; }
		
	}
	
}



void Syntax_Error(record latest, std::ofstream& out, std::string expected) {
	std::cerr << "Syntax Error: Expected " << expected << " on line " << line <<"\n";
	std::cerr << "Received " << latest.getToken() << " \"" << latest.getLexeme() << "\"\n";
	// 
	exit(1);
}
void Lexeme_Check(std::ofstream& out,std::ifstream& source, std::string lexeme) {
	record latest = callLexer(out,source);
	if (latest.getLexeme() != lexeme) {
		Syntax_Error(latest, out,lexeme);
	}
}

void Rat20F(std::ofstream& out, std::ifstream& source);
record OFD(std::ofstream& out, std::ifstream& source); 
record IDs(std::ofstream& out, std::ifstream& source, record latest);
record IDs(std::ofstream& out, std::ifstream& source, record latest, bool make, std::string a);
record IDs_Cont(std::ofstream& out, std::ifstream& source);
record IDs_Cont(std::ofstream& out, std::ifstream& source, bool make, std::string a);
void Compound(std::ofstream& out, std::ifstream& source);
void Statement(std::ofstream& out, std::ifstream& source, record latest);
record State_List(std::ofstream& out, std::ifstream& source, record latest);
record Expression(std::ofstream& out, std::ifstream& source, record latest);
record Para_List(std::ofstream& out, std::ifstream& source, record latest);
record Decla_List(std::ofstream& out, std::ifstream& source, record latest);
record Func_Def(std::ofstream& out, std::ifstream& source);
void Assign(std::ofstream& out, std::ifstream& source, record latest);
void If(std::ofstream& out, std::ifstream& source); 
void Return(std::ofstream& out, std::ifstream& source);
void IfP(std::ofstream& out, std::ifstream& source, record latest);
void ReturnP(std::ofstream& out, std::ifstream& source);

int main(int argc, const char* argv[]) {
	char c;
	if (argv[1] == nullptr) {
		std::cerr << "No Input File name Detected\n";
		std::cin >> c;
		return 2;
	}
	if (argv[2] == nullptr) {
		std::cerr << "No Output File name Detected\n";
		std::cin >> c;
		return 2;
	}
	std::ifstream source(argv[1]);
	std::ofstream out(argv[2]);
	if (!out.is_open()) {
		std::cout << "Output file failed to open\n";
		std::cin >> c;
		return 2;
	}
	if (!source.is_open()){
		std::cout << "Input file failed to open\n";
		std::cin >> c;
		return 2;
	}

	Rat20F(out, source);

	print_Symbols(out);
	print_Instr(out);

	out.close();
	source.close();
	return 0;
}
// RJ's Section

void Empty(std::ofstream& out, std::ifstream& source) {
	//Do nothing
}

record PrimaryP(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<Primary>' ::= ( <IDs> ) | <Empty>\n";
	record latest = callLexer(out, source);
	if (latest.getLexeme() != "(") {
		return latest;
	}
	else 
		latest = IDs(out, source, callLexer(out, source),false,"");
	if (latest.getLexeme() != ")") {
		Syntax_Error(latest, out, ")");
	}
	else 
		return callLexer(out, source);
}

record Primary(std::ofstream& out, std::ifstream& source, record latest) {
	if (display)
		out << "\t<Primary> ::= <Identifier> <Primary>' | <Integer> | ( <Expression> ) | <Real> | true | false\n";

	// <Identifier> <Primary>'
	if (latest.getToken() == "identifier") {
		gen_instr("PUSHM", get_address(latest.getLexeme()));
		return PrimaryP(out, source);
	}
	// (<Expression>)
	else if (latest.getLexeme() == "(") {
		latest = Expression(out, source, callLexer(out, source));
		if (latest.getLexeme() != ")") {
			Syntax_Error(latest, out, ")");
		}
		else
			return callLexer(out, source);
	}
	// <Integer>
	else if (latest.getToken() == "int") {
		/* do i return the lexeme here? */
		arith_Table.push_back(latest);
		gen_instr("PUSHM", latest.getLexeme());
		return callLexer(out, source);
	}
	// <Real>
	else if (latest.getToken() == "real")
		/* do i return the lexeme here? */
		return callLexer(out, source);
	// true | false 
	else if (latest.getLexeme() == "true" || latest.getLexeme() == "false") {
		if (latest.getLexeme() == "true") 
			gen_instr("PUSHM", "1");
		else 
			gen_instr("PUSHM", "0");
		return callLexer(out, source);
	}
	else 
		Syntax_Error(latest, out, "identifier or int or ( or real or true or false");
}

record Factor(std::ofstream& out, std::ifstream& source,record latest) {
	if (display)
		out << "\t<Factor> ::= - <Primary> | <Primary>\n";

	if (latest.getLexeme() == "-")
		return Primary(out, source, callLexer(out, source));
	else
		return Primary(out, source, latest);
}

record TermP(std::ofstream& out, std::ifstream& source, record latest) {
	if (display)
		out << "\t<Term>' ::= * <Factor> <Term>' | / <Factor> <Term>' | <Empty>\n";
	record save = latest;
	if (latest.getLexeme() == "*" || latest.getLexeme() == "/") {
		latest = Factor(out, source, callLexer(out, source));
		arith_Check();
		if (save.getLexeme() == "*")
			gen_instr("MUL", "nil");
		else if (save.getLexeme() == "/")
			gen_instr("DIV", "nil");
		return TermP(out,source,latest);
	}
	else
		return latest;
}

record Term(std::ofstream& out, std::ifstream& source, record latest) {
	if (display)
		out << "\t<Term> ::= <Factor> <Term>'\n";
	latest = Factor(out, source, latest);
	return TermP(out, source,latest);
}

record ExpressionP(std::ofstream& out, std::ifstream& source, record latest) {
	if (display)
		out << "\t<Expression>' ::= + <Term> <Expression>' | - <Term> <Expression>' | <Empty>\n";
	std::string save = latest.getLexeme();
	if (latest.getLexeme() == "+" || latest.getLexeme() == "-") {
		latest = Term(out, source, callLexer(out, source));
		arith_Check();
		if (save == "+") gen_instr("ADD", "nil");
		else if (save == "-") gen_instr("SUB", "nil");
		return ExpressionP(out, source, latest);
	}
	else
		return latest;
}

record Expression(std::ofstream& out, std::ifstream& source,record latest) {
	if (display)
		out << "\t<Expression> ::= <Term> <Expression>'\n";
	latest = Term(out, source, latest);
	return ExpressionP(out, source,latest);
}

void Relop(std::ofstream& out, std::ifstream& source, record latest) {
	if (display)
		out << "\t<Relop> ::= == | != | > | < | <= | =>\n";

	if (latest.getLexeme() == "==" || latest.getLexeme() == "!=" || latest.getLexeme() == ">" || latest.getLexeme() == "<" || latest.getLexeme() == "<=" || latest.getLexeme() == "=>")
		return;
}

record Condition(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<Condition> ::= <Expression> <Relop> <Expression>\n";
	record latest = Expression(out, source, callLexer(out, source));
	Relop(out, source,latest);
	record save = Expression(out, source, callLexer(out, source));
	if (latest.getLexeme() == "<") {
		gen_instr("LES", "nil");
		push_jumpStack(instr_address - 1); /* another stack need */
		gen_instr("JUMPZ", "nil");
	}
	else if (latest.getLexeme() == "<=") {
		gen_instr("LES", "nil");
		push_jumpStack(instr_address - 1); /* another stack need */
		gen_instr("JUMPZ", "nil");
	}
	else if (latest.getLexeme() == ">") {
		gen_instr("GRT", "nil");
		push_jumpStack(instr_address - 1); /* another stack need */
		gen_instr("JUMPZ", "nil");
	}
	else if (latest.getLexeme() == ">=") {
		gen_instr("GRT", "nil");
		push_jumpStack(instr_address - 1); /* another stack need */
		gen_instr("JUMPZ", "nil");
	}
	else if (latest.getLexeme() == "==") {
		gen_instr("EQU", "nil");
		push_jumpStack(instr_address - 1); /* another stack need */
		gen_instr("JUMPZ", "nil");
	}
	else if (latest.getLexeme() == "!=") {
		gen_instr("NEQ", "nil");
		push_jumpStack(instr_address - 1); /* another stack need */
		gen_instr("JUMPZ", "nil");
	}
	return save;
}

void While(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<While> ::= while ( <Condition> ) <Statement>\n";
	std::string addr = std::to_string(instr_address);
	gen_instr("LABEL", "nil");
	Lexeme_Check(out, source, "(");
	record latest = Condition(out, source);
	if (latest.getLexeme() != ")") {
		Syntax_Error(latest, out, ")");
	}
	 Statement(out, source, callLexer(out, source));
	 gen_instr("JUMP", addr);
	 back_patch(instr_address);
	 return;
}

void Scan(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<Scan> ::= get ( <IDs> );\n";

	Lexeme_Check(out, source, "(");
	record latest = IDs(out, source, callLexer(out, source),false,"");
	if (latest.getLexeme() != ")") {
		Syntax_Error(latest, out, ")");
	}
	gen_instr("STDIN", std::to_string(instr_address));
	Lexeme_Check(out, source, ";");
}

void Print(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<Print> ::= put ( <Expression> );\n";

	Lexeme_Check(out, source, "(");
	record latest = Expression(out, source, callLexer(out, source));
	if (latest.getLexeme() != ")") {
		Syntax_Error(latest, out, ")");
	}
	gen_instr("STDOUT", std::to_string(instr_address));
	Lexeme_Check(out, source, ";");
	
}

// Vien's Section

record IDs_Cont(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<IDs>' ::= ,  <IDs>  |  <Empty>'\n";
	record latest = callLexer(out, source);
	if (latest.getLexeme() == ",")
		return IDs(out, source, callLexer(out, source));
	else
		return latest;
}

record IDs_Cont(std::ofstream& out, std::ifstream& source, bool make, std::string a) {
	if (display)
		out << "\t<IDs>' ::= ,  <IDs>  |  <Empty>'\n";
	record latest = callLexer(out, source);
	if (latest.getLexeme() == ",")
		return IDs(out, source, callLexer(out, source), make, a);
	else
		return latest;
}

record IDs(std::ofstream& out, std::ifstream& source, record latest, bool make, std::string a) {
	if (display)
		out << "\t<IDs> ::= <Identifier> <IDs>'\n";
	if (latest.getToken() != "identifier")
		Syntax_Error(latest, out, "an identifier");
	else if (make) {
		std::vector<std::string>::const_iterator word;
		word = std::find(keywords.begin(), keywords.end(), a);
		latest.setType(*word);
		make_Sym(latest);
	}
	else { 
		arith_Table.push_back(latest);
		gen_instr("PUSHM", get_address(latest.getLexeme()));
	}
	return IDs_Cont(out, source, make,a);
}
record IDs(std::ofstream& out, std::ifstream& source, record latest) {
	if (display)
		out << "\t<IDs> ::= <Identifier> <IDs>'\n";
	if (latest.getToken() != "identifier")
		Syntax_Error(latest, out, "an identifier");
	return IDs_Cont(out, source);
}

void Body(std::ofstream& out, std::ifstream& source,record latest) {
	if (display)
		out << "\t<Body> ::= { < Statement List> }\n";
	if (latest.getLexeme() != "{") {
		Syntax_Error(latest, out, "{");
	}
	latest = callLexer(out, source);
	latest = State_List(out, source, latest);
	if (latest.getLexeme() != "}") {
		Syntax_Error(latest, out, "}");
	}
}

void Qualifier(std::ofstream& out, std::ifstream& source, record latest) {
	if (display)
		out << "\t<Qualifier> ::= int | boolean | real\n";
	if (latest.getLexeme() == "int" || latest.getLexeme() == "boolean" || latest.getLexeme() == "real")
		return;
	else
		Syntax_Error(latest, out, "int, boolean, or real");
}

void Parameter(std::ofstream& out, std::ifstream& source, record a) {
	if (display)
		out << "\t<Parameter> ::= <IDs> <Qualifier>\n";
	record latest = IDs(out, source, a);
	Qualifier(out, source, latest);
}

record Decla(std::ofstream& out, std::ifstream& source, record a) {
	if (display)
		out << "\t<Parameter> ::= <Qualifier> <IDs>\n";
	Qualifier(out, source, a);
	record latest = callLexer(out, source);

	return IDs(out, source, latest, true,a.getLexeme());
}

record Para_List_Cont(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<Parameter List>\' ::= ,  <Parameter List>  |  <Empty>\n";
	record latest = callLexer(out, source);
	if (latest.getLexeme() == ",")
		return Para_List(out, source, callLexer(out, source));
	else
		return latest;
}

record Para_List(std::ofstream& out, std::ifstream& source, record latest) {
	if (display)
		out << "\t<Parameter List> ::= <Parameter> <Parameter List>'\n";
	Parameter(out, source, latest);
	return Para_List_Cont(out, source);
}

record Decla_List_Cont(std::ofstream& out, std::ifstream& source, record latest) {
	if (display)
		out << "\t<Declaration List>\' ::= <Declaration List>  |  <Empty>\n";
	if (latest.getLexeme() == "int" || latest.getLexeme() == "boolean" || latest.getLexeme() == "real")
		return Decla_List(out, source, latest);
	else
		return latest;
}

record Decla_List(std::ofstream& out, std::ifstream& source, record latest) {
	if (display)
		out << "\t<Declaration List> ::= <Declaration> ; <Declaration List>\'\n";
	record l = Decla(out, source, latest);
	if (l.getLexeme() != ";")
		Syntax_Error(l, out, ";");
	return Decla_List_Cont(out, source, callLexer(out, source));
}

record State_List_Cont(std::ofstream& out, std::ifstream& source, record latest) {
	if (display)
		out << "\t<Statement List>\' ::= <Statement List>  |  <Empty>\n";
	// <Compound>
	if (latest.getLexeme() == "{") {
		return State_List(out, source, latest);
	}
	//<Assign>
	else if (latest.getToken() == "identifier")
		return State_List(out, source, latest);
	//<If>
	else if (latest.getLexeme() == "if") {
		return State_List(out, source, latest);
	}
	//<Return>
	else if (latest.getLexeme() == "return") {
		return State_List(out, source, latest);
	}
	//<Print>
	else if (latest.getLexeme() == "put") {
		return State_List(out, source, latest);
	}
	//<Scan>
	else if (latest.getLexeme() == "get") {
		return State_List(out, source, latest);
	}
	//<While>
	else if (latest.getLexeme() == "while") {
		return State_List(out, source, latest);
	}
	else
		return latest;
}

record State_List(std::ofstream& out, std::ifstream& source, record latest) {
	if (display)
		out << "\t<Statement List> ::= <Statement> <Statement List>\'\n";
	Statement(out, source, latest);
	return State_List_Cont(out, source, callLexer(out, source));
}

record OPL(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<Opt Parameter List> ::= <Parameter List> | <Empty>\n";
	record latest = callLexer(out, source);
	if (latest.getToken() != "identifier") {
		return latest;
	}
	return Para_List(out, source, latest);

}

record ODL(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<Opt Declaration List> ::= <Declaration List> | <Empty>\n";
	record latest = callLexer(out, source);
	if (latest.getLexeme() == "int" || latest.getLexeme() == "boolean" || latest.getLexeme() == "real") {
		return Decla_List(out, source, latest);
	}
	else
		return latest;

}

void Func(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<Function> ::= function <Identifier> ( <Opt Parameter List> ) <Opt Declaration List> <Body>\n";

	record latest = callLexer(out, source);
	latest = IDs(out, source, latest);

	if (latest.getLexeme() != "(") {
		Syntax_Error(latest, out, "(");
	}
	latest = OPL(out, source);
	if (latest.getLexeme() != ")") {
		Syntax_Error(latest, out, ")");
	}
	latest = ODL(out, source);
	Body(out, source,latest);

}

record Func_Def_Cont(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<Function Definitions>' ::= <Function Definitions> | <Empty>\n";
	record latest = callLexer(out, source);
	if (latest.getLexeme() != "function") {
		return latest;
	}
	else
		return Func_Def(out, source);
}

record Func_Def(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<Function Definitions> ::= <Function> <Function Definitions>'\n";
	Func(out, source);
	return Func_Def_Cont(out, source);
}
record OFD(std::ofstream& out, std::ifstream& source) {

	if (display)
		out << "\t<Opt Function Definitions> ::= <Function Definitions> | <Empty>\n";
	record latest = callLexer(out, source);
	if (latest.getLexeme() != "function") {
		return latest;
	}
	else
		return Func_Def(out, source);
}

void Statement(std::ofstream& out, std::ifstream& source, record latest) {
	if (display)
		out << "\t<Statement ::= <Compound> | <Assign> | <If> | <Return> | <Print> | <Scan> | <While>\n";

	// <Compound>
	if (latest.getLexeme() == "{") {
		Compound(out, source);
	}
	//<Assign>
	else if (latest.getToken() == "identifier")
		Assign(out, source,latest);
	//<If>
	else if (latest.getLexeme() == "if") {
		If(out, source);
	}
	//<Return>
	else if (latest.getLexeme() == "return") {
		Return(out, source);
	}
	//<Print>
	else if (latest.getLexeme() == "put") {
		Print(out, source);

	}
	//<Scan>
	else if (latest.getLexeme() == "get") {
		Scan(out, source);
	}
	//<While>
	else if (latest.getLexeme() == "while") {
		While(out, source);
	}
	else
		Syntax_Error(latest, out, "{ or identifier or if or return or put or get or while");

}

void Compound(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<Compound> ::= { <Statement List> }\n";
	record latest = State_List(out, source, callLexer(out, source));
	if (latest.getLexeme() != "}")
		Syntax_Error(latest, out, "}");
}

void Assign(std::ofstream& out, std::ifstream& source,record latest) {
	if (display)
		out << "\t<Assign> :: <Identifier> = <Expression>; \n";

	std::string save = latest.getLexeme();

	Lexeme_Check(out, source, "=");

	latest = Expression(out, source,callLexer(out, source));
	gen_instr("POPM", get_address(save));
	if (latest.getLexeme() != ";")
		Syntax_Error(latest, out, ";");
}

void If(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<If> ::= if ( <Condition> ) <Statement> <If>'\n";
	int addr = instr_address;
	Lexeme_Check(out, source, "(");
	record latest = Condition(out, source);
	if (latest.getLexeme() != ")")
		Syntax_Error(latest, out, ")");
	
	Statement(out, source, callLexer(out, source));
	back_patch(addr);
	IfP(out, source, callLexer(out, source));
	
}

void IfP(std::ofstream& out, std::ifstream& source, record latest){
	if (display)
		out << "\t<If>' ::= fi | else <Statement> fi\n";

	if (latest.getLexeme() == "fi")
		return;

	else if (latest.getLexeme() == "else") {
		Statement(out, source, callLexer(out, source));
		Lexeme_Check(out, source, "fi");
	}
}

void Return(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<Return> ::= return <Return>'\n";
	ReturnP(out, source);
	return;
}

void ReturnP(std::ofstream& out, std::ifstream& source) {
	if (display)
		out << "\t<Return>' ::= ; | <Expression>;\n";
	record latest = callLexer(out, source);
	if (latest.getLexeme() == ";")
		return;
	else
		latest = Expression(out, source, latest);

	if (latest.getLexeme() != ";") {
		Syntax_Error(latest, out, ";");
	}
	
}

void Rat20F(std::ofstream& out, std::ifstream& source) {
	//display = true;
	if (display)
		out << "\t<Rat20F> ::= $$ <Opt Declaration List> <Statement List> $$\n";

	Lexeme_Check(out, source, "$$");

	record latest = ODL(out, source); // <Opt Declaration List>

	latest = State_List(out, source, latest);
	if (latest.getLexeme() != "$$")
		Syntax_Error(latest, out, "$$");

}