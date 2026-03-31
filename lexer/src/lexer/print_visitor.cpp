#include "print_visitor.hpp"
#include "ast.hpp"

PrintVisitor::PrintVisitor(const std::string& filename) {
  out_.open(filename);
}

PrintVisitor::~PrintVisitor() {
  if (out_.is_open()) {
    out_.close();
  }
}

void PrintVisitor::PrintIndent() {
  for (int i = 0; i < indent_; ++i) out_ << "  ";
}

void PrintVisitor::Visit(LiteralExpression* expr) {
  out_ << "Literal(" << expr->GetValue() << ")";
}

void PrintVisitor::Visit(VariableExpression* expr) {
  out_ << "Variable(" << expr->GetName() << ")";
}

void PrintVisitor::Visit(BinaryExpression* expr) {
  out_ << "BinaryExpr(";
  expr->GetLeft()->Accept(this);
  out_ << " == ";
  expr->GetRight()->Accept(this);
  out_ << ")";
}

void PrintVisitor::Visit(VarStatement* stmt) {
  PrintIndent();
  out_ << "Declare: " << stmt->GetName() << "\n";
}

void PrintVisitor::Visit(AssignStatement* stmt) {
  PrintIndent();
  out_ << "Assign: " << stmt->GetName() << " = ";
  stmt->GetExpression()->Accept(this);
  out_ << "\n";
}

void PrintVisitor::Visit(PrintStatement* stmt) {
  PrintIndent();
  out_ << "Print: ";
  stmt->GetExpression()->Accept(this);
  out_ << "\n";
}

void PrintVisitor::Visit(IfStatement* stmt) {
  PrintIndent();
  out_ << "If: ";
  stmt->GetCondition()->Accept(this);
  out_ << "\n";
  indent_++;
  for (const auto& s : stmt->GetThenBranch()) s->Accept(this);
  indent_--;
  if (!stmt->GetElseBranch().empty()) {
    PrintIndent();
    out_ << "Else:\n";
    indent_++;
    for (const auto& s : stmt->GetElseBranch()) s->Accept(this);
    indent_--;
  }
}

void PrintVisitor::Visit(WhileStatement* stmt) {
  PrintIndent();
  out_ << "While: ";
  stmt->GetCondition()->Accept(this);
  out_ << "\n";
  indent_++;
  for (const auto& s : stmt->GetBody()) s->Accept(this);
  indent_--;
}

void PrintVisitor::Visit(MainFunction* prog) {
  out_ << "MainFunction:\n";
  indent_++;
  for (const auto& stmt : prog->GetStatements()) stmt->Accept(this);
  indent_--;
}