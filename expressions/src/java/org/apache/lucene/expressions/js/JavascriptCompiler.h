#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/expressions/Expression.h"

#include  "core/src/java/org/apache/lucene/expressions/js/JavascriptParser.h"
#include  "core/src/java/org/apache/lucene/expressions/js/CompileContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/PrecedenceContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/NumericContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/ExternalContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/UnaryContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/MuldivContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/AddsubContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/BwshiftContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/BoolcompContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/BooleqneContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/BwandContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/BwxorContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/BworContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/BoolandContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/BoolorContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/ConditionalContext.h"
#include  "core/src/java/org/apache/lucene/expressions/js/ExpressionContext.h"

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::expressions::js
{

using org::antlr::v4::runtime::tree::ParseTree;
using Expression = org::apache::lucene::expressions::Expression;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using org::objectweb::asm_::ClassWriter;
using org::objectweb::asm_::Opcodes;
using org::objectweb::asm_::Type;

/**
 * An expression compiler for javascript expressions.
 * <p>
 * Example:
 * <pre class="prettyprint">
 *   Expression foo =
 * JavascriptCompiler.compile("((0.3*popularity)/10.0)+(0.7*score)");
 * </pre>
 * <p>
 * See the {@link org.apache.lucene.expressions.js package documentation} for
 * the supported syntax and default functions.
 * <p>
 * You can compile with an alternate set of functions via {@link
 * #compile(std::wstring, Map, ClassLoader)}. For example: <pre class="prettyprint">
 *   Map&lt;std::wstring,Method&gt; functions = new HashMap&lt;&gt;();
 *   // add all the default functions
 *   functions.putAll(JavascriptCompiler.DEFAULT_FUNCTIONS);
 *   // add cbrt()
 *   functions.put("cbrt", Math.class.getMethod("cbrt", double.class));
 *   // call compile with customized function map_obj
 *   Expression foo = JavascriptCompiler.compile("cbrt(score)+ln(popularity)",
 *                                               functions,
 *                                               getClass().getClassLoader());
 * </pre>
 *
 * @lucene.experimental
 */
class JavascriptCompiler final
    : public std::enable_shared_from_this<JavascriptCompiler>
{
  GET_CLASS_NAME(JavascriptCompiler)
public:
  class Loader final : public ClassLoader
  {
    GET_CLASS_NAME(Loader)
  public:
    Loader(std::shared_ptr<ClassLoader> parent);

    std::type_info define(const std::wstring &className,
                          std::deque<char> &bytecode);

  protected:
    std::shared_ptr<Loader> shared_from_this()
    {
      return std::static_pointer_cast<Loader>(ClassLoader::shared_from_this());
    }
  };

private:
  static const int CLASSFILE_VERSION = Opcodes::V1_8;

  // We use the same class name for all generated classes as they all have their
  // own class loader. The source code is displayed as "source file name" in
  // stack trace.
  static const std::wstring COMPILED_EXPRESSION_CLASS;
  static const std::wstring COMPILED_EXPRESSION_INTERNAL;

public:
  static const std::shared_ptr<Type> EXPRESSION_TYPE;
  static const std::shared_ptr<Type> FUNCTION_VALUES_TYPE;

private:
  static const std::shared_ptr<org::objectweb::asm_::commons::Method>
      EXPRESSION_CTOR, EVALUATE_METHOD;

public:
  static const std::shared_ptr<org::objectweb::asm_::commons::Method>
      DOUBLE_VAL_METHOD;

  /** create an ASM Method object from return type, method name, and parameters.
   */
private:
  static std::shared_ptr<org::objectweb::asm_::commons::Method>
  getAsmMethod(std::type_info rtype, const std::wstring &name,
               std::deque<std::type_info> &ptypes);

  // This maximum length is theoretically 65535 bytes, but as it's CESU-8
  // encoded we dont know how large it is in bytes, so be safe rcmuir: "If your
  // ranking function is that large you need to check yourself into a mental
  // institution!"
  static constexpr int MAX_SOURCE_LENGTH = 16384;

public:
  const std::wstring sourceText;
  const std::unordered_map<std::wstring, std::shared_ptr<Method>> functions;

  /**
   * Compiles the given expression.
   *
   * @param sourceText The expression to compile
   * @return A new compiled expression
   * @throws ParseException on failure to compile
   */
  static std::shared_ptr<Expression>
  compile(const std::wstring &sourceText) ;

  /**
   * Compiles the given expression with the supplied custom functions.
   * <p>
   * Functions must be {@code public static}, return {@code double} and
   * can take from zero to 256 {@code double} parameters.
   *
   * @param sourceText The expression to compile
   * @param functions map_obj of std::wstring names to functions
   * @param parent a {@code ClassLoader} that should be used as the parent of
the loaded class.
   *   It must contain all classes referred to by the given {@code functions}.
GET_CLASS_NAME(.)
   * @return A new compiled expression
   * @throws ParseException on failure to compile
   */
  static std::shared_ptr<Expression>
  compile(const std::wstring &sourceText,
          std::unordered_map<std::wstring, std::shared_ptr<Method>> &functions,
          std::shared_ptr<ClassLoader> parent) ;

  /**
   * This method is unused, it is just here to make sure that the function
   * signatures don't change. If this method fails to compile, you also have to
   * change the byte code generator to correctly use the FunctionValues class.
   */
private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"unused", "null"}) private static void
  // unusedTestCompile() throws java.io.IOException
  static void unusedTestCompile() ;

  /**
   * Constructs a compiler for expressions.
   * @param sourceText The expression to compile
   */
  JavascriptCompiler(const std::wstring &sourceText);

  /**
   * Constructs a compiler for expressions with specific set of functions
   * @param sourceText The expression to compile
   */
  JavascriptCompiler(
      const std::wstring &sourceText,
      std::unordered_map<std::wstring, std::shared_ptr<Method>> &functions);

  /**
   * Compiles the given expression with the specified parent classloader
   *
   * @return A new compiled expression
   * @throws ParseException on failure to compile
   */
  std::shared_ptr<Expression>
  compileExpression(std::shared_ptr<ClassLoader> parent) ;

  /**
   * Parses the sourceText into an ANTLR 4 parse tree
   *
   * @return The ANTLR parse tree
   * @throws ParseException on failure to parse
   */
  std::shared_ptr<ParseTree> getAntlrParseTree() ;

  /**
   * Sends the bytecode of class file to {@link ClassWriter}.
   */
  void generateClass(std::shared_ptr<ParseTree> parseTree,
                     std::shared_ptr<ClassWriter> classWriter,
                     std::unordered_map<std::wstring, int>
                         &externalsMap) ;

private:
  class JavascriptBaseVisitorAnonymousInnerClass
      : public JavascriptBaseVisitor<std::shared_ptr<Void>>
  {
    GET_CLASS_NAME(JavascriptBaseVisitorAnonymousInnerClass)
  private:
    std::shared_ptr<JavascriptCompiler> outerInstance;

    std::unordered_map<std::wstring, int> externalsMap;
    std::shared_ptr<GeneratorAdapter> gen;

  public:
    JavascriptBaseVisitorAnonymousInnerClass(
        std::shared_ptr<JavascriptCompiler> outerInstance,
        std::unordered_map<std::wstring, int> &externalsMap,
        std::shared_ptr<GeneratorAdapter> gen);

  private:
    const std::shared_ptr<Deque<std::shared_ptr<Type>>> typeStack;

  public:
    std::shared_ptr<Void> visitCompile(
        std::shared_ptr<JavascriptParser::CompileContext> ctx) override;

    std::shared_ptr<Void> visitPrecedence(
        std::shared_ptr<JavascriptParser::PrecedenceContext> ctx) override;

    std::shared_ptr<Void> visitNumeric(
        std::shared_ptr<JavascriptParser::NumericContext> ctx) override;

    std::shared_ptr<Void> visitExternal(
        std::shared_ptr<JavascriptParser::ExternalContext> ctx) override;

    std::shared_ptr<Void>
    visitUnary(std::shared_ptr<JavascriptParser::UnaryContext> ctx) override;

    std::shared_ptr<Void>
    visitMuldiv(std::shared_ptr<JavascriptParser::MuldivContext> ctx) override;

    std::shared_ptr<Void>
    visitAddsub(std::shared_ptr<JavascriptParser::AddsubContext> ctx) override;

    std::shared_ptr<Void> visitBwshift(
        std::shared_ptr<JavascriptParser::BwshiftContext> ctx) override;

    std::shared_ptr<Void> visitBoolcomp(
        std::shared_ptr<JavascriptParser::BoolcompContext> ctx) override;

    std::shared_ptr<Void> visitBooleqne(
        std::shared_ptr<JavascriptParser::BooleqneContext> ctx) override;

    std::shared_ptr<Void>
    visitBwand(std::shared_ptr<JavascriptParser::BwandContext> ctx) override;

    std::shared_ptr<Void>
    visitBwxor(std::shared_ptr<JavascriptParser::BwxorContext> ctx) override;

    std::shared_ptr<Void>
    visitBwor(std::shared_ptr<JavascriptParser::BworContext> ctx) override;

    std::shared_ptr<Void> visitBooland(
        std::shared_ptr<JavascriptParser::BoolandContext> ctx) override;

    std::shared_ptr<Void>
    visitBoolor(std::shared_ptr<JavascriptParser::BoolorContext> ctx) override;

    std::shared_ptr<Void> visitConditional(
        std::shared_ptr<JavascriptParser::ConditionalContext> ctx) override;

  private:
    void pushArith(int operator_, std::shared_ptr<ExpressionContext> left,
                   std::shared_ptr<ExpressionContext> right);

    void pushShift(int operator_, std::shared_ptr<ExpressionContext> left,
                   std::shared_ptr<ExpressionContext> right);

    void pushBitwise(int operator_, std::shared_ptr<ExpressionContext> left,
                     std::shared_ptr<ExpressionContext> right);

    void pushBinaryOp(int operator_, std::shared_ptr<ExpressionContext> left,
                      std::shared_ptr<ExpressionContext> right,
                      std::shared_ptr<Type> leftType,
                      std::shared_ptr<Type> rightType,
                      std::shared_ptr<Type> returnType);

    void pushCond(int operator_, std::shared_ptr<ExpressionContext> left,
                  std::shared_ptr<ExpressionContext> right);

    void pushBoolean(bool truth);

    void pushLong(int64_t i);

  protected:
    std::shared_ptr<JavascriptBaseVisitorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<JavascriptBaseVisitorAnonymousInnerClass>(
          JavascriptBaseVisitor<Void>::shared_from_this());
    }
  };

public:
  static std::wstring normalizeQuotes(const std::wstring &text);

  static int findSingleQuoteStringEnd(const std::wstring &text, int start);

  /**
   * The default set of functions available to expressions.
   * <p>
   * See the {@link org.apache.lucene.expressions.js package documentation}
   * for a deque.
   */
  static const std::unordered_map<std::wstring, std::shared_ptr<Method>>
      DEFAULT_FUNCTIONS;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static JavascriptCompiler::StaticConstructor staticConstructor;

  /** Check Method signature for compatibility. */
  static void checkFunction(std::shared_ptr<Method> method);

  /** Cross check if declaring class of given method is the same as
   * returned by the given parent {@link ClassLoader} on string lookup.
GET_CLASS_NAME(of)
   * This prevents {@link NoClassDefFoundError}.
   */
  static void checkFunctionClassLoader(std::shared_ptr<Method> method,
                                       std::shared_ptr<ClassLoader> parent);
};

} // #include  "core/src/java/org/apache/lucene/expressions/js/
