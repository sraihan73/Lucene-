using namespace std;

#include "TestPositionIncrement.h"

namespace org::apache::lucene::search
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockPayloadAnalyzer = org::apache::lucene::analysis::MockPayloadAnalyzer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using TextField = org::apache::lucene::document::TextField;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using MultiFields = org::apache::lucene::index::MultiFields;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using SpanCollector = org::apache::lucene::search::spans::SpanCollector;
using SpanNearQuery = org::apache::lucene::search::spans::SpanNearQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using Spans = org::apache::lucene::search::spans::Spans;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using SpanWeight = org::apache::lucene::search::spans::SpanWeight;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestPositionIncrement::testSetPosition() 
{
  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  shared_ptr<Directory> store = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), store, analyzer);
  shared_ptr<Document> d = make_shared<Document>();
  d->push_back(newTextField(L"field", L"bogus", Field::Store::YES));
  writer->addDocument(d);
  shared_ptr<IndexReader> reader = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  shared_ptr<PostingsEnum> pos = MultiFields::getTermPositionsEnum(
      searcher->getIndexReader(), L"field", make_shared<BytesRef>(L"1"));
  pos->nextDoc();
  // first token should be at position 0
  assertEquals(0, pos->nextPosition());

  pos = MultiFields::getTermPositionsEnum(searcher->getIndexReader(), L"field",
                                          make_shared<BytesRef>(L"2"));
  pos->nextDoc();
  // second token should be at position 2
  assertEquals(2, pos->nextPosition());

  shared_ptr<PhraseQuery> q;
  std::deque<std::shared_ptr<ScoreDoc>> hits;

  q = make_shared<PhraseQuery>(L"field", L"1", L"2");
  hits = searcher->search(q, 1000)->scoreDocs;
  assertEquals(0, hits.size());

  // same as previous, using the builder with implicit positions
  shared_ptr<PhraseQuery::Builder> builder =
      make_shared<PhraseQuery::Builder>();
  builder->add(make_shared<Term>(L"field", L"1"));
  builder->add(make_shared<Term>(L"field", L"2"));
  q = builder->build();
  hits = searcher->search(q, 1000)->scoreDocs;
  assertEquals(0, hits.size());

  // same as previous, just specify positions explicitely.
  builder = make_shared<PhraseQuery::Builder>();
  builder->add(make_shared<Term>(L"field", L"1"), 0);
  builder->add(make_shared<Term>(L"field", L"2"), 1);
  q = builder->build();
  hits = searcher->search(q, 1000)->scoreDocs;
  assertEquals(0, hits.size());

  // specifying correct positions should find the phrase.
  builder = make_shared<PhraseQuery::Builder>();
  builder->add(make_shared<Term>(L"field", L"1"), 0);
  builder->add(make_shared<Term>(L"field", L"2"), 2);
  q = builder->build();
  hits = searcher->search(q, 1000)->scoreDocs;
  assertEquals(1, hits.size());

  q = make_shared<PhraseQuery>(L"field", L"2", L"3");
  hits = searcher->search(q, 1000)->scoreDocs;
  assertEquals(1, hits.size());

  q = make_shared<PhraseQuery>(L"field", L"3", L"4");
  hits = searcher->search(q, 1000)->scoreDocs;
  assertEquals(0, hits.size());

  // phrase query would find it when correct positions are specified.
  builder = make_shared<PhraseQuery::Builder>();
  builder->add(make_shared<Term>(L"field", L"3"), 0);
  builder->add(make_shared<Term>(L"field", L"4"), 0);
  q = builder->build();
  hits = searcher->search(q, 1000)->scoreDocs;
  assertEquals(1, hits.size());

  // phrase query should fail for non existing searched term
  // even if there exist another searched terms in the same searched position.
  builder = make_shared<PhraseQuery::Builder>();
  builder->add(make_shared<Term>(L"field", L"3"), 0);
  builder->add(make_shared<Term>(L"field", L"9"), 0);
  q = builder->build();
  hits = searcher->search(q, 1000)->scoreDocs;
  assertEquals(0, hits.size());

  // multi-phrase query should succed for non existing searched term
  // because there exist another searched terms in the same searched position.
  shared_ptr<MultiPhraseQuery::Builder> mqb =
      make_shared<MultiPhraseQuery::Builder>();
  mqb->add(
      std::deque<std::shared_ptr<Term>>{make_shared<Term>(L"field", L"3"),
                                         make_shared<Term>(L"field", L"9")},
      0);
  hits = searcher->search(mqb->build(), 1000)->scoreDocs;
  assertEquals(1, hits.size());

  q = make_shared<PhraseQuery>(L"field", L"2", L"4");
  hits = searcher->search(q, 1000)->scoreDocs;
  assertEquals(1, hits.size());

  q = make_shared<PhraseQuery>(L"field", L"3", L"5");
  hits = searcher->search(q, 1000)->scoreDocs;
  assertEquals(1, hits.size());

  q = make_shared<PhraseQuery>(L"field", L"4", L"5");
  hits = searcher->search(q, 1000)->scoreDocs;
  assertEquals(1, hits.size());

  q = make_shared<PhraseQuery>(L"field", L"2", L"5");
  hits = searcher->search(q, 1000)->scoreDocs;
  assertEquals(0, hits.size());

  delete reader;
  delete store;
}

TestPositionIncrement::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestPositionIncrement> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPositionIncrement::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<TokenizerAnonymousInnerClass>(shared_from_this()));
}

TestPositionIncrement::AnalyzerAnonymousInnerClass::
    TokenizerAnonymousInnerClass::TokenizerAnonymousInnerClass(
        shared_ptr<AnalyzerAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
  TOKENS = {L"1", L"2", L"3", L"4", L"5"};
  INCREMENTS = {1, 2, 1, 0, 1};
  i = 0;
  posIncrAtt = addAttribute(PositionIncrementAttribute::typeid);
  termAtt = addAttribute(CharTermAttribute::typeid);
  offsetAtt = addAttribute(OffsetAttribute::typeid);
}

bool TestPositionIncrement::AnalyzerAnonymousInnerClass::
    TokenizerAnonymousInnerClass::incrementToken()
{
  if (i == TOKENS->length) {
    return false;
  }
  clearAttributes();
  termAtt->append(TOKENS[i]);
  offsetAtt::setOffset(i, i);
  posIncrAtt::setPositionIncrement(INCREMENTS[i]);
  i++;
  return true;
}

void TestPositionIncrement::AnalyzerAnonymousInnerClass::
    TokenizerAnonymousInnerClass::reset() 
{
  outerInstance->outerInstance.super.reset();
  this->i = 0;
}

void TestPositionIncrement::PayloadSpanCollector::collectLeaf(
    shared_ptr<PostingsEnum> postings, int position,
    shared_ptr<Term> term) 
{
  if (postings->getPayload() != nullptr) {
    payloads.push_back(BytesRef::deepCopyOf(postings->getPayload()));
  }
}

void TestPositionIncrement::PayloadSpanCollector::reset() { payloads.clear(); }

void TestPositionIncrement::testPayloadsPos0() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), dir, make_shared<MockPayloadAnalyzer>());
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(
      L"content",
      make_shared<StringReader>(L"a a b c d e a f g h i j a b k k")));
  writer->addDocument(doc);

  shared_ptr<IndexReader> *const readerFromWriter = writer->getReader();
  shared_ptr<LeafReader> r = getOnlyLeafReader(readerFromWriter);

  shared_ptr<PostingsEnum> tp =
      r->postings(make_shared<Term>(L"content", L"a"), PostingsEnum::ALL);

  int count = 0;
  assertTrue(tp->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  // "a" occurs 4 times
  assertEquals(4, tp->freq());
  assertEquals(0, tp->nextPosition());
  assertEquals(1, tp->nextPosition());
  assertEquals(3, tp->nextPosition());
  assertEquals(6, tp->nextPosition());

  // only one doc has "a"
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, tp->nextDoc());

  shared_ptr<IndexSearcher> is =
      newSearcher(getOnlyLeafReader(readerFromWriter));

  shared_ptr<SpanTermQuery> stq1 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"content", L"a"));
  shared_ptr<SpanTermQuery> stq2 =
      make_shared<SpanTermQuery>(make_shared<Term>(L"content", L"k"));
  std::deque<std::shared_ptr<SpanQuery>> sqs = {stq1, stq2};
  shared_ptr<SpanNearQuery> snq = make_shared<SpanNearQuery>(sqs, 30, false);

  count = 0;
  bool sawZero = false;
  if (VERBOSE) {
    wcout << L"\ngetPayloadSpans test" << endl;
  }
  shared_ptr<PayloadSpanCollector> collector =
      make_shared<PayloadSpanCollector>();
  shared_ptr<Spans> pspans = snq->createWeight(is, false, 1.0f)
                                 ->getSpans(is->getIndexReader()->leaves()[0],
                                            SpanWeight::Postings::PAYLOADS);
  while (pspans->nextDoc() != Spans::NO_MORE_DOCS) {
    while (pspans->nextStartPosition() != Spans::NO_MORE_POSITIONS) {
      if (VERBOSE) {
        wcout << L"doc " << pspans->docID() << L": span "
              << pspans->startPosition() << L" to " << pspans->endPosition()
              << endl;
      }
      collector->reset();
      pspans->collect(collector);
      sawZero |= pspans->startPosition() == 0;
      for (auto payload : collector->payloads) {
        count++;
        if (VERBOSE) {
          // C++ TODO: There is no native C++ equivalent to 'toString':
          wcout << L"  payload: " << Term::toString(payload) << endl;
        }
      }
    }
  }
  assertTrue(sawZero);
  assertEquals(8, count);

  // System.out.println("\ngetSpans test");
  shared_ptr<Spans> spans = snq->createWeight(is, false, 1.0f)
                                ->getSpans(is->getIndexReader()->leaves()[0],
                                           SpanWeight::Postings::POSITIONS);
  count = 0;
  sawZero = false;
  while (spans->nextDoc() != Spans::NO_MORE_DOCS) {
    while (spans->nextStartPosition() != Spans::NO_MORE_POSITIONS) {
      count++;
      sawZero |= spans->startPosition() == 0;
      // System.out.println(spans.doc() + " - " + spans.start() + " - " +
      // spans.end());
    }
  }
  assertEquals(4, count);
  assertTrue(sawZero);

  delete writer;
  delete is->getIndexReader();
  delete dir;
}
} // namespace org::apache::lucene::search