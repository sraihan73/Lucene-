using namespace std;

#include "JavascriptParser.h"

namespace org::apache::lucene::expressions::js
{
using namespace org::antlr::v4::runtime::atn;
using org::antlr::v4::runtime::dfa::DFA;
using namespace org::antlr::v4::runtime;
using namespace org::antlr::v4::runtime::tree;

JavascriptParser::StaticConstructor::StaticConstructor()
{
  RuntimeMetaData::checkVersion(L"4.5.1", RuntimeMetaData::VERSION);
  tokenNames = std::deque<wstring>(_SYMBOLIC_NAMES.size());
  for (int i = 0; i < tokenNames.size(); i++) {
    tokenNames[i] = VOCABULARY->getLiteralName(i);
    if (tokenNames[i] == L"") {
      tokenNames[i] = VOCABULARY->getSymbolicName(i);
    }

    if (tokenNames[i] == L"") {
      tokenNames[i] = L"<INVALID>";
    }
  }
  _decisionToDFA =
      std::deque<std::shared_ptr<DFA>>(_ATN->getNumberOfDecisions());
  for (int i = 0; i < _ATN->getNumberOfDecisions(); i++) {
    _decisionToDFA[i] = make_shared<DFA>(_ATN->getDecisionState(i), i);
  }
}

JavascriptParser::StaticConstructor JavascriptParser::staticConstructor;
std::deque<std::shared_ptr<org::antlr::v4::runtime::dfa::DFA>> const
    JavascriptParser::_decisionToDFA;
const shared_ptr<PredictionContextCache> JavascriptParser::_sharedContextCache =
    make_shared<PredictionContextCache>();
std::deque<wstring> const JavascriptParser::ruleNames = {L"compile",
                                                          L"expression"};
std::deque<wstring> const JavascriptParser::_LITERAL_NAMES = {
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, L"'<<'", L"'>>'", L"'>>>'",
    nullptr, L"'<='", nullptr, L"'>='", L"'=='", L"'!='", nullptr,
    nullptr, nullptr, L"'&&'", L"'||'"};
std::deque<wstring> const JavascriptParser::_SYMBOLIC_NAMES = {
    nullptr,   L"LP",     L"RP",    L"COMMA", L"BOOLNOT",  L"BWNOT",
    L"MUL",    L"DIV",    L"REM",   L"ADD",   L"SUB",      L"LSH",
    L"RSH",    L"USH",    L"LT",    L"LTE",   L"GT",       L"GTE",
    L"EQ",     L"NE",     L"BWAND", L"BWXOR", L"BWOR",     L"BOOLAND",
    L"BOOLOR", L"COND",   L"COLON", L"WS",    L"VARIABLE", L"OCTAL",
    L"HEX",    L"DECIMAL"};
const shared_ptr<Vocabulary> JavascriptParser::VOCABULARY =
    make_shared<VocabularyImpl>(_LITERAL_NAMES, _SYMBOLIC_NAMES);
std::deque<wstring> const JavascriptParser::tokenNames;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @Deprecated public std::wstring[] getTokenNames()
std::deque<wstring> JavascriptParser::getTokenNames() { return tokenNames; }

shared_ptr<Vocabulary> JavascriptParser::getVocabulary() { return VOCABULARY; }

wstring JavascriptParser::getGrammarFileName() { return L"Javascript.g4"; }

std::deque<wstring> JavascriptParser::getRuleNames() { return ruleNames; }

wstring JavascriptParser::getSerializedATN() { return _serializedATN; }

shared_ptr<ATN> JavascriptParser::getATN() { return _ATN; }

JavascriptParser::JavascriptParser(shared_ptr<TokenStream> input)
    : Parser(input)
{
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  _interp = make_shared<ParserATNSimulator>(
      shared_from_this(), _ATN, _decisionToDFA, _sharedContextCache);
}

shared_ptr<ExpressionContext> JavascriptParser::CompileContext::expression()
{
  return getRuleContext(ExpressionContext::typeid, 0);
}

shared_ptr<TerminalNode> JavascriptParser::CompileContext::EOF()
{
  return getToken(JavascriptParser::EOF, 0);
}

JavascriptParser::CompileContext::CompileContext(
    shared_ptr<ParserRuleContext> parent, int invokingState)
    : ParserRuleContext(parent, invokingState)
{
}

int JavascriptParser::CompileContext::getRuleIndex() { return RULE_compile; }

template <typename T, typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public <T> T accept(ParseTreeVisitor<?
// extends T> visitor)
T JavascriptParser::CompileContext::accept(
    shared_ptr<ParseTreeVisitor<T1>> visitor)
{
  if (std::dynamic_pointer_cast<JavascriptVisitor>(visitor) != nullptr) {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: return ((JavascriptVisitor<? extends
    // T>)visitor).visitCompile(this);
          return (std::static_pointer_cast<JavascriptVisitor<? extends T>>(visitor))->visitCompile(shared_from_this());
  } else {
    return visitor->visitChildren(shared_from_this());
  }
}

shared_ptr<CompileContext>
JavascriptParser::compile() 
{
  shared_ptr<CompileContext> _localctx =
      make_shared<CompileContext>(_ctx, getState());
  enterRule(_localctx, 0, RULE_compile);
  try {
    enterOuterAlt(_localctx, 1);
    {
      setState(4);
      expression(0);
      setState(5);
      match(EOF);
    }
  } catch (const RecognitionException &re) {
    _localctx->exception = re;
    _errHandler::reportError(shared_from_this(), re);
    _errHandler::recover(shared_from_this(), re);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    exitRule();
  }
  return _localctx;
}

JavascriptParser::ExpressionContext::ExpressionContext(
    shared_ptr<ParserRuleContext> parent, int invokingState)
    : ParserRuleContext(parent, invokingState)
{
}

int JavascriptParser::ExpressionContext::getRuleIndex()
{
  return RULE_expression;
}

JavascriptParser::ExpressionContext::ExpressionContext() {}

void JavascriptParser::ExpressionContext::copyFrom(
    shared_ptr<ExpressionContext> ctx)
{
  ParserRuleContext::copyFrom(ctx);
}

deque<std::shared_ptr<ExpressionContext>>
JavascriptParser::ConditionalContext::expression()
{
  return getRuleContexts(ExpressionContext::typeid);
}

shared_ptr<ExpressionContext>
JavascriptParser::ConditionalContext::expression(int i)
{
  return getRuleContext(ExpressionContext::typeid, i);
}

shared_ptr<TerminalNode> JavascriptParser::ConditionalContext::COND()
{
  return getToken(JavascriptParser::COND, 0);
}

shared_ptr<TerminalNode> JavascriptParser::ConditionalContext::COLON()
{
  return getToken(JavascriptParser::COLON, 0);
}

JavascriptParser::ConditionalContext::ConditionalContext(
    shared_ptr<ExpressionContext> ctx)
{
  copyFrom(ctx);
}

template <typename T, typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public <T> T accept(ParseTreeVisitor<?
// extends T> visitor)
T JavascriptParser::ConditionalContext::accept(
    shared_ptr<ParseTreeVisitor<T1>> visitor)
{
  if (std::dynamic_pointer_cast<JavascriptVisitor>(visitor) != nullptr) {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: return ((JavascriptVisitor<? extends
    // T>)visitor).visitConditional(this);
          return (std::static_pointer_cast<JavascriptVisitor<? extends T>>(visitor))->visitConditional(shared_from_this());
  } else {
    return visitor->visitChildren(shared_from_this());
  }
}

deque<std::shared_ptr<ExpressionContext>>
JavascriptParser::BoolorContext::expression()
{
  return getRuleContexts(ExpressionContext::typeid);
}

shared_ptr<ExpressionContext> JavascriptParser::BoolorContext::expression(int i)
{
  return getRuleContext(ExpressionContext::typeid, i);
}

shared_ptr<TerminalNode> JavascriptParser::BoolorContext::BOOLOR()
{
  return getToken(JavascriptParser::BOOLOR, 0);
}

JavascriptParser::BoolorContext::BoolorContext(
    shared_ptr<ExpressionContext> ctx)
{
  copyFrom(ctx);
}

template <typename T, typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public <T> T accept(ParseTreeVisitor<?
// extends T> visitor)
T JavascriptParser::BoolorContext::accept(
    shared_ptr<ParseTreeVisitor<T1>> visitor)
{
  if (std::dynamic_pointer_cast<JavascriptVisitor>(visitor) != nullptr) {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: return ((JavascriptVisitor<? extends
    // T>)visitor).visitBoolor(this);
          return (std::static_pointer_cast<JavascriptVisitor<? extends T>>(visitor))->visitBoolor(shared_from_this());
  } else {
    return visitor->visitChildren(shared_from_this());
  }
}

deque<std::shared_ptr<ExpressionContext>>
JavascriptParser::BoolcompContext::expression()
{
  return getRuleContexts(ExpressionContext::typeid);
}

shared_ptr<ExpressionContext>
JavascriptParser::BoolcompContext::expression(int i)
{
  return getRuleContext(ExpressionContext::typeid, i);
}

shared_ptr<TerminalNode> JavascriptParser::BoolcompContext::LT()
{
  return getToken(JavascriptParser::LT, 0);
}

shared_ptr<TerminalNode> JavascriptParser::BoolcompContext::LTE()
{
  return getToken(JavascriptParser::LTE, 0);
}

shared_ptr<TerminalNode> JavascriptParser::BoolcompContext::GT()
{
  return getToken(JavascriptParser::GT, 0);
}

shared_ptr<TerminalNode> JavascriptParser::BoolcompContext::GTE()
{
  return getToken(JavascriptParser::GTE, 0);
}

JavascriptParser::BoolcompContext::BoolcompContext(
    shared_ptr<ExpressionContext> ctx)
{
  copyFrom(ctx);
}

template <typename T, typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public <T> T accept(ParseTreeVisitor<?
// extends T> visitor)
T JavascriptParser::BoolcompContext::accept(
    shared_ptr<ParseTreeVisitor<T1>> visitor)
{
  if (std::dynamic_pointer_cast<JavascriptVisitor>(visitor) != nullptr) {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: return ((JavascriptVisitor<? extends
    // T>)visitor).visitBoolcomp(this);
          return (std::static_pointer_cast<JavascriptVisitor<? extends T>>(visitor))->visitBoolcomp(shared_from_this());
  } else {
    return visitor->visitChildren(shared_from_this());
  }
}

shared_ptr<TerminalNode> JavascriptParser::NumericContext::OCTAL()
{
  return getToken(JavascriptParser::OCTAL, 0);
}

shared_ptr<TerminalNode> JavascriptParser::NumericContext::HEX()
{
  return getToken(JavascriptParser::HEX, 0);
}

shared_ptr<TerminalNode> JavascriptParser::NumericContext::DECIMAL()
{
  return getToken(JavascriptParser::DECIMAL, 0);
}

JavascriptParser::NumericContext::NumericContext(
    shared_ptr<ExpressionContext> ctx)
{
  copyFrom(ctx);
}

template <typename T, typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public <T> T accept(ParseTreeVisitor<?
// extends T> visitor)
T JavascriptParser::NumericContext::accept(
    shared_ptr<ParseTreeVisitor<T1>> visitor)
{
  if (std::dynamic_pointer_cast<JavascriptVisitor>(visitor) != nullptr) {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: return ((JavascriptVisitor<? extends
    // T>)visitor).visitNumeric(this);
          return (std::static_pointer_cast<JavascriptVisitor<? extends T>>(visitor))->visitNumeric(shared_from_this());
  } else {
    return visitor->visitChildren(shared_from_this());
  }
}

deque<std::shared_ptr<ExpressionContext>>
JavascriptParser::AddsubContext::expression()
{
  return getRuleContexts(ExpressionContext::typeid);
}

shared_ptr<ExpressionContext> JavascriptParser::AddsubContext::expression(int i)
{
  return getRuleContext(ExpressionContext::typeid, i);
}

shared_ptr<TerminalNode> JavascriptParser::AddsubContext::ADD()
{
  return getToken(JavascriptParser::ADD, 0);
}

shared_ptr<TerminalNode> JavascriptParser::AddsubContext::SUB()
{
  return getToken(JavascriptParser::SUB, 0);
}

JavascriptParser::AddsubContext::AddsubContext(
    shared_ptr<ExpressionContext> ctx)
{
  copyFrom(ctx);
}

template <typename T, typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public <T> T accept(ParseTreeVisitor<?
// extends T> visitor)
T JavascriptParser::AddsubContext::accept(
    shared_ptr<ParseTreeVisitor<T1>> visitor)
{
  if (std::dynamic_pointer_cast<JavascriptVisitor>(visitor) != nullptr) {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: return ((JavascriptVisitor<? extends
    // T>)visitor).visitAddsub(this);
          return (std::static_pointer_cast<JavascriptVisitor<? extends T>>(visitor))->visitAddsub(shared_from_this());
  } else {
    return visitor->visitChildren(shared_from_this());
  }
}

shared_ptr<ExpressionContext> JavascriptParser::UnaryContext::expression()
{
  return getRuleContext(ExpressionContext::typeid, 0);
}

shared_ptr<TerminalNode> JavascriptParser::UnaryContext::BOOLNOT()
{
  return getToken(JavascriptParser::BOOLNOT, 0);
}

shared_ptr<TerminalNode> JavascriptParser::UnaryContext::BWNOT()
{
  return getToken(JavascriptParser::BWNOT, 0);
}

shared_ptr<TerminalNode> JavascriptParser::UnaryContext::ADD()
{
  return getToken(JavascriptParser::ADD, 0);
}

shared_ptr<TerminalNode> JavascriptParser::UnaryContext::SUB()
{
  return getToken(JavascriptParser::SUB, 0);
}

JavascriptParser::UnaryContext::UnaryContext(shared_ptr<ExpressionContext> ctx)
{
  copyFrom(ctx);
}

template <typename T, typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public <T> T accept(ParseTreeVisitor<?
// extends T> visitor)
T JavascriptParser::UnaryContext::accept(
    shared_ptr<ParseTreeVisitor<T1>> visitor)
{
  if (std::dynamic_pointer_cast<JavascriptVisitor>(visitor) != nullptr) {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: return ((JavascriptVisitor<? extends
    // T>)visitor).visitUnary(this);
          return (std::static_pointer_cast<JavascriptVisitor<? extends T>>(visitor))->visitUnary(shared_from_this());
  } else {
    return visitor->visitChildren(shared_from_this());
  }
}

shared_ptr<TerminalNode> JavascriptParser::PrecedenceContext::LP()
{
  return getToken(JavascriptParser::LP, 0);
}

shared_ptr<ExpressionContext> JavascriptParser::PrecedenceContext::expression()
{
  return getRuleContext(ExpressionContext::typeid, 0);
}

shared_ptr<TerminalNode> JavascriptParser::PrecedenceContext::RP()
{
  return getToken(JavascriptParser::RP, 0);
}

JavascriptParser::PrecedenceContext::PrecedenceContext(
    shared_ptr<ExpressionContext> ctx)
{
  copyFrom(ctx);
}

template <typename T, typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public <T> T accept(ParseTreeVisitor<?
// extends T> visitor)
T JavascriptParser::PrecedenceContext::accept(
    shared_ptr<ParseTreeVisitor<T1>> visitor)
{
  if (std::dynamic_pointer_cast<JavascriptVisitor>(visitor) != nullptr) {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: return ((JavascriptVisitor<? extends
    // T>)visitor).visitPrecedence(this);
          return (std::static_pointer_cast<JavascriptVisitor<? extends T>>(visitor))->visitPrecedence(shared_from_this());
  } else {
    return visitor->visitChildren(shared_from_this());
  }
}

deque<std::shared_ptr<ExpressionContext>>
JavascriptParser::MuldivContext::expression()
{
  return getRuleContexts(ExpressionContext::typeid);
}

shared_ptr<ExpressionContext> JavascriptParser::MuldivContext::expression(int i)
{
  return getRuleContext(ExpressionContext::typeid, i);
}

shared_ptr<TerminalNode> JavascriptParser::MuldivContext::MUL()
{
  return getToken(JavascriptParser::MUL, 0);
}

shared_ptr<TerminalNode> JavascriptParser::MuldivContext::DIV()
{
  return getToken(JavascriptParser::DIV, 0);
}

shared_ptr<TerminalNode> JavascriptParser::MuldivContext::REM()
{
  return getToken(JavascriptParser::REM, 0);
}

JavascriptParser::MuldivContext::MuldivContext(
    shared_ptr<ExpressionContext> ctx)
{
  copyFrom(ctx);
}

template <typename T, typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public <T> T accept(ParseTreeVisitor<?
// extends T> visitor)
T JavascriptParser::MuldivContext::accept(
    shared_ptr<ParseTreeVisitor<T1>> visitor)
{
  if (std::dynamic_pointer_cast<JavascriptVisitor>(visitor) != nullptr) {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: return ((JavascriptVisitor<? extends
    // T>)visitor).visitMuldiv(this);
          return (std::static_pointer_cast<JavascriptVisitor<? extends T>>(visitor))->visitMuldiv(shared_from_this());
  } else {
    return visitor->visitChildren(shared_from_this());
  }
}

shared_ptr<TerminalNode> JavascriptParser::ExternalContext::VARIABLE()
{
  return getToken(JavascriptParser::VARIABLE, 0);
}

shared_ptr<TerminalNode> JavascriptParser::ExternalContext::LP()
{
  return getToken(JavascriptParser::LP, 0);
}

shared_ptr<TerminalNode> JavascriptParser::ExternalContext::RP()
{
  return getToken(JavascriptParser::RP, 0);
}

deque<std::shared_ptr<ExpressionContext>>
JavascriptParser::ExternalContext::expression()
{
  return getRuleContexts(ExpressionContext::typeid);
}

shared_ptr<ExpressionContext>
JavascriptParser::ExternalContext::expression(int i)
{
  return getRuleContext(ExpressionContext::typeid, i);
}

deque<std::shared_ptr<TerminalNode>> JavascriptParser::ExternalContext::COMMA()
{
  return getTokens(JavascriptParser::COMMA);
}

shared_ptr<TerminalNode> JavascriptParser::ExternalContext::COMMA(int i)
{
  return getToken(JavascriptParser::COMMA, i);
}

JavascriptParser::ExternalContext::ExternalContext(
    shared_ptr<ExpressionContext> ctx)
{
  copyFrom(ctx);
}

template <typename T, typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public <T> T accept(ParseTreeVisitor<?
// extends T> visitor)
T JavascriptParser::ExternalContext::accept(
    shared_ptr<ParseTreeVisitor<T1>> visitor)
{
  if (std::dynamic_pointer_cast<JavascriptVisitor>(visitor) != nullptr) {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: return ((JavascriptVisitor<? extends
    // T>)visitor).visitExternal(this);
          return (std::static_pointer_cast<JavascriptVisitor<? extends T>>(visitor))->visitExternal(shared_from_this());
  } else {
    return visitor->visitChildren(shared_from_this());
  }
}

deque<std::shared_ptr<ExpressionContext>>
JavascriptParser::BwshiftContext::expression()
{
  return getRuleContexts(ExpressionContext::typeid);
}

shared_ptr<ExpressionContext>
JavascriptParser::BwshiftContext::expression(int i)
{
  return getRuleContext(ExpressionContext::typeid, i);
}

shared_ptr<TerminalNode> JavascriptParser::BwshiftContext::LSH()
{
  return getToken(JavascriptParser::LSH, 0);
}

shared_ptr<TerminalNode> JavascriptParser::BwshiftContext::RSH()
{
  return getToken(JavascriptParser::RSH, 0);
}

shared_ptr<TerminalNode> JavascriptParser::BwshiftContext::USH()
{
  return getToken(JavascriptParser::USH, 0);
}

JavascriptParser::BwshiftContext::BwshiftContext(
    shared_ptr<ExpressionContext> ctx)
{
  copyFrom(ctx);
}

template <typename T, typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public <T> T accept(ParseTreeVisitor<?
// extends T> visitor)
T JavascriptParser::BwshiftContext::accept(
    shared_ptr<ParseTreeVisitor<T1>> visitor)
{
  if (std::dynamic_pointer_cast<JavascriptVisitor>(visitor) != nullptr) {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: return ((JavascriptVisitor<? extends
    // T>)visitor).visitBwshift(this);
          return (std::static_pointer_cast<JavascriptVisitor<? extends T>>(visitor))->visitBwshift(shared_from_this());
  } else {
    return visitor->visitChildren(shared_from_this());
  }
}

deque<std::shared_ptr<ExpressionContext>>
JavascriptParser::BworContext::expression()
{
  return getRuleContexts(ExpressionContext::typeid);
}

shared_ptr<ExpressionContext> JavascriptParser::BworContext::expression(int i)
{
  return getRuleContext(ExpressionContext::typeid, i);
}

shared_ptr<TerminalNode> JavascriptParser::BworContext::BWOR()
{
  return getToken(JavascriptParser::BWOR, 0);
}

JavascriptParser::BworContext::BworContext(shared_ptr<ExpressionContext> ctx)
{
  copyFrom(ctx);
}

template <typename T, typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public <T> T accept(ParseTreeVisitor<?
// extends T> visitor)
T JavascriptParser::BworContext::accept(
    shared_ptr<ParseTreeVisitor<T1>> visitor)
{
  if (std::dynamic_pointer_cast<JavascriptVisitor>(visitor) != nullptr) {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: return ((JavascriptVisitor<? extends
    // T>)visitor).visitBwor(this);
          return (std::static_pointer_cast<JavascriptVisitor<? extends T>>(visitor))->visitBwor(shared_from_this());
  } else {
    return visitor->visitChildren(shared_from_this());
  }
}

deque<std::shared_ptr<ExpressionContext>>
JavascriptParser::BoolandContext::expression()
{
  return getRuleContexts(ExpressionContext::typeid);
}

shared_ptr<ExpressionContext>
JavascriptParser::BoolandContext::expression(int i)
{
  return getRuleContext(ExpressionContext::typeid, i);
}

shared_ptr<TerminalNode> JavascriptParser::BoolandContext::BOOLAND()
{
  return getToken(JavascriptParser::BOOLAND, 0);
}

JavascriptParser::BoolandContext::BoolandContext(
    shared_ptr<ExpressionContext> ctx)
{
  copyFrom(ctx);
}

template <typename T, typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public <T> T accept(ParseTreeVisitor<?
// extends T> visitor)
T JavascriptParser::BoolandContext::accept(
    shared_ptr<ParseTreeVisitor<T1>> visitor)
{
  if (std::dynamic_pointer_cast<JavascriptVisitor>(visitor) != nullptr) {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: return ((JavascriptVisitor<? extends
    // T>)visitor).visitBooland(this);
          return (std::static_pointer_cast<JavascriptVisitor<? extends T>>(visitor))->visitBooland(shared_from_this());
  } else {
    return visitor->visitChildren(shared_from_this());
  }
}

deque<std::shared_ptr<ExpressionContext>>
JavascriptParser::BwxorContext::expression()
{
  return getRuleContexts(ExpressionContext::typeid);
}

shared_ptr<ExpressionContext> JavascriptParser::BwxorContext::expression(int i)
{
  return getRuleContext(ExpressionContext::typeid, i);
}

shared_ptr<TerminalNode> JavascriptParser::BwxorContext::BWXOR()
{
  return getToken(JavascriptParser::BWXOR, 0);
}

JavascriptParser::BwxorContext::BwxorContext(shared_ptr<ExpressionContext> ctx)
{
  copyFrom(ctx);
}

template <typename T, typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public <T> T accept(ParseTreeVisitor<?
// extends T> visitor)
T JavascriptParser::BwxorContext::accept(
    shared_ptr<ParseTreeVisitor<T1>> visitor)
{
  if (std::dynamic_pointer_cast<JavascriptVisitor>(visitor) != nullptr) {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: return ((JavascriptVisitor<? extends
    // T>)visitor).visitBwxor(this);
          return (std::static_pointer_cast<JavascriptVisitor<? extends T>>(visitor))->visitBwxor(shared_from_this());
  } else {
    return visitor->visitChildren(shared_from_this());
  }
}

deque<std::shared_ptr<ExpressionContext>>
JavascriptParser::BwandContext::expression()
{
  return getRuleContexts(ExpressionContext::typeid);
}

shared_ptr<ExpressionContext> JavascriptParser::BwandContext::expression(int i)
{
  return getRuleContext(ExpressionContext::typeid, i);
}

shared_ptr<TerminalNode> JavascriptParser::BwandContext::BWAND()
{
  return getToken(JavascriptParser::BWAND, 0);
}

JavascriptParser::BwandContext::BwandContext(shared_ptr<ExpressionContext> ctx)
{
  copyFrom(ctx);
}

template <typename T, typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public <T> T accept(ParseTreeVisitor<?
// extends T> visitor)
T JavascriptParser::BwandContext::accept(
    shared_ptr<ParseTreeVisitor<T1>> visitor)
{
  if (std::dynamic_pointer_cast<JavascriptVisitor>(visitor) != nullptr) {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: return ((JavascriptVisitor<? extends
    // T>)visitor).visitBwand(this);
          return (std::static_pointer_cast<JavascriptVisitor<? extends T>>(visitor))->visitBwand(shared_from_this());
  } else {
    return visitor->visitChildren(shared_from_this());
  }
}

deque<std::shared_ptr<ExpressionContext>>
JavascriptParser::BooleqneContext::expression()
{
  return getRuleContexts(ExpressionContext::typeid);
}

shared_ptr<ExpressionContext>
JavascriptParser::BooleqneContext::expression(int i)
{
  return getRuleContext(ExpressionContext::typeid, i);
}

shared_ptr<TerminalNode> JavascriptParser::BooleqneContext::EQ()
{
  return getToken(JavascriptParser::EQ, 0);
}

shared_ptr<TerminalNode> JavascriptParser::BooleqneContext::NE()
{
  return getToken(JavascriptParser::NE, 0);
}

JavascriptParser::BooleqneContext::BooleqneContext(
    shared_ptr<ExpressionContext> ctx)
{
  copyFrom(ctx);
}

template <typename T, typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public <T> T accept(ParseTreeVisitor<?
// extends T> visitor)
T JavascriptParser::BooleqneContext::accept(
    shared_ptr<ParseTreeVisitor<T1>> visitor)
{
  if (std::dynamic_pointer_cast<JavascriptVisitor>(visitor) != nullptr) {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: return ((JavascriptVisitor<? extends
    // T>)visitor).visitBooleqne(this);
          return (std::static_pointer_cast<JavascriptVisitor<? extends T>>(visitor))->visitBooleqne(shared_from_this());
  } else {
    return visitor->visitChildren(shared_from_this());
  }
}

shared_ptr<ExpressionContext>
JavascriptParser::expression() 
{
  return expression(0);
}

shared_ptr<ExpressionContext>
JavascriptParser::expression(int _p) 
{
  shared_ptr<ParserRuleContext> _parentctx = _ctx;
  int _parentState = getState();
  shared_ptr<ExpressionContext> _localctx =
      make_shared<ExpressionContext>(_ctx, _parentState);
  shared_ptr<ExpressionContext> _prevctx = _localctx;
  int _startState = 2;
  enterRecursionRule(_localctx, 2, RULE_expression, _p);
  int _la;
  try {
    int _alt;
    enterOuterAlt(_localctx, 1);
    {
      setState(30);
      switch (_input::LA(1)) {
      case BOOLNOT:
      case BWNOT:
      case ADD:
      case SUB: {
        _localctx = make_shared<UnaryContext>(_localctx);
        _ctx = _localctx;
        _prevctx = _localctx;

        setState(8);
        _la = _input::LA(1);
        if (!((((_la) & ~0x3f) == 0 &&
               ((1LL << _la) & ((1LL << BOOLNOT) | (1LL << BWNOT) |
                                (1LL << ADD) | (1LL << SUB))) != 0))) {
          _errHandler::recoverInline(shared_from_this());
        } else {
          consume();
        }
        setState(9);
        expression(12);
      } break;
      case LP: {
        _localctx = make_shared<PrecedenceContext>(_localctx);
        _ctx = _localctx;
        _prevctx = _localctx;
        setState(10);
        match(LP);
        setState(11);
        expression(0);
        setState(12);
        match(RP);
      } break;
      case OCTAL:
      case HEX:
      case DECIMAL: {
        _localctx = make_shared<NumericContext>(_localctx);
        _ctx = _localctx;
        _prevctx = _localctx;
        setState(14);
        _la = _input::LA(1);
        if (!((((_la) & ~0x3f) == 0 &&
               ((1LL << _la) &
                ((1LL << OCTAL) | (1LL << HEX) | (1LL << DECIMAL))) != 0))) {
          _errHandler::recoverInline(shared_from_this());
        } else {
          consume();
        }
      } break;
      case VARIABLE: {
        _localctx = make_shared<ExternalContext>(_localctx);
        _ctx = _localctx;
        _prevctx = _localctx;
        setState(15);
        match(VARIABLE);
        setState(28);
        switch (getInterpreter().adaptivePredict(_input, 2, _ctx)) {
        case 1: {
          setState(16);
          match(LP);
          setState(25);
          _la = _input::LA(1);
          if ((((_la) & ~0x3f) == 0 &&
               ((1LL << _la) &
                ((1LL << LP) | (1LL << BOOLNOT) | (1LL << BWNOT) |
                 (1LL << ADD) | (1LL << SUB) | (1LL << VARIABLE) |
                 (1LL << OCTAL) | (1LL << HEX) | (1LL << DECIMAL))) != 0)) {
            {
              setState(17);
              expression(0);
              setState(22);
              _errHandler::sync(shared_from_this());
              _la = _input::LA(1);
              while (_la == COMMA) {
                {
                  {
                    setState(18);
                    match(COMMA);
                    setState(19);
                    expression(0);
                  }
                }
                setState(24);
                _errHandler::sync(shared_from_this());
                _la = _input::LA(1);
              }
            }
          }

          setState(27);
          match(RP);
        } break;
        }
      } break;
      default:
        throw make_shared<NoViableAltException>(shared_from_this());
      }
      _ctx->stop = _input::LT(-1);
      setState(70);
      _errHandler::sync(shared_from_this());
      _alt = getInterpreter().adaptivePredict(_input, 5, _ctx);
      while (_alt != 2 &&
             _alt != org::antlr::v4::runtime::atn::ATN::INVALID_ALT_NUMBER) {
        if (_alt == 1) {
          if (_parseListeners != nullptr) {
            triggerExitRuleEvent();
          }
          _prevctx = _localctx;
          {
            setState(68);
            switch (getInterpreter().adaptivePredict(_input, 4, _ctx)) {
            case 1: {
              _localctx = make_shared<MuldivContext>(
                  make_shared<ExpressionContext>(_parentctx, _parentState));
              pushNewRecursionContext(_localctx, _startState, RULE_expression);
              setState(32);
              if (!(precpred(_ctx, 11))) {
                throw make_shared<FailedPredicateException>(
                    shared_from_this(), L"precpred(_ctx, 11)");
              }
              setState(33);
              _la = _input::LA(1);
              if (!((((_la) & ~0x3f) == 0 &&
                     ((1LL << _la) &
                      ((1LL << MUL) | (1LL << DIV) | (1LL << REM))) != 0))) {
                _errHandler::recoverInline(shared_from_this());
              } else {
                consume();
              }
              setState(34);
              expression(12);
            } break;
            case 2: {
              _localctx = make_shared<AddsubContext>(
                  make_shared<ExpressionContext>(_parentctx, _parentState));
              pushNewRecursionContext(_localctx, _startState, RULE_expression);
              setState(35);
              if (!(precpred(_ctx, 10))) {
                throw make_shared<FailedPredicateException>(
                    shared_from_this(), L"precpred(_ctx, 10)");
              }
              setState(36);
              _la = _input::LA(1);
              if (!(_la == ADD || _la == SUB)) {
                _errHandler::recoverInline(shared_from_this());
              } else {
                consume();
              }
              setState(37);
              expression(11);
            } break;
            case 3: {
              _localctx = make_shared<BwshiftContext>(
                  make_shared<ExpressionContext>(_parentctx, _parentState));
              pushNewRecursionContext(_localctx, _startState, RULE_expression);
              setState(38);
              if (!(precpred(_ctx, 9))) {
                throw make_shared<FailedPredicateException>(
                    shared_from_this(), L"precpred(_ctx, 9)");
              }
              setState(39);
              _la = _input::LA(1);
              if (!((((_la) & ~0x3f) == 0 &&
                     ((1LL << _la) &
                      ((1LL << LSH) | (1LL << RSH) | (1LL << USH))) != 0))) {
                _errHandler::recoverInline(shared_from_this());
              } else {
                consume();
              }
              setState(40);
              expression(10);
            } break;
            case 4: {
              _localctx = make_shared<BoolcompContext>(
                  make_shared<ExpressionContext>(_parentctx, _parentState));
              pushNewRecursionContext(_localctx, _startState, RULE_expression);
              setState(41);
              if (!(precpred(_ctx, 8))) {
                throw make_shared<FailedPredicateException>(
                    shared_from_this(), L"precpred(_ctx, 8)");
              }
              setState(42);
              _la = _input::LA(1);
              if (!((((_la) & ~0x3f) == 0 &&
                     ((1LL << _la) & ((1LL << LT) | (1LL << LTE) | (1LL << GT) |
                                      (1LL << GTE))) != 0))) {
                _errHandler::recoverInline(shared_from_this());
              } else {
                consume();
              }
              setState(43);
              expression(9);
            } break;
            case 5: {
              _localctx = make_shared<BooleqneContext>(
                  make_shared<ExpressionContext>(_parentctx, _parentState));
              pushNewRecursionContext(_localctx, _startState, RULE_expression);
              setState(44);
              if (!(precpred(_ctx, 7))) {
                throw make_shared<FailedPredicateException>(
                    shared_from_this(), L"precpred(_ctx, 7)");
              }
              setState(45);
              _la = _input::LA(1);
              if (!(_la == EQ || _la == NE)) {
                _errHandler::recoverInline(shared_from_this());
              } else {
                consume();
              }
              setState(46);
              expression(8);
            } break;
            case 6: {
              _localctx = make_shared<BwandContext>(
                  make_shared<ExpressionContext>(_parentctx, _parentState));
              pushNewRecursionContext(_localctx, _startState, RULE_expression);
              setState(47);
              if (!(precpred(_ctx, 6))) {
                throw make_shared<FailedPredicateException>(
                    shared_from_this(), L"precpred(_ctx, 6)");
              }
              setState(48);
              match(BWAND);
              setState(49);
              expression(7);
            } break;
            case 7: {
              _localctx = make_shared<BwxorContext>(
                  make_shared<ExpressionContext>(_parentctx, _parentState));
              pushNewRecursionContext(_localctx, _startState, RULE_expression);
              setState(50);
              if (!(precpred(_ctx, 5))) {
                throw make_shared<FailedPredicateException>(
                    shared_from_this(), L"precpred(_ctx, 5)");
              }
              setState(51);
              match(BWXOR);
              setState(52);
              expression(6);
            } break;
            case 8: {
              _localctx = make_shared<BworContext>(
                  make_shared<ExpressionContext>(_parentctx, _parentState));
              pushNewRecursionContext(_localctx, _startState, RULE_expression);
              setState(53);
              if (!(precpred(_ctx, 4))) {
                throw make_shared<FailedPredicateException>(
                    shared_from_this(), L"precpred(_ctx, 4)");
              }
              setState(54);
              match(BWOR);
              setState(55);
              expression(5);
            } break;
            case 9: {
              _localctx = make_shared<BoolandContext>(
                  make_shared<ExpressionContext>(_parentctx, _parentState));
              pushNewRecursionContext(_localctx, _startState, RULE_expression);
              setState(56);
              if (!(precpred(_ctx, 3))) {
                throw make_shared<FailedPredicateException>(
                    shared_from_this(), L"precpred(_ctx, 3)");
              }
              setState(57);
              match(BOOLAND);
              setState(58);
              expression(4);
            } break;
            case 10: {
              _localctx = make_shared<BoolorContext>(
                  make_shared<ExpressionContext>(_parentctx, _parentState));
              pushNewRecursionContext(_localctx, _startState, RULE_expression);
              setState(59);
              if (!(precpred(_ctx, 2))) {
                throw make_shared<FailedPredicateException>(
                    shared_from_this(), L"precpred(_ctx, 2)");
              }
              setState(60);
              match(BOOLOR);
              setState(61);
              expression(3);
            } break;
            case 11: {
              _localctx = make_shared<ConditionalContext>(
                  make_shared<ExpressionContext>(_parentctx, _parentState));
              pushNewRecursionContext(_localctx, _startState, RULE_expression);
              setState(62);
              if (!(precpred(_ctx, 1))) {
                throw make_shared<FailedPredicateException>(
                    shared_from_this(), L"precpred(_ctx, 1)");
              }
              setState(63);
              match(COND);
              setState(64);
              expression(0);
              setState(65);
              match(COLON);
              setState(66);
              expression(1);
            } break;
            }
          }
        }
        setState(72);
        _errHandler::sync(shared_from_this());
        _alt = getInterpreter().adaptivePredict(_input, 5, _ctx);
      }
    }
  } catch (const RecognitionException &re) {
    _localctx->exception = re;
    _errHandler::reportError(shared_from_this(), re);
    _errHandler::recover(shared_from_this(), re);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    unrollRecursionContexts(_parentctx);
  }
  return _localctx;
}

bool JavascriptParser::sempred(shared_ptr<RuleContext> _localctx, int ruleIndex,
                               int predIndex)
{
  switch (ruleIndex) {
  case 1:
    return expression_sempred(
        std::static_pointer_cast<ExpressionContext>(_localctx), predIndex);
  }
  return true;
}

bool JavascriptParser::expression_sempred(
    shared_ptr<ExpressionContext> _localctx, int predIndex)
{
  switch (predIndex) {
  case 0:
    return precpred(_ctx, 11);
  case 1:
    return precpred(_ctx, 10);
  case 2:
    return precpred(_ctx, 9);
  case 3:
    return precpred(_ctx, 8);
  case 4:
    return precpred(_ctx, 7);
  case 5:
    return precpred(_ctx, 6);
  case 6:
    return precpred(_ctx, 5);
  case 7:
    return precpred(_ctx, 4);
  case 8:
    return precpred(_ctx, 3);
  case 9:
    return precpred(_ctx, 2);
  case 10:
    return precpred(_ctx, 1);
  }
  return true;
}

const wstring JavascriptParser::_serializedATN =
    wstring(L"\3\u0430\ud6d1\u8206\uad2d\u4417\uaef1\u8d80\uaadd\3!"
            L"L\4\2\t\2\4\3\t\3") +
    L"\3\2\3\2\3\2\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\7\3\27" +
    L"\n\3\f\3\16\3\32\13\3\5\3\34\n\3\3\3\5\3\37\n\3\5\3!\n\3\3\3\3\3\3\3\3" +
    L"\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3" +
    L"\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\7\3G\n\3" +
    L"\f\3\16\3J\13\3\3\3\2\3\4\4\2\4\2\t\4\2\6\7\13\f\3\2\37!\3\2\b\n\3\2\13" +
    L"\f\3\2\r\17\3\2\20\23\3\2\24\25Z\2\6\3\2\2\2\4 \3\2\2\2\6\7\5\4\3\2\7" +
    L"\b\7\2\2\3\b\3\3\2\2\2\t\n\b\3\1\2\n\13\t\2\2\2\13!\5\4\3\16\f\r\7\3\2" +
    L"\2\r\16\5\4\3\2\16\17\7\4\2\2\17!\3\2\2\2\20!\t\3\2\2\21\36\7\36\2\2\22" +
    L"\33\7\3\2\2\23\30\5\4\3\2\24\25\7\5\2\2\25\27\5\4\3\2\26\24\3\2\2\2\27" +
    L"\32\3\2\2\2\30\26\3\2\2\2\30\31\3\2\2\2\31\34\3\2\2\2\32\30\3\2\2\2\33" +
    L"\23\3\2\2\2\33\34\3\2\2\2\34\35\3\2\2\2\35\37\7\4\2\2\36\22\3\2\2\2\36" +
    L"\37\3\2\2\2\37!\3\2\2\2 \t\3\2\2\2 \f\3\2\2\2 \20\3\2\2\2 \21\3\2\2\2" +
    L"!H\3\2\2\2\"#\f\r\2\2#$\t\4\2\2$G\5\4\3\16%&\f\f\2\2&\'\t\5\2\2\'G\5\4" +
    L"\3\r()\f\13\2\2)*\t\6\2\2*G\5\4\3\f+,\f\n\2\2,-\t\7\2\2-G\5\4\3\13./\f" +
    L"\t\2\2/\60\t\b\2\2\60G\5\4\3\n\61\62\f\b\2\2\62\63\7\26\2\2\63G\5\4\3" +
    L"\t\64\65\f\7\2\2\65\66\7\27\2\2\66G\5\4\3\b\678\f\6\2\289\7\30\2\29G\5" +
    L"\4\3\7:;\f\5\2\2;<\7\31\2\2<G\5\4\3\6=>\f\4\2\2>?\7\32\2\2?G\5\4\3\5@" +
    L"A\f\3\2\2AB\7\33\2\2BC\5\4\3\2CD\7\34\2\2DE\5\4\3\3EG\3\2\2\2F\"\3\2\2" +
    L"\2F%\3\2\2\2F(\3\2\2\2F+\3\2\2\2F.\3\2\2\2F\61\3\2\2\2F\64\3\2\2\2F\67" +
    L"\3\2\2\2F:\3\2\2\2F=\3\2\2\2F@\3\2\2\2GJ\3\2\2\2HF\3\2\2\2HI\3\2\2\2I" +
    L"\5\3\2\2\2JH\3\2\2\2\b\30\33\36 FH";
const shared_ptr<ATN> JavascriptParser::_ATN =
    (make_shared<ATNDeserializer>())->deserialize(_serializedATN.toCharArray());
} // namespace org::apache::lucene::expressions::js