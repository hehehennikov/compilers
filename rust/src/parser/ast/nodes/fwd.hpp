#pragma once

namespace parser::ast::nodes {

class Base;
class Expression;
class Statement;
class Declaration;
class TypeNode;
class PatternNode;

// miscs
class Attribute;
class Lifetime;
class GenericParameter;
class WherePredicate;

// expressions
class LiteralExpr;
class IdentExpr;
class UnaryExpr;
class BinaryExpr;
class CastExpr;
class MemberAccessExpr;
class CallExpr;
class BlockExpr;
class IfExpr;
class MatchExpr;
class NewExpr;
class TryExpr;

// patterns
class WildcardPattern;
class BindingPattern;
class LiteralPattern;
class TuplePattern;
class StructPattern;

// types
class PrimitiveType;
class ReferenceType;
class PointerType;
class ArrayType;
class GenericType;
class FunctionType;

// statements
class LetStmt;
class AssignStmt;
class ReturnStmt;
class WhileStmt;
class LoopStmt;
class ForInStmt;
class BreakStmt;
class ContinueStmt;
class StaticIfStmt;
class ExprStmt;

// declarations
class FuncDecl;
class StructDecl;
class EnumDecl;
class ImplDecl;
class FunctionDecl;
class TraitDecl;
class TypeAliasDecl;

}  // namespace parser::ast::nodes