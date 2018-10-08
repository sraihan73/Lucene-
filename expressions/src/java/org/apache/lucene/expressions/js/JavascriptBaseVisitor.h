#pragma once
#include "stringhelper.h"
#include <memory>

// ANTLR GENERATED CODE: DO NOT EDIT
namespace org::apache::lucene::expressions::js
{
using org::antlr::v4::runtime::tree::AbstractParseTreeVisitor;

/**
 * This class provides an empty implementation of {@link JavascriptVisitor},
 * which can be extended to create a visitor which only needs to handle a subset
 * of the available methods.
 *
 * @param <T> The return type of the visit operation. Use {@link Void} for
 * operations with no return type.
 */
template <typename T>
class JavascriptBaseVisitor : public AbstractParseTreeVisitor<T>,
                              public JavascriptVisitor<T>
{
  GET_CLASS_NAME(JavascriptBaseVisitor)
  /**
   * {@inheritDoc}
   *
   * <p>The default implementation returns the result of calling
   * {@link #visitChildren} on {@code ctx}.</p>
   */
public:
  T visitCompile(std::shared_ptr<JavascriptParser::CompileContext> ctx) override
  {
    return visitChildren(ctx);
  }
  T visitConditional(
      std::shared_ptr<JavascriptParser::ConditionalContext> ctx) override
  {
    return visitChildren(ctx);
  }
  T visitBoolor(std::shared_ptr<JavascriptParser::BoolorContext> ctx) override
  {
    return visitChildren(ctx);
  }
  T visitBoolcomp(
      std::shared_ptr<JavascriptParser::BoolcompContext> ctx) override
  {
    return visitChildren(ctx);
  }
  T visitNumeric(std::shared_ptr<JavascriptParser::NumericContext> ctx) override
  {
    return visitChildren(ctx);
  }
  T visitAddsub(std::shared_ptr<JavascriptParser::AddsubContext> ctx) override
  {
    return visitChildren(ctx);
  }
  T visitUnary(std::shared_ptr<JavascriptParser::UnaryContext> ctx) override
  {
    return visitChildren(ctx);
  }
  T visitPrecedence(
      std::shared_ptr<JavascriptParser::PrecedenceContext> ctx) override
  {
    return visitChildren(ctx);
  }
  T visitMuldiv(std::shared_ptr<JavascriptParser::MuldivContext> ctx) override
  {
    return visitChildren(ctx);
  }
  T visitExternal(
      std::shared_ptr<JavascriptParser::ExternalContext> ctx) override
  {
    return visitChildren(ctx);
  }
  T visitBwshift(std::shared_ptr<JavascriptParser::BwshiftContext> ctx) override
  {
    return visitChildren(ctx);
  }
  T visitBwor(std::shared_ptr<JavascriptParser::BworContext> ctx) override
  {
    return visitChildren(ctx);
  }
  T visitBooland(std::shared_ptr<JavascriptParser::BoolandContext> ctx) override
  {
    return visitChildren(ctx);
  }
  T visitBwxor(std::shared_ptr<JavascriptParser::BwxorContext> ctx) override
  {
    return visitChildren(ctx);
  }
  T visitBwand(std::shared_ptr<JavascriptParser::BwandContext> ctx) override
  {
    return visitChildren(ctx);
  }
  T visitBooleqne(
      std::shared_ptr<JavascriptParser::BooleqneContext> ctx) override
  {
    return visitChildren(ctx);
  }

protected:
  std::shared_ptr<JavascriptBaseVisitor> shared_from_this()
  {
    return std::static_pointer_cast<JavascriptBaseVisitor>(
        org.antlr.v4.runtime.tree
            .AbstractParseTreeVisitor<T>::shared_from_this());
  }
};

} // namespace org::apache::lucene::expressions::js
