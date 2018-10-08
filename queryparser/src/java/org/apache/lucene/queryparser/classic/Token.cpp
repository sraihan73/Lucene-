using namespace std;

#include "Token.h"

namespace org::apache::lucene::queryparser::classic
{

any Token::getValue() { return nullptr; }

Token::Token() {}

Token::Token(int kind) : Token(kind, nullptr) {}

Token::Token(int kind, const wstring &image)
{
  this->kind = kind;
  this->image = image;
}

wstring Token::toString() { return image; }

shared_ptr<Token> Token::newToken(int ofKind, const wstring &image)
{
  switch (ofKind) {
  default:
    return make_shared<Token>(ofKind, image);
  }
}

shared_ptr<Token> Token::newToken(int ofKind) { return newToken(ofKind, L""); }
} // namespace org::apache::lucene::queryparser::classic