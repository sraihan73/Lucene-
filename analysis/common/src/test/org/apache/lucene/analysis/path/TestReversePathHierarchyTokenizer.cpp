using namespace std;

#include "TestReversePathHierarchyTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/path/ReversePathHierarchyTokenizer.h"

namespace org::apache::lucene::analysis::path
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
//    import static
//    org.apache.lucene.analysis.path.ReversePathHierarchyTokenizer.DEFAULT_DELIMITER;
//    import static
//    org.apache.lucene.analysis.path.ReversePathHierarchyTokenizer.DEFAULT_SKIP;

void TestReversePathHierarchyTokenizer::testBasicReverse() 
{
  wstring path = L"/a/b/c";
  shared_ptr<ReversePathHierarchyTokenizer> t =
      make_shared<ReversePathHierarchyTokenizer>(
          newAttributeFactory(), DEFAULT_DELIMITER, DEFAULT_DELIMITER,
          DEFAULT_SKIP);
  t->setReader(make_shared<StringReader>(path));
  assertTokenStreamContents(
      t, std::deque<wstring>{L"/a/b/c", L"a/b/c", L"b/c", L"c"},
      std::deque<int>{0, 1, 3, 5}, std::deque<int>{6, 6, 6, 6},
      std::deque<int>{1, 0, 0, 0}, path.length());
}

void TestReversePathHierarchyTokenizer::testEndOfDelimiterReverse() throw(
    runtime_error)
{
  wstring path = L"/a/b/c/";
  shared_ptr<ReversePathHierarchyTokenizer> t =
      make_shared<ReversePathHierarchyTokenizer>(
          newAttributeFactory(), DEFAULT_DELIMITER, DEFAULT_DELIMITER,
          DEFAULT_SKIP);
  t->setReader(make_shared<StringReader>(path));
  assertTokenStreamContents(
      t, std::deque<wstring>{L"/a/b/c/", L"a/b/c/", L"b/c/", L"c/"},
      std::deque<int>{0, 1, 3, 5}, std::deque<int>{7, 7, 7, 7},
      std::deque<int>{1, 0, 0, 0}, path.length());
}

void TestReversePathHierarchyTokenizer::testStartOfCharReverse() throw(
    runtime_error)
{
  wstring path = L"a/b/c";
  shared_ptr<ReversePathHierarchyTokenizer> t =
      make_shared<ReversePathHierarchyTokenizer>(
          newAttributeFactory(), DEFAULT_DELIMITER, DEFAULT_DELIMITER,
          DEFAULT_SKIP);
  t->setReader(make_shared<StringReader>(path));
  assertTokenStreamContents(t, std::deque<wstring>{L"a/b/c", L"b/c", L"c"},
                            std::deque<int>{0, 2, 4},
                            std::deque<int>{5, 5, 5},
                            std::deque<int>{1, 0, 0}, path.length());
}

void TestReversePathHierarchyTokenizer::
    testStartOfCharEndOfDelimiterReverse() 
{
  wstring path = L"a/b/c/";
  shared_ptr<ReversePathHierarchyTokenizer> t =
      make_shared<ReversePathHierarchyTokenizer>(
          newAttributeFactory(), DEFAULT_DELIMITER, DEFAULT_DELIMITER,
          DEFAULT_SKIP);
  t->setReader(make_shared<StringReader>(path));
  assertTokenStreamContents(t, std::deque<wstring>{L"a/b/c/", L"b/c/", L"c/"},
                            std::deque<int>{0, 2, 4},
                            std::deque<int>{6, 6, 6},
                            std::deque<int>{1, 0, 0}, path.length());
}

void TestReversePathHierarchyTokenizer::testOnlyDelimiterReverse() throw(
    runtime_error)
{
  wstring path = L"/";
  shared_ptr<ReversePathHierarchyTokenizer> t =
      make_shared<ReversePathHierarchyTokenizer>(
          newAttributeFactory(), DEFAULT_DELIMITER, DEFAULT_DELIMITER,
          DEFAULT_SKIP);
  t->setReader(make_shared<StringReader>(path));
  assertTokenStreamContents(t, std::deque<wstring>{L"/"}, std::deque<int>{0},
                            std::deque<int>{1}, std::deque<int>{1},
                            path.length());
}

void TestReversePathHierarchyTokenizer::testOnlyDelimitersReverse() throw(
    runtime_error)
{
  wstring path = L"//";
  shared_ptr<ReversePathHierarchyTokenizer> t =
      make_shared<ReversePathHierarchyTokenizer>(
          newAttributeFactory(), DEFAULT_DELIMITER, DEFAULT_DELIMITER,
          DEFAULT_SKIP);
  t->setReader(make_shared<StringReader>(path));
  assertTokenStreamContents(t, std::deque<wstring>{L"//", L"/"},
                            std::deque<int>{0, 1}, std::deque<int>{2, 2},
                            std::deque<int>{1, 0}, path.length());
}

void TestReversePathHierarchyTokenizer::testEndOfDelimiterReverseSkip() throw(
    runtime_error)
{
  wstring path = L"/a/b/c/";
  shared_ptr<ReversePathHierarchyTokenizer> t =
      make_shared<ReversePathHierarchyTokenizer>(
          newAttributeFactory(), DEFAULT_DELIMITER, DEFAULT_DELIMITER, 1);
  t->setReader(make_shared<StringReader>(path));
  make_shared<StringReader>(path);
  assertTokenStreamContents(t, std::deque<wstring>{L"/a/b/", L"a/b/", L"b/"},
                            std::deque<int>{0, 1, 3},
                            std::deque<int>{5, 5, 5},
                            std::deque<int>{1, 0, 0}, path.length());
}

void TestReversePathHierarchyTokenizer::testStartOfCharReverseSkip() throw(
    runtime_error)
{
  wstring path = L"a/b/c";
  shared_ptr<ReversePathHierarchyTokenizer> t =
      make_shared<ReversePathHierarchyTokenizer>(
          newAttributeFactory(), DEFAULT_DELIMITER, DEFAULT_DELIMITER, 1);
  t->setReader(make_shared<StringReader>(path));
  assertTokenStreamContents(t, std::deque<wstring>{L"a/b/", L"b/"},
                            std::deque<int>{0, 2}, std::deque<int>{4, 4},
                            std::deque<int>{1, 0}, path.length());
}

void TestReversePathHierarchyTokenizer::
    testStartOfCharEndOfDelimiterReverseSkip() 
{
  wstring path = L"a/b/c/";
  shared_ptr<ReversePathHierarchyTokenizer> t =
      make_shared<ReversePathHierarchyTokenizer>(
          newAttributeFactory(), DEFAULT_DELIMITER, DEFAULT_DELIMITER, 1);
  t->setReader(make_shared<StringReader>(path));
  assertTokenStreamContents(t, std::deque<wstring>{L"a/b/", L"b/"},
                            std::deque<int>{0, 2}, std::deque<int>{4, 4},
                            std::deque<int>{1, 0}, path.length());
}

void TestReversePathHierarchyTokenizer::testOnlyDelimiterReverseSkip() throw(
    runtime_error)
{
  wstring path = L"/";
  shared_ptr<ReversePathHierarchyTokenizer> t =
      make_shared<ReversePathHierarchyTokenizer>(
          newAttributeFactory(), DEFAULT_DELIMITER, DEFAULT_DELIMITER, 1);
  t->setReader(make_shared<StringReader>(path));
  assertTokenStreamContents(t, std::deque<wstring>(), std::deque<int>(),
                            std::deque<int>(), std::deque<int>(),
                            path.length());
}

void TestReversePathHierarchyTokenizer::testOnlyDelimitersReverseSkip() throw(
    runtime_error)
{
  wstring path = L"//";
  shared_ptr<ReversePathHierarchyTokenizer> t =
      make_shared<ReversePathHierarchyTokenizer>(
          newAttributeFactory(), DEFAULT_DELIMITER, DEFAULT_DELIMITER, 1);
  t->setReader(make_shared<StringReader>(path));
  assertTokenStreamContents(t, std::deque<wstring>{L"/"}, std::deque<int>{0},
                            std::deque<int>{1}, std::deque<int>{1},
                            path.length());
}

void TestReversePathHierarchyTokenizer::testReverseSkip2() 
{
  wstring path = L"/a/b/c/";
  shared_ptr<ReversePathHierarchyTokenizer> t =
      make_shared<ReversePathHierarchyTokenizer>(
          newAttributeFactory(), DEFAULT_DELIMITER, DEFAULT_DELIMITER, 2);
  t->setReader(make_shared<StringReader>(path));
  assertTokenStreamContents(t, std::deque<wstring>{L"/a/", L"a/"},
                            std::deque<int>{0, 1}, std::deque<int>{3, 3},
                            std::deque<int>{1, 0}, path.length());
}

void TestReversePathHierarchyTokenizer::testRandomStrings() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  // TODO: properly support positionLengthAttribute
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER, 20, false, false);
  delete a;
}

TestReversePathHierarchyTokenizer::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestReversePathHierarchyTokenizer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents> TestReversePathHierarchyTokenizer::
    AnalyzerAnonymousInnerClass::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<ReversePathHierarchyTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), DEFAULT_DELIMITER,
      DEFAULT_DELIMITER, DEFAULT_SKIP);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

void TestReversePathHierarchyTokenizer::testRandomHugeStrings() throw(
    runtime_error)
{
  shared_ptr<Random> random = TestReversePathHierarchyTokenizer::random();
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  // TODO: properly support positionLengthAttribute
  checkRandomData(random, a, 100 * RANDOM_MULTIPLIER, 1027, false, false);
  delete a;
}

TestReversePathHierarchyTokenizer::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestReversePathHierarchyTokenizer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents> TestReversePathHierarchyTokenizer::
    AnalyzerAnonymousInnerClass2::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<ReversePathHierarchyTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), DEFAULT_DELIMITER,
      DEFAULT_DELIMITER, DEFAULT_SKIP);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}
} // namespace org::apache::lucene::analysis::path