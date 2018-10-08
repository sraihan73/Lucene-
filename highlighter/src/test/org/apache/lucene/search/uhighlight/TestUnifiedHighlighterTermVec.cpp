using namespace std;

#include "TestUnifiedHighlighterTermVec.h"

namespace org::apache::lucene::search::uhighlight
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using namespace org::apache::lucene::index;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Sort = org::apache::lucene::search::Sort;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::After;
using org::junit::Before;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before public void doBefore() throws java.io.IOException
void TestUnifiedHighlighterTermVec::doBefore() 
{
  indexAnalyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE,
                                true); // whitespace, punctuation, lowercase
  dir = newDirectory();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @After public void doAfter() throws java.io.IOException
void TestUnifiedHighlighterTermVec::doAfter()  { delete dir; }

void TestUnifiedHighlighterTermVec::testFetchTermVecsOncePerDoc() throw(
    IOException)
{
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);

  // Declare some number of fields with random field type; but at least one will
  // have term vectors.
  constexpr int numTvFields = 1 + random()->nextInt(3);
  deque<wstring> fields = deque<wstring>(numTvFields);
  deque<std::shared_ptr<FieldType>> fieldTypes =
      deque<std::shared_ptr<FieldType>>(numTvFields);
  for (int i = 0; i < numTvFields; i++) {
    fields.push_back(L"body" + to_wstring(i));
    fieldTypes.push_back(UHTestHelper::randomFieldType(random()));
  }
  // ensure at least one has TVs by setting one randomly to it:
  fieldTypes[random()->nextInt(fieldTypes.size())] = UHTestHelper::tvType;

  constexpr int numDocs = 1 + random()->nextInt(3);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    for (auto field : fields) {
      doc->push_back(
          make_shared<Field>(field, L"some test text", UHTestHelper::tvType));
    }
    iw->addDocument(doc);
  }

  // Wrap the reader to ensure we only fetch TVs once per doc
  shared_ptr<DirectoryReader> originalReader = iw->getReader();
  shared_ptr<IndexReader> ir =
      make_shared<AssertOnceTermVecDirectoryReader>(originalReader);
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      make_shared<UnifiedHighlighter>(searcher, indexAnalyzer);
  shared_ptr<BooleanQuery::Builder> queryBuilder =
      make_shared<BooleanQuery::Builder>();
  for (auto field : fields) {
    queryBuilder->add(make_shared<TermQuery>(make_shared<Term>(field, L"test")),
                      BooleanClause::Occur::MUST);
  }
  shared_ptr<BooleanQuery> query = queryBuilder->build();
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  assertEquals(numDocs, topDocs->totalHits);
  unordered_map<wstring, std::deque<wstring>> fieldToSnippets =
      highlighter->highlightFields(
          fields.toArray(std::deque<wstring>(numTvFields)), query, topDocs);
  std::deque<wstring> expectedSnippetsByDoc(numDocs);
  Arrays::fill(expectedSnippetsByDoc, L"some <b>test</b> text");
  for (auto field : fields) {
    assertArrayEquals(expectedSnippetsByDoc, fieldToSnippets[field]);
  }

  delete ir;
}

const shared_ptr<SubReaderWrapper> TestUnifiedHighlighterTermVec::
    AssertOnceTermVecDirectoryReader::SUB_READER_WRAPPER =
        make_shared<SubReaderWrapperAnonymousInnerClass>();

TestUnifiedHighlighterTermVec::AssertOnceTermVecDirectoryReader::
    SubReaderWrapperAnonymousInnerClass::SubReaderWrapperAnonymousInnerClass()
{
}

shared_ptr<LeafReader>
TestUnifiedHighlighterTermVec::AssertOnceTermVecDirectoryReader::
    SubReaderWrapperAnonymousInnerClass::wrap(shared_ptr<LeafReader> reader)
{
  return make_shared<FilterLeafReaderAnonymousInnerClass>(shared_from_this(),
                                                          reader);
}

TestUnifiedHighlighterTermVec::AssertOnceTermVecDirectoryReader::
    SubReaderWrapperAnonymousInnerClass::FilterLeafReaderAnonymousInnerClass::
        FilterLeafReaderAnonymousInnerClass(
            shared_ptr<SubReaderWrapperAnonymousInnerClass> outerInstance,
            shared_ptr<org::apache::lucene::index::LeafReader> reader)
    : FilterLeafReader(reader)
{
  this->outerInstance = outerInstance;
  seenDocIDs = make_shared<BitSet>();
}

shared_ptr<Fields>
TestUnifiedHighlighterTermVec::AssertOnceTermVecDirectoryReader::
    SubReaderWrapperAnonymousInnerClass::FilterLeafReaderAnonymousInnerClass::
        getTermVectors(int docID) 
{
  // if we're invoked by ParallelLeafReader then we can't do our assertion. TODO
  // see LUCENE-6868
  if (calledBy(ParallelLeafReader::typeid) == false &&
      calledBy(CheckIndex::typeid) == false) {
    assertFalse(L"Should not request TVs for doc more than once.",
                seenDocIDs->get(docID));
    seenDocIDs::set(docID);
  }

  return outerInstance->outerInstance.super.getTermVectors(docID);
}

shared_ptr<CacheHelper> TestUnifiedHighlighterTermVec::
    AssertOnceTermVecDirectoryReader::SubReaderWrapperAnonymousInnerClass::
        FilterLeafReaderAnonymousInnerClass::getCoreCacheHelper()
{
  return nullptr;
}

shared_ptr<CacheHelper> TestUnifiedHighlighterTermVec::
    AssertOnceTermVecDirectoryReader::SubReaderWrapperAnonymousInnerClass::
        FilterLeafReaderAnonymousInnerClass::getReaderCacheHelper()
{
  return nullptr;
}

TestUnifiedHighlighterTermVec::AssertOnceTermVecDirectoryReader::
    AssertOnceTermVecDirectoryReader(shared_ptr<DirectoryReader> in_) throw(
        IOException)
    : FilterDirectoryReader(in_, SUB_READER_WRAPPER)
{
}

shared_ptr<DirectoryReader>
TestUnifiedHighlighterTermVec::AssertOnceTermVecDirectoryReader::
    doWrapDirectoryReader(shared_ptr<DirectoryReader> in_) 
{
  return make_shared<AssertOnceTermVecDirectoryReader>(in_);
}

shared_ptr<CacheHelper> TestUnifiedHighlighterTermVec::
    AssertOnceTermVecDirectoryReader::getReaderCacheHelper()
{
  return nullptr;
}

bool TestUnifiedHighlighterTermVec::calledBy(type_info clazz)
{
  for (shared_ptr<StackTraceElement> stackTraceElement :
       Thread::currentThread().getStackTrace()) {
    if (stackTraceElement->getClassName().equals(clazz.getName())) {
      return true;
    }
  }
  return false;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test(expected = IllegalArgumentException.class) public void
// testUserFailedToIndexOffsets() throws java.io.IOException
void TestUnifiedHighlighterTermVec::testUserFailedToIndexOffsets() throw(
    IOException)
{
  shared_ptr<FieldType> fieldType =
      make_shared<FieldType>(UHTestHelper::tvType); // note: it's indexed too
  fieldType->setStoreTermVectorPositions(random()->nextBoolean());
  fieldType->setStoreTermVectorOffsets(false);

  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, indexAnalyzer);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(L"body", L"term vectors", fieldType));
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<UnifiedHighlighter> highlighter =
      make_shared<UnifiedHighlighter>(searcher, indexAnalyzer);
  shared_ptr<TermQuery> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"vectors"));
  shared_ptr<TopDocs> topDocs = searcher->search(query, 10, Sort::INDEXORDER);
  try {
    highlighter->highlight(L"body", query, topDocs, 1); // should throw
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete ir;
  }
}
} // namespace org::apache::lucene::search::uhighlight