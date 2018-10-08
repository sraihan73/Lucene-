using namespace std;

#include "TestNGramPhraseQuery.h"

namespace org::apache::lucene::search
{
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<org::apache::lucene::index::IndexReader>
    TestNGramPhraseQuery::reader;
shared_ptr<org::apache::lucene::store::Directory>
    TestNGramPhraseQuery::directory;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestNGramPhraseQuery::beforeClass() 
{
  directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), directory);
  delete writer;
  reader = DirectoryReader::open(directory);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void TestNGramPhraseQuery::afterClass() 
{
  delete reader;
  reader.reset();
  delete directory;
  directory.reset();
}

void TestNGramPhraseQuery::testRewrite() 
{
  // bi-gram test ABC => AB/BC => AB/BC
  shared_ptr<NGramPhraseQuery> pq1 = make_shared<NGramPhraseQuery>(
      2, make_shared<PhraseQuery>(L"f", L"AB", L"BC"));

  shared_ptr<Query> q = pq1->rewrite(reader);
  assertSame(q->rewrite(reader), q);
  shared_ptr<PhraseQuery> rewritten1 = std::static_pointer_cast<PhraseQuery>(q);
  assertArrayEquals(
      std::deque<std::shared_ptr<Term>>{make_shared<Term>(L"f", L"AB"),
                                         make_shared<Term>(L"f", L"BC")},
      rewritten1->getTerms());
  assertArrayEquals(std::deque<int>{0, 1}, rewritten1->getPositions());

  // bi-gram test ABCD => AB/BC/CD => AB//CD
  shared_ptr<NGramPhraseQuery> pq2 = make_shared<NGramPhraseQuery>(
      2, make_shared<PhraseQuery>(L"f", L"AB", L"BC", L"CD"));

  q = pq2->rewrite(reader);
  assertTrue(std::dynamic_pointer_cast<PhraseQuery>(q) != nullptr);
  assertNotSame(pq2, q);
  shared_ptr<PhraseQuery> rewritten2 = std::static_pointer_cast<PhraseQuery>(q);
  assertArrayEquals(
      std::deque<std::shared_ptr<Term>>{make_shared<Term>(L"f", L"AB"),
                                         make_shared<Term>(L"f", L"CD")},
      rewritten2->getTerms());
  assertArrayEquals(std::deque<int>{0, 2}, rewritten2->getPositions());

  // tri-gram test ABCDEFGH => ABC/BCD/CDE/DEF/EFG/FGH => ABC///DEF//FGH
  shared_ptr<NGramPhraseQuery> pq3 = make_shared<NGramPhraseQuery>(
      3, make_shared<PhraseQuery>(L"f", L"ABC", L"BCD", L"CDE", L"DEF", L"EFG",
                                  L"FGH"));

  q = pq3->rewrite(reader);
  assertTrue(std::dynamic_pointer_cast<PhraseQuery>(q) != nullptr);
  assertNotSame(pq3, q);
  shared_ptr<PhraseQuery> rewritten3 = std::static_pointer_cast<PhraseQuery>(q);
  assertArrayEquals(
      std::deque<std::shared_ptr<Term>>{make_shared<Term>(L"f", L"ABC"),
                                         make_shared<Term>(L"f", L"DEF"),
                                         make_shared<Term>(L"f", L"FGH")},
      rewritten3->getTerms());
  assertArrayEquals(std::deque<int>{0, 3, 5}, rewritten3->getPositions());
}
} // namespace org::apache::lucene::search