using namespace std;

#include "Test02Boolean.h"

namespace org::apache::lucene::queryparser::surround::query
{
using junit::framework::TestSuite;
using junit::textui::TestRunner;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void Test02Boolean::main(std::deque<wstring> &args)
{
  TestRunner::run(make_shared<TestSuite>(Test02Boolean::typeid));
}

void Test02Boolean::setUp() 
{
  LuceneTestCase::setUp();
  db1 = make_shared<SingleFieldTestDb>(random(), docs1, fieldName);
}

void Test02Boolean::normalTest1(const wstring &query,
                                std::deque<int> &expdnrs) 
{
  shared_ptr<BooleanQueryTst> bqt = make_shared<BooleanQueryTst>(
      query, expdnrs, db1, fieldName, shared_from_this(),
      make_shared<BasicQueryFactory>(maxBasicQueries));
  bqt->setVerbose(verbose);
  bqt->doTest();
}

void Test02Boolean::test02Terms01() 
{
  std::deque<int> expdnrs = {0};
  normalTest1(L"word1", expdnrs);
}

void Test02Boolean::test02Terms02() 
{
  std::deque<int> expdnrs = {0, 1, 3};
  normalTest1(L"word*", expdnrs);
}

void Test02Boolean::test02Terms03() 
{
  std::deque<int> expdnrs = {2};
  normalTest1(L"ord2", expdnrs);
}

void Test02Boolean::test02Terms04() 
{
  std::deque<int> expdnrs = {};
  normalTest1(L"kxork*", expdnrs);
}

void Test02Boolean::test02Terms05() 
{
  std::deque<int> expdnrs = {0, 1, 3};
  normalTest1(L"wor*", expdnrs);
}

void Test02Boolean::test02Terms06() 
{
  std::deque<int> expdnrs = {};
  normalTest1(L"ab", expdnrs);
}

void Test02Boolean::test02Terms10() 
{
  std::deque<int> expdnrs = {};
  normalTest1(L"abc?", expdnrs);
}

void Test02Boolean::test02Terms13() 
{
  std::deque<int> expdnrs = {0, 1, 3};
  normalTest1(L"word?", expdnrs);
}

void Test02Boolean::test02Terms14() 
{
  std::deque<int> expdnrs = {0, 1, 3};
  normalTest1(L"w?rd?", expdnrs);
}

void Test02Boolean::test02Terms20() 
{
  std::deque<int> expdnrs = {0, 1, 3};
  normalTest1(L"w*rd?", expdnrs);
}

void Test02Boolean::test02Terms21() 
{
  std::deque<int> expdnrs = {3};
  normalTest1(L"w*rd??", expdnrs);
}

void Test02Boolean::test02Terms22() 
{
  std::deque<int> expdnrs = {3};
  normalTest1(L"w*?da?", expdnrs);
}

void Test02Boolean::test02Terms23() 
{
  std::deque<int> expdnrs = {};
  normalTest1(L"w?da?", expdnrs);
}

void Test02Boolean::test03And01() 
{
  std::deque<int> expdnrs = {0};
  normalTest1(L"word1 AND word2", expdnrs);
}

void Test02Boolean::test03And02() 
{
  std::deque<int> expdnrs = {3};
  normalTest1(L"word* and ord*", expdnrs);
}

void Test02Boolean::test03And03() 
{
  std::deque<int> expdnrs = {0};
  normalTest1(L"and(word1,word2)", expdnrs);
}

void Test02Boolean::test04Or01() 
{
  std::deque<int> expdnrs = {0, 3};
  normalTest1(L"word1 or word2", expdnrs);
}

void Test02Boolean::test04Or02() 
{
  std::deque<int> expdnrs = {0, 1, 2, 3};
  normalTest1(L"word* OR ord*", expdnrs);
}

void Test02Boolean::test04Or03() 
{
  std::deque<int> expdnrs = {0, 3};
  normalTest1(L"OR (word1, word2)", expdnrs);
}

void Test02Boolean::test05Not01() 
{
  std::deque<int> expdnrs = {3};
  normalTest1(L"word2 NOT word1", expdnrs);
}

void Test02Boolean::test05Not02() 
{
  std::deque<int> expdnrs = {0};
  normalTest1(L"word2* not ord*", expdnrs);
}

void Test02Boolean::test06AndOr01() 
{
  std::deque<int> expdnrs = {0};
  normalTest1(L"(word1 or ab)and or(word2,xyz, defg)", expdnrs);
}

void Test02Boolean::test07AndOrNot02() 
{
  std::deque<int> expdnrs = {0};
  normalTest1(L"or( word2* not ord*, and(xyz,def))", expdnrs);
}
} // namespace org::apache::lucene::queryparser::surround::query