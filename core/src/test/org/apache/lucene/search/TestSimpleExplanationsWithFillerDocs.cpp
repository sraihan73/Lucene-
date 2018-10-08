using namespace std;

#include "TestSimpleExplanationsWithFillerDocs.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using Term = org::apache::lucene::index::Term;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::apache::lucene::util::LuceneTestCase::Slow;
using org::junit::Assume;
using org::junit::BeforeClass;
int TestSimpleExplanationsWithFillerDocs::PRE_FILLER_DOCS = 0;
wstring TestSimpleExplanationsWithFillerDocs::EXTRA = nullptr;
const shared_ptr<org::apache::lucene::document::Document>
    TestSimpleExplanationsWithFillerDocs::EMPTY_DOC =
        make_shared<org::apache::lucene::document::Document>();

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void replaceIndex() throws
// Exception
void TestSimpleExplanationsWithFillerDocs::replaceIndex() 
{
  EXTRA = random()->nextBoolean() ? L"" : L"extra";
  PRE_FILLER_DOCS = TestUtil::nextInt(random(), 0, (NUM_FILLER_DOCS / 2));

  // free up what our super class created that we won't be using
  delete reader;
  delete directory;

  directory = newDirectory();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.RandomIndexWriter
  // writer = new org.apache.lucene.index.RandomIndexWriter(random(), directory,
  // newIndexWriterConfig(analyzer).setMergePolicy(newLogMergePolicy())))
  {
    org::apache::lucene::index::RandomIndexWriter writer =
        org::apache::lucene::index::RandomIndexWriter(
            random(), directory,
            newIndexWriterConfig(analyzer)->setMergePolicy(
                newLogMergePolicy()));

    for (int filler = 0; filler < PRE_FILLER_DOCS; filler++) {
      writer->addDocument(makeFillerDoc());
    }
    for (int i = 0; i < docFields.size(); i++) {
      writer->addDocument(createDoc(i));

      for (int filler = 0; filler < NUM_FILLER_DOCS; filler++) {
        writer->addDocument(makeFillerDoc());
      }
    }
    reader = writer->getReader();
    searcher = newSearcher(reader);
  }
}

shared_ptr<Document> TestSimpleExplanationsWithFillerDocs::makeFillerDoc()
{
  if (L"" == EXTRA) {
    return EMPTY_DOC;
  }
  shared_ptr<Document> doc =
      createDoc(TestUtil::nextInt(random(), 0, docFields.size() - 1));
  doc->push_back(newStringField(EXTRA, EXTRA, Field::Store::NO));
  return doc;
}

void TestSimpleExplanationsWithFillerDocs::qtest(
    shared_ptr<Query> q, std::deque<int> &expDocNrs) 
{

  expDocNrs = Arrays::copyOf(expDocNrs, expDocNrs.size());
  for (int i = 0; i < expDocNrs.size(); i++) {
    expDocNrs[i] = PRE_FILLER_DOCS + ((NUM_FILLER_DOCS + 1) * expDocNrs[i]);
  }

  if (L"" != EXTRA) {
    shared_ptr<BooleanQuery::Builder> builder =
        make_shared<BooleanQuery::Builder>();
    builder->add(make_shared<BooleanClause>(q, BooleanClause::Occur::MUST));
    builder->add(make_shared<BooleanClause>(
        make_shared<TermQuery>(make_shared<Term>(EXTRA, EXTRA)),
        BooleanClause::Occur::MUST_NOT));
    q = builder->build();
  }
  TestSimpleExplanations::qtest(q, expDocNrs);
}

void TestSimpleExplanationsWithFillerDocs::testMA1() 
{
  Assume::assumeNotNull(L"test is not viable with empty filler docs", EXTRA);
  TestSimpleExplanations::testMA1();
}

void TestSimpleExplanationsWithFillerDocs::testMA2() 
{
  Assume::assumeNotNull(L"test is not viable with empty filler docs", EXTRA);
  TestSimpleExplanations::testMA2();
}
} // namespace org::apache::lucene::search