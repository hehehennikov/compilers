#include "parser/parser.hpp"
#include "parser/precedence.hpp"
#include "common/diagnostic.hpp"
#include "ast/nodes/all.hpp"

#include <algorithm>

namespace parser {

using namespace ast::nodes;

bool IsAssignmentOperator(lexer::TokenType type) {
  return type == lexer::TokenType::Equal ||
         type == lexer::TokenType::PlusEq ||
         type == lexer::TokenType::MinusEq ||
         type == lexer::TokenType::StarEq ||
         type == lexer::TokenType::SlashEq ||
         type == lexer::TokenType::AmpersandEq ||
         type == lexer::TokenType::PipeEq ||
         type == lexer::TokenType::CaretEq ||
         type == lexer::TokenType::LessLessEq ||
         type == lexer::TokenType::GreaterGreaterEq;
}

bool IsPrefixOperator(lexer::TokenType type) {
  return type == lexer::TokenType::Minus ||
         type == lexer::TokenType::Exclamation ||
         type == lexer::TokenType::Ampersand ||
         type == lexer::TokenType::Star ||
         type == lexer::TokenType::Tilda ||
         type == lexer::TokenType::LogicNot;
}

Parser::Parser(lexer::Lexer& lexer) : lexer_(lexer) {
  Advance();
}

void Parser::Advance() {
  previous_token_ = std::move(current_token_);

  auto result = lexer_.NextToken();
  if (result.has_value()) {
    current_token_ = std::move(result.value());
  } else {
    std::exit(1);
  }
}

bool Parser::Check(lexer::TokenType type) const {
  return current_token_.type == type;
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
      common::DiagnosticLevel::Error,
      current_token_.loc,
      std::string(message)
  );
}

bool Parser::IsAtEnd() const {
  return current_token_.type == lexer::TokenType::Eof;
}

std::vector<std::unique_ptr<Base>> Parser::ParseProgram() {
  std::vector<std::unique_ptr<Base>> program;

  while (not IsAtEnd()) {
    auto item = ParseTopLevelItem();

    if (item != nullptr) {
      program.push_back(std::move(item));
    } else {
      // to prevent infinite loop on syntax errors
      Synchronize();
    }
  }

  return program;
}

std::unique_ptr<Base> Parser::ParseTopLevelItem() {
  auto attrs = ParseAttributes();
  std::unique_ptr<Base> result = nullptr;

  switch (current_token_.type) {
    case lexer::TokenType::Module:
      result = ParseModuleDecl();
      break;
    case lexer::TokenType::Import:
      result = ParseImportDecl();
      break;
    case lexer::TokenType::Export:
      result = ParseExportDecl();
      break;
    default:
      result = ParseStatement();
      break;
  }

  if (result != nullptr && not attrs.empty()) {
    result->attrs = std::move(attrs);
  }

  return result;
}

void Parser::Synchronize() {
  Advance();

  while (not IsAtEnd()) {
    if (previous_token_.type == lexer::TokenType::Semi) {
      return;
    }

    switch (current_token_.type) {
      case lexer::TokenType::Let:
      case lexer::TokenType::Func:
      case lexer::TokenType::Struct:
      case lexer::TokenType::Trait:
      case lexer::TokenType::Return:
      case lexer::TokenType::If:
        return;
      default:
        Advance();
    }
  }
}

std::unique_ptr<Statement> Parser::ParseStatement() {
  auto attrs = ParseAttributes();

  std::unique_ptr<Statement> result = nullptr;

  switch (current_token_.type) {
    case lexer::TokenType::Let:
      result = ParseLet();
      break;
    case lexer::TokenType::Return:
      result = ParseReturn();
      break;
    case lexer::TokenType::While:
      result = ParseWhile();
      break;
    case lexer::TokenType::For:
      result = ParseForIn();
      break;
    case lexer::TokenType::Break:
      result = ParseBreak();
      break;
    case lexer::TokenType::Continue:
      result = ParseContinue();
      break;
    case lexer::TokenType::Defer:
      result = ParseDefer();
      break;
    case lexer::TokenType::Static:
      result = ParseStaticIf();
      break;
    case lexer::TokenType::Func:
      result = ParseFuncDecl();
      break;
    case lexer::TokenType::Struct:
      result = ParseStructDecl();
      break;
    case lexer::TokenType::Enum:
      result = ParseEnumDecl();
      break;
    case lexer::TokenType::Trait:
      result = ParseTraitDecl();
      break;
    case lexer::TokenType::Impl:
      result = ParseImplDecl();
      break;
    case lexer::TokenType::Using:
      result = ParseTypeAlias();
      break;
    default:
      result = ParseAssignOrExprStmt();
      break;
  }

  if (result != nullptr && not attrs.empty()) {
    result->attrs = std::move(attrs);
  }

  return result;
}

std::unique_ptr<LetStmt> Parser::ParseLet() {
  auto loc = current_token_.loc;
  Advance(); // consume `let`

  bool is_mut = Match(lexer::TokenType::Mut);
  auto pat = ParsePattern();

  std::unique_ptr<Type> type = nullptr;
  if (Match(lexer::TokenType::Colon)) {
    type = ParseType();
  }

  std::unique_ptr<Expression> init = nullptr;
  if (Match(lexer::TokenType::Equal)) {
    init = ParseExpression(Precedence::LOWEST);
  }

  Expect(lexer::TokenType::Semi, "Expected ';' after let statement");
  return std::make_unique<LetStmt>(std::move(pat), is_mut, std::move(type), std::move(init), loc);
}

std::unique_ptr<ReturnStmt> Parser::ParseReturn() {
  auto location = current_token_.loc;
  Advance(); // consume `return`

  std::unique_ptr<Expression> value = nullptr;

  // semicolon = empty return (return unit)
  if (not Check(lexer::TokenType::Semi)) {
    value = ParseExpression(Precedence::LOWEST);
  }

  Expect(lexer::TokenType::Semi, "Expected ';' after return statement");

  return std::make_unique<ReturnStmt>(std::move(value), location);
}

std::unique_ptr<WhileStmt> Parser::ParseWhile() {
  auto location = current_token_.loc;
  Advance(); // consume 'while'

  auto condition = ParseExpression(Precedence::LOWEST);
  auto body = ParseBlockExpr();

  return std::make_unique<WhileStmt>(
      std::move(condition),
      std::move(body),
      location
  );
}

std::unique_ptr<ForInStmt> Parser::ParseForIn() {
  auto location = current_token_.loc;
  Advance(); // consume 'for'

  auto pattern = ParsePattern();
  Expect(lexer::TokenType::In, "Expected 'in' keyword in for loop");

  auto iterable = ParseExpression(Precedence::LOWEST);
  auto body = ParseBlockExpr();

  return std::make_unique<ForInStmt>(
      std::move(pattern),
      std::move(iterable),
      std::move(body),
      location
  );
}

std::unique_ptr<BreakStmt> Parser::ParseBreak() {
  auto location = current_token_.loc;
  Advance(); // consume 'break'

  std::unique_ptr<Expression> value = nullptr;

  // break can return a value from a loop block
  if (not Check(lexer::TokenType::Semi)) {
    value = ParseExpression(Precedence::LOWEST);
  }

  Expect(lexer::TokenType::Semi, "Expected ';' after break");

  return std::make_unique<BreakStmt>(std::move(value), location);
}

std::unique_ptr<ContinueStmt> Parser::ParseContinue() {
  auto location = current_token_.loc;
  Advance(); // consume 'continue'

  Expect(lexer::TokenType::Semi, "Expected ';' after continue");

  return std::make_unique<ContinueStmt>(location);
}

std::unique_ptr<DeferStmt> Parser::ParseDefer() {
  auto location = current_token_.loc;
  Advance(); // consume 'defer'

  auto body = ParseBlockExpr();

  return std::make_unique<DeferStmt>(std::move(body), location);
}


std::unique_ptr<Statement> Parser::ParseAssignOrExprStmt() {
  auto start_location = current_token_.loc;
  auto expr = ParseExpression(Precedence::LOWEST);

  if (expr == nullptr) {
    return nullptr;
  }

  if (IsAssignmentOperator(current_token_.type)) {
    auto op = current_token_.type;
    Advance();

    auto rhs = ParseExpression(Precedence::LOWEST);
    Expect(lexer::TokenType::Semi, "Expected ';' after assignment");

    return std::make_unique<AssignStmt>(std::move(expr), op, std::move(rhs), start_location);
  }

  if (Match(lexer::TokenType::Semi)) {
    return std::make_unique<ExprStmt>(std::move(expr), start_location);
  }

  return std::make_unique<ExprStmt>(std::move(expr), start_location);
}

std::unique_ptr<Expression> Parser::ParseExpression(Precedence precedence) {
  auto left = ParsePrefix();
  if (left == nullptr) {
    return nullptr;
  }

  while (not IsAtEnd() && static_cast<int>(precedence) < static_cast<int>(GetCurrentPrecedence())) {
    left = ParseInfix(std::move(left));
  }

  return left;
}

std::unique_ptr<Expression> Parser::ParsePrefix() {
  auto location = current_token_.loc;
  auto type = current_token_.type;

  // Literals & IDs
  if (IsLiteral(type)) {
    auto node = std::make_unique<LiteralExpr>(current_token_.data, location);
    Advance();
    return node;
  }

  if (Check(lexer::TokenType::Identifier)) {
    auto path = ParsePath();
    return std::make_unique<IdentExpr>(path->segments.back().identifier, location);
  }

  // ALL Unary operators
  if (IsPrefixOperator(type)) {
    Advance();
    // Use PREFIX precedence to handle things like -x.field correctly
    auto operand = ParseExpression(Precedence::PREFIX);
    return std::make_unique<UnaryExpr>(type, std::move(operand), location);
  }

  // Parentheses
  if (Match(lexer::TokenType::LParen)) {
    auto expr = ParseExpression(Precedence::LOWEST);
    Expect(lexer::TokenType::RParen, "Expected ')' after grouped expression");
    return expr;
  }

  // Control Flow / Blocks
  if (type == lexer::TokenType::LBrace) return ParseBlockExpr();
  if (Check(lexer::TokenType::If))      return ParseIfExpr();
  if (Check(lexer::TokenType::Match))   return ParseMatchExpr();
  if (Check(lexer::TokenType::Loop))    return ParseLoopExpr();
  if (Check(lexer::TokenType::New))     return ParseNewExpr();

  // Systems
  if (Match(lexer::TokenType::Sizeof)) {
    Expect(lexer::TokenType::LParen, "Expected '('");
    auto t = ParseType();
    Expect(lexer::TokenType::RParen, "Expected ')'");
    return std::make_unique<SizeofExpr>(std::move(t), location);
  }

  common::DiagnosticEngine::GetInstance().Report(
      common::DiagnosticLevel::Error, location, "Unexpected token at start of expression");

  return nullptr;
}

std::unique_ptr<Expression> Parser::ParseInfix(std::unique_ptr<Expression> left) {
  auto loc = current_token_.loc;
  auto op = current_token_.type;
  auto prec = GetCurrentPrecedence();
  Advance();

  switch (op) {
    case lexer::TokenType::As:
      return std::make_unique<CastExpr>(std::move(left), ParseType(), loc);

    case lexer::TokenType::LParen: {
      std::vector<std::unique_ptr<Expression>> args;
      if (not Check(lexer::TokenType::RParen)) {
        do { args.push_back(ParseExpression(Precedence::LOWEST)); } while (Match(lexer::TokenType::Comma));
      }
      Expect(lexer::TokenType::RParen, "Expected ')'");
      return std::make_unique<CallExpr>(std::move(left), std::move(args), loc);
    }

    case lexer::TokenType::Dot: {
      if (not Check(lexer::TokenType::Identifier) && !Check(lexer::TokenType::Move)) {
        return nullptr;
      }
      std::string member = (current_token_.type == lexer::TokenType::Move) ? "move" : std::get<std::string>(current_token_.data);
      Advance();
      if (Match(lexer::TokenType::LParen)) {
          std::vector<std::unique_ptr<Expression>> args;
          if (not Check(lexer::TokenType::RParen)) {
            do { args.push_back(ParseExpression(Precedence::LOWEST)); } while (Match(lexer::TokenType::Comma));
          }
          Expect(lexer::TokenType::RParen, "Expected ')'");
          return std::make_unique<MethodCallExpr>(std::move(left), member, std::move(args), loc);
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
  auto location = current_token_.loc;
  Expect(lexer::TokenType::LBrace, "Expected '{' to start a block");

  std::vector<std::unique_ptr<Statement>> statements;
  std::unique_ptr<Expression> final_expression = nullptr;

  while (!Check(lexer::TokenType::RBrace) && !IsAtEnd()) {
    if (Check(lexer::TokenType::Let) || Check(lexer::TokenType::While) ||
        Check(lexer::TokenType::Return) || Check(lexer::TokenType::If) ||
        Check(lexer::TokenType::For)) {

      auto stmt = ParseStatement();
      if (stmt != nullptr) {
        statements.push_back(std::move(stmt));
      }
      continue;
        }

    auto expr = ParseExpression(Precedence::LOWEST);
    if (expr == nullptr) {
      Advance();
      continue;
    }

    if (Match(lexer::TokenType::Semi)) {
      statements.push_back(std::make_unique<ExprStmt>(std::move(expr), location));
    } else {
      if (Check(lexer::TokenType::RBrace)) {
        final_expression = std::move(expr);
      } else {
        common::DiagnosticEngine::GetInstance().Report(
            common::DiagnosticLevel::Error, current_token_.loc, "Expected ';' after expression");
        statements.push_back(std::make_unique<ExprStmt>(std::move(expr), location));
      }
    }
  }

  Expect(lexer::TokenType::RBrace, "Expected '}' after block");

  return std::make_unique<BlockExpr>(
      std::move(statements),
      std::move(final_expression),
      location
  );
}

std::unique_ptr<IfExpr> Parser::ParseIfExpr() {
  auto location = current_token_.loc;
  Advance(); // consume 'if'

  auto condition = ParseExpression(Precedence::LOWEST);
  auto then_branch = ParseBlockExpr();

  std::unique_ptr<BlockExpr> else_branch = nullptr;

  if (Match(lexer::TokenType::Else)) {
    // handle 'else if' recursion
    if (Check(lexer::TokenType::If)) {
      std::vector<std::unique_ptr<Statement>> nested_stmts;
      auto nested_if = ParseIfExpr();
      auto nested_loc = nested_if->location;

      nested_stmts.push_back(std::make_unique<ExprStmt>(std::move(nested_if), nested_loc));

      else_branch = std::make_unique<BlockExpr>(
          std::move(nested_stmts),
          nullptr,
          nested_loc
      );
    } else {
      else_branch = ParseBlockExpr();
    }
  }

  return std::make_unique<IfExpr>(
      std::move(condition),
      std::move(then_branch),
      std::move(else_branch),
      location
  );
}

std::unique_ptr<LoopExpr> Parser::ParseLoopExpr() {
  auto location = current_token_.loc;
  Advance(); // consume 'loop'

  auto body = ParseBlockExpr();

  return std::make_unique<LoopExpr>(std::move(body), location);
}

std::unique_ptr<FuncDecl> Parser::ParseFuncDecl() {
  auto loc = current_token_.loc;
  Advance(); // 'func'
  std::string name = std::get<std::string>(current_token_.data);
  Advance();

  auto generics = ParseGenericParameters();

  Expect(lexer::TokenType::LParen, "Expected '('");
  std::vector<Parameter> params;
  if (!Check(lexer::TokenType::RParen)) {
    do {
      auto p_pat = ParsePattern();
      Expect(lexer::TokenType::Colon, "Expected ':'");
      auto p_type = ParseType();
      params.push_back({std::move(p_pat), std::move(p_type)});
    } while (Match(lexer::TokenType::Comma));
  }
  Expect(lexer::TokenType::RParen, "Expected ')'");

  std::unique_ptr<Type> ret = nullptr;
  if (Match(lexer::TokenType::Arrow)) {
    ret = ParseType();
  }

  auto where_clause = ParseWhereClause();

  std::unique_ptr<RequiresClause> req = nullptr;
  if (Match(lexer::TokenType::Requires)) {
    req = std::make_unique<RequiresClause>(ParseExpression(Precedence::LOWEST), current_token_.loc);
  }

  auto body = ParseBlockExpr();
  return std::make_unique<FuncDecl>(name, std::move(generics), std::move(params),
                                    std::move(ret), std::move(where_clause),
                                    std::move(req), std::move(body), true, loc);
}

std::unique_ptr<StructDecl> Parser::ParseStructDecl() {
  auto loc = current_token_.loc;
  Advance(); // 'struct'
  std::string name = std::get<std::string>(current_token_.data);
  Advance();

  auto gens = ParseGenericParameters();
  auto wheres = ParseWhereClause();

  Expect(lexer::TokenType::LBrace, "Expected '{'");
  std::vector<StructField> fields;
  while (!Check(lexer::TokenType::RBrace) && !IsAtEnd()) {
    if (Check(lexer::TokenType::Public) || Check(lexer::TokenType::Private)) {
        Advance(); Match(lexer::TokenType::Colon);
        continue;
    }
    std::string f_name = std::get<std::string>(current_token_.data);
    Advance();
    Expect(lexer::TokenType::Colon, "Expected ':'");
    fields.push_back({f_name, ParseType(), true});
    Match(lexer::TokenType::Comma);
  }
  Expect(lexer::TokenType::RBrace, "Expected '}'");

  return std::make_unique<StructDecl>(name, std::move(gens), std::move(fields), std::move(wheres), loc);
}

std::unique_ptr<ModuleDecl> Parser::ParseModuleDecl() {
  auto loc = current_token_.loc;
  Advance(); // 'module'
  auto path = ParsePath();
  Expect(lexer::TokenType::Semi, "Expected ';' after module declaration");
  return std::make_unique<ModuleDecl>(std::move(path), loc);
}

std::unique_ptr<ImportDecl> Parser::ParseImportDecl() {
  auto loc = current_token_.loc;
  Advance(); // 'import'
  auto path = ParsePath();
  Expect(lexer::TokenType::Semi, "Expected ';' after import");
  return std::make_unique<ImportDecl>(std::move(path), loc);
}

std::unique_ptr<ExportDecl> Parser::ParseExportDecl() {
  auto loc = current_token_.loc;
  Advance(); // 'export'
  return std::make_unique<ExportDecl>(ParseStatement(), loc);
}

std::unique_ptr<Type> Parser::ParseType() {
  auto loc = current_token_.loc;
  if (IsPrimitiveType(current_token_.type)) {
    auto t = std::make_unique<PrimitiveType>(current_token_.type, loc);
    Advance();
    return t;
  }
  if (Match(lexer::TokenType::Ampersand)) {
    bool is_mut = Match(lexer::TokenType::Mut);
    return std::make_unique<ReferenceType>(is_mut, nullptr, ParseType(), loc);
  }

  auto path = ParsePath();
  if (Match(lexer::TokenType::Less)) {
    std::vector<std::unique_ptr<Type>> args;
    do { args.push_back(ParseType()); } while (Match(lexer::TokenType::Comma));
    Expect(lexer::TokenType::Greater, "Expected '>'");
    return std::make_unique<GenericType>(std::move(path), std::move(args), loc);
  }
  return std::make_unique<GenericType>(std::move(path), std::vector<std::unique_ptr<Type>>{}, loc);
}

std::unique_ptr<Path> Parser::ParsePath() {
  auto loc = current_token_.loc;
  bool is_abs = Match(lexer::TokenType::DoubleColon);
  std::vector<PathSegment> segments;

  do {
    std::string id = std::get<std::string>(current_token_.data);
    Advance();
    std::vector<std::unique_ptr<Type>> generics;
    if (Match(lexer::TokenType::Less)) {
      do { generics.push_back(ParseType()); } while (Match(lexer::TokenType::Comma));
      Expect(lexer::TokenType::Greater, "Expected '>'");
    }
    segments.push_back({id, std::move(generics)});
  } while (Match(lexer::TokenType::DoubleColon));

  return std::make_unique<Path>(std::move(segments), is_abs, loc);
}

std::unique_ptr<Pattern> Parser::ParsePattern() {
  auto loc = current_token_.loc;
  if (Match(lexer::TokenType::Underscore)) {
    return std::make_unique<WildcardPattern>(loc);
  }
  if (Check(lexer::TokenType::Identifier)) {
    std::string name = std::get<std::string>(current_token_.data);
    Advance();
    return std::make_unique<BindingPattern>(name, false, loc);
  }
  return nullptr;
}

std::vector<std::unique_ptr<Attribute>> Parser::ParseAttributes() {
  std::vector<std::unique_ptr<Attribute>> attrs;
  while (Check(lexer::TokenType::Hash)) {
    Advance(); // '#'
    Expect(lexer::TokenType::LBracket, "Expected '['");
    std::string name = std::get<std::string>(current_token_.data);
    Advance();
    Expect(lexer::TokenType::RBracket, "Expected ']'");
    attrs.push_back(std::make_unique<Attribute>(name, std::vector<std::unique_ptr<Expression>>{}, current_token_.loc));
  }
  return attrs;
}

std::vector<std::unique_ptr<GenericParameter>> Parser::ParseGenericParameters() {
  std::vector<std::unique_ptr<GenericParameter>> params;

  if (Match(lexer::TokenType::Less)) {
    do {
      auto loc = current_token_.loc;

      if (not Check(lexer::TokenType::Identifier)) {
        break;
      }

      std::string name = std::get<std::string>(current_token_.data);
      Advance();

      std::vector<std::unique_ptr<Type>> bounds;
      if (Match(lexer::TokenType::Colon)) {
        do {
          bounds.push_back(ParseType());
        } while (Match(lexer::TokenType::Plus));
      }

      params.push_back(std::make_unique<GenericParameter>(
          std::move(name),
          std::move(bounds),
          loc
      ));

    } while (Match(lexer::TokenType::Comma));

    Expect(lexer::TokenType::Greater, "Expected '>' after generic parameters");
  }

  return params;
}

std::vector<std::unique_ptr<WherePredicate>> Parser::ParseWhereClause() {
  std::vector<std::unique_ptr<WherePredicate>> predicates;

  if (Match(lexer::TokenType::Where)) {
    do {
      auto loc = current_token_.loc;
      auto target_type = ParseType();

      Expect(lexer::TokenType::Colon, "Expected ':' after type in where clause");

      std::vector<std::unique_ptr<Type>> bounds;
      do {
        bounds.push_back(ParseType());
      } while (Match(lexer::TokenType::Plus));

      predicates.push_back(std::make_unique<WherePredicate>(
          std::move(target_type),
          std::move(bounds),
          loc
      ));

    } while (Match(lexer::TokenType::Comma));
  }

  return predicates;
}

Precedence Parser::GetCurrentPrecedence() const {
  if (auto it = Precedences.find(current_token_.type); it != Precedences.end()) {
    return it->second;
  }
  return Precedence::LOWEST;
}

bool Parser::IsLiteral(lexer::TokenType type) const {
  return type == lexer::TokenType::LiteralInt || type == lexer::TokenType::LiteralFloat ||
         type == lexer::TokenType::LiteralBool || type == lexer::TokenType::LiteralString;
}

bool Parser::IsPrimitiveType(lexer::TokenType type) const {
  return static_cast<int>(type) >= static_cast<int>(lexer::TokenType::Int8) &&
         static_cast<int>(type) <= static_cast<int>(lexer::TokenType::USize);
}

std::unique_ptr<EnumDecl> Parser::ParseEnumDecl() {
  auto location = current_token_.loc;
  Advance(); // consume 'enum'

  if (!Check(lexer::TokenType::Identifier)) {
    return nullptr;
  }
  std::string name = std::get<std::string>(current_token_.data);
  Advance();

  auto generics = ParseGenericParameters();
  auto where_clause = ParseWhereClause();

  Expect(lexer::TokenType::LBrace, "Expected '{' for enum body");

  std::vector<EnumVariant> variants;
  while (not Check(lexer::TokenType::RBrace) && !IsAtEnd()) {
    if (not Check(lexer::TokenType::Identifier)) {
      break;
    }

    std::string v_name = std::get<std::string>(current_token_.data);
    Advance();

    std::vector<std::unique_ptr<Type>> types;
    if (Match(lexer::TokenType::LParen)) {
      if (not Check(lexer::TokenType::RParen)) {
        do {
          types.push_back(ParseType());
        } while (Match(lexer::TokenType::Comma));
      }
      Expect(lexer::TokenType::RParen, "Expected ')' after variant data");
    }

    variants.push_back({std::move(v_name), std::move(types)});
    Match(lexer::TokenType::Comma);
  }

  Expect(lexer::TokenType::RBrace, "Expected '}' after enum variants");
  return std::make_unique<EnumDecl>(
      std::move(name),
      std::move(generics),
      std::move(variants),
      std::move(where_clause),
      location
  );
}

std::unique_ptr<TraitDecl> Parser::ParseTraitDecl() {
  auto location = current_token_.loc;
  Advance(); // consume 'trait'

  std::string name = std::get<std::string>(current_token_.data);
  Advance();

  auto generics = ParseGenericParameters();
  auto where_clause = ParseWhereClause();

  Expect(lexer::TokenType::LBrace, "Expected '{' for trait body");

  std::vector<std::unique_ptr<FuncDecl>> methods;
  while (not Check(lexer::TokenType::RBrace) && !IsAtEnd()) {
    methods.push_back(ParseFuncDecl());
  }

  Expect(lexer::TokenType::RBrace, "Expected '}' after trait");
  return std::make_unique<TraitDecl>(
      std::move(name),
      std::move(generics),
      std::move(methods),
      std::move(where_clause),
      location
  );
}

std::unique_ptr<ImplDecl> Parser::ParseImplDecl() {
  auto location = current_token_.loc;
  Advance(); // consume 'impl'

  auto generics = ParseGenericParameters();

  // could be 'impl Trait for Type' or just 'impl Type'
  auto first_path = ParsePath();
  std::unique_ptr<Path> trait_path = nullptr;
  std::unique_ptr<Type> target_type = nullptr;

  if (Match(lexer::TokenType::For)) {
    trait_path = std::move(first_path);
    target_type = ParseType();
  } else {
    target_type = std::make_unique<GenericType>(
        std::move(first_path),
        std::vector<std::unique_ptr<Type>>{},
        location
    );
  }

  auto where_clause = ParseWhereClause();

  Expect(lexer::TokenType::LBrace, "Expected '{' for impl body");

  std::vector<std::unique_ptr<FuncDecl>> methods;
  while (not Check(lexer::TokenType::RBrace) && not IsAtEnd()) {
    methods.push_back(ParseFuncDecl());
  }

  Expect(lexer::TokenType::RBrace, "Expected '}'");
  return std::make_unique<ImplDecl>(
      std::move(trait_path),
      std::move(target_type),
      std::move(generics),
      std::move(methods),
      std::move(where_clause),
      location
  );
}

std::unique_ptr<MatchExpr> Parser::ParseMatchExpr() {
  auto location = current_token_.loc;
  Advance(); // consume 'match'

  auto value = ParseExpression(Precedence::LOWEST);
  Expect(lexer::TokenType::LBrace, "Expected '{' after match value");

  std::vector<MatchArm> arms;
  while (not Check(lexer::TokenType::RBrace) && not IsAtEnd()) {
    auto pattern = ParsePattern();

    std::unique_ptr<Expression> guard = nullptr;
    if (Match(lexer::TokenType::If)) {
      guard = ParseExpression(Precedence::LOWEST);
    }

    Expect(lexer::TokenType::FatArrow, "Expected '=>' after pattern");
    auto body = ParseExpression(Precedence::LOWEST);

    arms.push_back({std::move(pattern), std::move(body), std::move(guard)});
    Match(lexer::TokenType::Comma); // optional comma
  }

  Expect(lexer::TokenType::RBrace, "Expected '}' after match arms");
  return std::make_unique<MatchExpr>(std::move(value), std::move(arms), location);
}

std::unique_ptr<LambdaExpr> Parser::ParseLambdaExpr() {
  auto location = current_token_.loc;

  // syntax: [move] |param: type| -> ret { body }
  bool is_move = Match(lexer::TokenType::Move);

  Expect(lexer::TokenType::Pipe, "Expected '|' for lambda parameters");

  std::vector<GenericParameter> params;
  if (not Check(lexer::TokenType::Pipe)) {
    do {
      std::string p_name = std::get<std::string>(current_token_.data);
      Advance();

      std::vector<std::unique_ptr<Type>> bounds;
      if (Match(lexer::TokenType::Colon)) {
        bounds.push_back(ParseType());
      }
      params.emplace_back(std::move(p_name), std::move(bounds), current_token_.loc);
    } while (Match(lexer::TokenType::Comma));
  }

  Expect(lexer::TokenType::Pipe, "Expected closing '|'");

  // optional return type
  if (Match(lexer::TokenType::Arrow)) {
    ParseType();
  }

  auto body = ParseBlockExpr();
  return std::make_unique<LambdaExpr>(is_move, std::move(params), std::move(body), location);
}

std::unique_ptr<NewExpr> Parser::ParseNewExpr() {
  auto location = current_token_.loc;
  Advance(); // consume 'new'

  auto type = ParseType();
  std::vector<std::unique_ptr<Expression>> args;

  if (Match(lexer::TokenType::LParen)) {
    if (not Check(lexer::TokenType::RParen)) {
      do {
        args.push_back(ParseExpression(Precedence::LOWEST));
      } while (Match(lexer::TokenType::Comma));
    }
    Expect(lexer::TokenType::RParen, "Expected ')'");
  }

  return std::make_unique<NewExpr>(std::move(type), std::move(args), location);
}

std::unique_ptr<StaticIfStmt> Parser::ParseStaticIf() {
  auto location = current_token_.loc;
  Advance(); // consume 'static'

  if (!Match(lexer::TokenType::If)) {
    return nullptr;
  }

  auto condition = ParseExpression(Precedence::LOWEST);
  auto then_branch = ParseBlockExpr();

  std::unique_ptr<BlockExpr> else_branch = nullptr;
  if (Match(lexer::TokenType::Else)) {
    else_branch = ParseBlockExpr();
  }

  return std::make_unique<StaticIfStmt>(
      std::move(condition),
      std::move(then_branch),
      std::move(else_branch),
      location
  );
}

std::unique_ptr<TypeAliasDecl> Parser::ParseTypeAlias() {
  auto location = current_token_.loc;
  Advance(); // consume 'using'

  std::string name = std::get<std::string>(current_token_.data);
  Advance();

  auto generics = ParseGenericParameters();
  Expect(lexer::TokenType::Equal, "Expected '=' in type alias");

  auto target = ParseType();
  Expect(lexer::TokenType::Semi, "Expected ';' after alias");

  return std::make_unique<TypeAliasDecl>(
      std::move(name),
      std::move(generics),
      std::move(target),
      location
  );
}

} // namespace parser