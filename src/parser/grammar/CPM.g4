/*******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Camilo Sanchez (Camiloasc1) 2020 Martin Mirchev (Marti2203)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 * NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * ****************************************************************************
 */

/*
This is parser and lexer in one file, because I couldn't find a way
for CMake to generate Lexer source files as well with antlr4_generate,
when the files were separated.
*/


grammar CPM;

@header {
#include <set>
#include <string>
#include <cstdio>
}

@parser::members {
    // user defined classes and structs
    // note: this and functions related to thsi
    // treat all classes and structs like they are
    // at the global scopes -> UB for nested classes/structs
    std::set<std::string> user_types;

    bool ty_exists(const std::string & t) {
        return user_types.contains(t);
    }

    // returns true if next token names a user defined type,
    // inspired by the antlr book pg 208
    bool ty_exists() {
        return ty_exists(getCurrentToken()->getText());
    }

    void add_ty(const std::string & t) {
        user_types.insert(t);
//        printf("inserted type %s: %d\n", t.data(), b);
    }
}

//options {
//	tokenVocab = lexer14;
//}
/*Basic concepts*/

translationUnit: declarationseq? EOF;

/*Expressions*/

primaryExpression:
    // allows automatic string concat, char * str = "Hello " "World";
	literal+
	| This
	| LeftParen commaExpression RightParen
	| idExpression;

idExpression: unqualifiedId;

unqualifiedId: Identifier;

postfixExpression:
	primaryExpression
	| postfixExpression LeftBracket commaExpression RightBracket
	// call
	| postfixExpression LeftParen expressionList? RightParen
	// member access
	| postfixExpression (Dot | Arrow) idExpression
	| postfixExpression (PlusPlus | MinusMinus);
/*
 add a middle layer to eliminate duplicated function decl
 */

expressionList: assignmentExpression (Comma assignmentExpression)*;

unaryExpression:
	postfixExpression
	// this contained unaryExpression instead of castExpressoin,
	// but that's wrong, see it in grammar:
	// https://alx71hub.github.io/hcb/#basic.link
	// and also cpp operator precedence, which sets * dereference
	// and c-style casting to the same level
	// the previous version with unaryExpression was unable to parse
	// *(int*)a
	| unaryOperator castExpression
	| Sizeof LeftParen theTypeId RightParen;

unaryOperator: op = (PlusPlus | MinusMinus | Star | And | Plus | Tilde | Minus | Not | Sizeof);

castExpression:
	unaryExpression
	| LeftParen theTypeId RightParen castExpression;

conditionalExpression:
	binaryExpression (
		Question commaExpression Colon assignmentExpression
	)?;

assignmentExpression:
	conditionalExpression
	| binaryExpression assignmentOperator assignmentExpression;

/* This rule replaces rules logicalOrExpression...pointerMemberExpression
    However, pointerMemberExpression sub-rule isn't included because I don't
    expect to use it soon.
    ORDER MATTERS for priorities
*/
binaryExpression:
    castExpression
    | lhs = binaryExpression op = (Star | Div | Mod) rhs = binaryExpression
    | lhs = binaryExpression op = (Plus | Minus) rhs = binaryExpression
    | lhs = binaryExpression (gr = Greater Greater | less = Less Less) rhs = binaryExpression
    | lhs = binaryExpression op = (Less | Greater | LessEqual | GreaterEqual) rhs = binaryExpression
    | lhs = binaryExpression op = (Equal | NotEqual) rhs = binaryExpression
    | lhs = binaryExpression op = And rhs = binaryExpression
    | lhs = binaryExpression op = Caret rhs = binaryExpression
    | lhs = binaryExpression op = Or rhs = binaryExpression
    | lhs = binaryExpression op = AndAnd rhs = binaryExpression
    | lhs = binaryExpression op = OrOr rhs = binaryExpression
    ;

assignmentOperator:
	op = (Assign
	| StarAssign
	| DivAssign
	| ModAssign
	| PlusAssign
	| MinusAssign
	| RightShiftAssign
	| LeftShiftAssign
	| AndAssign
	| XorAssign
	| OrAssign);

commaExpression: assignmentExpression (Comma assignmentExpression)*;

constantExpression: conditionalExpression;
/*Statements*/

statement:
	declarationStatement
	| expressionStatement
	| compoundStatement
	| selectionStatement
	| iterationStatement
	| jumpStatement
    ;

expressionStatement: commaExpression? Semi;

compoundStatement: LeftBrace statementSeq? RightBrace;

statementSeq: statement+;

selectionStatement:
	If LeftParen condition RightParen statement (Else statement)?;

condition:
	commaExpression;

iterationStatement:
	While LeftParen condition RightParen statement
	| Do statement While LeftParen condition RightParen Semi
	| For LeftParen (
		forInitStatement condition? Semi commaExpression?
	) RightParen statement;

forInitStatement: expressionStatement | simpleDeclaration;

jumpStatement:
	(
		Break IntegerLiteral?
		| Continue IntegerLiteral?
		| Return commaExpression?
	) Semi;

declarationStatement: simpleDeclaration;
/*Declarations*/

declarationseq: declaration+;

declaration:
	simpleDeclaration
	| functionDefinition
	| classDefinition
	| emptyDeclaration;

simpleDeclaration:
	declSpecifierSeq initDeclaratorList? Semi;

emptyDeclaration: Semi;

declSpecifier:
    typeSpecifier;

declSpecifierSeq: declSpecifier+;

typeSpecifier:
	simpleTypeSpecifier
//	| elaboratedTypeSpecifier
	| Const;

typeSpecifierSeq: typeSpecifier+;

simpleTypeSpecifier:
    {ty_exists()}? theTypeName
	| Char
	| Bool
	| Int
	| Double
	| Void;

theTypeName:
	className;

/*Declarators*/

initDeclaratorList: initDeclarator (Comma initDeclarator)*;

initDeclarator: declarator initializer?;

declarator: pointerDeclarator;

pointerDeclarator: pointerOperator* noPointerDeclarator;

noPointerDeclarator:
    // value
	declaratorID
	| noPointerDeclarator (
	    // function
		parametersAndQualifiers
		// array
		| LeftBracket constantExpression? RightBracket
	)
	// allows for function pointer, pointers to arrays
	| LeftParen pointerDeclarator RightParen;

parametersAndQualifiers:
	LeftParen parameterDeclarationClause? RightParen Const?;

pointerOperator:
	Star Const?;

declaratorID: idExpression;

// pointerOperator is here for simplicity right now
// see commit 189daff505 or the original grammar for
// the more complex version
theTypeId: typeSpecifierSeq pointerOperator*;

parameterDeclarationClause:
	parameterDeclarationList (Comma Ellipsis?)?;

parameterDeclarationList:
	parameterDeclaration (Comma parameterDeclaration)*;

parameterDeclaration:
	 declSpecifierSeq declarator (
			Assign assignmentExpression
		)?;

functionDefinition:
    // optional declSpecifierSeq necessary for constructors
	declSpecifierSeq? declarator functionBody;

functionBody:
	compoundStatement;

initializer:
    Assign assignmentExpression
	// constructor call?
	| LeftParen expressionList RightParen;

/*Classes*/

className: Identifier;

classDefinition:
	classHead LeftBrace memberSpecification? RightBrace;

classHead:
	classKey classHeadName {add_ty($classHeadName.text);} ;

// nested specifier for out-of-class nested class definition
classHeadName: className;

classKey: Class | Struct;

memberSpecification: memberSpecElem+;

memberSpecElem: memberdeclaration | accessSpecifier Colon;

memberdeclaration:
    // if constructor declaration (without definition) was allowed, declSpecifierSeq
    // should be optional
	declSpecifierSeq memberDeclaratorList Semi
	| functionDefinition
	| emptyDeclaration;

memberDeclaratorList:
	memberDeclarator (Comma memberDeclarator)*;

memberDeclarator:
	declarator;

accessSpecifier: Private | Public;

literal:
	IntegerLiteral
	| CharacterLiteral
	| FloatingLiteral
	| StringLiteral
	| BooleanLiteral
	| PointerLiteral;


/**** LEXER ****/

IntegerLiteral:
	DecimalLiteral;

CharacterLiteral:
	'\'' Cchar+ '\'';

FloatingLiteral:
	Fractionalconstant Exponentpart? Floatingsuffix?
	| Digitsequence Exponentpart Floatingsuffix?;

StringLiteral: '"' Schar* '"';

BooleanLiteral: False_ | True_;

PointerLiteral: Nullptr;

/*Keywords*/

Bool: 'bool';

Break: 'break';

Char: 'char';

Class: 'class';

Const: 'const';

Continue: 'continue';

Do: 'do';

Double: 'double';

Else: 'else';

//DO NOT RENAME - PYTHON NEEDS True and False
False_: 'false';

For: 'for';

If: 'if';

Int: 'int';

Nullptr: 'nullptr';

Private: 'private';

Public: 'public';

Return: 'return';

Sizeof: 'sizeof';

Struct: 'struct';

This: 'this';

//DO NOT RENAME - PYTHON NEEDS True and False
True_: 'true';

Void: 'void';

While: 'while';
/*Operators*/

LeftParen: '(';

RightParen: ')';

LeftBracket: '[';

RightBracket: ']';

LeftBrace: '{';

RightBrace: '}';

Plus: '+';

Minus: '-';

Star: '*';

Div: '/';

Mod: '%';

Caret: '^';

And: '&';

Or: '|';

Tilde: '~';

Not: '!' | 'not';

Assign: '=';

Less: '<';

Greater: '>';

PlusAssign: '+=';

MinusAssign: '-=';

StarAssign: '*=';

DivAssign: '/=';

ModAssign: '%=';

XorAssign: '^=';

AndAssign: '&=';

OrAssign: '|=';

LeftShiftAssign: '<<=';

RightShiftAssign: '>>=';

Equal: '==';

NotEqual: '!=';

LessEqual: '<=';

GreaterEqual: '>=';

AndAnd: '&&' | 'and';

OrOr: '||' | 'or';

PlusPlus: '++';

MinusMinus: '--';

Comma: ',';

Arrow: '->';

Question: '?';

Colon: ':';

Doublecolon: '::';

Semi: ';';

Dot: '.';

Ellipsis: '...';

Identifier:	Identifiernondigit (Identifiernondigit | DIGIT)*;

fragment Identifiernondigit: NONDIGIT;

fragment NONDIGIT: [a-zA-Z_];

fragment DIGIT: [0-9];

DecimalLiteral: DIGIT+;

OctalLiteral: '0' ('\''? OCTALDIGIT)*;

HexadecimalLiteral: ('0x' | '0X') HEXADECIMALDIGIT (
		'\''? HEXADECIMALDIGIT
	)*;

BinaryLiteral: ('0b' | '0B') BINARYDIGIT ('\''? BINARYDIGIT)*;

fragment NONZERODIGIT: [1-9];

fragment OCTALDIGIT: [0-7];

fragment HEXADECIMALDIGIT: [0-9a-fA-F];

fragment BINARYDIGIT: [01];

fragment Cchar:
	~ ['\\\r\n]
	| Escapesequence;

fragment Escapesequence:
	Simpleescapesequence
	| Octalescapesequence
	| Hexadecimalescapesequence;

fragment Simpleescapesequence:
	'\\\''
	| '\\"'
	| '\\?'
	| '\\\\'
	| '\\a'
	| '\\b'
	| '\\f'
	| '\\n'
	| '\\r'
	| ('\\' ('\r' '\n'? | '\n'))
	| '\\t'
	| '\\v';

fragment Octalescapesequence:
	'\\' OCTALDIGIT
	| '\\' OCTALDIGIT OCTALDIGIT
	| '\\' OCTALDIGIT OCTALDIGIT OCTALDIGIT;

fragment Hexadecimalescapesequence: '\\x' HEXADECIMALDIGIT+;

fragment Fractionalconstant:
	Digitsequence? '.' Digitsequence
	| Digitsequence '.';

fragment Exponentpart:
	'e' SIGN? Digitsequence
	| 'E' SIGN? Digitsequence;

fragment SIGN: [+-];

fragment Digitsequence: DIGIT ('\''? DIGIT)*;

fragment Floatingsuffix: [flFL];

fragment Encodingprefix: 'u8' | 'u' | 'U' | 'L';

fragment Schar:
	~ ["\\\r\n]
	| Escapesequence;

Whitespace: [ \t]+ -> skip;

Newline: ('\r' '\n'? | '\n') -> skip;

BlockComment: '/*' .*? '*/' -> skip;

LineComment: '//' ~ [\r\n]* -> skip;


