#include "parser/parser.hpp"

#include <common/diagnostic.hpp>

#include <parser/ast/nodes/misc.hpp>
#include <parser/ast/nodes/expressions.hpp>
#include <parser/ast/nodes/types.hpp>
#include <parser/ast/nodes/pattern.hpp>
#include <parser/ast/nodes/statements.hpp>
#include <parser/ast/nodes/declarations.hpp>

namespace parser {

using namespace ast::nodes;

Parser::Parser(lexer::Lexer& lexer) : lexer_(lexer) {
  Advance();
}

// cursor
void Parser::Advance() {
  prev_tok_ = cur_tok_;
  auto res = lexer_.NextToken();
  if (res) {
    cur_tok_ = std::move(*res);
  } else {
    ///TODO:
    exit(1);
  }
}

bool Parser::Check(lexer::TokenType type) const {
  return cur_tok_.type == type;
}

bool Parser::Match(lexer::TokenType type) {
  if (Check(type)) {
    Advance();

    return true;
  }
  return false;
}

void Parser::Expect(lexer::TokenType type, std::string_view message) {
  if (Check(type)) {
    Advance();

    return;
  }

  common::DiagnosticEngine::GetInstance().Report(
      common::DiagnosticLevel::Error, cur_tok_.loc, std::string(message));
}

Precedence Parser::GetCurrentPrecedence() {
  auto it = Precedences.find(cur_tok_.type);
  if (it != Precedences.end()) {
    return it->second;
  }

  return Precedence::LOWEST;
}

bool Parser::IsAtEnd() const {
  return cur_tok_.type == lexer::TokenType::Eof;
}

std::vector<std::unique_ptr<Statement>> Parser::ParseProgram() {
  std::vector<std::unique_ptr<Statement>> program;

  while (not IsAtEnd()) {
    auto stmt = ParseStatement();
    if (stmt) {
      program.push_back(std::move(stmt));
    }
  }

  return program;
}

std::unique_ptr<Statement> Parser::ParseStatement() {
  auto attrs = ParseAttributes();

  std::unique_ptr<Statement> stmt;
  switch (cur_tok_.type) {
    case lexer::TokenType::Let:
      stmt = ParseLet();
      break;
    case lexer::TokenType::Return:
      stmt = ParseReturn();
      break;
    case lexer::TokenType::While:
      stmt = ParseWhile();
      break;
    case lexer::TokenType::Loop:
      stmt = ParseLoop();
      break;
    case lexer::TokenType::For:
      stmt = ParseForIn();
      break;
    case lexer::TokenType::Break:
      stmt = ParseBreak();
      break;
    case lexer::TokenType::Continue:
      stmt = ParseContinue();
      break;
    case lexer::TokenType::StaticIf:
      stmt = ParseStaticIf();
      break;

    case lexer::TokenType::Func:
      stmt = ParseFuncDecl();
      break;
    case lexer::TokenType::Struct:
      stmt = ParseStructDecl();
      break;
    case lexer::TokenType::Enum:
      stmt = ParseEnumDecl();
      break;
    case lexer::TokenType::Trait:
      stmt = ParseTraitDecl();
      break;
    case lexer::TokenType::Impl:
      stmt = ParseImplDecl();
      break;
    case lexer::TokenType::Using:
      stmt = ParseTypeAlias();
      break;

    default:
      stmt = ParseAssignOrExprStmt();
      break;
  }

  if (stmt && !attrs.empty()) {
    stmt->attrs = std::move(attrs);
  }

  return stmt;
}

std::unique_ptr<WhileStmt> Parser::ParseWhile() {
  auto loc = cur_tok_.loc;
  Advance(); // 'while'
  auto cond = ParseExpression(Precedence::LOWEST);
  auto body = ParseBlockExpr();
  return std::make_unique<WhileStmt>(std::move(cond), std::move(body), loc);
}

std::unique_ptr<ForInStmt> Parser::ParseForIn() {
  auto loc = cur_tok_.loc;
  Advance(); // 'for'
  auto pat = ParsePattern();
  Expect(lexer::TokenType::In, "Expected 'in' after pattern");
  auto iter = ParseExpression(Precedence::LOWEST);
  auto body = ParseBlockExpr();
  return std::make_unique<ForInStmt>(std::move(pat), std::move(iter), std::move(body), loc);
}

std::unique_ptr<ReturnStmt> Parser::ParseReturn() {
  auto loc = cur_tok_.loc;
  Advance(); // 'return'
  std::unique_ptr<Expression> val = nullptr;
  if (!Check(lexer::TokenType::Semi)) val = ParseExpression(Precedence::LOWEST);
  Expect(lexer::TokenType::Semi, "Expected ';' after return");
  return std::make_unique<ReturnStmt>(std::move(val), loc);
}

std::unique_ptr<BreakStmt> Parser::ParseBreak() {
  auto loc = cur_tok_.loc;
  Advance(); Expect(lexer::TokenType::Semi, "Expected ';'");
  return std::make_unique<BreakStmt>(loc);
}

std::unique_ptr<ContinueStmt> Parser::ParseContinue() {
  auto loc = cur_tok_.loc;
  Advance(); Expect(lexer::TokenType::Semi, "Expected ';'");
  return std::make_unique<ContinueStmt>(loc);
}

std::unique_ptr<LetStmt> Parser::ParseLet() {
  auto loc = cur_tok_.loc;
  Advance(); // 'let'

  bool is_mut = Match(lexer::TokenType::Mut);
  bool is_lazy = Match(lexer::TokenType::Lazy);

  auto pat = ParsePattern();

  std::unique_ptr<TypeNode> type = nullptr;
  if (Match(lexer::TokenType::Colon)) {
    type = ParseType();
  }

  std::unique_ptr<Expression> init = nullptr;
  if (Match(lexer::TokenType::Equal)) {
    init = ParseExpression(Precedence::LOWEST);
  }

  Expect(lexer::TokenType::Semi, "Expected ';' after let statement");
  return std::make_unique<LetStmt>(std::move(pat), is_mut, is_lazy, std::move(type), std::move(init), loc);
}

std::unique_ptr<Statement> Parser::ParseAssignOrExprStmt() {
  auto loc = cur_tok_.loc;
  auto expr = ParseExpression(Precedence::LOWEST);

  if (Match(lexer::TokenType::Equal)) {
    auto rhs = ParseExpression(Precedence::LOWEST);
    Expect(lexer::TokenType::Semi, "Expected ';' after assignment");
    return std::make_unique<AssignStmt>(std::move(expr), std::move(rhs), loc);
  }

  Match(lexer::TokenType::Semi);
  return std::make_unique<ExprStmt>(std::move(expr), loc);
}

// exprs
std::unique_ptr<Expression> Parser::ParseExpression(Precedence precedence) {
  auto left = ParsePrefix();
  if (not left) {
    return nullptr;
  }

  while (not IsAtEnd() && (int)precedence < (int)GetCurrentPrecedence()) {
    left = ParseInfix(std::move(left));
  }

  return left;
}

std::unique_ptr<MatchExpr> Parser::ParseMatchExpr() {
  auto loc = cur_tok_.loc;
  Advance(); // 'match'
  auto value = ParseExpression(Precedence::LOWEST);
  Expect(lexer::TokenType::LBrace, "Expected '{' after match value");

  std::vector<MatchExpr::Arm> arms;
  while (!Check(lexer::TokenType::RBrace) && !IsAtEnd()) {
    auto pat = ParsePattern();
    Expect(lexer::TokenType::FatArrow, "Expected '=>' after pattern");
    auto body = ParseExpression(Precedence::LOWEST);
    arms.push_back({std::move(pat), std::move(body)});
    Match(lexer::TokenType::Comma);
  }
  Expect(lexer::TokenType::RBrace, "Expected '}' after match arms");
  return std::make_unique<MatchExpr>(std::move(value), std::move(arms), loc);
}


std::unique_ptr<NewExpr> Parser::ParseNewExpr() {
  auto loc = cur_tok_.loc;
  Advance(); // 'new'
  auto type = ParseType();

  std::vector<std::unique_ptr<Expression>> args;
  if (Match(lexer::TokenType::LParen)) {
    if (!Check(lexer::TokenType::RParen)) {
      do { args.push_back(ParseExpression(Precedence::LOWEST)); } while (Match(lexer::TokenType::Comma));
    }
    Expect(lexer::TokenType::RParen, "Expected ')'");
  }
  return std::make_unique<NewExpr>(std::move(type), std::move(args), loc);
}

std::unique_ptr<Expression> Parser::ParsePrefix() {
  auto loc = cur_tok_.loc;
  auto type = cur_tok_.type;

  // literals
  if (IsLiteral(type)) {
    auto node = std::make_unique<LiteralExpr>(cur_tok_.data, loc);
    Advance();
    return node;
  }

  // idents
  if (type == lexer::TokenType::Identifier) {
    auto name = cur_tok_.As<std::string>().value().get();
    Advance();
    return std::make_unique<IdentExpr>(name, loc);
  }

  // parentheses
  if (Match(lexer::TokenType::LParen)) {
    auto expr = ParseExpression(Precedence::LOWEST);
    Expect(lexer::TokenType::RParen, "Expected ')'");
    return expr;
  }

  // unary
  if (type == lexer::TokenType::Minus || type == lexer::TokenType::Exclamation ||
      type == lexer::TokenType::Ampersand || type == lexer::TokenType::Star) {
    Advance();
    return std::make_unique<UnaryExpr>(type, ParseExpression(Precedence::PREFIX), loc);
  }

  // complex
  if (type == lexer::TokenType::LBrace) {
    return ParseBlockExpr();
  }
  if (Check(lexer::TokenType::If)) {
    return ParseIfExpr();
  }
  if (Check(lexer::TokenType::Match)) {
    return ParseMatchExpr();
  }
  if (Check(lexer::TokenType::New)) {
    return ParseNewExpr();
  }

  return nullptr;
}

std::unique_ptr<Expression> Parser::ParseInfix(std::unique_ptr<Expression> left) {
  auto loc = cur_tok_.loc;
  auto op = cur_tok_.type;
  auto prec = GetCurrentPrecedence();
  Advance();

  switch (op) {
    case lexer::TokenType::As:
      return std::make_unique<CastExpr>(std::move(left), ParseType(), loc);

    case lexer::TokenType::LParen:
      return ParseCallArguments(std::move(left));

    case lexer::TokenType::Dot: {
      auto member = cur_tok_.As<std::string>().value().get();
      Advance();
      if (Check(lexer::TokenType::LParen)) {
         //TODO: MethodCallExpr
      }
      return std::make_unique<MemberAccessExpr>(std::move(left), member, loc);
    }

    case lexer::TokenType::Question:
      return std::make_unique<TryExpr>(std::move(left), loc);

    default:
      return std::make_unique<BinaryExpr>(std::move(left), op, ParseExpression(prec), loc);
  }
}

std::unique_ptr<BlockExpr> Parser::ParseBlockExpr() {
  auto loc = cur_tok_.loc;
  Expect(lexer::TokenType::LBrace, "Expected '{'");

  std::vector<std::unique_ptr<Statement>> stmts;
  std::unique_ptr<Expression> final_expr = nullptr;

  while (not Check(lexer::TokenType::RBrace) && !IsAtEnd()) {
    if (Check(lexer::TokenType::Let) || Check(lexer::TokenType::Return)) {
      stmts.push_back(ParseStatement());
    } else {
      auto expr = ParseExpression(Precedence::LOWEST);
      if (Match(lexer::TokenType::Semi)) {
        stmts.push_back(std::make_unique<ExprStmt>(std::move(expr), loc));
      } else {
        final_expr = std::move(expr);
        break;
      }
    }
  }
  Expect(lexer::TokenType::RBrace, "Expected '}'");
  return std::make_unique<BlockExpr>(std::move(stmts), std::move(final_expr), loc);
}

std::unique_ptr<IfExpr> Parser::ParseIfExpr() {
  auto loc = cur_tok_.loc;
  Advance(); // 'if'
  auto cond = ParseExpression(Precedence::LOWEST);
  auto then_b = ParseBlockExpr();
  std::unique_ptr<BlockExpr> else_b = nullptr;
  if (Match(lexer::TokenType::Else)) {
    if (Check(lexer::TokenType::If)) {
       // logic for else if...
    } else {
      else_b = ParseBlockExpr();
    }
  }
  return std::make_unique<IfExpr>(std::move(cond), std::move(then_b), std::move(else_b), loc);
}

// decls
std::unique_ptr<FuncDecl> Parser::ParseFuncDecl() {
  auto loc = cur_tok_.loc;
  Advance(); // 'func'
  std::string name = cur_tok_.As<std::string>().value().get();
  Advance();

  // generics
  if (Match(lexer::TokenType::Less)) { /* ParseGenericParameters() */ }

  // parameters
  Expect(lexer::TokenType::LParen, "Expected '('");
  std::vector<Parameter> params;
  if (not Check(lexer::TokenType::RParen)) {
    do {
      auto p_pat = ParsePattern();
      Expect(lexer::TokenType::Colon, "Expected ':'");
      auto p_type = ParseType();
      params.push_back({std::move(p_pat), std::move(p_type)});
    } while (Match(lexer::TokenType::Comma));
  }
  Expect(lexer::TokenType::RParen, "Expected ')'");

  // return type
  std::unique_ptr<TypeNode> ret = nullptr;
  if (Match(lexer::TokenType::Arrow)) {
    ret = ParseType();
  }

  // where clause
  if (Check(lexer::TokenType::Where)) { /* ParseWhereClause() */ }

  // body
  auto body = ParseBlockExpr();
  return std::make_unique<FuncDecl>(name, std::move(params), std::move(ret), std::move(body), loc);
}

std::unique_ptr<EnumDecl> Parser::ParseEnumDecl() {
  auto loc = cur_tok_.loc;
  Advance(); // 'enum'
  std::string name = cur_tok_.As<std::string>().value().get();
  Advance();
  Expect(lexer::TokenType::LBrace, "Expected '{'");

  std::vector<EnumVariant> variants;
  while (!Check(lexer::TokenType::RBrace)) {
    std::string v_name = cur_tok_.As<std::string>().value().get();
    Advance();
    std::vector<std::unique_ptr<TypeNode>> types;
    if (Match(lexer::TokenType::LParen)) {
      do { types.push_back(ParseType()); } while (Match(lexer::TokenType::Comma));
      Expect(lexer::TokenType::RParen, "Expected ')'");
    }
    variants.push_back({v_name, std::move(types)});
    Match(lexer::TokenType::Comma);
  }
  Expect(lexer::TokenType::RBrace, "Expected '}'");
  return std::make_unique<EnumDecl>(name, std::move(variants), loc);
}

std::unique_ptr<ImplDecl> Parser::ParseImplDecl() {
  auto loc = cur_tok_.loc;
  Advance(); // 'impl'
  auto first_type = ParseType();

  std::unique_ptr<TypeNode> trait = nullptr;
  std::unique_ptr<TypeNode> target = nullptr;

  if (Match(lexer::TokenType::For)) {
    trait = std::move(first_type);
    target = ParseType();
  } else {
    target = std::move(first_type);
  }

  Expect(lexer::TokenType::LBrace, "Expected '{'");
  std::vector<std::unique_ptr<FuncDecl>> methods;
  while (!Check(lexer::TokenType::RBrace)) {
    methods.push_back(ParseFuncDecl());
  }
  Expect(lexer::TokenType::RBrace, "Expected '}'");
  return std::make_unique<ImplDecl>(std::move(trait), std::move(target), std::move(methods), loc);
}

std::unique_ptr<TypeAliasDecl> Parser::ParseTypeAlias() {
  auto loc = cur_tok_.loc;
  Advance(); // 'using'
  std::string name = cur_tok_.As<std::string>().value().get();
  Advance();
  Expect(lexer::TokenType::Equal, "Expected '='");
  auto target = ParseType();
  Expect(lexer::TokenType::Semi, "Expected ';'");
  return std::make_unique<TypeAliasDecl>(name, std::move(target), loc);
}

// types & patterns
std::unique_ptr<TypeNode> Parser::ParseType() {
  auto loc = cur_tok_.loc;
  if (IsPrimitiveType(cur_tok_.type)) {
    auto t = std::make_unique<PrimitiveType>(cur_tok_.type, loc);
    Advance();
    return t;
  }
  if (Match(lexer::TokenType::Ampersand)) {
    bool is_mut = Match(lexer::TokenType::Mut);
    return std::make_unique<ReferenceType>(is_mut, nullptr, ParseType(), loc);
  }
  if (Match(lexer::TokenType::Identifier)) {
    std::string name = prev_tok_.As<std::string>().value().get();
    if (Match(lexer::TokenType::Less)) { // GenericType<T>
       std::vector<std::unique_ptr<TypeNode>> args;
       do { args.push_back(ParseType()); } while (Match(lexer::TokenType::Comma));
       Expect(lexer::TokenType::Greater, "Expected '>'");
       return std::make_unique<GenericType>(name, std::move(args), loc);
    }
    return std::make_unique<PrimitiveType>(lexer::TokenType::Identifier, loc); // fallback to user type
  }
  return nullptr;
}

std::unique_ptr<FunctionType> Parser::ParseFunctionType() {
  auto loc = cur_tok_.loc;
  Advance(); // 'func'
  Expect(lexer::TokenType::LParen, "Expected '('");
  std::vector<std::unique_ptr<TypeNode>> params;
  if (!Check(lexer::TokenType::RParen)) {
    do { params.push_back(ParseType()); } while (Match(lexer::TokenType::Comma));
  }
  Expect(lexer::TokenType::RParen, "Expected ')'");
  Expect(lexer::TokenType::Arrow, "Expected '->'");
  return std::make_unique<FunctionType>(std::move(params), ParseType(), loc);
}

std::unique_ptr<PatternNode> Parser::ParsePattern() {
  auto loc = cur_tok_.loc;
  if (Match(lexer::TokenType::Underscore)) return std::make_unique<WildcardPattern>(loc);
  if (Check(lexer::TokenType::Identifier)) {
    auto name = cur_tok_.As<std::string>().value().get();
    Advance();
    return std::make_unique<BindingPattern>(name, loc);
  }
  return nullptr;
}

// helpers
bool Parser::IsLiteral(lexer::TokenType type) const {
  return type == lexer::TokenType::LiteralInt || type == lexer::TokenType::LiteralFloat ||
         type == lexer::TokenType::LiteralBool || type == lexer::TokenType::LiteralString;
}

bool Parser::IsPrimitiveType(lexer::TokenType type) const {
  return (int)type >= (int)lexer::TokenType::Int8 && (int)type <= (int)lexer::TokenType::USize;
}

std::vector<std::unique_ptr<Attribute>> Parser::ParseAttributes() {
  std::vector<std::unique_ptr<Attribute>> attrs;
  while (Check(lexer::TokenType::Hash)) {
    attrs.push_back(ParseSingleAttribute());
  }
  return attrs;
}

std::unique_ptr<Attribute> Parser::ParseSingleAttribute() {
  auto loc = cur_tok_.loc;
  Advance(); // '#'
  Expect(lexer::TokenType::LBracket, "Expected '[' after '#'");
  std::string name = cur_tok_.As<std::string>().value().get();
  Advance();
  ///TODO: attr args
  Expect(lexer::TokenType::RBracket, "Expected ']'");
  return std::make_unique<Attribute>(name, std::vector<std::unique_ptr<Expression>>{}, loc);
}

std::vector<std::unique_ptr<GenericParameter>> Parser::ParseGenericParameters() {
  std::vector<std::unique_ptr<GenericParameter>> params;
  if (Match(lexer::TokenType::Less)) {
    do {
      auto loc = cur_tok_.loc;
      std::string name = cur_tok_.As<std::string>().value().get();
      Advance();
      std::vector<std::unique_ptr<TypeNode>> bounds;
      if (Match(lexer::TokenType::Colon)) {
        do { bounds.push_back(ParseType()); } while (Match(lexer::TokenType::Plus));
      }
      params.push_back(std::make_unique<GenericParameter>(name, std::move(bounds), loc));
    } while (Match(lexer::TokenType::Comma));
    Expect(lexer::TokenType::Greater, "Expected '>'");
  }
  return params;
}

std::vector<std::unique_ptr<WherePredicate>> Parser::ParseWhereClause() {
  std::vector<std::unique_ptr<WherePredicate>> predicates;
  if (Match(lexer::TokenType::Where)) {
    do {
      auto loc = cur_tok_.loc;
      auto target = ParseType();
      Expect(lexer::TokenType::Colon, "Expected ':'");
      std::vector<std::unique_ptr<TypeNode>> bounds;
      do { bounds.push_back(ParseType()); } while (Match(lexer::TokenType::Plus));
      predicates.push_back(std::make_unique<WherePredicate>(std::move(target), std::move(bounds), loc));
    } while (Match(lexer::TokenType::Comma));
  }
  return predicates;
}

std::unique_ptr<Lifetime> Parser::ParseLifetime() {
  auto loc = cur_tok_.loc;
  std::string name = cur_tok_.As<std::string>().value().get();
  Advance();
  return std::make_unique<Lifetime>(name, loc);
}

} // namespace parser