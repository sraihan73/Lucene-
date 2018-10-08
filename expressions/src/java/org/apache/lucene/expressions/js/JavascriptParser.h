#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::expressions::js
{
class ExpressionContext;
}

// ANTLR GENERATED CODE: DO NOT EDIT
namespace org::apache::lucene::expressions::js
{
using namespace org::antlr::v4::runtime::atn;
using org::antlr::v4::runtime::dfa::DFA;
using namespace org::antlr::v4::runtime;
using namespace org::antlr::v4::runtime::tree;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings({"all", "warnings", "unchecked", "unused",
// "cast"}) class JavascriptParser extends Parser
class JavascriptParser : public Parser
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
  static JavascriptParser::StaticConstructor staticConstructor;

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
  static constexpr int RULE_compile = 0, RULE_expression = 1;
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

  std::wstring getGrammarFileName() override;
  std::deque<std::wstring> getRuleNames() override;
  std::wstring getSerializedATN() override;
  std::shared_ptr<ATN> getATN() override;

  JavascriptParser(std::shared_ptr<TokenStream> input);

public:
  class CompileContext : public ParserRuleContext
  {
    GET_CLASS_NAME(CompileContext)
  public:
    virtual std::shared_ptr<ExpressionContext> expression();
    virtual std::shared_ptr<TerminalNode> EOF();
    CompileContext(std::shared_ptr<ParserRuleContext> parent,
                   int invokingState);
    int getRuleIndex() override;
    template <typename T, typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public <T> T
    // accept(ParseTreeVisitor<? extends T> visitor)
    T accept(std::shared_ptr<ParseTreeVisitor<T1>> visitor);

  protected:
    std::shared_ptr<CompileContext> shared_from_this()
    {
      return std::static_pointer_cast<CompileContext>(
          ParserRuleContext::shared_from_this());
    }
  };

public:
  std::shared_ptr<CompileContext> compile() ;

public:
  class ExpressionContext : public ParserRuleContext
  {
    GET_CLASS_NAME(ExpressionContext)
  public:
    ExpressionContext(std::shared_ptr<ParserRuleContext> parent,
                      int invokingState);
    int getRuleIndex() override;

    ExpressionContext();
    virtual void copyFrom(std::shared_ptr<ExpressionContext> ctx);

  protected:
    std::shared_ptr<ExpressionContext> shared_from_this()
    {
      return std::static_pointer_cast<ExpressionContext>(
          ParserRuleContext::shared_from_this());
    }
  };

public:
  class ConditionalContext : public ExpressionContext
  {
    GET_CLASS_NAME(ConditionalContext)
  public:
    virtual std::deque<std::shared_ptr<ExpressionContext>> expression();
    virtual std::shared_ptr<ExpressionContext> expression(int i);
    virtual std::shared_ptr<TerminalNode> COND();
    virtual std::shared_ptr<TerminalNode> COLON();
    ConditionalContext(std::shared_ptr<ExpressionContext> ctx);
    template <typename T, typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public <T> T
    // accept(ParseTreeVisitor<? extends T> visitor)
    T accept(std::shared_ptr<ParseTreeVisitor<T1>> visitor);

  protected:
    std::shared_ptr<ConditionalContext> shared_from_this()
    {
      return std::static_pointer_cast<ConditionalContext>(
          ExpressionContext::shared_from_this());
    }
  };

public:
  class BoolorContext : public ExpressionContext
  {
    GET_CLASS_NAME(BoolorContext)
  public:
    virtual std::deque<std::shared_ptr<ExpressionContext>> expression();
    virtual std::shared_ptr<ExpressionContext> expression(int i);
    virtual std::shared_ptr<TerminalNode> BOOLOR();
    BoolorContext(std::shared_ptr<ExpressionContext> ctx);
    template <typename T, typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public <T> T
    // accept(ParseTreeVisitor<? extends T> visitor)
    T accept(std::shared_ptr<ParseTreeVisitor<T1>> visitor);

  protected:
    std::shared_ptr<BoolorContext> shared_from_this()
    {
      return std::static_pointer_cast<BoolorContext>(
          ExpressionContext::shared_from_this());
    }
  };

public:
  class BoolcompContext : public ExpressionContext
  {
    GET_CLASS_NAME(BoolcompContext)
  public:
    virtual std::deque<std::shared_ptr<ExpressionContext>> expression();
    virtual std::shared_ptr<ExpressionContext> expression(int i);
    virtual std::shared_ptr<TerminalNode> LT();
    virtual std::shared_ptr<TerminalNode> LTE();
    virtual std::shared_ptr<TerminalNode> GT();
    virtual std::shared_ptr<TerminalNode> GTE();
    BoolcompContext(std::shared_ptr<ExpressionContext> ctx);
    template <typename T, typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public <T> T
    // accept(ParseTreeVisitor<? extends T> visitor)
    T accept(std::shared_ptr<ParseTreeVisitor<T1>> visitor);

  protected:
    std::shared_ptr<BoolcompContext> shared_from_this()
    {
      return std::static_pointer_cast<BoolcompContext>(
          ExpressionContext::shared_from_this());
    }
  };

public:
  class NumericContext : public ExpressionContext
  {
    GET_CLASS_NAME(NumericContext)
  public:
    virtual std::shared_ptr<TerminalNode> OCTAL();
    virtual std::shared_ptr<TerminalNode> HEX();
    virtual std::shared_ptr<TerminalNode> DECIMAL();
    NumericContext(std::shared_ptr<ExpressionContext> ctx);
    template <typename T, typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public <T> T
    // accept(ParseTreeVisitor<? extends T> visitor)
    T accept(std::shared_ptr<ParseTreeVisitor<T1>> visitor);

  protected:
    std::shared_ptr<NumericContext> shared_from_this()
    {
      return std::static_pointer_cast<NumericContext>(
          ExpressionContext::shared_from_this());
    }
  };

public:
  class AddsubContext : public ExpressionContext
  {
    GET_CLASS_NAME(AddsubContext)
  public:
    virtual std::deque<std::shared_ptr<ExpressionContext>> expression();
    virtual std::shared_ptr<ExpressionContext> expression(int i);
    virtual std::shared_ptr<TerminalNode> ADD();
    virtual std::shared_ptr<TerminalNode> SUB();
    AddsubContext(std::shared_ptr<ExpressionContext> ctx);
    template <typename T, typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public <T> T
    // accept(ParseTreeVisitor<? extends T> visitor)
    T accept(std::shared_ptr<ParseTreeVisitor<T1>> visitor);

  protected:
    std::shared_ptr<AddsubContext> shared_from_this()
    {
      return std::static_pointer_cast<AddsubContext>(
          ExpressionContext::shared_from_this());
    }
  };

public:
  class UnaryContext : public ExpressionContext
  {
    GET_CLASS_NAME(UnaryContext)
  public:
    virtual std::shared_ptr<ExpressionContext> expression();
    virtual std::shared_ptr<TerminalNode> BOOLNOT();
    virtual std::shared_ptr<TerminalNode> BWNOT();
    virtual std::shared_ptr<TerminalNode> ADD();
    virtual std::shared_ptr<TerminalNode> SUB();
    UnaryContext(std::shared_ptr<ExpressionContext> ctx);
    template <typename T, typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public <T> T
    // accept(ParseTreeVisitor<? extends T> visitor)
    T accept(std::shared_ptr<ParseTreeVisitor<T1>> visitor);

  protected:
    std::shared_ptr<UnaryContext> shared_from_this()
    {
      return std::static_pointer_cast<UnaryContext>(
          ExpressionContext::shared_from_this());
    }
  };

public:
  class PrecedenceContext : public ExpressionContext
  {
    GET_CLASS_NAME(PrecedenceContext)
  public:
    virtual std::shared_ptr<TerminalNode> LP();
    virtual std::shared_ptr<ExpressionContext> expression();
    virtual std::shared_ptr<TerminalNode> RP();
    PrecedenceContext(std::shared_ptr<ExpressionContext> ctx);
    template <typename T, typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public <T> T
    // accept(ParseTreeVisitor<? extends T> visitor)
    T accept(std::shared_ptr<ParseTreeVisitor<T1>> visitor);

  protected:
    std::shared_ptr<PrecedenceContext> shared_from_this()
    {
      return std::static_pointer_cast<PrecedenceContext>(
          ExpressionContext::shared_from_this());
    }
  };

public:
  class MuldivContext : public ExpressionContext
  {
    GET_CLASS_NAME(MuldivContext)
  public:
    virtual std::deque<std::shared_ptr<ExpressionContext>> expression();
    virtual std::shared_ptr<ExpressionContext> expression(int i);
    virtual std::shared_ptr<TerminalNode> MUL();
    virtual std::shared_ptr<TerminalNode> DIV();
    virtual std::shared_ptr<TerminalNode> REM();
    MuldivContext(std::shared_ptr<ExpressionContext> ctx);
    template <typename T, typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public <T> T
    // accept(ParseTreeVisitor<? extends T> visitor)
    T accept(std::shared_ptr<ParseTreeVisitor<T1>> visitor);

  protected:
    std::shared_ptr<MuldivContext> shared_from_this()
    {
      return std::static_pointer_cast<MuldivContext>(
          ExpressionContext::shared_from_this());
    }
  };

public:
  class ExternalContext : public ExpressionContext
  {
    GET_CLASS_NAME(ExternalContext)
  public:
    virtual std::shared_ptr<TerminalNode> VARIABLE();
    virtual std::shared_ptr<TerminalNode> LP();
    virtual std::shared_ptr<TerminalNode> RP();
    virtual std::deque<std::shared_ptr<ExpressionContext>> expression();
    virtual std::shared_ptr<ExpressionContext> expression(int i);
    virtual std::deque<std::shared_ptr<TerminalNode>> COMMA();
    virtual std::shared_ptr<TerminalNode> COMMA(int i);
    ExternalContext(std::shared_ptr<ExpressionContext> ctx);
    template <typename T, typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public <T> T
    // accept(ParseTreeVisitor<? extends T> visitor)
    T accept(std::shared_ptr<ParseTreeVisitor<T1>> visitor);

  protected:
    std::shared_ptr<ExternalContext> shared_from_this()
    {
      return std::static_pointer_cast<ExternalContext>(
          ExpressionContext::shared_from_this());
    }
  };

public:
  class BwshiftContext : public ExpressionContext
  {
    GET_CLASS_NAME(BwshiftContext)
  public:
    virtual std::deque<std::shared_ptr<ExpressionContext>> expression();
    virtual std::shared_ptr<ExpressionContext> expression(int i);
    virtual std::shared_ptr<TerminalNode> LSH();
    virtual std::shared_ptr<TerminalNode> RSH();
    virtual std::shared_ptr<TerminalNode> USH();
    BwshiftContext(std::shared_ptr<ExpressionContext> ctx);
    template <typename T, typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public <T> T
    // accept(ParseTreeVisitor<? extends T> visitor)
    T accept(std::shared_ptr<ParseTreeVisitor<T1>> visitor);

  protected:
    std::shared_ptr<BwshiftContext> shared_from_this()
    {
      return std::static_pointer_cast<BwshiftContext>(
          ExpressionContext::shared_from_this());
    }
  };

public:
  class BworContext : public ExpressionContext
  {
    GET_CLASS_NAME(BworContext)
  public:
    virtual std::deque<std::shared_ptr<ExpressionContext>> expression();
    virtual std::shared_ptr<ExpressionContext> expression(int i);
    virtual std::shared_ptr<TerminalNode> BWOR();
    BworContext(std::shared_ptr<ExpressionContext> ctx);
    template <typename T, typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public <T> T
    // accept(ParseTreeVisitor<? extends T> visitor)
    T accept(std::shared_ptr<ParseTreeVisitor<T1>> visitor);

  protected:
    std::shared_ptr<BworContext> shared_from_this()
    {
      return std::static_pointer_cast<BworContext>(
          ExpressionContext::shared_from_this());
    }
  };

public:
  class BoolandContext : public ExpressionContext
  {
    GET_CLASS_NAME(BoolandContext)
  public:
    virtual std::deque<std::shared_ptr<ExpressionContext>> expression();
    virtual std::shared_ptr<ExpressionContext> expression(int i);
    virtual std::shared_ptr<TerminalNode> BOOLAND();
    BoolandContext(std::shared_ptr<ExpressionContext> ctx);
    template <typename T, typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public <T> T
    // accept(ParseTreeVisitor<? extends T> visitor)
    T accept(std::shared_ptr<ParseTreeVisitor<T1>> visitor);

  protected:
    std::shared_ptr<BoolandContext> shared_from_this()
    {
      return std::static_pointer_cast<BoolandContext>(
          ExpressionContext::shared_from_this());
    }
  };

public:
  class BwxorContext : public ExpressionContext
  {
    GET_CLASS_NAME(BwxorContext)
  public:
    virtual std::deque<std::shared_ptr<ExpressionContext>> expression();
    virtual std::shared_ptr<ExpressionContext> expression(int i);
    virtual std::shared_ptr<TerminalNode> BWXOR();
    BwxorContext(std::shared_ptr<ExpressionContext> ctx);
    template <typename T, typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public <T> T
    // accept(ParseTreeVisitor<? extends T> visitor)
    T accept(std::shared_ptr<ParseTreeVisitor<T1>> visitor);

  protected:
    std::shared_ptr<BwxorContext> shared_from_this()
    {
      return std::static_pointer_cast<BwxorContext>(
          ExpressionContext::shared_from_this());
    }
  };

public:
  class BwandContext : public ExpressionContext
  {
    GET_CLASS_NAME(BwandContext)
  public:
    virtual std::deque<std::shared_ptr<ExpressionContext>> expression();
    virtual std::shared_ptr<ExpressionContext> expression(int i);
    virtual std::shared_ptr<TerminalNode> BWAND();
    BwandContext(std::shared_ptr<ExpressionContext> ctx);
    template <typename T, typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public <T> T
    // accept(ParseTreeVisitor<? extends T> visitor)
    T accept(std::shared_ptr<ParseTreeVisitor<T1>> visitor);

  protected:
    std::shared_ptr<BwandContext> shared_from_this()
    {
      return std::static_pointer_cast<BwandContext>(
          ExpressionContext::shared_from_this());
    }
  };

public:
  class BooleqneContext : public ExpressionContext
  {
    GET_CLASS_NAME(BooleqneContext)
  public:
    virtual std::deque<std::shared_ptr<ExpressionContext>> expression();
    virtual std::shared_ptr<ExpressionContext> expression(int i);
    virtual std::shared_ptr<TerminalNode> EQ();
    virtual std::shared_ptr<TerminalNode> NE();
    BooleqneContext(std::shared_ptr<ExpressionContext> ctx);
    template <typename T, typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: @Override public <T> T
    // accept(ParseTreeVisitor<? extends T> visitor)
    T accept(std::shared_ptr<ParseTreeVisitor<T1>> visitor);

  protected:
    std::shared_ptr<BooleqneContext> shared_from_this()
    {
      return std::static_pointer_cast<BooleqneContext>(
          ExpressionContext::shared_from_this());
    }
  };

public:
  std::shared_ptr<ExpressionContext> expression() ;

private:
  std::shared_ptr<ExpressionContext>
  expression(int _p) ;

public:
  virtual bool sempred(std::shared_ptr<RuleContext> _localctx, int ruleIndex,
                       int predIndex);

private:
  bool expression_sempred(std::shared_ptr<ExpressionContext> _localctx,
                          int predIndex);

public:
  static const std::wstring _serializedATN;
  static const std::shared_ptr<ATN> _ATN;

protected:
  std::shared_ptr<JavascriptParser> shared_from_this()
  {
    return std::static_pointer_cast<JavascriptParser>(
        Parser::shared_from_this());
  }
};

} // namespace org::apache::lucene::expressions::js
