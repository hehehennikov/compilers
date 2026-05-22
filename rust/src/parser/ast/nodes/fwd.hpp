#pragma once

namespace parser::ast::nodes {

class Base;
class Expression;
class Statement;
class Declaration;
class Type;
class Pattern;
class ModuleItem;

class ModuleDecl;
class ImportDecl;
class ExportDecl;
class ExternDecl;
class Visibility;

class Attribute;
class Path;
class Lifetime;
class GenericParameter;
class RequiresClause;
class WherePredicate;
struct MatchArm;
struct StructField;
struct EnumVariant;

class LiteralExpr;
class IdentExpr;
class SelfExpr;
class BorrowExpr;
class UnitExpr;

class UnaryExpr;
class BinaryExpr;
class SpaceshipExpr;
class CastExpr;

class CallExpr;
class MethodCallExpr;
class MemberAccessExpr;
class IndexAccessExpr;

class BlockExpr;
class IfExpr;
class MatchExpr;
class LoopExpr;
class TryExpr;

class NewExpr;
class LambdaExpr;

class SizeofExpr;
class AlignofExpr;
class DecltypeExpr;
class ReflectExpr;

class ExprStmt;
class LetStmt;
class AssignStmt;
class WhileStmt;
class ForInStmt;
class BreakStmt;
class ContinueStmt;
class ReturnStmt;
class DeferStmt;
class StaticIfStmt;

class FuncDecl;
class StructDecl;
class EnumDecl;
class TraitDecl;
class ImplDecl;
class TypeAliasDecl;

class PrimitiveType;
class ReferenceType;
class PointerType;
class ArrayType;
class GenericType;
class FunctionType;
class PlaceholderType;

class WildcardPattern;
class BindingPattern;
class LiteralPattern;
class RangePattern;
class TuplePattern;
class StructPattern;
class ReferencePattern;

}  // namespace parser::ast::nodes