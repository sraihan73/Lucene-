using namespace std;

#include "TestScorerPerf.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using BitSetIterator = org::apache::lucene::util::BitSetIterator;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestScorerPerf::createDummySearcher() 
{
  // Create a dummy index with nothing in it.
  // This could possibly fail if Lucene starts checking for docid ranges...
  d = newDirectory();
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      d, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  iw->addDocument(make_shared<Document>());
  delete iw;
  r = DirectoryReader::open(d);
  s = newSearcher(r);
  s->setQueryCache(nullptr);
}

void TestScorerPerf::createRandomTerms(
    int nDocs, int nTerms, double power,
    shared_ptr<Directory> dir) 
{
  std::deque<int> freq(nTerms);
  terms = std::deque<std::shared_ptr<Term>>(nTerms);
  for (int i = 0; i < nTerms; i++) {
    int f = (nTerms + 1) - i; // make first terms less frequent
    freq[i] = static_cast<int>(ceil(pow(f, power)));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    terms[i] = make_shared<Term>(
        L"f", Character::toString(static_cast<wchar_t>(L'A' + i)));
  }

  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setOpenMode(OpenMode::CREATE));
  for (int i = 0; i < nDocs; i++) {
    shared_ptr<Document> d = make_shared<Document>();
    for (int j = 0; j < nTerms; j++) {
      if (random()->nextInt(freq[j]) == 0) {
        d->push_back(newStringField(L"f", terms[j]->text(), Field::Store::NO));
        // System.out.println(d);
      }
    }
    iw->addDocument(d);
  }
  iw->forceMerge(1);
  delete iw;
}

shared_ptr<FixedBitSet> TestScorerPerf::randBitSet(int sz, int numBitsToSet)
{
  shared_ptr<FixedBitSet> set = make_shared<FixedBitSet>(sz);
  for (int i = 0; i < numBitsToSet; i++) {
    set->set(random()->nextInt(sz));
  }
  return set;
}

std::deque<std::shared_ptr<FixedBitSet>>
TestScorerPerf::randBitSets(int numSets, int setSize)
{
  std::deque<std::shared_ptr<FixedBitSet>> sets(numSets);
  for (int i = 0; i < sets.size(); i++) {
    sets[i] = randBitSet(setSize, random()->nextInt(setSize));
  }
  return sets;
}

void TestScorerPerf::CountingHitCollector::collect(int doc)
{
  count++;
  sum += docBase +
         doc; // use it to avoid any possibility of being eliminated by hotspot
}

int TestScorerPerf::CountingHitCollector::getCount() { return count; }

int TestScorerPerf::CountingHitCollector::getSum() { return sum; }

void TestScorerPerf::CountingHitCollector::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  docBase = context->docBase;
}

bool TestScorerPerf::CountingHitCollector::needsScores() { return false; }

TestScorerPerf::MatchingHitCollector::MatchingHitCollector(
    shared_ptr<FixedBitSet> answer)
{
  this->answer = answer;
}

void TestScorerPerf::MatchingHitCollector::collect(int doc, float score)
{

  pos = answer->nextSetBit(pos + 1);
  if (pos != doc + docBase) {
    throw runtime_error(L"Expected doc " + to_wstring(pos) + L" but got " +
                        to_wstring(doc) + to_wstring(docBase));
  }
  CountingHitCollector::collect(doc);
}

TestScorerPerf::BitSetQuery::BitSetQuery(shared_ptr<FixedBitSet> docs)
    : docs(docs)
{
}

shared_ptr<Weight>
TestScorerPerf::BitSetQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                          bool needsScores,
                                          float boost) 
{
  return make_shared<ConstantScoreWeightAnonymousInnerClass>(shared_from_this(),
                                                             boost);
}

TestScorerPerf::BitSetQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<BitSetQuery> outerInstance, float boost)
    : ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Scorer>
TestScorerPerf::BitSetQuery::ConstantScoreWeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  return make_shared<ConstantScoreScorer>(
      shared_from_this(), score(),
      make_shared<BitSetIterator>(
          outerInstance->docs, outerInstance->docs->approximateCardinality()));
}

bool TestScorerPerf::BitSetQuery::ConstantScoreWeightAnonymousInnerClass::
    isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return false;
}

wstring TestScorerPerf::BitSetQuery::toString(const wstring &field)
{
  return L"randomBitSetFilter";
}

bool TestScorerPerf::BitSetQuery::equals(any other)
{
  return sameClassAs(other) &&
         docs->equals((any_cast<std::shared_ptr<BitSetQuery>>(other)).docs);
}

int TestScorerPerf::BitSetQuery::hashCode()
{
  return 31 * classHash() + docs->hashCode();
}

shared_ptr<FixedBitSet>
TestScorerPerf::addClause(shared_ptr<BooleanQuery::Builder> bq,
                          shared_ptr<FixedBitSet> result)
{
  shared_ptr<FixedBitSet> *const rnd = sets[random()->nextInt(sets.size())];
  shared_ptr<Query> q = make_shared<BitSetQuery>(rnd);
  bq->add(q, BooleanClause::Occur::MUST);
  if (validate) {
    if (result == nullptr) {
      result = rnd->clone();
    } else {
      result->and (rnd);
    }
  }
  return result;
}

int TestScorerPerf::doConjunctions(int iter, int maxClauses) 
{
  int ret = 0;

  for (int i = 0; i < iter; i++) {
    int nClauses = random()->nextInt(maxClauses - 1) + 2; // min 2 clauses
    shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
    shared_ptr<FixedBitSet> result = nullptr;
    for (int j = 0; j < nClauses; j++) {
      result = addClause(bq, result);
    }

    shared_ptr<CountingHitCollector> hc =
        validate ? make_shared<MatchingHitCollector>(result)
                 : make_shared<CountingHitCollector>();
    s->search(bq->build(), hc);
    ret += hc->getSum();

    if (validate) {
      assertEquals(result->cardinality(), hc->getCount());
    }
    // System.out.println(hc.getCount());
  }

  return ret;
}

int TestScorerPerf::doNestedConjunctions(int iter, int maxOuterClauses,
                                         int maxClauses) 
{
  int ret = 0;
  int64_t nMatches = 0;

  for (int i = 0; i < iter; i++) {
    int oClauses = random()->nextInt(maxOuterClauses - 1) + 2;
    shared_ptr<BooleanQuery::Builder> oq = make_shared<BooleanQuery::Builder>();
    shared_ptr<FixedBitSet> result = nullptr;

    for (int o = 0; o < oClauses; o++) {

      int nClauses = random()->nextInt(maxClauses - 1) + 2; // min 2 clauses
      shared_ptr<BooleanQuery::Builder> bq =
          make_shared<BooleanQuery::Builder>();
      for (int j = 0; j < nClauses; j++) {
        result = addClause(bq, result);
      }

      oq->add(bq->build(), BooleanClause::Occur::MUST);
    } // outer

    shared_ptr<CountingHitCollector> hc =
        validate ? make_shared<MatchingHitCollector>(result)
                 : make_shared<CountingHitCollector>();
    s->search(oq->build(), hc);
    nMatches += hc->getCount();
    ret += hc->getSum();
    if (validate) {
      assertEquals(result->cardinality(), hc->getCount());
    }
    // System.out.println(hc.getCount());
  }
  if (VERBOSE) {
    wcout << L"Average number of matches=" << (nMatches / iter) << endl;
  }
  return ret;
}

int TestScorerPerf::doTermConjunctions(shared_ptr<IndexSearcher> s,
                                       int termsInIndex, int maxClauses,
                                       int iter) 
{
  int ret = 0;

  int64_t nMatches = 0;
  for (int i = 0; i < iter; i++) {
    int nClauses = random()->nextInt(maxClauses - 1) + 2; // min 2 clauses
    shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
    shared_ptr<BitSet> termflag = make_shared<BitSet>(termsInIndex);
    for (int j = 0; j < nClauses; j++) {
      int tnum;
      // don't pick same clause twice
      tnum = random()->nextInt(termsInIndex);
      if (termflag->get(tnum)) {
        tnum = termflag->nextClearBit(tnum);
      }
      if (tnum < 0 || tnum >= termsInIndex) {
        tnum = termflag->nextClearBit(0);
      }
      termflag->set(tnum);
      shared_ptr<Query> tq = make_shared<TermQuery>(terms[tnum]);
      bq->add(tq, BooleanClause::Occur::MUST);
    }

    shared_ptr<CountingHitCollector> hc = make_shared<CountingHitCollector>();
    s->search(bq->build(), hc);
    nMatches += hc->getCount();
    ret += hc->getSum();
  }
  if (VERBOSE) {
    wcout << L"Average number of matches=" << (nMatches / iter) << endl;
  }

  return ret;
}

int TestScorerPerf::doNestedTermConjunctions(shared_ptr<IndexSearcher> s,
                                             int termsInIndex,
                                             int maxOuterClauses,
                                             int maxClauses,
                                             int iter) 
{
  int ret = 0;
  int64_t nMatches = 0;
  for (int i = 0; i < iter; i++) {
    int oClauses = random()->nextInt(maxOuterClauses - 1) + 2;
    shared_ptr<BooleanQuery::Builder> oq = make_shared<BooleanQuery::Builder>();
    for (int o = 0; o < oClauses; o++) {

      int nClauses = random()->nextInt(maxClauses - 1) + 2; // min 2 clauses
      shared_ptr<BooleanQuery::Builder> bq =
          make_shared<BooleanQuery::Builder>();
      shared_ptr<BitSet> termflag = make_shared<BitSet>(termsInIndex);
      for (int j = 0; j < nClauses; j++) {
        int tnum;
        // don't pick same clause twice
        tnum = random()->nextInt(termsInIndex);
        if (termflag->get(tnum)) {
          tnum = termflag->nextClearBit(tnum);
        }
        if (tnum < 0 || tnum >= 25) {
          tnum = termflag->nextClearBit(0);
        }
        termflag->set(tnum);
        shared_ptr<Query> tq = make_shared<TermQuery>(terms[tnum]);
        bq->add(tq, BooleanClause::Occur::MUST);
      } // inner

      oq->add(bq->build(), BooleanClause::Occur::MUST);
    } // outer

    shared_ptr<CountingHitCollector> hc = make_shared<CountingHitCollector>();
    s->search(oq->build(), hc);
    nMatches += hc->getCount();
    ret += hc->getSum();
  }
  if (VERBOSE) {
    wcout << L"Average number of matches=" << (nMatches / iter) << endl;
  }
  return ret;
}

int TestScorerPerf::doSloppyPhrase(shared_ptr<IndexSearcher> s,
                                   int termsInIndex, int maxClauses,
                                   int iter) 
{
  int ret = 0;

  for (int i = 0; i < iter; i++) {
    int nClauses = random()->nextInt(maxClauses - 1) + 2; // min 2 clauses
    shared_ptr<PhraseQuery::Builder> builder =
        make_shared<PhraseQuery::Builder>();
    for (int j = 0; j < nClauses; j++) {
      int tnum = random()->nextInt(termsInIndex);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      builder->add(make_shared<Term>(
          L"f", Character::toString(static_cast<wchar_t>(tnum + L'A'))));
    }
    // slop could be random too
    builder->setSlop(termsInIndex);
    shared_ptr<PhraseQuery> q = builder->build();

    shared_ptr<CountingHitCollector> hc = make_shared<CountingHitCollector>();
    s->search(q, hc);
    ret += hc->getSum();
  }

  return ret;
}

void TestScorerPerf::testConjunctions() 
{
  // test many small sets... the bugs will be found on boundary conditions
  createDummySearcher();
  validate = true;
  sets = randBitSets(atLeast(1000), atLeast(10));
  doConjunctions(atLeast(10000), atLeast(5));
  doNestedConjunctions(atLeast(10000), atLeast(3), atLeast(3));
  delete r;
  delete d;
}
} // namespace org::apache::lucene::search