using namespace std;

#include "TestExpressionValidation.h"

namespace org::apache::lucene::expressions
{
using JavascriptCompiler =
    org::apache::lucene::expressions::js::JavascriptCompiler;
using SortField = org::apache::lucene::search::SortField;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestExpressionValidation::testValidExternals() 
{
  shared_ptr<SimpleBindings> bindings = make_shared<SimpleBindings>();
  bindings->add(make_shared<SortField>(L"valid0", SortField::Type::INT));
  bindings->add(make_shared<SortField>(L"valid1", SortField::Type::INT));
  bindings->add(make_shared<SortField>(L"valid2", SortField::Type::INT));
  bindings->add(make_shared<SortField>(L"_score", SortField::Type::SCORE));
  bindings->add(L"valide0", JavascriptCompiler::compile(
                                L"valid0 - valid1 + valid2 + _score"));
  bindings->validate();
  bindings->add(L"valide1", JavascriptCompiler::compile(L"valide0 + valid0"));
  bindings->validate();
  bindings->add(L"valide2", JavascriptCompiler::compile(L"valide0 * valide1"));
  bindings->validate();
}

void TestExpressionValidation::testInvalidExternal() 
{
  shared_ptr<SimpleBindings> bindings = make_shared<SimpleBindings>();
  bindings->add(make_shared<SortField>(L"valid", SortField::Type::INT));
  bindings->add(L"invalid", JavascriptCompiler::compile(L"badreference"));
  invalid_argument expected =
      expectThrows(invalid_argument::typeid, [&]() { bindings->validate(); });
  assertTrue(expected.what()->contains(L"Invalid reference"));
}

void TestExpressionValidation::testInvalidExternal2() 
{
  shared_ptr<SimpleBindings> bindings = make_shared<SimpleBindings>();
  bindings->add(make_shared<SortField>(L"valid", SortField::Type::INT));
  bindings->add(L"invalid",
                JavascriptCompiler::compile(L"valid + badreference"));
  invalid_argument expected =
      expectThrows(invalid_argument::typeid, [&]() { bindings->validate(); });
  assertTrue(expected.what()->contains(L"Invalid reference"));
}

void TestExpressionValidation::testSelfRecursion() 
{
  shared_ptr<SimpleBindings> bindings = make_shared<SimpleBindings>();
  bindings->add(L"cycle0", JavascriptCompiler::compile(L"cycle0"));
  invalid_argument expected =
      expectThrows(invalid_argument::typeid, [&]() { bindings->validate(); });
  assertTrue(expected.what()->contains(L"Cycle detected"));
}

void TestExpressionValidation::testCoRecursion() 
{
  shared_ptr<SimpleBindings> bindings = make_shared<SimpleBindings>();
  bindings->add(L"cycle0", JavascriptCompiler::compile(L"cycle1"));
  bindings->add(L"cycle1", JavascriptCompiler::compile(L"cycle0"));
  invalid_argument expected =
      expectThrows(invalid_argument::typeid, [&]() { bindings->validate(); });
  assertTrue(expected.what()->contains(L"Cycle detected"));
}

void TestExpressionValidation::testCoRecursion2() 
{
  shared_ptr<SimpleBindings> bindings = make_shared<SimpleBindings>();
  bindings->add(L"cycle0", JavascriptCompiler::compile(L"cycle1"));
  bindings->add(L"cycle1", JavascriptCompiler::compile(L"cycle2"));
  bindings->add(L"cycle2", JavascriptCompiler::compile(L"cycle0"));
  invalid_argument expected =
      expectThrows(invalid_argument::typeid, [&]() { bindings->validate(); });
  assertTrue(expected.what()->contains(L"Cycle detected"));
}

void TestExpressionValidation::testCoRecursion3() 
{
  shared_ptr<SimpleBindings> bindings = make_shared<SimpleBindings>();
  bindings->add(L"cycle0", JavascriptCompiler::compile(L"100"));
  bindings->add(L"cycle1", JavascriptCompiler::compile(L"cycle0 + cycle2"));
  bindings->add(L"cycle2", JavascriptCompiler::compile(L"cycle0 + cycle1"));
  invalid_argument expected =
      expectThrows(invalid_argument::typeid, [&]() { bindings->validate(); });
  assertTrue(expected.what()->contains(L"Cycle detected"));
}

void TestExpressionValidation::testCoRecursion4() 
{
  shared_ptr<SimpleBindings> bindings = make_shared<SimpleBindings>();
  bindings->add(L"cycle0", JavascriptCompiler::compile(L"100"));
  bindings->add(L"cycle1", JavascriptCompiler::compile(L"100"));
  bindings->add(L"cycle2",
                JavascriptCompiler::compile(L"cycle1 + cycle0 + cycle3"));
  bindings->add(L"cycle3",
                JavascriptCompiler::compile(L"cycle0 + cycle1 + cycle2"));
  invalid_argument expected =
      expectThrows(invalid_argument::typeid, [&]() { bindings->validate(); });
  assertTrue(expected.what()->contains(L"Cycle detected"));
}
} // namespace org::apache::lucene::expressions