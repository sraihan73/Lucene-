using namespace std;

#include "TestMultiPhraseQuery.h"

namespace org::apache::lucene::search
{
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using Token = org::apache::lucene::analysis::Token;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using MultiFields = org::apache::lucene::index::MultiFields;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Directory = org::apache::lucene::store::Directory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Ignore;

void TestMultiPhraseQuery::testPhrasePrefix() 
{
  shared_ptr<Directory> indexStore = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), indexStore);
  add(L"blueberry pie", writer);
  add(L"blueberry strudel", writer);
  add(L"blueberry pizza", writer);
  add(L"blueberry chewing gum", writer);
  add(L"bluebird pizza", writer);
  add(L"bluebird foobar pizza", writer);
  add(L"piccadilly circus", writer);

  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  // search for "blueberry pi*":
  shared_ptr<MultiPhraseQuery::Builder> query1builder =
      make_shared<MultiPhraseQuery::Builder>();
  // search for "strawberry pi*":
  shared_ptr<MultiPhraseQuery::Builder> query2builder =
      make_shared<MultiPhraseQuery::Builder>();
  query1builder->add(make_shared<Term>(L"body", L"blueberry"));
  query2builder->add(make_shared<Term>(L"body", L"strawberry"));

  deque<std::shared_ptr<Term>> termsWithPrefix = deque<std::shared_ptr<Term>>();

  // this TermEnum gives "piccadilly", "pie" and "pizza".
  wstring prefix = L"pi";
  shared_ptr<TermsEnum> te = MultiFields::getTerms(reader, L"body")->begin();
  te->seekCeil(make_shared<BytesRef>(prefix));
  do {
    wstring s = te->term()->utf8ToString();
    if (StringHelper::startsWith(s, prefix)) {
      termsWithPrefix.push_back(make_shared<Term>(L"body", s));
    } else {
      break;
    }
  } while (te->next() != nullptr);

  query1builder->add(
      termsWithPrefix.toArray(std::deque<std::shared_ptr<Term>>(0)));
  shared_ptr<MultiPhraseQuery> query1 = query1builder->build();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"body:\"blueberry (piccadilly pie pizza)\"",
               query1->toString());

  query2builder->add(
      termsWithPrefix.toArray(std::deque<std::shared_ptr<Term>>(0)));
  shared_ptr<MultiPhraseQuery> query2 = query2builder->build();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"body:\"strawberry (piccadilly pie pizza)\"",
               query2->toString());

  std::deque<std::shared_ptr<ScoreDoc>> result;
  result = searcher->search(query1, 1000)->scoreDocs;
  assertEquals(2, result.size());
  result = searcher->search(query2, 1000)->scoreDocs;
  assertEquals(0, result.size());

  // search for "blue* pizza":
  shared_ptr<MultiPhraseQuery::Builder> query3builder =
      make_shared<MultiPhraseQuery::Builder>();
  termsWithPrefix.clear();
  prefix = L"blue";
  te->seekCeil(make_shared<BytesRef>(prefix));

  do {
    if (StringHelper::startsWith(te->term()->utf8ToString(), prefix)) {
      termsWithPrefix.push_back(
          make_shared<Term>(L"body", te->term()->utf8ToString()));
    }
  } while (te->next() != nullptr);

  query3builder->add(
      termsWithPrefix.toArray(std::deque<std::shared_ptr<Term>>(0)));
  query3builder->add(make_shared<Term>(L"body", L"pizza"));

  shared_ptr<MultiPhraseQuery> query3 = query3builder->build();

  result = searcher->search(query3, 1000)->scoreDocs;
  assertEquals(2, result.size()); // blueberry pizza, bluebird pizza
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"body:\"(blueberry bluebird) pizza\"", query3->toString());

  // test slop:
  query3builder->setSlop(1);
  query3 = query3builder->build();
  result = searcher->search(query3, 1000)->scoreDocs;

  // just make sure no exc:
  searcher->explain(query3, 0);

  assertEquals(3, result.size()); // blueberry pizza, bluebird pizza, bluebird
                                  // foobar pizza

  shared_ptr<MultiPhraseQuery::Builder> query4builder =
      make_shared<MultiPhraseQuery::Builder>();
  expectThrows(invalid_argument::typeid, [&]() {
    query4builder->add(make_shared<Term>(L"field1", L"foo"));
    query4builder->add(make_shared<Term>(L"field2", L"foobar"));
  });

  delete writer;
  delete reader;
  delete indexStore;
}

void TestMultiPhraseQuery::testTall() 
{
  shared_ptr<Directory> indexStore = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), indexStore);
  add(L"blueberry chocolate pie", writer);
  add(L"blueberry chocolate tart", writer);
  shared_ptr<IndexReader> r = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(r);
  shared_ptr<MultiPhraseQuery::Builder> qb =
      make_shared<MultiPhraseQuery::Builder>();
  qb->add(make_shared<Term>(L"body", L"blueberry"));
  qb->add(make_shared<Term>(L"body", L"chocolate"));
  qb->add(std::deque<std::shared_ptr<Term>>{
      make_shared<Term>(L"body", L"pie"), make_shared<Term>(L"body", L"tart")});
  assertEquals(2, searcher->search(qb->build(), 1)->totalHits);
  delete r;
  delete indexStore;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Ignore public void testMultiSloppyWithRepeats() throws
// java.io.IOException
void TestMultiPhraseQuery::testMultiSloppyWithRepeats() 
{
  shared_ptr<Directory> indexStore = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), indexStore);
  add(L"a b c d e f g h i k", writer);
  shared_ptr<IndexReader> r = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(r);

  shared_ptr<MultiPhraseQuery::Builder> qb =
      make_shared<MultiPhraseQuery::Builder>();
  // this will fail, when the scorer would propagate [a] rather than [a,b],
  qb->add(std::deque<std::shared_ptr<Term>>{make_shared<Term>(L"body", L"a"),
                                             make_shared<Term>(L"body", L"b")});
  qb->add(std::deque<std::shared_ptr<Term>>{make_shared<Term>(L"body", L"a")});
  qb->setSlop(6);
  assertEquals(
      1, searcher->search(qb->build(), 1)->totalHits); // should match on "a b"

  delete r;
  delete indexStore;
}

void TestMultiPhraseQuery::testMultiExactWithRepeats() 
{
  shared_ptr<Directory> indexStore = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), indexStore);
  add(L"a b c d e f g h i k", writer);
  shared_ptr<IndexReader> r = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(r);
  shared_ptr<MultiPhraseQuery::Builder> qb =
      make_shared<MultiPhraseQuery::Builder>();
  qb->add(std::deque<std::shared_ptr<Term>>{make_shared<Term>(L"body", L"a"),
                                             make_shared<Term>(L"body", L"d")},
          0);
  qb->add(std::deque<std::shared_ptr<Term>>{make_shared<Term>(L"body", L"a"),
                                             make_shared<Term>(L"body", L"f")},
          2);
  assertEquals(
      1, searcher->search(qb->build(), 1)->totalHits); // should match on "a b"
  delete r;
  delete indexStore;
}

void TestMultiPhraseQuery::add(
    const wstring &s, shared_ptr<RandomIndexWriter> writer) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"body", s, Field::Store::YES));
  writer->addDocument(doc);
}

void TestMultiPhraseQuery::
    testBooleanQueryContainingSingleTermPrefixQuery() 
{
  // this tests against bug 33161 (now fixed)
  // In order to cause the bug, the outer query must have more than one term
  // and all terms required.
  // The contained PhraseMultiQuery must contain exactly one term array.
  shared_ptr<Directory> indexStore = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), indexStore);
  add(L"blueberry pie", writer);
  add(L"blueberry chewing gum", writer);
  add(L"blue raspberry pie", writer);

  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  // This query will be equivalent to +body:pie +body:"blue*"
  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(make_shared<TermQuery>(make_shared<Term>(L"body", L"pie")),
         BooleanClause::Occur::MUST);

  shared_ptr<MultiPhraseQuery::Builder> troubleBuilder =
      make_shared<MultiPhraseQuery::Builder>();
  troubleBuilder->add(std::deque<std::shared_ptr<Term>>{
      make_shared<Term>(L"body", L"blueberry"),
      make_shared<Term>(L"body", L"blue")});
  q->add(troubleBuilder->build(), BooleanClause::Occur::MUST);

  // exception will be thrown here without fix
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(q->build(), 1000)->scoreDocs;

  assertEquals(L"Wrong number of hits", 2, hits.size());

  // just make sure no exc:
  searcher->explain(q->build(), 0);

  delete writer;
  delete reader;
  delete indexStore;
}

void TestMultiPhraseQuery::testPhrasePrefixWithBooleanQuery() 
{
  shared_ptr<Directory> indexStore = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), indexStore);
  add(L"This is a test", L"object", writer);
  add(L"a note", L"note", writer);

  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  // This query will be equivalent to +type:note +body:"a t*"
  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(make_shared<TermQuery>(make_shared<Term>(L"type", L"note")),
         BooleanClause::Occur::MUST);

  shared_ptr<MultiPhraseQuery::Builder> troubleBuilder =
      make_shared<MultiPhraseQuery::Builder>();
  troubleBuilder->add(make_shared<Term>(L"body", L"a"));
  troubleBuilder->add(
      std::deque<std::shared_ptr<Term>>{make_shared<Term>(L"body", L"test"),
                                         make_shared<Term>(L"body", L"this")});
  q->add(troubleBuilder->build(), BooleanClause::Occur::MUST);

  // exception will be thrown here without fix for #35626:
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(q->build(), 1000)->scoreDocs;
  assertEquals(L"Wrong number of hits", 0, hits.size());
  delete writer;
  delete reader;
  delete indexStore;
}

void TestMultiPhraseQuery::testNoDocs() 
{
  shared_ptr<Directory> indexStore = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), indexStore);
  add(L"a note", L"note", writer);

  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  shared_ptr<MultiPhraseQuery::Builder> qb =
      make_shared<MultiPhraseQuery::Builder>();
  qb->add(make_shared<Term>(L"body", L"a"));
  qb->add(
      std::deque<std::shared_ptr<Term>>{make_shared<Term>(L"body", L"nope"),
                                         make_shared<Term>(L"body", L"nope")});
  shared_ptr<MultiPhraseQuery> q = qb->build();
  assertEquals(L"Wrong number of hits", 0, searcher->search(q, 1)->totalHits);

  // just make sure no exc:
  searcher->explain(q, 0);

  delete writer;
  delete reader;
  delete indexStore;
}

void TestMultiPhraseQuery::testHashCodeAndEquals()
{
  shared_ptr<MultiPhraseQuery::Builder> query1builder =
      make_shared<MultiPhraseQuery::Builder>();
  shared_ptr<MultiPhraseQuery> query1 = query1builder->build();

  shared_ptr<MultiPhraseQuery::Builder> query2builder =
      make_shared<MultiPhraseQuery::Builder>();
  shared_ptr<MultiPhraseQuery> query2 = query2builder->build();

  assertEquals(query1->hashCode(), query2->hashCode());
  assertEquals(query1, query2);

  shared_ptr<Term> term1 = make_shared<Term>(L"someField", L"someText");

  query1builder->add(term1);
  query1 = query1builder->build();

  query2builder->add(term1);
  query2 = query2builder->build();

  assertEquals(query1->hashCode(), query2->hashCode());
  assertEquals(query1, query2);

  shared_ptr<Term> term2 = make_shared<Term>(L"someField", L"someMoreText");

  query1builder->add(term2);
  query1 = query1builder->build();

  assertFalse(query1->hashCode() == query2->hashCode());
  assertFalse(query1->equals(query2));

  query2builder->add(term2);
  query2 = query2builder->build();

  assertEquals(query1->hashCode(), query2->hashCode());
  assertEquals(query1, query2);
}

void TestMultiPhraseQuery::add(
    const wstring &s, const wstring &type,
    shared_ptr<RandomIndexWriter> writer) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"body", s, Field::Store::YES));
  doc->push_back(newStringField(L"type", type, Field::Store::NO));
  writer->addDocument(doc);
}

void TestMultiPhraseQuery::testEmptyToString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  (make_shared<MultiPhraseQuery::Builder>())->build()->toString();
}

void TestMultiPhraseQuery::testZeroPosIncr() 
{
  shared_ptr<Directory> dir = make_shared<RAMDirectory>();
  std::deque<std::shared_ptr<Token>> tokens(3);
  tokens[0] = make_shared<Token>();
  tokens[0]->append(L"a");
  tokens[0]->setPositionIncrement(1);
  tokens[1] = make_shared<Token>();
  tokens[1]->append(L"b");
  tokens[1]->setPositionIncrement(0);
  tokens[2] = make_shared<Token>();
  tokens[2]->append(L"c");
  tokens[2]->setPositionIncrement(0);

  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<TextField>(L"field", make_shared<CannedTokenStream>(tokens)));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(
      make_shared<TextField>(L"field", make_shared<CannedTokenStream>(tokens)));
  writer->addDocument(doc);
  shared_ptr<IndexReader> r = writer->getReader();
  delete writer;
  shared_ptr<IndexSearcher> s = newSearcher(r);
  shared_ptr<MultiPhraseQuery::Builder> mpqb =
      make_shared<MultiPhraseQuery::Builder>();
  // mpq.setSlop(1);

  // NOTE: not great that if we do the else clause here we
  // get different scores!  MultiPhraseQuery counts that
  // phrase as occurring twice per doc (it should be 1, I
  // think?).  This is because MultipleTermPositions is able to
  // return the same position more than once (0, in this
  // case):
  if (true) {
    mpqb->add(
        std::deque<std::shared_ptr<Term>>{make_shared<Term>(L"field", L"b"),
                                           make_shared<Term>(L"field", L"c")},
        0);
    mpqb->add(
        std::deque<std::shared_ptr<Term>>{make_shared<Term>(L"field", L"a")},
        0);
  } else {
    mpqb->add(
        std::deque<std::shared_ptr<Term>>{make_shared<Term>(L"field", L"a")},
        0);
    mpqb->add(
        std::deque<std::shared_ptr<Term>>{make_shared<Term>(L"field", L"b"),
                                           make_shared<Term>(L"field", L"c")},
        0);
  }
  shared_ptr<TopDocs> hits = s->search(mpqb->build(), 2);
  assertEquals(2, hits->totalHits);
  assertEquals(hits->scoreDocs[0]->score, hits->scoreDocs[1]->score, 1e-5);
  /*
  for(int hit=0;hit<hits.totalHits;hit++) {
    ScoreDoc sd = hits.scoreDocs[hit];
    System.out.println("  hit doc=" + sd.doc + " score=" + sd.score);
  }
  */
  delete r;
  delete dir;
}

shared_ptr<Token> TestMultiPhraseQuery::makeToken(const wstring &text,
                                                  int posIncr)
{
  shared_ptr<Token> *const t = make_shared<Token>();
  t->append(text);
  t->setPositionIncrement(posIncr);
  return t;
}

std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    TestMultiPhraseQuery::INCR_0_DOC_TOKENS =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            makeToken(L"x", 1), makeToken(L"a", 1), makeToken(L"1", 0),
            makeToken(L"m", 1), makeToken(L"b", 1), makeToken(L"1", 0),
            makeToken(L"n", 1), makeToken(L"c", 1), makeToken(L"y", 1)};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    TestMultiPhraseQuery::INCR_0_QUERY_TOKENS_AND =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            makeToken(L"a", 1), makeToken(L"1", 0), makeToken(L"b", 1),
            makeToken(L"1", 0), makeToken(L"c", 1)};
std::deque<
    std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>> const
    TestMultiPhraseQuery::INCR_0_QUERY_TOKENS_AND_OR_MATCH = std::deque<
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>>{
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            makeToken(L"a", 1)},
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            makeToken(L"x", 1), makeToken(L"1", 0)},
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            makeToken(L"b", 2)},
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            makeToken(L"x", 2), makeToken(L"1", 0)},
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            makeToken(L"c", 3)}};
std::deque<
    std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>> const
    TestMultiPhraseQuery::INCR_0_QUERY_TOKENS_AND_OR_NO_MATCHN = std::deque<
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>>{
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            makeToken(L"x", 1)},
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            makeToken(L"a", 1), makeToken(L"1", 0)},
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            makeToken(L"x", 2)},
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            makeToken(L"b", 2), makeToken(L"1", 0)},
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            makeToken(L"c", 3)}};

void TestMultiPhraseQuery::testZeroPosIncrSloppyParsedAnd() 
{
  shared_ptr<MultiPhraseQuery::Builder> qb =
      make_shared<MultiPhraseQuery::Builder>();
  qb->add(std::deque<std::shared_ptr<Term>>{make_shared<Term>(L"field", L"a"),
                                             make_shared<Term>(L"field", L"1")},
          -1);
  qb->add(std::deque<std::shared_ptr<Term>>{make_shared<Term>(L"field", L"b"),
                                             make_shared<Term>(L"field", L"1")},
          0);
  qb->add(std::deque<std::shared_ptr<Term>>{make_shared<Term>(L"field", L"c")},
          1);
  doTestZeroPosIncrSloppy(qb->build(), 0);
  qb->setSlop(1);
  doTestZeroPosIncrSloppy(qb->build(), 0);
  qb->setSlop(2);
  doTestZeroPosIncrSloppy(qb->build(), 1);
}

void TestMultiPhraseQuery::doTestZeroPosIncrSloppy(
    shared_ptr<Query> q, int nExpected) 
{
  shared_ptr<Directory> dir = newDirectory(); // random dir
  shared_ptr<IndexWriterConfig> cfg = newIndexWriterConfig(nullptr);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, cfg);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(
      L"field", make_shared<CannedTokenStream>(INCR_0_DOC_TOKENS)));
  writer->addDocument(doc);
  shared_ptr<IndexReader> r = DirectoryReader::open(writer);
  delete writer;
  shared_ptr<IndexSearcher> s = newSearcher(r);

  if (VERBOSE) {
    wcout << L"QUERY=" << q << endl;
  }

  shared_ptr<TopDocs> hits = s->search(q, 1);
  assertEquals(L"wrong number of results", nExpected, hits->totalHits);

  if (VERBOSE) {
    for (int hit = 0; hit < hits->totalHits; hit++) {
      shared_ptr<ScoreDoc> sd = hits->scoreDocs[hit];
      wcout << L"  hit doc=" << sd->doc << L" score=" << sd->score << endl;
    }
  }

  delete r;
  delete dir;
}

void TestMultiPhraseQuery::testZeroPosIncrSloppyPqAnd() 
{
  shared_ptr<PhraseQuery::Builder> builder =
      make_shared<PhraseQuery::Builder>();
  int pos = -1;
  for (auto tap : INCR_0_QUERY_TOKENS_AND) {
    pos += tap->getPositionIncrement();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    builder->add(make_shared<Term>(L"field", tap->toString()), pos);
  }
  builder->setSlop(0);
  doTestZeroPosIncrSloppy(builder->build(), 0);
  builder->setSlop(1);
  doTestZeroPosIncrSloppy(builder->build(), 0);
  builder->setSlop(2);
  doTestZeroPosIncrSloppy(builder->build(), 1);
}

void TestMultiPhraseQuery::testZeroPosIncrSloppyMpqAnd() 
{
  shared_ptr<MultiPhraseQuery::Builder> *const mpqb =
      make_shared<MultiPhraseQuery::Builder>();
  int pos = -1;
  for (auto tap : INCR_0_QUERY_TOKENS_AND) {
    pos += tap->getPositionIncrement();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    mpqb->add(std::deque<std::shared_ptr<Term>>{make_shared<Term>(
                  L"field", tap->toString())},
              pos); // AND logic
  }
  doTestZeroPosIncrSloppy(mpqb->build(), 0);
  mpqb->setSlop(1);
  doTestZeroPosIncrSloppy(mpqb->build(), 0);
  mpqb->setSlop(2);
  doTestZeroPosIncrSloppy(mpqb->build(), 1);
}

void TestMultiPhraseQuery::testZeroPosIncrSloppyMpqAndOrMatch() throw(
    IOException)
{
  shared_ptr<MultiPhraseQuery::Builder> *const mpqb =
      make_shared<MultiPhraseQuery::Builder>();
  for (auto tap : INCR_0_QUERY_TOKENS_AND_OR_MATCH) {
    std::deque<std::shared_ptr<Term>> terms = tapTerms(tap);
    constexpr int pos = tap[0]->getPositionIncrement() - 1;
    mpqb->add(terms, pos); // AND logic in pos, OR across lines
  }
  doTestZeroPosIncrSloppy(mpqb->build(), 0);
  mpqb->setSlop(1);
  doTestZeroPosIncrSloppy(mpqb->build(), 0);
  mpqb->setSlop(2);
  doTestZeroPosIncrSloppy(mpqb->build(), 1);
}

void TestMultiPhraseQuery::testZeroPosIncrSloppyMpqAndOrNoMatch() throw(
    IOException)
{
  shared_ptr<MultiPhraseQuery::Builder> *const mpqb =
      make_shared<MultiPhraseQuery::Builder>();
  for (auto tap : INCR_0_QUERY_TOKENS_AND_OR_NO_MATCHN) {
    std::deque<std::shared_ptr<Term>> terms = tapTerms(tap);
    constexpr int pos = tap[0]->getPositionIncrement() - 1;
    mpqb->add(terms, pos); // AND logic in pos, OR across lines
  }
  doTestZeroPosIncrSloppy(mpqb->build(), 0);
  mpqb->setSlop(2);
  doTestZeroPosIncrSloppy(mpqb->build(), 0);
}

std::deque<std::shared_ptr<Term>>
TestMultiPhraseQuery::tapTerms(std::deque<std::shared_ptr<Token>> &tap)
{
  std::deque<std::shared_ptr<Term>> terms(tap.size());
  for (int i = 0; i < terms.size(); i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    terms[i] = make_shared<Term>(L"field", tap[i]->toString());
  }
  return terms;
}

void TestMultiPhraseQuery::testNegativeSlop() 
{
  shared_ptr<MultiPhraseQuery::Builder> queryBuilder =
      make_shared<MultiPhraseQuery::Builder>();
  queryBuilder->add(make_shared<Term>(L"field", L"two"));
  queryBuilder->add(make_shared<Term>(L"field", L"one"));
  expectThrows(invalid_argument::typeid, [&]() { queryBuilder->setSlop(-2); });
}
} // namespace org::apache::lucene::search