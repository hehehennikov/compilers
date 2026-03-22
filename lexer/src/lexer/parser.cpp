#include "parser.hpp"

Parser::Parser(std::vector<Token> tokens)
    : tokens_(std::move(tokens)) {}

const Token& Parser::Peek() const {
  return tokens_[cursor_];
}

Token Parser::Consume() {
  return tokens_[cursor_++];
}

bool Parser::Match(TokenType type) {
  if (Peek().type == type) {
    Consume();

    return true;
  }

  return false;
}

std::expected<std::unique_ptr<Expression>, Error> Parser::ParseExpression() {
  const auto& [_, value] = Peek();
  if (Match(TokenType::Number)) {
    auto lit = std::make_unique<LiteralExpression>(std::stoi(value));

    if (Match(TokenType::Equals)) {
      auto right = ParseExpression();
      if (not right) {
        return right;
      }

      return std::make_unique<BinaryExpression>(std::move(lit), std::move(*right));
    }

    return lit;
  }
  if (Match(TokenType::Identifier)) {
    auto var = std::make_unique<VariableExpression>(value);
    if (Match(TokenType::Equals)) {
      auto right = ParseExpression();

      if (not right) {
        return right;
      }

      return std::make_unique<BinaryExpression>(std::move(var), std::move(*right));
    }
    return var;
  }

  return std::unexpected(Error{ErrorType::ParseError, "Expected expression"});
}

std::expected<std::unique_ptr<Statement>, Error> Parser::ParseStatement() {
  if (Match(TokenType::Var)) {
    std::string name = Consume().value;

    Match(TokenType::Colon);
    Match(TokenType::IntType);
    Match(TokenType::Semicolon);

    return std::make_unique<VarStatement>(name);
  }
  if (Peek().type == TokenType::Identifier) {
    std::string name = Consume().value;

    Match(TokenType::Assign);
    auto expr = ParseExpression();
    Match(TokenType::Semicolon);

    return std::make_unique<AssignStatement>(name, std::move(*expr));
  }
  if (Match(TokenType::Print)) {
    Match(TokenType::LParen);
    auto expr = ParseExpression();
    Match(TokenType::RParen);
    Match(TokenType::Semicolon);

    return std::make_unique<PrintStatement>(std::move(*expr));
  }
  if (Match(TokenType::If)) {
    Match(TokenType::LParen);
    auto cond = ParseExpression();
    Match(TokenType::RParen);
    Match(TokenType::LBrace);

    std::vector<std::unique_ptr<Statement>> then_stmts;
    while (Peek().type != TokenType::RBrace) {
      then_stmts.push_back(std::move(*ParseStatement()));
    }

    Match(TokenType::RBrace);
    Match(TokenType::Else);
    Match(TokenType::LBrace);

    std::vector<std::unique_ptr<Statement>> else_stmts;

    while (Peek().type != TokenType::RBrace) {
      else_stmts.push_back(std::move(*ParseStatement()));
    }
    Match(TokenType::RBrace);

    return std::make_unique<IfStatement>(std::move(*cond), std::move(then_stmts), std::move(else_stmts));
  }

  return std::unexpected(Error{ErrorType::ParseError, "Unknown statement"});
}

std::expected<std::unique_ptr<MainFunction>, Error> Parser::Parse() {
  std::vector<std::unique_ptr<Statement>> statements;
  while (Peek().type != TokenType::Eof) {
    auto stmt = ParseStatement();
    if (not stmt) {
      return std::unexpected(stmt.error());
    }

    statements.push_back(std::move(*stmt));
  }

  return std::make_unique<MainFunction>(std::move(statements));
}