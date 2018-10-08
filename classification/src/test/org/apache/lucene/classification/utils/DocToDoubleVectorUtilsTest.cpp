using namespace std;

#include "DocToDoubleVectorUtilsTest.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/Field.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/FieldType.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/TextField.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/MultiFields.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Terms.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/MatchAllDocsQuery.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/index/RandomIndexWriter.h"
#include "../../../../../../java/org/apache/lucene/classification/utils/DocToDoubleVectorUtils.h"

namespace org::apache::lucene::classification::utils
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using IndexReader = org::apache::lucene::index::IndexReader;
using MultiFields = org::apache::lucene::index::MultiFields;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Terms = org::apache::lucene::index::Terms;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::After;
using org::junit::Before;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @Before public void setUp() throws Exception
void DocToDoubleVectorUtilsTest::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();
  shared_ptr<RandomIndexWriter> indexWriter =
      make_shared<RandomIndexWriter>(random(), dir);

  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_STORED);
  ft->setStoreTermVectors(true);
  ft->setStoreTermVectorOffsets(true);
  ft->setStoreTermVectorPositions(true);

  shared_ptr<Document> doc;
  for (int i = 0; i < 10; i++) {
    doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<Field>(L"id", Integer::toString(i), ft));
    doc->push_back(make_shared<Field>(L"text",
                                      random()->nextInt(10) + L" " +
                                          random()->nextInt(10) + L" " +
                                          random()->nextInt(10),
                                      ft));
    indexWriter->addDocument(doc);
  }

  indexWriter->commit();

  index = indexWriter->getReader();

  delete indexWriter;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @After public void tearDown() throws Exception
void DocToDoubleVectorUtilsTest::tearDown() 
{
  delete index;
  delete dir;
  LuceneTestCase::tearDown();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDenseFreqDoubleArrayConversion() throws
// Exception
void DocToDoubleVectorUtilsTest::testDenseFreqDoubleArrayConversion() throw(
    runtime_error)
{
  shared_ptr<IndexSearcher> indexSearcher = make_shared<IndexSearcher>(index);
  for (auto scoreDoc : indexSearcher
                           ->search(make_shared<MatchAllDocsQuery>(),
                                    numeric_limits<int>::max())
                           ->scoreDocs) {
    shared_ptr<Terms> docTerms = index->getTermVector(scoreDoc->doc, L"text");
    std::deque<optional<double>> deque =
        DocToDoubleVectorUtils::toDenseLocalFreqDoubleArray(docTerms);
    assertNotNull(deque);
    assertTrue(deque.size() > 0);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSparseFreqDoubleArrayConversion() throws
// Exception
void DocToDoubleVectorUtilsTest::testSparseFreqDoubleArrayConversion() throw(
    runtime_error)
{
  shared_ptr<Terms> fieldTerms = MultiFields::getTerms(index, L"text");
  if (fieldTerms != nullptr && fieldTerms->size() != -1) {
    shared_ptr<IndexSearcher> indexSearcher = make_shared<IndexSearcher>(index);
    for (auto scoreDoc : indexSearcher
                             ->search(make_shared<MatchAllDocsQuery>(),
                                      numeric_limits<int>::max())
                             ->scoreDocs) {
      shared_ptr<Terms> docTerms = index->getTermVector(scoreDoc->doc, L"text");
      std::deque<optional<double>> deque =
          DocToDoubleVectorUtils::toSparseLocalFreqDoubleArray(docTerms,
                                                               fieldTerms);
      assertNotNull(deque);
      assertTrue(deque.size() > 0);
    }
  }
}
} // namespace org::apache::lucene::classification::utils