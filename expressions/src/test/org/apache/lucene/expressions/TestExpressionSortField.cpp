using namespace std;

#include "TestExpressionSortField.h"

namespace org::apache::lucene::expressions
{
using JavascriptCompiler =
    org::apache::lucene::expressions::js::JavascriptCompiler;
using SortField = org::apache::lucene::search::SortField;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestExpressionSortField::testToString() 
{
  shared_ptr<Expression> expr =
      JavascriptCompiler::compile(L"sqrt(_score) + ln(popularity)");

  shared_ptr<SimpleBindings> bindings = make_shared<SimpleBindings>();
  bindings->add(make_shared<SortField>(L"_score", SortField::Type::SCORE));
  bindings->add(make_shared<SortField>(L"popularity", SortField::Type::INT));

  shared_ptr<SortField> sf = expr->getSortField(bindings, true);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"<expr(sqrt(_score) + ln(popularity))>!", sf->toString());
}

void TestExpressionSortField::testEquals() 
{
  shared_ptr<Expression> expr =
      JavascriptCompiler::compile(L"sqrt(_score) + ln(popularity)");

  shared_ptr<SimpleBindings> bindings = make_shared<SimpleBindings>();
  bindings->add(make_shared<SortField>(L"_score", SortField::Type::SCORE));
  bindings->add(make_shared<SortField>(L"popularity", SortField::Type::INT));

  shared_ptr<SimpleBindings> otherBindings = make_shared<SimpleBindings>();
  otherBindings->add(make_shared<SortField>(L"_score", SortField::Type::LONG));
  otherBindings->add(
      make_shared<SortField>(L"popularity", SortField::Type::INT));

  shared_ptr<SortField> sf1 = expr->getSortField(bindings, true);

  // different order
  shared_ptr<SortField> sf2 = expr->getSortField(bindings, false);
  assertFalse(sf1->equals(sf2));

  // different bindings
  sf2 = expr->getSortField(otherBindings, true);
  assertFalse(sf1->equals(sf2));

  // different expression
  shared_ptr<Expression> other = JavascriptCompiler::compile(L"popularity/2");
  sf2 = other->getSortField(bindings, true);
  assertFalse(sf1->equals(sf2));

  // null
  assertFalse(sf1->equals(nullptr));

  // same instance:
  assertEquals(sf1, sf1);
}

void TestExpressionSortField::testNeedsScores() 
{
  shared_ptr<SimpleBindings> bindings = make_shared<SimpleBindings>();
  // refers to score directly
  shared_ptr<Expression> exprA = JavascriptCompiler::compile(L"_score");
  // constant
  shared_ptr<Expression> exprB = JavascriptCompiler::compile(L"0");
  // field
  shared_ptr<Expression> exprC = JavascriptCompiler::compile(L"intfield");

  // score + constant
  shared_ptr<Expression> exprD = JavascriptCompiler::compile(L"_score + 0");
  // field + constant
  shared_ptr<Expression> exprE = JavascriptCompiler::compile(L"intfield + 0");

  // expression + constant (score ref'd)
  shared_ptr<Expression> exprF = JavascriptCompiler::compile(L"a + 0");
  // expression + constant
  shared_ptr<Expression> exprG = JavascriptCompiler::compile(L"e + 0");

  // several variables (score ref'd)
  shared_ptr<Expression> exprH =
      JavascriptCompiler::compile(L"b / c + e * g - sqrt(f)");
  // several variables
  shared_ptr<Expression> exprI = JavascriptCompiler::compile(L"b / c + e * g");

  bindings->add(make_shared<SortField>(L"_score", SortField::Type::SCORE));
  bindings->add(make_shared<SortField>(L"intfield", SortField::Type::INT));
  bindings->add(L"a", exprA);
  bindings->add(L"b", exprB);
  bindings->add(L"c", exprC);
  bindings->add(L"d", exprD);
  bindings->add(L"e", exprE);
  bindings->add(L"f", exprF);
  bindings->add(L"g", exprG);
  bindings->add(L"h", exprH);
  bindings->add(L"i", exprI);

  assertTrue(exprA->getSortField(bindings, true)->needsScores());
  assertFalse(exprB->getSortField(bindings, true)->needsScores());
  assertFalse(exprC->getSortField(bindings, true)->needsScores());
  assertTrue(exprD->getSortField(bindings, true)->needsScores());
  assertFalse(exprE->getSortField(bindings, true)->needsScores());
  assertTrue(exprF->getSortField(bindings, true)->needsScores());
  assertFalse(exprG->getSortField(bindings, true)->needsScores());
  assertTrue(exprH->getSortField(bindings, true)->needsScores());
  assertFalse(exprI->getSortField(bindings, false)->needsScores());
}
} // namespace org::apache::lucene::expressions