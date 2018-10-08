using namespace std;

#include "TestPathHierarchyTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/charfilter/MappingCharFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/charfilter/NormalizeCharMap.h"
#include "../../../../../../java/org/apache/lucene/analysis/path/PathHierarchyTokenizer.h"

namespace org::apache::lucene::analysis::path
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using MappingCharFilter =
    org::apache::lucene::analysis::charfilter::MappingCharFilter;
using NormalizeCharMap =
    org::apache::lucene::analysis::charfilter::NormalizeCharMap;
//    import static
//    org.apache.lucene.analysis.path.PathHierarchyTokenizer.DEFAULT_DELIMITER;
//    import static
//    org.apache.lucene.analysis.path.PathHierarchyTokenizer.DEFAULT_SKIP;

void TestPathHierarchyTokenizer::testBasic() 
{
  wstring path = L"/a/b/c";
  shared_ptr<PathHierarchyTokenizer> t = make_shared<PathHierarchyTokenizer>(
      newAttributeFactory(), DEFAULT_DELIMITER, DEFAULT_DELIMITER,
      DEFAULT_SKIP);
  t->setReader(make_shared<StringReader>(path));
  assertTokenStreamContents(t, std::deque<wstring>{L"/a", L"/a/b", L"/a/b/c"},
                            std::deque<int>{0, 0, 0},
                            std::deque<int>{2, 4, 6},
                            std::deque<int>{1, 0, 0}, path.length());
}

void TestPathHierarchyTokenizer::testEndOfDelimiter() 
{
  wstring path = L"/a/b/c/";
  shared_ptr<PathHierarchyTokenizer> t = make_shared<PathHierarchyTokenizer>(
      newAttributeFactory(), DEFAULT_DELIMITER, DEFAULT_DELIMITER,
      DEFAULT_SKIP);
  t->setReader(make_shared<StringReader>(path));
  assertTokenStreamContents(
      t, std::deque<wstring>{L"/a", L"/a/b", L"/a/b/c", L"/a/b/c/"},
      std::deque<int>{0, 0, 0, 0}, std::deque<int>{2, 4, 6, 7},
      std::deque<int>{1, 0, 0, 0}, path.length());
}

void TestPathHierarchyTokenizer::testStartOfChar() 
{
  wstring path = L"a/b/c";
  shared_ptr<PathHierarchyTokenizer> t = make_shared<PathHierarchyTokenizer>(
      newAttributeFactory(), DEFAULT_DELIMITER, DEFAULT_DELIMITER,
      DEFAULT_SKIP);
  t->setReader(make_shared<StringReader>(path));
  assertTokenStreamContents(t, std::deque<wstring>{L"a", L"a/b", L"a/b/c"},
                            std::deque<int>{0, 0, 0},
                            std::deque<int>{1, 3, 5},
                            std::deque<int>{1, 0, 0}, path.length());
}

void TestPathHierarchyTokenizer::testStartOfCharEndOfDelimiter() throw(
    runtime_error)
{
  wstring path = L"a/b/c/";
  shared_ptr<PathHierarchyTokenizer> t = make_shared<PathHierarchyTokenizer>(
      newAttributeFactory(), DEFAULT_DELIMITER, DEFAULT_DELIMITER,
      DEFAULT_SKIP);
  t->setReader(make_shared<StringReader>(path));
  assertTokenStreamContents(
      t, std::deque<wstring>{L"a", L"a/b", L"a/b/c", L"a/b/c/"},
      std::deque<int>{0, 0, 0, 0}, std::deque<int>{1, 3, 5, 6},
      std::deque<int>{1, 0, 0, 0}, path.length());
}

void TestPathHierarchyTokenizer::testOnlyDelimiter() 
{
  wstring path = L"/";
  shared_ptr<PathHierarchyTokenizer> t = make_shared<PathHierarchyTokenizer>(
      newAttributeFactory(), DEFAULT_DELIMITER, DEFAULT_DELIMITER,
      DEFAULT_SKIP);
  t->setReader(make_shared<StringReader>(path));
  assertTokenStreamContents(t, std::deque<wstring>{L"/"}, std::deque<int>{0},
                            std::deque<int>{1}, std::deque<int>{1},
                            path.length());
}

void TestPathHierarchyTokenizer::testOnlyDelimiters() 
{
  wstring path = L"//";
  shared_ptr<PathHierarchyTokenizer> t = make_shared<PathHierarchyTokenizer>(
      newAttributeFactory(), DEFAULT_DELIMITER, DEFAULT_DELIMITER,
      DEFAULT_SKIP);
  t->setReader(make_shared<StringReader>(path));
  assertTokenStreamContents(t, std::deque<wstring>{L"/", L"//"},
                            std::deque<int>{0, 0}, std::deque<int>{1, 2},
                            std::deque<int>{1, 0}, path.length());
}

void TestPathHierarchyTokenizer::testReplace() 
{
  wstring path = L"/a/b/c";
  shared_ptr<PathHierarchyTokenizer> t = make_shared<PathHierarchyTokenizer>(
      newAttributeFactory(), L'/', L'\\', DEFAULT_SKIP);
  t->setReader(make_shared<StringReader>(path));
  assertTokenStreamContents(
      t, std::deque<wstring>{L"\\a", L"\\a\\b", L"\\a\\b\\c"},
      std::deque<int>{0, 0, 0}, std::deque<int>{2, 4, 6},
      std::deque<int>{1, 0, 0}, path.length());
}

void TestPathHierarchyTokenizer::testWindowsPath() 
{
  wstring path = L"c:\\a\\b\\c";
  shared_ptr<PathHierarchyTokenizer> t = make_shared<PathHierarchyTokenizer>(
      newAttributeFactory(), L'\\', L'\\', DEFAULT_SKIP);
  t->setReader(make_shared<StringReader>(path));
  assertTokenStreamContents(
      t, std::deque<wstring>{L"c:", L"c:\\a", L"c:\\a\\b", L"c:\\a\\b\\c"},
      std::deque<int>{0, 0, 0, 0}, std::deque<int>{2, 4, 6, 8},
      std::deque<int>{1, 0, 0, 0}, path.length());
}

void TestPathHierarchyTokenizer::testNormalizeWinDelimToLinuxDelim() throw(
    runtime_error)
{
  shared_ptr<NormalizeCharMap::Builder> builder =
      make_shared<NormalizeCharMap::Builder>();
  builder->add(L"\\", L"/");
  shared_ptr<NormalizeCharMap> normMap = builder->build();
  wstring path = L"c:\\a\\b\\c";
  shared_ptr<Reader> cs =
      make_shared<MappingCharFilter>(normMap, make_shared<StringReader>(path));
  shared_ptr<PathHierarchyTokenizer> t = make_shared<PathHierarchyTokenizer>(
      newAttributeFactory(), DEFAULT_DELIMITER, DEFAULT_DELIMITER,
      DEFAULT_SKIP);
  t->setReader(cs);
  assertTokenStreamContents(
      t, std::deque<wstring>{L"c:", L"c:/a", L"c:/a/b", L"c:/a/b/c"},
      std::deque<int>{0, 0, 0, 0}, std::deque<int>{2, 4, 6, 8},
      std::deque<int>{1, 0, 0, 0}, path.length());
}

void TestPathHierarchyTokenizer::testBasicSkip() 
{
  wstring path = L"/a/b/c";
  shared_ptr<PathHierarchyTokenizer> t = make_shared<PathHierarchyTokenizer>(
      newAttributeFactory(), DEFAULT_DELIMITER, DEFAULT_DELIMITER, 1);
  t->setReader(make_shared<StringReader>(path));
  assertTokenStreamContents(t, std::deque<wstring>{L"/b", L"/b/c"},
                            std::deque<int>{2, 2}, std::deque<int>{4, 6},
                            std::deque<int>{1, 0}, path.length());
}

void TestPathHierarchyTokenizer::testEndOfDelimiterSkip() 
{
  wstring path = L"/a/b/c/";
  shared_ptr<PathHierarchyTokenizer> t = make_shared<PathHierarchyTokenizer>(
      newAttributeFactory(), DEFAULT_DELIMITER, DEFAULT_DELIMITER, 1);
  t->setReader(make_shared<StringReader>(path));
  assertTokenStreamContents(t, std::deque<wstring>{L"/b", L"/b/c", L"/b/c/"},
                            std::deque<int>{2, 2, 2},
                            std::deque<int>{4, 6, 7},
                            std::deque<int>{1, 0, 0}, path.length());
}

void TestPathHierarchyTokenizer::testStartOfCharSkip() 
{
  wstring path = L"a/b/c";
  shared_ptr<PathHierarchyTokenizer> t = make_shared<PathHierarchyTokenizer>(
      newAttributeFactory(), DEFAULT_DELIMITER, DEFAULT_DELIMITER, 1);
  t->setReader(make_shared<StringReader>(path));
  assertTokenStreamContents(t, std::deque<wstring>{L"/b", L"/b/c"},
                            std::deque<int>{1, 1}, std::deque<int>{3, 5},
                            std::deque<int>{1, 0}, path.length());
}

void TestPathHierarchyTokenizer::testStartOfCharEndOfDelimiterSkip() throw(
    runtime_error)
{
  wstring path = L"a/b/c/";
  shared_ptr<PathHierarchyTokenizer> t = make_shared<PathHierarchyTokenizer>(
      newAttributeFactory(), DEFAULT_DELIMITER, DEFAULT_DELIMITER, 1);
  t->setReader(make_shared<StringReader>(path));
  assertTokenStreamContents(t, std::deque<wstring>{L"/b", L"/b/c", L"/b/c/"},
                            std::deque<int>{1, 1, 1},
                            std::deque<int>{3, 5, 6},
                            std::deque<int>{1, 0, 0}, path.length());
}

void TestPathHierarchyTokenizer::testOnlyDelimiterSkip() 
{
  wstring path = L"/";
  shared_ptr<PathHierarchyTokenizer> t = make_shared<PathHierarchyTokenizer>(
      newAttributeFactory(), DEFAULT_DELIMITER, DEFAULT_DELIMITER, 1);
  t->setReader(make_shared<StringReader>(path));
  assertTokenStreamContents(t, std::deque<wstring>(), std::deque<int>(),
                            std::deque<int>(), std::deque<int>(),
                            path.length());
}

void TestPathHierarchyTokenizer::testOnlyDelimitersSkip() 
{
  wstring path = L"//";
  shared_ptr<PathHierarchyTokenizer> t = make_shared<PathHierarchyTokenizer>(
      newAttributeFactory(), DEFAULT_DELIMITER, DEFAULT_DELIMITER, 1);
  t->setReader(make_shared<StringReader>(path));
  assertTokenStreamContents(t, std::deque<wstring>{L"/"}, std::deque<int>{1},
                            std::deque<int>{2}, std::deque<int>{1},
                            path.length());
}

void TestPathHierarchyTokenizer::testRandomStrings() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  // TODO: properly support positionLengthAttribute
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER, 20, false, false);
  delete a;
}

TestPathHierarchyTokenizer::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestPathHierarchyTokenizer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPathHierarchyTokenizer::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<PathHierarchyTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), DEFAULT_DELIMITER,
      DEFAULT_DELIMITER, DEFAULT_SKIP);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

void TestPathHierarchyTokenizer::testRandomHugeStrings() 
{
  shared_ptr<Random> random = TestPathHierarchyTokenizer::random();
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  // TODO: properly support positionLengthAttribute
  checkRandomData(random, a, 100 * RANDOM_MULTIPLIER, 1027, false, false);
  delete a;
}

TestPathHierarchyTokenizer::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestPathHierarchyTokenizer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPathHierarchyTokenizer::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<PathHierarchyTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), DEFAULT_DELIMITER,
      DEFAULT_DELIMITER, DEFAULT_SKIP);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}
} // namespace org::apache::lucene::analysis::path