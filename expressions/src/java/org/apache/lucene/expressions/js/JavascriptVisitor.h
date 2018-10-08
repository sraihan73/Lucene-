#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::expressions::js
{
class JavascriptParser;
}

namespace org::apache::lucene::expressions::js
{
class CompileContext;
}
namespace org::apache::lucene::expressions::js
{
class ConditionalContext;
}
namespace org::apache::lucene::expressions::js
{
class BoolorContext;
}
namespace org::apache::lucene::expressions::js
{
class BoolcompContext;
}
namespace org::apache::lucene::expressions::js
{
class NumericContext;
}
namespace org::apache::lucene::expressions::js
{
class AddsubContext;
}
namespace org::apache::lucene::expressions::js
{
class UnaryContext;
}
namespace org::apache::lucene::expressions::js
{
class PrecedenceContext;
}
namespace org::apache::lucene::expressions::js
{
class MuldivContext;
}
namespace org::apache::lucene::expressions::js
{
class ExternalContext;
}
namespace org::apache::lucene::expressions::js
{
class BwshiftContext;
}
namespace org::apache::lucene::expressions::js
{
class BworContext;
}
namespace org::apache::lucene::expressions::js
{
class BoolandContext;
}
namespace org::apache::lucene::expressions::js
{
class BwxorContext;
}
namespace org::apache::lucene::expressions::js
{
class BwandContext;
}
namespace org::apache::lucene::expressions::js
{
class BooleqneContext;
}

// ANTLR GENERATED CODE: DO NOT EDIT
namespace org::apache::lucene::expressions::js
{
using org::antlr::v4::runtime::tree::ParseTreeVisitor;

/**
 * This interface defines a complete generic visitor for a parse tree produced
 * by {@link JavascriptParser}.
 *
 * @param <T> The return type of the visit operation. Use {@link Void} for
 * operations with no return type.
 */
template <typename T>
class JavascriptVisitor : public ParseTreeVisitor<T>
{
  GET_CLASS_NAME(JavascriptVisitor)
  /**
   * Visit a parse tree produced by {@link JavascriptParser#compile}.
   * @param ctx the parse tree
   * @return the visitor result
   */
public:
  virtual T
  visitCompile(std::shared_ptr<JavascriptParser::CompileContext> ctx) = 0;
  /**
   * Visit a parse tree produced by the {@code conditional}
   * labeled alternative in {@link JavascriptParser#expression}.
   * @param ctx the parse tree
   * @return the visitor result
   */
  virtual T visitConditional(
      std::shared_ptr<JavascriptParser::ConditionalContext> ctx) = 0;
  /**
   * Visit a parse tree produced by the {@code boolor}
   * labeled alternative in {@link JavascriptParser#expression}.
   * @param ctx the parse tree
   * @return the visitor result
   */
  virtual T
  visitBoolor(std::shared_ptr<JavascriptParser::BoolorContext> ctx) = 0;
  /**
   * Visit a parse tree produced by the {@code boolcomp}
   * labeled alternative in {@link JavascriptParser#expression}.
   * @param ctx the parse tree
   * @return the visitor result
   */
  virtual T
  visitBoolcomp(std::shared_ptr<JavascriptParser::BoolcompContext> ctx) = 0;
  /**
   * Visit a parse tree produced by the {@code numeric}
   * labeled alternative in {@link JavascriptParser#expression}.
   * @param ctx the parse tree
   * @return the visitor result
   */
  virtual T
  visitNumeric(std::shared_ptr<JavascriptParser::NumericContext> ctx) = 0;
  /**
   * Visit a parse tree produced by the {@code addsub}
   * labeled alternative in {@link JavascriptParser#expression}.
   * @param ctx the parse tree
   * @return the visitor result
   */
  virtual T
  visitAddsub(std::shared_ptr<JavascriptParser::AddsubContext> ctx) = 0;
  /**
   * Visit a parse tree produced by the {@code unary}
   * labeled alternative in {@link JavascriptParser#expression}.
   * @param ctx the parse tree
   * @return the visitor result
   */
  virtual T visitUnary(std::shared_ptr<JavascriptParser::UnaryContext> ctx) = 0;
  /**
   * Visit a parse tree produced by the {@code precedence}
   * labeled alternative in {@link JavascriptParser#expression}.
   * @param ctx the parse tree
   * @return the visitor result
   */
  virtual T
  visitPrecedence(std::shared_ptr<JavascriptParser::PrecedenceContext> ctx) = 0;
  /**
   * Visit a parse tree produced by the {@code muldiv}
   * labeled alternative in {@link JavascriptParser#expression}.
   * @param ctx the parse tree
   * @return the visitor result
   */
  virtual T
  visitMuldiv(std::shared_ptr<JavascriptParser::MuldivContext> ctx) = 0;
  /**
   * Visit a parse tree produced by the {@code external}
   * labeled alternative in {@link JavascriptParser#expression}.
   * @param ctx the parse tree
   * @return the visitor result
   */
  virtual T
  visitExternal(std::shared_ptr<JavascriptParser::ExternalContext> ctx) = 0;
  /**
   * Visit a parse tree produced by the {@code bwshift}
   * labeled alternative in {@link JavascriptParser#expression}.
   * @param ctx the parse tree
   * @return the visitor result
   */
  virtual T
  visitBwshift(std::shared_ptr<JavascriptParser::BwshiftContext> ctx) = 0;
  /**
   * Visit a parse tree produced by the {@code bwor}
   * labeled alternative in {@link JavascriptParser#expression}.
   * @param ctx the parse tree
   * @return the visitor result
   */
  virtual T visitBwor(std::shared_ptr<JavascriptParser::BworContext> ctx) = 0;
  /**
   * Visit a parse tree produced by the {@code booland}
   * labeled alternative in {@link JavascriptParser#expression}.
   * @param ctx the parse tree
   * @return the visitor result
   */
  virtual T
  visitBooland(std::shared_ptr<JavascriptParser::BoolandContext> ctx) = 0;
  /**
   * Visit a parse tree produced by the {@code bwxor}
   * labeled alternative in {@link JavascriptParser#expression}.
   * @param ctx the parse tree
   * @return the visitor result
   */
  virtual T visitBwxor(std::shared_ptr<JavascriptParser::BwxorContext> ctx) = 0;
  /**
   * Visit a parse tree produced by the {@code bwand}
   * labeled alternative in {@link JavascriptParser#expression}.
   * @param ctx the parse tree
   * @return the visitor result
   */
  virtual T visitBwand(std::shared_ptr<JavascriptParser::BwandContext> ctx) = 0;
  /**
   * Visit a parse tree produced by the {@code booleqne}
   * labeled alternative in {@link JavascriptParser#expression}.
   * @param ctx the parse tree
   * @return the visitor result
   */
  virtual T
  visitBooleqne(std::shared_ptr<JavascriptParser::BooleqneContext> ctx) = 0;
};

} // namespace org::apache::lucene::expressions::js
