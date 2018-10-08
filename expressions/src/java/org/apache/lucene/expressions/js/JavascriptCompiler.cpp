using namespace std;

#include "JavascriptCompiler.h"

namespace org::apache::lucene::expressions::js
{
using org::antlr::v4::runtime::ANTLRInputStream;
using org::antlr::v4::runtime::CommonTokenStream;
using org::antlr::v4::runtime::tree::ParseTree;
using Expression = org::apache::lucene::expressions::Expression;
using ExpressionContext =
    org::apache::lucene::expressions::js::JavascriptParser::ExpressionContext;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using IOUtils = org::apache::lucene::util::IOUtils;
using org::objectweb::asm_::ClassWriter;
using org::objectweb::asm_::Label;
using org::objectweb::asm_::Opcodes;
using org::objectweb::asm_::Type;
using org::objectweb::asm_::commons::GeneratorAdapter;

JavascriptCompiler::Loader::Loader(shared_ptr<ClassLoader> parent)
    : ClassLoader(parent)
{
}

type_info JavascriptCompiler::Loader::define(const wstring &className,
                                             std::deque<char> &bytecode)
{
  return defineClass(className, bytecode, 0, bytecode.size())
      .asSubclass(Expression::typeid);
}

const wstring JavascriptCompiler::COMPILED_EXPRESSION_CLASS =
    JavascriptCompiler::typeid->getName() + L"$CompiledExpression";
const wstring JavascriptCompiler::COMPILED_EXPRESSION_INTERNAL =
    StringHelper::replace(COMPILED_EXPRESSION_CLASS, L'.', L'/');
const shared_ptr<org::objectweb::asm_::Type>
    JavascriptCompiler::EXPRESSION_TYPE = org::objectweb::asm_::Type::getType(
        org::apache::lucene::expressions::Expression::typeid);
const shared_ptr<org::objectweb::asm_::Type>
    JavascriptCompiler::FUNCTION_VALUES_TYPE =
        org::objectweb::asm_::Type::getType(
            org::apache::lucene::search::DoubleValues::typeid);
const shared_ptr<org::objectweb::asm_::commons::Method> EXPRESSION_CTOR,
    JavascriptCompiler::EVALUATE_METHOD =
        getAsmMethod(double ::typeid, L"evaluate",
                     {std::deque<std::shared_ptr<
                         org::apache::lucene::search::DoubleValues>>::typeid});
const shared_ptr<org::objectweb::asm_::commons::Method>
    JavascriptCompiler::DOUBLE_VAL_METHOD =
        getAsmMethod(double ::typeid, L"doubleValue");

shared_ptr<org::objectweb::asm_::commons::Method>
JavascriptCompiler::getAsmMethod(type_info rtype, const wstring &name,
                                 deque<type_info> &ptypes)
{
  return make_shared<org::objectweb::asm_::commons::Method>(
      name, MethodType::methodType(rtype, ptypes).toMethodDescriptorString());
}

shared_ptr<Expression>
JavascriptCompiler::compile(const wstring &sourceText) 
{
  return (make_shared<JavascriptCompiler>(sourceText))
      ->compileExpression(JavascriptCompiler::typeid->getClassLoader());
}

shared_ptr<Expression> JavascriptCompiler::compile(
    const wstring &sourceText,
    unordered_map<wstring, std::shared_ptr<Method>> &functions,
    shared_ptr<ClassLoader> parent) 
{
  if (parent == nullptr) {
    throw make_shared<NullPointerException>(
        L"A parent ClassLoader must be given.");
  }
  for (auto m : functions) {
    checkFunctionClassLoader(m->second, parent);
    checkFunction(m->second);
  }
  return (make_shared<JavascriptCompiler>(sourceText, functions))
      ->compileExpression(parent);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings({"unused", "null"}) private static void
// unusedTestCompile() throws java.io.IOException
void JavascriptCompiler::unusedTestCompile() 
{
  shared_ptr<DoubleValues> f = nullptr;
  double ret = f->doubleValue();
}

JavascriptCompiler::JavascriptCompiler(const wstring &sourceText)
    : JavascriptCompiler(sourceText, DEFAULT_FUNCTIONS)
{
}

JavascriptCompiler::JavascriptCompiler(
    const wstring &sourceText,
    unordered_map<wstring, std::shared_ptr<Method>> &functions)
    : sourceText(sourceText), functions(functions)
{
  if (sourceText == L"") {
    throw make_shared<NullPointerException>();
  }
}

shared_ptr<Expression> JavascriptCompiler::compileExpression(
    shared_ptr<ClassLoader> parent) 
{
  const unordered_map<wstring, int> externalsMap =
      make_shared<LinkedHashMap<wstring, int>>();
  shared_ptr<ClassWriter> *const classWriter = make_shared<ClassWriter>(
      ClassWriter::COMPUTE_FRAMES | ClassWriter::COMPUTE_MAXS);

  try {
    generateClass(getAntlrParseTree(), classWriter, externalsMap);

    constexpr type_info evaluatorClass =
        (make_shared<Loader>(parent))
            ->define(COMPILED_EXPRESSION_CLASS, classWriter->toByteArray());
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: final Constructor<? extends
    // org.apache.lucene.expressions.Expression> constructor =
    // evaluatorClass.getConstructor(std::wstring.class, std::wstring[].class);
    shared_ptr < Constructor <
        ? extends Expression >> *const constructor =
              evaluatorClass.getConstructor(wstring::typeid,
                                            std::deque<wstring>::typeid);

    return constructor->newInstance(
        sourceText, externalsMap.keySet().toArray(
                        std::deque<wstring>(externalsMap.size())));
  } catch (const runtime_error &re) {
    if (std::dynamic_pointer_cast<ParseException>(re.getCause()) != nullptr) {
      throw std::static_pointer_cast<ParseException>(re.getCause());
    }
    throw re;
  } catch (const ReflectiveOperationException &exception) {
    throw make_shared<IllegalStateException>(
        L"An internal error occurred attempting to compile the expression (" +
            sourceText + L").",
        exception);
  }
}

shared_ptr<ParseTree>
JavascriptCompiler::getAntlrParseTree() 
{
  shared_ptr<ANTLRInputStream> *const antlrInputStream =
      make_shared<ANTLRInputStream>(sourceText);
  shared_ptr<JavascriptErrorHandlingLexer> *const javascriptLexer =
      make_shared<JavascriptErrorHandlingLexer>(antlrInputStream);
  javascriptLexer->removeErrorListeners();
  shared_ptr<JavascriptParser> *const javascriptParser =
      make_shared<JavascriptParser>(
          make_shared<CommonTokenStream>(javascriptLexer));
  javascriptParser->removeErrorListeners();
  javascriptParser->setErrorHandler(
      make_shared<JavascriptParserErrorStrategy>());
  return javascriptParser->compile();
}

void JavascriptCompiler::generateClass(
    shared_ptr<ParseTree> parseTree, shared_ptr<ClassWriter> classWriter,
    unordered_map<wstring, int> &externalsMap) 
{
  classWriter->visit(CLASSFILE_VERSION,
                     Opcodes::ACC_PUBLIC | Opcodes::ACC_SUPER |
                         Opcodes::ACC_FINAL,
                     COMPILED_EXPRESSION_INTERNAL, nullptr,
                     EXPRESSION_TYPE->getInternalName(), nullptr);
  const wstring clippedSourceText =
      (sourceText.length() <= MAX_SOURCE_LENGTH)
          ? sourceText
          : (sourceText.substr(0, MAX_SOURCE_LENGTH - 3) + L"...");
  classWriter->visitSource(clippedSourceText, nullptr);

  shared_ptr<GeneratorAdapter> *const constructor =
      make_shared<GeneratorAdapter>(Opcodes::ACC_PUBLIC, EXPRESSION_CTOR,
                                    nullptr, nullptr, classWriter);
  constructor->loadThis();
  constructor->loadArgs();
  constructor->invokeConstructor(EXPRESSION_TYPE, EXPRESSION_CTOR);
  constructor->returnValue();
  constructor->endMethod();

  shared_ptr<GeneratorAdapter> *const gen = make_shared<GeneratorAdapter>(
      Opcodes::ACC_PUBLIC, EVALUATE_METHOD, nullptr, nullptr, classWriter);

  // to completely hide the ANTLR visitor we use an anonymous impl:
  make_shared<JavascriptBaseVisitorAnonymousInnerClass>(shared_from_this(),
                                                        externalsMap, gen)
      .visit(parseTree);

  gen->returnValue();
  gen->endMethod();

  classWriter->visitEnd();
}

JavascriptCompiler::JavascriptBaseVisitorAnonymousInnerClass::
    JavascriptBaseVisitorAnonymousInnerClass(
        shared_ptr<JavascriptCompiler> outerInstance,
        unordered_map<wstring, int> &externalsMap,
        shared_ptr<GeneratorAdapter> gen)
{
  this->outerInstance = outerInstance;
  this->externalsMap = externalsMap;
  this->gen = gen;
  typeStack = make_shared<ArrayDeque<>>();
}

shared_ptr<Void>
JavascriptCompiler::JavascriptBaseVisitorAnonymousInnerClass::visitCompile(
    shared_ptr<JavascriptParser::CompileContext> ctx)
{
  typeStack::push(Type::DOUBLE_TYPE);
  visit(ctx->expression());
  typeStack->pop();

  return nullptr;
}

shared_ptr<Void>
JavascriptCompiler::JavascriptBaseVisitorAnonymousInnerClass::visitPrecedence(
    shared_ptr<JavascriptParser::PrecedenceContext> ctx)
{
  visit(ctx->expression());

  return nullptr;
}

shared_ptr<Void>
JavascriptCompiler::JavascriptBaseVisitorAnonymousInnerClass::visitNumeric(
    shared_ptr<JavascriptParser::NumericContext> ctx)
{
  if (ctx->HEX() != nullptr) {
    // C++ TODO: Only single-argument parse and valueOf methods are converted:
    // ORIGINAL LINE: pushLong(Long.parseLong(ctx.HEX().getText().substring(2),
    // 16));
    pushLong(int64_t ::valueOf(ctx->HEX()->getText()->substr(2), 16));
  } else if (ctx->OCTAL() != nullptr) {
    // C++ TODO: Only single-argument parse and valueOf methods are converted:
    // ORIGINAL LINE:
    // pushLong(Long.parseLong(ctx.OCTAL().getText().substring(1), 8));
    pushLong(int64_t ::valueOf(ctx->OCTAL()->getText()->substr(1), 8));
  } else if (ctx->DECIMAL() != nullptr) {
    gen->push(static_cast<Double>(ctx->DECIMAL()->getText()));
    gen->cast(Type::DOUBLE_TYPE, typeStack->peek());
  } else {
    throw make_shared<IllegalStateException>(L"Unknown operation specified: " +
                                             ctx->getText());
  }

  return nullptr;
}

shared_ptr<Void>
JavascriptCompiler::JavascriptBaseVisitorAnonymousInnerClass::visitExternal(
    shared_ptr<JavascriptParser::ExternalContext> ctx)
{
  wstring text = ctx->VARIABLE()->getText();
  int arguments = ctx->expression().size();
  bool parens = ctx->LP() != nullptr && ctx->RP() != nullptr;
  shared_ptr<Method> method = parens ? outerInstance->functions[text] : nullptr;

  try {
    if (method != nullptr) {
      int arity = method->getParameterTypes()->length;

      if (arguments != arity) {
        throw make_shared<ParseException>(
            L"Invalid expression '" + outerInstance->sourceText +
                L"': Expected (" + to_wstring(arity) +
                L") arguments for function call (" + text + L"), but found (" +
                to_wstring(arguments) + L").",
            ctx->start.getStartIndex());
      }

      typeStack::push(Type::DOUBLE_TYPE);

      for (int argument = 0; argument < arguments; ++argument) {
        visit(ctx->expression(argument));
      }

      typeStack->pop();

      gen->invokeStatic(
          Type::getType(method->getDeclaringClass()),
          org::objectweb::asm_::commons::Method::getMethod(method));

      gen->cast(Type::DOUBLE_TYPE, typeStack->peek());
    } else if (!parens || arguments == 0 && text.find(L".") != wstring::npos) {
      int index;

      text = normalizeQuotes(ctx->getText());

      if (externalsMap.find(text) != externalsMap.end()) {
        index = externalsMap[text];
      } else {
        index = externalsMap.size();
        externalsMap.emplace(text, index);
      }

      gen->loadArg(0);
      gen->push(index);
      gen->arrayLoad(FUNCTION_VALUES_TYPE);
      gen->invokeVirtual(FUNCTION_VALUES_TYPE, DOUBLE_VAL_METHOD);
      gen->cast(Type::DOUBLE_TYPE, typeStack->peek());
    } else {
      throw make_shared<ParseException>(
          L"Invalid expression '" + outerInstance->sourceText +
              L"': Unrecognized function call (" + text + L").",
          ctx->start.getStartIndex());
    }
    return nullptr;
  } catch (const ParseException &e) {
    // The API doesn't allow checked exceptions here, so propagate up the stack.
    // This is unwrapped in getAntlrParseTree.
    throw runtime_error(e);
  }
}

shared_ptr<Void>
JavascriptCompiler::JavascriptBaseVisitorAnonymousInnerClass::visitUnary(
    shared_ptr<JavascriptParser::UnaryContext> ctx)
{
  if (ctx->BOOLNOT() != nullptr) {
    shared_ptr<Label> labelNotTrue = make_shared<Label>();
    shared_ptr<Label> labelNotReturn = make_shared<Label>();

    typeStack::push(Type::INT_TYPE);
    visit(ctx->expression());
    typeStack->pop();
    gen->visitJumpInsn(Opcodes::IFEQ, labelNotTrue);
    pushBoolean(false);
    gen->goTo(labelNotReturn);
    gen->visitLabel(labelNotTrue);
    pushBoolean(true);
    gen->visitLabel(labelNotReturn);

  } else if (ctx->BWNOT() != nullptr) {
    typeStack::push(Type::LONG_TYPE);
    visit(ctx->expression());
    typeStack->pop();
    gen->push(-1LL);
    gen->visitInsn(Opcodes::LXOR);
    gen->cast(Type::LONG_TYPE, typeStack->peek());

  } else if (ctx->ADD() != nullptr) {
    visit(ctx->expression());

  } else if (ctx->SUB() != nullptr) {
    typeStack::push(Type::DOUBLE_TYPE);
    visit(ctx->expression());
    typeStack->pop();
    gen->visitInsn(Opcodes::DNEG);
    gen->cast(Type::DOUBLE_TYPE, typeStack->peek());

  } else {
    throw make_shared<IllegalStateException>(L"Unknown operation specified: " +
                                             ctx->getText());
  }

  return nullptr;
}

shared_ptr<Void>
JavascriptCompiler::JavascriptBaseVisitorAnonymousInnerClass::visitMuldiv(
    shared_ptr<JavascriptParser::MuldivContext> ctx)
{
  int opcode;

  if (ctx->MUL() != nullptr) {
    opcode = Opcodes::DMUL;
  } else if (ctx->DIV() != nullptr) {
    opcode = Opcodes::DDIV;
  } else if (ctx->REM() != nullptr) {
    opcode = Opcodes::DREM;
  } else {
    throw make_shared<IllegalStateException>(L"Unknown operation specified: " +
                                             ctx->getText());
  }

  pushArith(opcode, ctx->expression(0), ctx->expression(1));

  return nullptr;
}

shared_ptr<Void>
JavascriptCompiler::JavascriptBaseVisitorAnonymousInnerClass::visitAddsub(
    shared_ptr<JavascriptParser::AddsubContext> ctx)
{
  int opcode;

  if (ctx->ADD() != nullptr) {
    opcode = Opcodes::DADD;
  } else if (ctx->SUB() != nullptr) {
    opcode = Opcodes::DSUB;
  } else {
    throw make_shared<IllegalStateException>(L"Unknown operation specified: " +
                                             ctx->getText());
  }

  pushArith(opcode, ctx->expression(0), ctx->expression(1));

  return nullptr;
}

shared_ptr<Void>
JavascriptCompiler::JavascriptBaseVisitorAnonymousInnerClass::visitBwshift(
    shared_ptr<JavascriptParser::BwshiftContext> ctx)
{
  int opcode;

  if (ctx->LSH() != nullptr) {
    opcode = Opcodes::LSHL;
  } else if (ctx->RSH() != nullptr) {
    opcode = Opcodes::LSHR;
  } else if (ctx->USH() != nullptr) {
    opcode = Opcodes::LUSHR;
  } else {
    throw make_shared<IllegalStateException>(L"Unknown operation specified: " +
                                             ctx->getText());
  }

  pushShift(opcode, ctx->expression(0), ctx->expression(1));

  return nullptr;
}

shared_ptr<Void>
JavascriptCompiler::JavascriptBaseVisitorAnonymousInnerClass::visitBoolcomp(
    shared_ptr<JavascriptParser::BoolcompContext> ctx)
{
  int opcode;

  if (ctx->LT() != nullptr) {
    opcode = GeneratorAdapter::LT;
  } else if (ctx->LTE() != nullptr) {
    opcode = GeneratorAdapter::LE;
  } else if (ctx->GT() != nullptr) {
    opcode = GeneratorAdapter::GT;
  } else if (ctx->GTE() != nullptr) {
    opcode = GeneratorAdapter::GE;
  } else {
    throw make_shared<IllegalStateException>(L"Unknown operation specified: " +
                                             ctx->getText());
  }

  pushCond(opcode, ctx->expression(0), ctx->expression(1));

  return nullptr;
}

shared_ptr<Void>
JavascriptCompiler::JavascriptBaseVisitorAnonymousInnerClass::visitBooleqne(
    shared_ptr<JavascriptParser::BooleqneContext> ctx)
{
  int opcode;

  if (ctx->EQ() != nullptr) {
    opcode = GeneratorAdapter::EQ;
  } else if (ctx->NE() != nullptr) {
    opcode = GeneratorAdapter::NE;
  } else {
    throw make_shared<IllegalStateException>(L"Unknown operation specified: " +
                                             ctx->getText());
  }

  pushCond(opcode, ctx->expression(0), ctx->expression(1));

  return nullptr;
}

shared_ptr<Void>
JavascriptCompiler::JavascriptBaseVisitorAnonymousInnerClass::visitBwand(
    shared_ptr<JavascriptParser::BwandContext> ctx)
{
  pushBitwise(Opcodes::LAND, ctx->expression(0), ctx->expression(1));

  return nullptr;
}

shared_ptr<Void>
JavascriptCompiler::JavascriptBaseVisitorAnonymousInnerClass::visitBwxor(
    shared_ptr<JavascriptParser::BwxorContext> ctx)
{
  pushBitwise(Opcodes::LXOR, ctx->expression(0), ctx->expression(1));

  return nullptr;
}

shared_ptr<Void>
JavascriptCompiler::JavascriptBaseVisitorAnonymousInnerClass::visitBwor(
    shared_ptr<JavascriptParser::BworContext> ctx)
{
  pushBitwise(Opcodes::LOR, ctx->expression(0), ctx->expression(1));

  return nullptr;
}

shared_ptr<Void>
JavascriptCompiler::JavascriptBaseVisitorAnonymousInnerClass::visitBooland(
    shared_ptr<JavascriptParser::BoolandContext> ctx)
{
  shared_ptr<Label> andFalse = make_shared<Label>();
  shared_ptr<Label> andEnd = make_shared<Label>();

  typeStack::push(Type::INT_TYPE);
  visit(ctx->expression(0));
  gen->visitJumpInsn(Opcodes::IFEQ, andFalse);
  visit(ctx->expression(1));
  gen->visitJumpInsn(Opcodes::IFEQ, andFalse);
  typeStack->pop();
  pushBoolean(true);
  gen->goTo(andEnd);
  gen->visitLabel(andFalse);
  pushBoolean(false);
  gen->visitLabel(andEnd);

  return nullptr;
}

shared_ptr<Void>
JavascriptCompiler::JavascriptBaseVisitorAnonymousInnerClass::visitBoolor(
    shared_ptr<JavascriptParser::BoolorContext> ctx)
{
  shared_ptr<Label> orTrue = make_shared<Label>();
  shared_ptr<Label> orEnd = make_shared<Label>();

  typeStack::push(Type::INT_TYPE);
  visit(ctx->expression(0));
  gen->visitJumpInsn(Opcodes::IFNE, orTrue);
  visit(ctx->expression(1));
  gen->visitJumpInsn(Opcodes::IFNE, orTrue);
  typeStack->pop();
  pushBoolean(false);
  gen->goTo(orEnd);
  gen->visitLabel(orTrue);
  pushBoolean(true);
  gen->visitLabel(orEnd);

  return nullptr;
}

shared_ptr<Void>
JavascriptCompiler::JavascriptBaseVisitorAnonymousInnerClass::visitConditional(
    shared_ptr<JavascriptParser::ConditionalContext> ctx)
{
  shared_ptr<Label> condFalse = make_shared<Label>();
  shared_ptr<Label> condEnd = make_shared<Label>();

  typeStack::push(Type::INT_TYPE);
  visit(ctx->expression(0));
  typeStack->pop();
  gen->visitJumpInsn(Opcodes::IFEQ, condFalse);
  visit(ctx->expression(1));
  gen->goTo(condEnd);
  gen->visitLabel(condFalse);
  visit(ctx->expression(2));
  gen->visitLabel(condEnd);

  return nullptr;
}

void JavascriptCompiler::JavascriptBaseVisitorAnonymousInnerClass::pushArith(
    int operator_, shared_ptr<ExpressionContext> left,
    shared_ptr<ExpressionContext> right)
{
  pushBinaryOp(operator_, left, right, Type::DOUBLE_TYPE, Type::DOUBLE_TYPE,
               Type::DOUBLE_TYPE);
}

void JavascriptCompiler::JavascriptBaseVisitorAnonymousInnerClass::pushShift(
    int operator_, shared_ptr<ExpressionContext> left,
    shared_ptr<ExpressionContext> right)
{
  pushBinaryOp(operator_, left, right, Type::LONG_TYPE, Type::INT_TYPE,
               Type::LONG_TYPE);
}

void JavascriptCompiler::JavascriptBaseVisitorAnonymousInnerClass::pushBitwise(
    int operator_, shared_ptr<ExpressionContext> left,
    shared_ptr<ExpressionContext> right)
{
  pushBinaryOp(operator_, left, right, Type::LONG_TYPE, Type::LONG_TYPE,
               Type::LONG_TYPE);
}

void JavascriptCompiler::JavascriptBaseVisitorAnonymousInnerClass::pushBinaryOp(
    int operator_, shared_ptr<ExpressionContext> left,
    shared_ptr<ExpressionContext> right, shared_ptr<Type> leftType,
    shared_ptr<Type> rightType, shared_ptr<Type> returnType)
{
  typeStack::push(leftType);
  visit(left);
  typeStack->pop();
  typeStack::push(rightType);
  visit(right);
  typeStack->pop();
  gen->visitInsn(operator_);
  gen->cast(returnType, typeStack->peek());
}

void JavascriptCompiler::JavascriptBaseVisitorAnonymousInnerClass::pushCond(
    int operator_, shared_ptr<ExpressionContext> left,
    shared_ptr<ExpressionContext> right)
{
  shared_ptr<Label> labelTrue = make_shared<Label>();
  shared_ptr<Label> labelReturn = make_shared<Label>();

  typeStack::push(Type::DOUBLE_TYPE);
  visit(left);
  visit(right);
  typeStack->pop();

  gen->ifCmp(Type::DOUBLE_TYPE, operator_, labelTrue);
  pushBoolean(false);
  gen->goTo(labelReturn);
  gen->visitLabel(labelTrue);
  pushBoolean(true);
  gen->visitLabel(labelReturn);
}

void JavascriptCompiler::JavascriptBaseVisitorAnonymousInnerClass::pushBoolean(
    bool truth)
{
  switch (typeStack->peek().getSort()) {
  case Type::INT:
    gen->push(truth);
    break;
  case Type::LONG:
    gen->push(truth ? 1LL : 0LL);
    break;
  case Type::DOUBLE:
    gen->push(truth ? 1.0 : 0.0);
    break;
  default:
    throw make_shared<IllegalStateException>(L"Invalid expected type: " +
                                             typeStack->peek());
  }
}

void JavascriptCompiler::JavascriptBaseVisitorAnonymousInnerClass::pushLong(
    int64_t i)
{
  switch (typeStack->peek().getSort()) {
  case Type::INT:
    gen->push(static_cast<int>(i));
    break;
  case Type::LONG:
    gen->push(i);
    break;
  case Type::DOUBLE:
    gen->push(static_cast<double>(i));
    break;
  default:
    throw make_shared<IllegalStateException>(L"Invalid expected type: " +
                                             typeStack->peek());
  }
}

wstring JavascriptCompiler::normalizeQuotes(const wstring &text)
{
  shared_ptr<StringBuilder> out = make_shared<StringBuilder>(text.length());
  bool inDoubleQuotes = false;
  for (int i = 0; i < text.length(); ++i) {
    wchar_t c = text[i];
    if (c == L'\\') {
      c = text[++i];
      if (c == L'\\') {
        out->append(L'\\'); // re-escape the backslash
      }
      // no escape for double quote
    } else if (c == L'\'') {
      if (inDoubleQuotes) {
        // escape in output
        out->append(L'\\');
      } else {
        int j = findSingleQuoteStringEnd(text, i);
        out->append(text, i,
                    j); // copy up to end quote (leave end for append below)
        i = j;
      }
    } else if (c == L'"') {
      c = L'\''; // change beginning/ending doubles to singles
      inDoubleQuotes = !inDoubleQuotes;
    }
    out->append(c);
  }
  return out->toString();
}

int JavascriptCompiler::findSingleQuoteStringEnd(const wstring &text, int start)
{
  ++start; // skip beginning
  while (text[start] != L'\'') {
    if (text[start] == L'\\') {
      ++start; // blindly consume escape value
    }
    ++start;
  }
  return start;
}

const unordered_map<wstring, std::shared_ptr<Method>>
    JavascriptCompiler::DEFAULT_FUNCTIONS;

JavascriptCompiler::StaticConstructor::StaticConstructor()
{
  unordered_map<wstring, std::shared_ptr<Method>> map_obj =
      unordered_map<wstring, std::shared_ptr<Method>>();
  try {
    shared_ptr<Properties> *const props = make_shared<Properties>();
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (java.io.Reader in =
    // org.apache.lucene.util.IOUtils.getDecodingReader(JavascriptCompiler.class,
    // JavascriptCompiler.class.getSimpleName() + ".properties",
    // java.nio.charset.StandardCharsets.UTF_8))
    {
      java::io::Reader in_ =
          org::apache::lucene::util::IOUtils::getDecodingReader(
              JavascriptCompiler::typeid,
              JavascriptCompiler::typeid->getSimpleName() + L".properties",
              java::nio::charset::StandardCharsets::UTF_8);
      props->load(in_);
    }
    for (wstring call : props->stringPropertyNames()) {
      const std::deque<wstring> vals = props->getProperty(call)->split(L",");
      if (vals.size() != 3) {
        throw make_shared<Error>(
            L"Syntax error while reading Javascript functions from resource");
      }
      constexpr type_info clazz =
          type_info::forName(StringHelper::trim(vals[0]));
      const wstring methodName = StringHelper::trim(vals[1]);
      constexpr int arity = static_cast<Integer>(StringHelper::trim(vals[2]));
      // C++ TODO: Most Java annotations will not have direct C++ equivalents:
      // ORIGINAL LINE: @SuppressWarnings({"rawtypes", "unchecked"}) Class[]
      // args = new Class[arity];
      std::deque<type_info> args(arity);
      Arrays::fill(args, double ::typeid);
      shared_ptr<Method> method = clazz.getMethod(methodName, args);
      checkFunction(method);
      map_obj.emplace(call, method);
    }
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (ReflectiveOperationException | IOException e) {
    throw make_shared<Error>(L"Cannot resolve function", e);
  }
  DEFAULT_FUNCTIONS = map_obj;
}

JavascriptCompiler::StaticConstructor JavascriptCompiler::staticConstructor;

void JavascriptCompiler::checkFunction(shared_ptr<Method> method)
{
  // check that the Method is public in some public reachable class:
  shared_ptr<MethodType> *const type;
  try {
    type = MethodHandles::publicLookup().unreflect(method).type();
  } catch (const IllegalAccessException &iae) {
    throw invalid_argument(
        method + L" is not accessible (declaring class or method not public).");
  }
  // do some checks if the signature is "compatible":
  if (!Modifier::isStatic(method->getModifiers())) {
    throw invalid_argument(method + L" is not static.");
  }
  for (int arg = 0, arity = type->parameterCount(); arg < arity; arg++) {
    if (type->parameterType(arg) != double ::typeid) {
      throw invalid_argument(method + L" must take only double parameters.");
    }
  }
  if (type->returnType() != double ::typeid) {
    throw invalid_argument(method + L" does not return a double.");
  }
}

void JavascriptCompiler::checkFunctionClassLoader(
    shared_ptr<Method> method, shared_ptr<ClassLoader> parent)
{
  bool ok = false;
  try {
    constexpr type_info clazz = method->getDeclaringClass();
    ok = type_info::forName(clazz.getName(), false, parent) == clazz;
  } catch (const ClassNotFoundException &e) {
    ok = false;
  }
  if (!ok) {
    throw invalid_argument(method +
                           L" is not declared by a class which is accessible "
                           L"by the given parent ClassLoader.");
  }
}
} // namespace org::apache::lucene::expressions::js