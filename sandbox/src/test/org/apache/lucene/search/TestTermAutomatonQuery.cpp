using namespace std;

#include "TestTermAutomatonQuery.h"

namespace org::apache::lucene::search
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using MockTokenFilter = org::apache::lucene::analysis::MockTokenFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Token = org::apache::lucene::analysis::Token;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using StoredField = org::apache::lucene::document::StoredField;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using Directory = org::apache::lucene::store::Directory;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using BitSetIterator = org::apache::lucene::util::BitSetIterator;
using BytesRef = org::apache::lucene::util::BytesRef;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using Automata = org::apache::lucene::util::automaton::Automata;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using Transition = org::apache::lucene::util::automaton::Transition;

void TestTermAutomatonQuery::testBasic1() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  // matches
  doc->push_back(
      newTextField(L"field", L"here comes the sun", Field::Store::NO));
  w->addDocument(doc);

  doc = make_shared<Document>();
  // doesn't match
  doc->push_back(
      newTextField(L"field", L"here comes the other sun", Field::Store::NO));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<IndexSearcher> s = newSearcher(r);

  shared_ptr<TermAutomatonQuery> q = make_shared<TermAutomatonQuery>(L"field");
  int init = q->createState();
  int s1 = q->createState();
  q->addTransition(init, s1, L"comes");
  int s2 = q->createState();
  q->addAnyTransition(s1, s2);
  int s3 = q->createState();
  q->setAccept(s3, true);
  q->addTransition(s2, s3, L"sun");
  q->finish();

  TestUtil::assertEquals(1, s->search(q, 1)->totalHits);

  delete w;
  delete r;
  delete dir;
}

void TestTermAutomatonQuery::testBasicSynonym() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field", L"here comes the sun", Field::Store::NO));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field", L"here comes the moon", Field::Store::NO));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<IndexSearcher> s = newSearcher(r);

  shared_ptr<TermAutomatonQuery> q = make_shared<TermAutomatonQuery>(L"field");
  int init = q->createState();
  int s1 = q->createState();
  q->addTransition(init, s1, L"comes");
  int s2 = q->createState();
  q->addAnyTransition(s1, s2);
  int s3 = q->createState();
  q->setAccept(s3, true);
  q->addTransition(s2, s3, L"sun");
  q->addTransition(s2, s3, L"moon");
  q->finish();

  TestUtil::assertEquals(2, s->search(q, 1)->totalHits);

  delete w;
  delete r;
  delete dir;
}

void TestTermAutomatonQuery::testBasicSlop() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field", L"here comes the sun", Field::Store::NO));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"here comes sun", Field::Store::NO));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field", L"here comes the other sun", Field::Store::NO));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<IndexSearcher> s = newSearcher(r);

  shared_ptr<TermAutomatonQuery> q = make_shared<TermAutomatonQuery>(L"field");
  int init = q->createState();
  int s1 = q->createState();
  q->addTransition(init, s1, L"comes");
  int s2 = q->createState();
  q->addAnyTransition(s1, s2);
  int s3 = q->createState();
  q->setAccept(s3, true);
  q->addTransition(s1, s3, L"sun");
  q->addTransition(s2, s3, L"sun");
  q->finish();

  TestUtil::assertEquals(2, s->search(q, 1)->totalHits);

  delete w;
  delete r;
  delete dir;
}

void TestTermAutomatonQuery::testPosLengthAtQueryTimeMock() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field", L"speedy wifi network", Field::Store::NO));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field", L"speedy wi fi network", Field::Store::NO));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field", L"fast wifi network", Field::Store::NO));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field", L"fast wi fi network", Field::Store::NO));
  w->addDocument(doc);

  // doesn't match:
  doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field", L"slow wi fi network", Field::Store::NO));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<IndexSearcher> s = newSearcher(r);

  shared_ptr<TermAutomatonQuery> q = make_shared<TermAutomatonQuery>(L"field");
  int init = q->createState();
  int s1 = q->createState();
  q->addTransition(init, s1, L"fast");
  q->addTransition(init, s1, L"speedy");
  int s2 = q->createState();
  int s3 = q->createState();
  q->addTransition(s1, s2, L"wi");
  q->addTransition(s1, s3, L"wifi");
  q->addTransition(s2, s3, L"fi");
  int s4 = q->createState();
  q->addTransition(s3, s4, L"network");
  q->setAccept(s4, true);
  q->finish();

  // System.out.println("DOT:\n" + q.toDot());

  TestUtil::assertEquals(4, s->search(q, 1)->totalHits);

  delete w;
  delete r;
  delete dir;
}

void TestTermAutomatonQuery::testPosLengthAtQueryTimeTrueish() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field", L"speedy wifi network", Field::Store::NO));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field", L"speedy wi fi network", Field::Store::NO));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field", L"fast wifi network", Field::Store::NO));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field", L"fast wi fi network", Field::Store::NO));
  w->addDocument(doc);

  // doesn't match:
  doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field", L"slow wi fi network", Field::Store::NO));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<IndexSearcher> s = newSearcher(r);

  shared_ptr<TokenStream> ts =
      make_shared<CannedTokenStream>(std::deque<std::shared_ptr<Token>>{
          token(L"fast", 1, 1), token(L"speedy", 0, 1), token(L"wi", 1, 1),
          token(L"wifi", 0, 2), token(L"fi", 1, 1), token(L"network", 1, 1)});

  shared_ptr<TermAutomatonQuery> q =
      (make_shared<TokenStreamToTermAutomatonQuery>())->toQuery(L"field", ts);
  // System.out.println("DOT: " + q.toDot());
  TestUtil::assertEquals(4, s->search(q, 1)->totalHits);

  delete w;
  delete r;
  delete dir;
}

void TestTermAutomatonQuery::testSegsMissingTerms() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field", L"here comes the sun", Field::Store::NO));
  w->addDocument(doc);
  w->commit();

  doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field", L"here comes the moon", Field::Store::NO));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<IndexSearcher> s = newSearcher(r);

  shared_ptr<TermAutomatonQuery> q = make_shared<TermAutomatonQuery>(L"field");
  int init = q->createState();
  int s1 = q->createState();
  q->addTransition(init, s1, L"comes");
  int s2 = q->createState();
  q->addAnyTransition(s1, s2);
  int s3 = q->createState();
  q->setAccept(s3, true);
  q->addTransition(s2, s3, L"sun");
  q->addTransition(s2, s3, L"moon");
  q->finish();

  TestUtil::assertEquals(2, s->search(q, 1)->totalHits);
  delete w;
  delete r;
  delete dir;
}

void TestTermAutomatonQuery::testInvalidLeadWithAny() 
{
  shared_ptr<TermAutomatonQuery> q = make_shared<TermAutomatonQuery>(L"field");
  int s0 = q->createState();
  int s1 = q->createState();
  int s2 = q->createState();
  q->setAccept(s2, true);
  q->addAnyTransition(s0, s1);
  q->addTransition(s1, s2, L"b");
  expectThrows(IllegalStateException::typeid, [&]() { q->finish(); });
}

void TestTermAutomatonQuery::testInvalidTrailWithAny() 
{
  shared_ptr<TermAutomatonQuery> q = make_shared<TermAutomatonQuery>(L"field");
  int s0 = q->createState();
  int s1 = q->createState();
  int s2 = q->createState();
  q->setAccept(s2, true);
  q->addTransition(s0, s1, L"b");
  q->addAnyTransition(s1, s2);
  expectThrows(IllegalStateException::typeid, [&]() { q->finish(); });
}

void TestTermAutomatonQuery::testAnyFromTokenStream() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field", L"here comes the sun", Field::Store::NO));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field", L"here comes the moon", Field::Store::NO));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"here comes sun", Field::Store::NO));
  w->addDocument(doc);

  // Should not match:
  doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field", L"here comes the other sun", Field::Store::NO));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<IndexSearcher> s = newSearcher(r);

  shared_ptr<TokenStream> ts =
      make_shared<CannedTokenStream>(std::deque<std::shared_ptr<Token>>{
          token(L"comes", 1, 1), token(L"comes", 0, 2), token(L"*", 1, 1),
          token(L"sun", 1, 1), token(L"moon", 0, 1)});

  shared_ptr<TermAutomatonQuery> q =
      (make_shared<TokenStreamToTermAutomatonQuery>())->toQuery(L"field", ts);
  // System.out.println("DOT: " + q.toDot());
  TestUtil::assertEquals(3, s->search(q, 1)->totalHits);

  delete w;
  delete r;
  delete dir;
}

shared_ptr<Token> TestTermAutomatonQuery::token(const wstring &term, int posInc,
                                                int posLength)
{
  shared_ptr<Token> *const t = make_shared<Token>(term, 0, term.length());
  t->setPositionIncrement(posInc);
  t->setPositionLength(posLength);
  return t;
}

TestTermAutomatonQuery::RandomSynonymFilter::RandomSynonymFilter(
    shared_ptr<TokenFilter> in_)
    : org::apache::lucene::analysis::TokenFilter(in_)
{
}

bool TestTermAutomatonQuery::RandomSynonymFilter::incrementToken() throw(
    IOException)
{
  if (synNext) {
    shared_ptr<AttributeSource::State> state = captureState();
    clearAttributes();
    restoreState(state);
    posIncAtt->setPositionIncrement(0);
    termAtt->append(L"" + (static_cast<wchar_t>(97) +
                           LuceneTestCase::random()->nextInt(3)));
    synNext = false;
    return true;
  }

  if (input->incrementToken()) {
    if (LuceneTestCase::random()->nextInt(10) == 8) {
      synNext = true;
    }
    return true;
  } else {
    return false;
  }
}

void TestTermAutomatonQuery::RandomSynonymFilter::reset() 
{
  TokenFilter::reset();
  synNext = false;
}

void TestTermAutomatonQuery::testRandom() 
{
  int numDocs = atLeast(100);
  shared_ptr<Directory> dir = newDirectory();

  // Adds occassional random synonyms:
  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());

  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    int numTokens = atLeast(10);

    shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
    for (int j = 0; j < numTokens; j++) {
      sb->append(L' ');
      sb->append(static_cast<wchar_t>(97 + random()->nextInt(3)));
    }
    wstring contents = sb->toString();
    doc->push_back(newTextField(L"field", contents, Field::Store::NO));
    doc->push_back(make_shared<StoredField>(L"id", L"" + to_wstring(i)));
    if (VERBOSE) {
      wcout << L"  doc " << i << L" -> " << contents << endl;
    }
    w->addDocument(doc);
  }

  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<IndexSearcher> s = newSearcher(r);

  // Used to match ANY using MultiPhraseQuery:
  std::deque<std::shared_ptr<Term>> allTerms = {
      make_shared<Term>(L"field", L"a"), make_shared<Term>(L"field", L"b"),
      make_shared<Term>(L"field", L"c")};
  int numIters = atLeast(1000);
  for (int iter = 0; iter < numIters; iter++) {

    // Build the (finite, no any transitions) TermAutomatonQuery and
    // also the "equivalent" BooleanQuery and make sure they match the
    // same docs:
    shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
    int count = TestUtil::nextInt(random(), 1, 5);
    shared_ptr<Set<std::shared_ptr<BytesRef>>> strings =
        unordered_set<std::shared_ptr<BytesRef>>();
    for (int i = 0; i < count; i++) {
      shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
      int numTokens = TestUtil::nextInt(random(), 1, 5);
      for (int j = 0; j < numTokens; j++) {
        if (j > 0 && j < numTokens - 1 && random()->nextInt(5) == 3) {
          sb->append(L'*');
        } else {
          sb->append(static_cast<wchar_t>(97 + random()->nextInt(3)));
        }
      }
      wstring string = sb->toString();
      shared_ptr<MultiPhraseQuery::Builder> mpqb =
          make_shared<MultiPhraseQuery::Builder>();
      for (int j = 0; j < string.length(); j++) {
        if (string[j] == L'*') {
          mpqb->add(allTerms);
        } else {
          mpqb->add(make_shared<Term>(L"field",
                                      L"" + StringHelper::toString(string[j])));
        }
      }
      bq->add(mpqb->build(), BooleanClause::Occur::SHOULD);
      strings->add(make_shared<BytesRef>(string));
    }

    deque<std::shared_ptr<BytesRef>> stringsList =
        deque<std::shared_ptr<BytesRef>>(strings);
    sort(stringsList.begin(), stringsList.end());

    shared_ptr<Automaton> a = Automata::makeStringUnion(stringsList);

    // Translate automaton to query:

    shared_ptr<TermAutomatonQuery> q =
        make_shared<TermAutomatonQuery>(L"field");
    int numStates = a->getNumStates();
    for (int i = 0; i < numStates; i++) {
      q->createState();
      q->setAccept(i, a->isAccept(i));
    }

    shared_ptr<Transition> t = make_shared<Transition>();
    for (int i = 0; i < numStates; i++) {
      int transCount = a->initTransition(i, t);
      for (int j = 0; j < transCount; j++) {
        a->getNextTransition(t);
        for (int label = t->min; label <= t->max; label++) {
          if (static_cast<wchar_t>(label) == L'*') {
            q->addAnyTransition(t->source, t->dest);
          } else {
            q->addTransition(
                t->source, t->dest,
                L"" + StringHelper::toString(static_cast<wchar_t>(label)));
          }
        }
      }
    }
    q->finish();

    if (VERBOSE) {
      wcout << L"TEST: iter=" << iter << endl;
      for (auto string : stringsList) {
        wcout << L"  string: " << string->utf8ToString() << endl;
      }
      wcout << q->toDot() << endl;
    }

    shared_ptr<Query> q1 = q;
    shared_ptr<Query> q2 = bq->build();
    if (random()->nextInt(5) == 1) {
      if (VERBOSE) {
        wcout << L"  use random filter" << endl;
      }
      shared_ptr<RandomQuery> filter =
          make_shared<RandomQuery>(random()->nextLong(), random()->nextFloat());
      q1 = (make_shared<BooleanQuery::Builder>())
               ->add(q1, Occur::MUST)
               ->add(filter, Occur::FILTER)
               ->build();
      q2 = (make_shared<BooleanQuery::Builder>())
               ->add(q2, Occur::MUST)
               ->add(filter, Occur::FILTER)
               ->build();
    }

    shared_ptr<TopDocs> hits1 = s->search(q1, numDocs);
    shared_ptr<TopDocs> hits2 = s->search(q2, numDocs);
    shared_ptr<Set<wstring>> hits1Docs = toDocIDs(s, hits1);
    shared_ptr<Set<wstring>> hits2Docs = toDocIDs(s, hits2);

    try {
      TestUtil::assertEquals(hits2->totalHits, hits1->totalHits);
      TestUtil::assertEquals(hits2Docs, hits1Docs);
    } catch (const AssertionError &ae) {
      wcout << L"FAILED:" << endl;
      for (auto id : hits1Docs) {
        if (hits2Docs->contains(id) == false) {
          wcout << wstring::format(Locale::ROOT,
                                   L"  id=%3s matched but should not have", id)
                << endl;
        }
      }
      for (auto id : hits2Docs) {
        if (hits1Docs->contains(id) == false) {
          wcout << wstring::format(Locale::ROOT,
                                   L"  id=%3s did not match but should have",
                                   id)
                << endl;
        }
      }
      throw ae;
    }
  }

  IOUtils::close({w, r, dir, analyzer});
}

TestTermAutomatonQuery::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestTermAutomatonQuery> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestTermAutomatonQuery::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<MockTokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true, 100);
  tokenizer->setEnableChecks(true);
  shared_ptr<TokenFilter> filt =
      make_shared<MockTokenFilter>(tokenizer, MockTokenFilter::EMPTY_STOPSET);
  filt = make_shared<RandomSynonymFilter>(filt);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, filt);
}

shared_ptr<Set<wstring>>
TestTermAutomatonQuery::toDocIDs(shared_ptr<IndexSearcher> s,
                                 shared_ptr<TopDocs> hits) 
{
  shared_ptr<Set<wstring>> result = unordered_set<wstring>();
  for (auto hit : hits->scoreDocs) {
    result->add(s->doc(hit->doc)[L"id"]);
  }
  return result;
}

TestTermAutomatonQuery::RandomQuery::RandomQuery(int64_t seed, float density)
    : seed(seed)
{
  this->density = density;
}

shared_ptr<Weight> TestTermAutomatonQuery::RandomQuery::createWeight(
    shared_ptr<IndexSearcher> searcher, bool needsScores,
    float boost) 
{
  return make_shared<ConstantScoreWeightAnonymousInnerClass>(shared_from_this(),
                                                             boost);
}

TestTermAutomatonQuery::RandomQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<RandomQuery> outerInstance, float boost)
    : ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Scorer>
TestTermAutomatonQuery::RandomQuery::ConstantScoreWeightAnonymousInnerClass::
    scorer(shared_ptr<LeafReaderContext> context) 
{
  int maxDoc = context->reader()->maxDoc();
  shared_ptr<FixedBitSet> bits = make_shared<FixedBitSet>(maxDoc);
  shared_ptr<Random> random =
      make_shared<Random>(outerInstance->seed ^ context->docBase);
  for (int docID = 0; docID < maxDoc; docID++) {
    if (random->nextFloat() <= outerInstance->density) {
      bits->set(docID);
      // System.out.println("  acc id=" + idSource.getInt(docID) + " docID=" +
      // docID);
    }
  }
  return make_shared<ConstantScoreScorer>(
      shared_from_this(), score(),
      make_shared<BitSetIterator>(bits, bits->approximateCardinality()));
}

bool TestTermAutomatonQuery::RandomQuery::
    ConstantScoreWeightAnonymousInnerClass::isCacheable(
        shared_ptr<LeafReaderContext> ctx)
{
  return false;
}

wstring TestTermAutomatonQuery::RandomQuery::toString(const wstring &field)
{
  return L"RandomFilter(seed=" + to_wstring(seed) + L",density=" +
         to_wstring(density) + L")";
}

bool TestTermAutomatonQuery::RandomQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool TestTermAutomatonQuery::RandomQuery::equalsTo(
    shared_ptr<RandomQuery> other)
{
  return seed == other->seed && density == other->density;
}

int TestTermAutomatonQuery::RandomQuery::hashCode()
{
  return classHash() ^ Objects::hash(seed, density);
}

void TestTermAutomatonQuery::testWithCycles1() 
{

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field", L"here comes here comes", Field::Store::NO));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"comes foo", Field::Store::NO));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<IndexSearcher> s = newSearcher(r);

  shared_ptr<TermAutomatonQuery> q = make_shared<TermAutomatonQuery>(L"field");
  int init = q->createState();
  int s1 = q->createState();
  int s2 = q->createState();
  q->addTransition(init, s1, L"here");
  q->addTransition(s1, s2, L"comes");
  q->addTransition(s2, s1, L"here");
  q->setAccept(s1, true);
  q->finish();

  TestUtil::assertEquals(1, s->search(q, 1)->totalHits);
  delete w;
  delete r;
  delete dir;
}

void TestTermAutomatonQuery::testWithCycles2() 
{

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"here comes kaoma", Field::Store::NO));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"here comes sun sun sun sun kaoma",
                              Field::Store::NO));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<IndexSearcher> s = newSearcher(r);

  shared_ptr<TermAutomatonQuery> q = make_shared<TermAutomatonQuery>(L"field");
  int init = q->createState();
  int s1 = q->createState();
  q->addTransition(init, s1, L"here");
  int s2 = q->createState();
  q->addTransition(s1, s2, L"comes");
  int s3 = q->createState();
  q->addTransition(s2, s3, L"sun");
  q->addTransition(s3, s3, L"sun");
  int s4 = q->createState();
  q->addTransition(s3, s4, L"kaoma");
  q->setAccept(s4, true);
  q->finish();

  TestUtil::assertEquals(1, s->search(q, 1)->totalHits);
  delete w;
  delete r;
  delete dir;
}

void TestTermAutomatonQuery::testTermDoesNotExist() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"x y z", Field::Store::NO));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<IndexSearcher> s = newSearcher(r);

  shared_ptr<TokenStream> ts = make_shared<CannedTokenStream>(
      std::deque<std::shared_ptr<Token>>{token(L"a", 1, 1)});

  shared_ptr<TermAutomatonQuery> q =
      (make_shared<TokenStreamToTermAutomatonQuery>())->toQuery(L"field", ts);
  // System.out.println("DOT: " + q.toDot());
  TestUtil::assertEquals(0, s->search(q, 1)->totalHits);

  delete w;
  delete r;
  delete dir;
}

void TestTermAutomatonQuery::testOneTermDoesNotExist() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"x y z", Field::Store::NO));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<IndexSearcher> s = newSearcher(r);

  shared_ptr<TokenStream> ts =
      make_shared<CannedTokenStream>(std::deque<std::shared_ptr<Token>>{
          token(L"a", 1, 1), token(L"x", 1, 1)});

  shared_ptr<TermAutomatonQuery> q =
      (make_shared<TokenStreamToTermAutomatonQuery>())->toQuery(L"field", ts);
  // System.out.println("DOT: " + q.toDot());
  TestUtil::assertEquals(0, s->search(q, 1)->totalHits);

  IOUtils::close({w, r, dir});
}

void TestTermAutomatonQuery::testEmptyString() 
{
  shared_ptr<TermAutomatonQuery> q = make_shared<TermAutomatonQuery>(L"field");
  int initState = q->createState();
  q->setAccept(initState, true);
  try {
    q->finish();
    fail(L"did not hit exc");
  } catch (const IllegalStateException &ise) {
    // expected
  }
}

void TestTermAutomatonQuery::testRewriteNoMatch() 
{
  shared_ptr<TermAutomatonQuery> q = make_shared<TermAutomatonQuery>(L"field");
  int initState = q->createState();
  q->finish();

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"x y z", Field::Store::NO));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = w->getReader();
  assertTrue(std::dynamic_pointer_cast<MatchNoDocsQuery>(q->rewrite(r)) !=
             nullptr);
  IOUtils::close({w, r, dir});
}

void TestTermAutomatonQuery::testRewriteTerm() 
{
  shared_ptr<TermAutomatonQuery> q = make_shared<TermAutomatonQuery>(L"field");
  int initState = q->createState();
  int s1 = q->createState();
  q->addTransition(initState, s1, L"foo");
  q->setAccept(s1, true);
  q->finish();

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"x y z", Field::Store::NO));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<Query> rewrite = q->rewrite(r);
  assertTrue(std::dynamic_pointer_cast<TermQuery>(rewrite) != nullptr);
  TestUtil::assertEquals(
      make_shared<Term>(L"field", L"foo"),
      (std::static_pointer_cast<TermQuery>(rewrite))->getTerm());
  IOUtils::close({w, r, dir});
}

void TestTermAutomatonQuery::testRewriteSimplePhrase() 
{
  shared_ptr<TermAutomatonQuery> q = make_shared<TermAutomatonQuery>(L"field");
  int initState = q->createState();
  int s1 = q->createState();
  int s2 = q->createState();
  q->addTransition(initState, s1, L"foo");
  q->addTransition(s1, s2, L"bar");
  q->setAccept(s2, true);
  q->finish();

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"x y z", Field::Store::NO));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<Query> rewrite = q->rewrite(r);
  assertTrue(std::dynamic_pointer_cast<PhraseQuery>(rewrite) != nullptr);
  std::deque<std::shared_ptr<Term>> terms =
      (std::static_pointer_cast<PhraseQuery>(rewrite))->getTerms();
  TestUtil::assertEquals(make_shared<Term>(L"field", L"foo"), terms[0]);
  TestUtil::assertEquals(make_shared<Term>(L"field", L"bar"), terms[1]);

  std::deque<int> positions =
      (std::static_pointer_cast<PhraseQuery>(rewrite))->getPositions();
  TestUtil::assertEquals(0, positions[0]);
  TestUtil::assertEquals(1, positions[1]);

  IOUtils::close({w, r, dir});
}

void TestTermAutomatonQuery::testRewritePhraseWithAny() 
{
  shared_ptr<TermAutomatonQuery> q = make_shared<TermAutomatonQuery>(L"field");
  int initState = q->createState();
  int s1 = q->createState();
  int s2 = q->createState();
  int s3 = q->createState();
  q->addTransition(initState, s1, L"foo");
  q->addAnyTransition(s1, s2);
  q->addTransition(s2, s3, L"bar");
  q->setAccept(s3, true);
  q->finish();

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"x y z", Field::Store::NO));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<Query> rewrite = q->rewrite(r);
  assertTrue(std::dynamic_pointer_cast<PhraseQuery>(rewrite) != nullptr);
  std::deque<std::shared_ptr<Term>> terms =
      (std::static_pointer_cast<PhraseQuery>(rewrite))->getTerms();
  TestUtil::assertEquals(make_shared<Term>(L"field", L"foo"), terms[0]);
  TestUtil::assertEquals(make_shared<Term>(L"field", L"bar"), terms[1]);

  std::deque<int> positions =
      (std::static_pointer_cast<PhraseQuery>(rewrite))->getPositions();
  TestUtil::assertEquals(0, positions[0]);
  TestUtil::assertEquals(2, positions[1]);

  IOUtils::close({w, r, dir});
}

void TestTermAutomatonQuery::testRewriteSimpleMultiPhrase() 
{
  shared_ptr<TermAutomatonQuery> q = make_shared<TermAutomatonQuery>(L"field");
  int initState = q->createState();
  int s1 = q->createState();
  q->addTransition(initState, s1, L"foo");
  q->addTransition(initState, s1, L"bar");
  q->setAccept(s1, true);
  q->finish();

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"x y z", Field::Store::NO));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<Query> rewrite = q->rewrite(r);
  assertTrue(std::dynamic_pointer_cast<MultiPhraseQuery>(rewrite) != nullptr);
  std::deque<std::deque<std::shared_ptr<Term>>> terms =
      (std::static_pointer_cast<MultiPhraseQuery>(rewrite))->getTermArrays();
  TestUtil::assertEquals(1, terms.size());
  TestUtil::assertEquals(2, terms[0].size());
  TestUtil::assertEquals(make_shared<Term>(L"field", L"foo"), terms[0][0]);
  TestUtil::assertEquals(make_shared<Term>(L"field", L"bar"), terms[0][1]);

  std::deque<int> positions =
      (std::static_pointer_cast<MultiPhraseQuery>(rewrite))->getPositions();
  TestUtil::assertEquals(1, positions.size());
  TestUtil::assertEquals(0, positions[0]);

  IOUtils::close({w, r, dir});
}

void TestTermAutomatonQuery::testRewriteMultiPhraseWithAny() throw(
    runtime_error)
{
  shared_ptr<TermAutomatonQuery> q = make_shared<TermAutomatonQuery>(L"field");
  int initState = q->createState();
  int s1 = q->createState();
  int s2 = q->createState();
  int s3 = q->createState();
  q->addTransition(initState, s1, L"foo");
  q->addTransition(initState, s1, L"bar");
  q->addAnyTransition(s1, s2);
  q->addTransition(s2, s3, L"baz");
  q->setAccept(s3, true);
  q->finish();

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"x y z", Field::Store::NO));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<Query> rewrite = q->rewrite(r);
  assertTrue(std::dynamic_pointer_cast<MultiPhraseQuery>(rewrite) != nullptr);
  std::deque<std::deque<std::shared_ptr<Term>>> terms =
      (std::static_pointer_cast<MultiPhraseQuery>(rewrite))->getTermArrays();
  TestUtil::assertEquals(2, terms.size());
  TestUtil::assertEquals(2, terms[0].size());
  TestUtil::assertEquals(make_shared<Term>(L"field", L"foo"), terms[0][0]);
  TestUtil::assertEquals(make_shared<Term>(L"field", L"bar"), terms[0][1]);
  TestUtil::assertEquals(1, terms[1].size());
  TestUtil::assertEquals(make_shared<Term>(L"field", L"baz"), terms[1][0]);

  std::deque<int> positions =
      (std::static_pointer_cast<MultiPhraseQuery>(rewrite))->getPositions();
  TestUtil::assertEquals(2, positions.size());
  TestUtil::assertEquals(0, positions[0]);
  TestUtil::assertEquals(2, positions[1]);

  IOUtils::close({w, r, dir});
}
} // namespace org::apache::lucene::search