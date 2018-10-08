#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/expressions/js/JavascriptParser.h"

#include  "core/src/java/org/apache/lucene/expressions/js/CompileContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/ConditionalContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/BoolorContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/BoolcompContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/NumericContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/AddsubContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/UnaryContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/PrecedenceContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/MuldivContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/ExternalContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/BwshiftContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/BworContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/BoolandContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/BwxorContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/BwandContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/BooleqneContext.h"

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

} // #include  "core/src/java/org/apache/lucene/expressions/js/
