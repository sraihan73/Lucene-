using namespace std;

#include "TestVariableContext.h"

namespace org::apache::lucene::expressions::js
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
//    import static
//    org.apache.lucene.expressions.js.VariableContext.Type.MEMBER; import
//    static org.apache.lucene.expressions.js.VariableContext.Type.STR_INDEX;
//    import static
//    org.apache.lucene.expressions.js.VariableContext.Type.INT_INDEX; import
//    static org.apache.lucene.expressions.js.VariableContext.Type.METHOD;

void TestVariableContext::testSimpleVar()
{
  std::deque<std::shared_ptr<VariableContext>> x =
      VariableContext::parse(L"foo");
  assertEquals(1, x.size());
  assertEquals(x[0]->type, MEMBER);
  assertEquals(x[0]->text, L"foo");
}

void TestVariableContext::testEmptyString()
{
  std::deque<std::shared_ptr<VariableContext>> x =
      VariableContext::parse(L"foo['']");
  assertEquals(2, x.size());
  assertEquals(x[1]->type, STR_INDEX);
  assertEquals(x[1]->text, L"");
}

void TestVariableContext::testUnescapeString()
{
  std::deque<std::shared_ptr<VariableContext>> x =
      VariableContext::parse(L"foo['\\'\\\\']");
  assertEquals(2, x.size());
  assertEquals(x[1]->type, STR_INDEX);
  assertEquals(x[1]->text, L"'\\");
}

void TestVariableContext::testMember()
{
  std::deque<std::shared_ptr<VariableContext>> x =
      VariableContext::parse(L"foo.bar");
  assertEquals(2, x.size());
  assertEquals(x[1]->type, MEMBER);
  assertEquals(x[1]->text, L"bar");
}

void TestVariableContext::testMemberFollowedByMember()
{
  std::deque<std::shared_ptr<VariableContext>> x =
      VariableContext::parse(L"foo.bar.baz");
  assertEquals(3, x.size());
  assertEquals(x[2]->type, MEMBER);
  assertEquals(x[2]->text, L"baz");
}

void TestVariableContext::testMemberFollowedByIntArray()
{
  std::deque<std::shared_ptr<VariableContext>> x =
      VariableContext::parse(L"foo.bar[1]");
  assertEquals(3, x.size());
  assertEquals(x[2]->type, INT_INDEX);
  assertEquals(x[2]->integer, 1);
}

void TestVariableContext::testMethodWithMember()
{
  std::deque<std::shared_ptr<VariableContext>> x =
      VariableContext::parse(L"m.m()");
  assertEquals(2, x.size());
  assertEquals(x[1]->type, METHOD);
  assertEquals(x[1]->text, L"m");
}

void TestVariableContext::testMethodWithStrIndex()
{
  std::deque<std::shared_ptr<VariableContext>> x =
      VariableContext::parse(L"member['blah'].getMethod()");
  assertEquals(3, x.size());
  assertEquals(x[2]->type, METHOD);
  assertEquals(x[2]->text, L"getMethod");
}

void TestVariableContext::testMethodWithNumericalIndex()
{
  std::deque<std::shared_ptr<VariableContext>> x =
      VariableContext::parse(L"member[0].getMethod()");
  assertEquals(3, x.size());
  assertEquals(x[2]->type, METHOD);
  assertEquals(x[2]->text, L"getMethod");
}
} // namespace org::apache::lucene::expressions::js