using namespace std;

#include "FuzzyTermOnShortTermsTest.h"

namespace org::apache::lucene::search
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::Test;
const wstring FuzzyTermOnShortTermsTest::FIELD = L"field";

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void test() throws Exception
void FuzzyTermOnShortTermsTest::test() 
{
  // proves rule that edit distance between the two terms
  // must be > smaller term for there to be a match
  shared_ptr<Analyzer> a = getAnalyzer();
  // these work
  countHits(a, std::deque<wstring>{L"abc"},
            make_shared<FuzzyQuery>(make_shared<Term>(FIELD, L"ab"), 1), 1);
  countHits(a, std::deque<wstring>{L"ab"},
            make_shared<FuzzyQuery>(make_shared<Term>(FIELD, L"abc"), 1), 1);

  countHits(a, std::deque<wstring>{L"abcde"},
            make_shared<FuzzyQuery>(make_shared<Term>(FIELD, L"abc"), 2), 1);
  countHits(a, std::deque<wstring>{L"abc"},
            make_shared<FuzzyQuery>(make_shared<Term>(FIELD, L"abcde"), 2), 1);

  // LUCENE-7439: these now work as well:

  countHits(a, std::deque<wstring>{L"ab"},
            make_shared<FuzzyQuery>(make_shared<Term>(FIELD, L"a"), 1), 1);
  countHits(a, std::deque<wstring>{L"a"},
            make_shared<FuzzyQuery>(make_shared<Term>(FIELD, L"ab"), 1), 1);

  countHits(a, std::deque<wstring>{L"abc"},
            make_shared<FuzzyQuery>(make_shared<Term>(FIELD, L"a"), 2), 1);
  countHits(a, std::deque<wstring>{L"a"},
            make_shared<FuzzyQuery>(make_shared<Term>(FIELD, L"abc"), 2), 1);

  countHits(a, std::deque<wstring>{L"abcd"},
            make_shared<FuzzyQuery>(make_shared<Term>(FIELD, L"ab"), 2), 1);
  countHits(a, std::deque<wstring>{L"ab"},
            make_shared<FuzzyQuery>(make_shared<Term>(FIELD, L"abcd"), 2), 1);
}

void FuzzyTermOnShortTermsTest::countHits(shared_ptr<Analyzer> analyzer,
                                          std::deque<wstring> &docs,
                                          shared_ptr<Query> q,
                                          int expected) 
{
  shared_ptr<Directory> d = getDirectory(analyzer, docs);
  shared_ptr<IndexReader> r = DirectoryReader::open(d);
  shared_ptr<IndexSearcher> s = make_shared<IndexSearcher>(r);
  shared_ptr<TotalHitCountCollector> c = make_shared<TotalHitCountCollector>();
  s->search(q, c);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(q->toString(), expected, c->getTotalHits());
  delete r;
  delete d;
}

shared_ptr<Analyzer> FuzzyTermOnShortTermsTest::getAnalyzer()
{
  return make_shared<AnalyzerAnonymousInnerClass>();
}

FuzzyTermOnShortTermsTest::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass()
{
}

shared_ptr<Analyzer::TokenStreamComponents>
FuzzyTermOnShortTermsTest::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, tokenizer);
}

shared_ptr<Directory> FuzzyTermOnShortTermsTest::getDirectory(
    shared_ptr<Analyzer> analyzer,
    std::deque<wstring> &vals) 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), directory,
      newIndexWriterConfig(analyzer)
          ->setMaxBufferedDocs(TestUtil::nextInt(random(), 100, 1000))
          ->setMergePolicy(newLogMergePolicy()));

  for (auto s : vals) {
    shared_ptr<Document> d = make_shared<Document>();
    d->push_back(newTextField(FIELD, s, Field::Store::YES));
    writer->addDocument(d);
  }
  delete writer;
  return directory;
}
} // namespace org::apache::lucene::search