#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// ANTLR GENERATED CODE: DO NOT EDIT
namespace org::apache::lucene::expressions::js
{
using org::antlr::v4::runtime::CharStream;
using org::antlr::v4::runtime::Lexer;
using namespace org::antlr::v4::runtime;
using namespace org::antlr::v4::runtime::atn;
using org::antlr::v4::runtime::dfa::DFA;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings({"all", "warnings", "unchecked", "unused",
// "cast"}) class JavascriptLexer extends org.antlr.v4.runtime.Lexer
class JavascriptLexer : public Lexer
{
private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static JavascriptLexer::StaticConstructor staticConstructor;

protected:
  static std::deque<std::shared_ptr<DFA>> const _decisionToDFA;
  static const std::shared_ptr<PredictionContextCache> _sharedContextCache;

public:
  static constexpr int LP = 1, RP = 2, COMMA = 3, BOOLNOT = 4, BWNOT = 5,
                       MUL = 6, DIV = 7, REM = 8, ADD = 9, SUB = 10, LSH = 11,
                       RSH = 12, USH = 13, LT = 14, LTE = 15, GT = 16, GTE = 17,
                       EQ = 18, NE = 19, BWAND = 20, BWXOR = 21, BWOR = 22,
                       BOOLAND = 23, BOOLOR = 24, COND = 25, COLON = 26,
                       WS = 27, VARIABLE = 28, OCTAL = 29, HEX = 30,
                       DECIMAL = 31;
  static std::deque<std::wstring> modeNames;

  static std::deque<std::wstring> const ruleNames;

private:
  static std::deque<std::wstring> const _LITERAL_NAMES;
  static std::deque<std::wstring> const _SYMBOLIC_NAMES;

public:
  static const std::shared_ptr<Vocabulary> VOCABULARY;

  /**
   * @deprecated Use {@link #VOCABULARY} instead.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final std::wstring[] tokenNames;
  static std::deque<std::wstring> const tokenNames;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Override @Deprecated public std::wstring[] getTokenNames()
  std::deque<std::wstring> getTokenNames() override;

  std::shared_ptr<Vocabulary> getVocabulary() override;

  JavascriptLexer(std::shared_ptr<CharStream> input);

  std::wstring getGrammarFileName() override;
  std::deque<std::wstring> getRuleNames() override;
  std::wstring getSerializedATN() override;
  std::deque<std::wstring> getModeNames() override;
  std::shared_ptr<ATN> getATN() override;

  static const std::wstring _serializedATN;
  static const std::shared_ptr<ATN> _ATN;

protected:
  std::shared_ptr<JavascriptLexer> shared_from_this()
  {
    return std::static_pointer_cast<JavascriptLexer>(
        org.antlr.v4.runtime.Lexer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/expressions/js/
