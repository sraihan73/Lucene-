using namespace std;

#include "TestUnifiedHighlighterRanking.h"

namespace org::apache::lucene::search::uhighlight
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using StringField = org::apache::lucene::document::StringField;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Sort = org::apache::lucene::search::Sort;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestUnifiedHighlighterRanking::testRanking() 
{
  // number of documents: we will check each one
  constexpr int numDocs = atLeast(100);
  // number of top-N snippets, we will check 1 .. N
  constexpr int maxTopN = 5;
  // maximum number of elements to put in a sentence.
  constexpr int maxSentenceLength = 10;
  // maximum number of sentences in a document
  constexpr int maxNumSentences = 20;

  shared_ptr<Directory> dir = newDirectory();
  indexAnalyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true);
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);
  shared_ptr<Document> document = make_shared<Document>();
  shared_ptr<Field> id = make_shared<StringField>(L"id", L"", Field::Store::NO);
  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  document->push_back(id);
  document->push_back(body);

  for (int i = 0; i < numDocs; i++) {
    shared_ptr<StringBuilder> bodyText = make_shared<StringBuilder>();
    int numSentences = TestUtil::nextInt(random(), 1, maxNumSentences);
    for (int j = 0; j < numSentences; j++) {
      bodyText->append(newSentence(random(), maxSentenceLength));
    }
    body->setStringValue(bodyText->toString());
    // C++ TODO: There is no native C++ equivalent to 'toString':
    id->setStringValue(Integer::toString(i));
    iw->addDocument(document);
  }

  shared_ptr<IndexReader> ir = iw->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  for (int i = 0; i < numDocs; i++) {
    checkDocument(searcher, i, maxTopN);
  }
  delete iw;
  delete ir;
  delete dir;
}

void TestUnifiedHighlighterRanking::checkDocument(
    shared_ptr<IndexSearcher> is, int doc, int maxTopN) 
{
  for (int ch = L'a'; ch <= L'z'; ch++) {
    shared_ptr<Term> term = make_shared<Term>(
        L"body", L"" + StringHelper::toString(static_cast<wchar_t>(ch)));
    // check a simple term query
    checkQuery(is, make_shared<TermQuery>(term), doc, maxTopN);
    // check a bool query
    shared_ptr<Term> nextTerm = make_shared<Term>(
        L"body", L"" + StringHelper::toString(static_cast<wchar_t>(ch + 1)));
    shared_ptr<BooleanQuery> bq =
        (make_shared<BooleanQuery::Builder>())
            ->add(make_shared<TermQuery>(term), BooleanClause::Occur::SHOULD)
            ->add(make_shared<TermQuery>(nextTerm),
                  BooleanClause::Occur::SHOULD)
            ->build();
    checkQuery(is, bq, doc, maxTopN);
  }
}

void TestUnifiedHighlighterRanking::checkQuery(shared_ptr<IndexSearcher> is,
                                               shared_ptr<Query> query, int doc,
                                               int maxTopN) 
{
  for (int n = 1; n < maxTopN; n++) {
    shared_ptr<FakePassageFormatter> *const f1 =
        make_shared<FakePassageFormatter>();
    shared_ptr<UnifiedHighlighter> p1 =
        make_shared<UnifiedHighlighterAnonymousInnerClass>(
            shared_from_this(), is, indexAnalyzer, f1);
    p1->setMaxLength(numeric_limits<int>::max() - 1);

    shared_ptr<FakePassageFormatter> *const f2 =
        make_shared<FakePassageFormatter>();
    shared_ptr<UnifiedHighlighter> p2 =
        make_shared<UnifiedHighlighterAnonymousInnerClass2>(
            shared_from_this(), is, indexAnalyzer, f2);
    p2->setMaxLength(numeric_limits<int>::max() - 1);

    shared_ptr<BooleanQuery::Builder> queryBuilder =
        make_shared<BooleanQuery::Builder>();
    queryBuilder->add(query, BooleanClause::Occur::MUST);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    queryBuilder->add(make_shared<TermQuery>(
                          make_shared<Term>(L"id", Integer::toString(doc))),
                      BooleanClause::Occur::MUST);
    shared_ptr<BooleanQuery> bq = queryBuilder->build();
    shared_ptr<TopDocs> td = is->search(bq, 1);
    p1->highlight(L"body", bq, td, n);
    p2->highlight(L"body", bq, td, n + 1);
    assertTrue(f2->seen.containsAll(f1->seen));
  }
}

TestUnifiedHighlighterRanking::UnifiedHighlighterAnonymousInnerClass::
    UnifiedHighlighterAnonymousInnerClass(
        shared_ptr<TestUnifiedHighlighterRanking> outerInstance,
        shared_ptr<IndexSearcher> is, shared_ptr<Analyzer> indexAnalyzer,
        shared_ptr<org::apache::lucene::search::uhighlight::
                       TestUnifiedHighlighterRanking::FakePassageFormatter>
            f1)
    : UnifiedHighlighter(is, indexAnalyzer)
{
  this->outerInstance = outerInstance;
  this->f1 = f1;
}

shared_ptr<PassageFormatter> TestUnifiedHighlighterRanking::
    UnifiedHighlighterAnonymousInnerClass::getFormatter(const wstring &field)
{
  TestUtil::assertEquals(L"body", field);
  return f1;
}

TestUnifiedHighlighterRanking::UnifiedHighlighterAnonymousInnerClass2::
    UnifiedHighlighterAnonymousInnerClass2(
        shared_ptr<TestUnifiedHighlighterRanking> outerInstance,
        shared_ptr<IndexSearcher> is, shared_ptr<Analyzer> indexAnalyzer,
        shared_ptr<org::apache::lucene::search::uhighlight::
                       TestUnifiedHighlighterRanking::FakePassageFormatter>
            f2)
    : UnifiedHighlighter(is, indexAnalyzer)
{
  this->outerInstance = outerInstance;
  this->f2 = f2;
}

shared_ptr<PassageFormatter> TestUnifiedHighlighterRanking::
    UnifiedHighlighterAnonymousInnerClass2::getFormatter(const wstring &field)
{
  TestUtil::assertEquals(L"body", field);
  return f2;
}

wstring TestUnifiedHighlighterRanking::newSentence(shared_ptr<Random> r,
                                                   int maxSentenceLength)
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  int numElements = TestUtil::nextInt(r, 1, maxSentenceLength);
  for (int i = 0; i < numElements; i++) {
    if (sb->length() > 0) {
      sb->append(L' ');
      sb->append(static_cast<wchar_t>(TestUtil::nextInt(r, L'a', L'z')));
    } else {
      // capitalize the first word to help breakiterator
      sb->append(static_cast<wchar_t>(TestUtil::nextInt(r, L'A', L'Z')));
    }
  }
  sb->append(L". "); // finalize sentence
  return sb->toString();
}

wstring TestUnifiedHighlighterRanking::FakePassageFormatter::format(
    std::deque<std::shared_ptr<Passage>> &passages, const wstring &content)
{
  for (auto p : passages) {
    // verify some basics about the passage
    assertTrue(p->getScore() >= 0);
    assertTrue(p->getNumMatches() > 0);
    assertTrue(p->getStartOffset() >= 0);
    assertTrue(p->getStartOffset() <= content.length());
    assertTrue(p->getEndOffset() >= p->getStartOffset());
    assertTrue(p->getEndOffset() <= content.length());
    // we use a very simple analyzer. so we can assert the matches are correct
    int lastMatchStart = -1;
    for (int i = 0; i < p->getNumMatches(); i++) {
      shared_ptr<BytesRef> term = p->getMatchTerms()[i];
      int matchStart = p->getMatchStarts()[i];
      assertTrue(matchStart >= 0);
      // must at least start within the passage
      assertTrue(matchStart < p->getEndOffset());
      int matchEnd = p->getMatchEnds()[i];
      assertTrue(matchEnd >= 0);
      // always moving forward
      assertTrue(matchStart >= lastMatchStart);
      lastMatchStart = matchStart;
      // single character terms
      TestUtil::assertEquals(matchStart + 1, matchEnd);
      // and the offsets must be correct...
      TestUtil::assertEquals(1, term->length);
      TestUtil::assertEquals(static_cast<wchar_t>(term->bytes[term->offset]),
                             towlower(content[matchStart]));
    }
    // record just the start/end offset for simplicity
    seen.insert(make_shared<Pair>(p->getStartOffset(), p->getEndOffset()));
  }
  return L"bogus!!!!!!";
}

TestUnifiedHighlighterRanking::Pair::Pair(int start, int end)
    : start(start), end(end)
{
}

int TestUnifiedHighlighterRanking::Pair::hashCode()
{
  constexpr int prime = 31;
  int result = 1;
  result = prime * result + end;
  result = prime * result + start;
  return result;
}

bool TestUnifiedHighlighterRanking::Pair::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (obj == nullptr) {
    return false;
  }
  if (getClass() != obj.type()) {
    return false;
  }
  shared_ptr<Pair> other = any_cast<std::shared_ptr<Pair>>(obj);
  if (end != other->end) {
    return false;
  }
  if (start != other->start) {
    return false;
  }
  return true;
}

wstring TestUnifiedHighlighterRanking::Pair::toString()
{
  return L"Pair [start=" + to_wstring(start) + L", end=" + to_wstring(end) +
         L"]";
}

void TestUnifiedHighlighterRanking::testCustomB() 
{
  shared_ptr<Directory> dir = newDirectory();
  indexAnalyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true);
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(indexAnalyzer);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(wstring(L"This is a test.  This test is a better test "
                               L"but the sentence is excruiatingly long, ") +
                       L"you have no idea how painful it was for me to type "
                       L"this long sentence into my IDE.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      make_shared<UnifiedHighlighterAnonymousInnerClass>(
          shared_from_this(), searcher, indexAnalyzer);
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"test"));
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  TestUtil::assertEquals(1, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs, 1);
  TestUtil::assertEquals(1, snippets.size());
  assertTrue(StringHelper::startsWith(
      snippets[0], L"This <b>test</b> is a better <b>test</b>"));

  delete ir;
  delete dir;
}

TestUnifiedHighlighterRanking::UnifiedHighlighterAnonymousInnerClass::
    UnifiedHighlighterAnonymousInnerClass(
        shared_ptr<TestUnifiedHighlighterRanking> outerInstance,
        shared_ptr<IndexSearcher> searcher, shared_ptr<Analyzer> indexAnalyzer)
    : UnifiedHighlighter(searcher, indexAnalyzer)
{
  this->outerInstance = outerInstance;
}

shared_ptr<PassageScorer>
TestUnifiedHighlighterRanking::UnifiedHighlighterAnonymousInnerClass::getScorer(
    const wstring &field)
{
  return make_shared<PassageScorer>(1.2f, 0, 87);
}

void TestUnifiedHighlighterRanking::testCustomK1() 
{
  shared_ptr<Directory> dir = newDirectory();
  indexAnalyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true);
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(indexAnalyzer);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  shared_ptr<Field> body = make_shared<Field>(L"body", L"", fieldType);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(body);

  body->setStringValue(
      wstring(L"This has only foo foo. ") +
      L"On the other hand this sentence contains both foo and bar. " +
      L"This has only bar bar bar bar bar bar bar bar bar bar bar bar.");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      make_shared<UnifiedHighlighterAnonymousInnerClass2>(
          shared_from_this(), searcher, indexAnalyzer);
  shared_ptr<BooleanQuery> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"body", L"foo")),
                BooleanClause::Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"body", L"bar")),
                BooleanClause::Occur::SHOULD)
          ->build();
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  TestUtil::assertEquals(1, topDocs->totalHits);
  std::deque<wstring> snippets =
      highlighter->highlight(L"body", query, topDocs, 1);
  TestUtil::assertEquals(1, snippets.size());
  assertTrue(StringHelper::startsWith(snippets[0], L"On the other hand"));

  delete ir;
  delete dir;
}

TestUnifiedHighlighterRanking::UnifiedHighlighterAnonymousInnerClass2::
    UnifiedHighlighterAnonymousInnerClass2(
        shared_ptr<TestUnifiedHighlighterRanking> outerInstance,
        shared_ptr<IndexSearcher> searcher, shared_ptr<Analyzer> indexAnalyzer)
    : UnifiedHighlighter(searcher, indexAnalyzer)
{
  this->outerInstance = outerInstance;
}

shared_ptr<PassageScorer> TestUnifiedHighlighterRanking::
    UnifiedHighlighterAnonymousInnerClass2::getScorer(const wstring &field)
{
  return make_shared<PassageScorer>(0, 0.75f, 87);
}
} // namespace org::apache::lucene::search::uhighlight