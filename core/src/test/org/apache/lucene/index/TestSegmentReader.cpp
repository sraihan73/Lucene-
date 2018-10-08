using namespace std;

#include "TestSegmentReader.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using Version = org::apache::lucene::util::Version;

void TestSegmentReader::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();
  DocHelper::setupDoc(testDoc);
  shared_ptr<SegmentCommitInfo> info =
      DocHelper::writeDoc(random(), dir, testDoc);
  reader =
      make_shared<SegmentReader>(info, Version::LATEST->major, IOContext::READ);
}

void TestSegmentReader::tearDown() 
{
  delete reader;
  delete dir;
  LuceneTestCase::tearDown();
}

void TestSegmentReader::test()
{
  assertTrue(dir != nullptr);
  assertTrue(reader != nullptr);
  assertTrue(DocHelper::nameValues.size() > 0);
  assertTrue(DocHelper::numFields(testDoc) == DocHelper::all.size());
}

void TestSegmentReader::testDocument() 
{
  assertTrue(reader->numDocs() == 1);
  assertTrue(reader->maxDoc() >= 1);
  shared_ptr<Document> result = reader->document(0);
  assertTrue(result->size() > 0);
  // There are 2 unstored fields on the document that are not preserved across
  // writing
  assertTrue(DocHelper::numFields(result) ==
             DocHelper::numFields(testDoc) - DocHelper::unstored.size());

  deque<std::shared_ptr<IndexableField>> fields = result->getFields();
  for (auto field : fields) {
    assertTrue(field != nullptr);
    assertTrue(DocHelper::nameValues.find(field->name()) !=
               DocHelper::nameValues.end());
  }
}

void TestSegmentReader::testGetFieldNameVariations()
{
  shared_ptr<deque<wstring>> allFieldNames = unordered_set<wstring>();
  shared_ptr<deque<wstring>> indexedFieldNames = unordered_set<wstring>();
  shared_ptr<deque<wstring>> notIndexedFieldNames =
      unordered_set<wstring>();
  shared_ptr<deque<wstring>> tvFieldNames = unordered_set<wstring>();
  shared_ptr<deque<wstring>> noTVFieldNames = unordered_set<wstring>();

  for (auto fieldInfo : reader->getFieldInfos()) {
    const wstring name = fieldInfo->name;
    allFieldNames->add(name);
    if (fieldInfo->getIndexOptions() != IndexOptions::NONE) {
      indexedFieldNames->add(name);
    } else {
      notIndexedFieldNames->add(name);
    }
    if (fieldInfo->hasVectors()) {
      tvFieldNames->add(name);
    } else if (fieldInfo->getIndexOptions() != IndexOptions::NONE) {
      noTVFieldNames->add(name);
    }
  }

  assertTrue(allFieldNames->size() == DocHelper::all.size());
  for (auto s : allFieldNames) {
    assertTrue(DocHelper::nameValues.find(s) != DocHelper::nameValues.end() ==
                   true ||
               s.equals(L""));
  }

  assertTrue(indexedFieldNames->size() == DocHelper::indexed.size());
  for (auto s : indexedFieldNames) {
    assertTrue(DocHelper::indexed.find(s) != DocHelper::indexed.end() == true ||
               s.equals(L""));
  }

  assertTrue(notIndexedFieldNames->size() == DocHelper::unindexed.size());
  // Get all indexed fields that are storing term vectors
  assertTrue(tvFieldNames->size() == DocHelper::termvector.size());

  assertTrue(noTVFieldNames->size() == DocHelper::notermvector.size());
}

void TestSegmentReader::testTerms() 
{
  shared_ptr<Fields> fields = MultiFields::getFields(reader);
  for (auto field : fields) {
    shared_ptr<Terms> terms = fields->terms(field);
    assertNotNull(terms);
    shared_ptr<TermsEnum> termsEnum = terms->begin();
    while (termsEnum->next() != nullptr) {
      shared_ptr<BytesRef> term = termsEnum->term();
      assertTrue(term != nullptr);
      wstring fieldValue = any_cast<wstring>(DocHelper::nameValues[field]);
      assertTrue(fieldValue.find(term->utf8ToString()) != wstring::npos);
    }
  }

  shared_ptr<PostingsEnum> termDocs =
      TestUtil::docs(random(), reader, DocHelper::TEXT_FIELD_1_KEY,
                     make_shared<BytesRef>(L"field"), nullptr, 0);
  assertTrue(termDocs->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);

  termDocs = TestUtil::docs(random(), reader, DocHelper::NO_NORMS_KEY,
                            make_shared<BytesRef>(DocHelper::NO_NORMS_TEXT),
                            nullptr, 0);

  assertTrue(termDocs->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);

  shared_ptr<PostingsEnum> positions = MultiFields::getTermPositionsEnum(
      reader, DocHelper::TEXT_FIELD_1_KEY, make_shared<BytesRef>(L"field"));
  // NOTE: prior rev of this test was failing to first
  // call next here:
  assertTrue(positions->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  assertTrue(positions->docID() == 0);
  assertTrue(positions->nextPosition() >= 0);
}

void TestSegmentReader::testNorms() 
{
  // TODO: Not sure how these work/should be tested
  /*
      try {
        byte [] norms = reader.norms(DocHelper.TEXT_FIELD_1_KEY);
        System.out.println("Norms: " + norms);
        assertTrue(norms != null);
      } catch (IOException e) {
        e.printStackTrace();
        assertTrue(false);
      }
  */

  checkNorms(reader);
}

void TestSegmentReader::checkNorms(shared_ptr<LeafReader> reader) throw(
    IOException)
{
  // test omit norms
  for (int i = 0; i < DocHelper::fields.size(); i++) {
    shared_ptr<IndexableField> f = DocHelper::fields[i];
    if (f->fieldType()->indexOptions() != IndexOptions::NONE) {
      TestUtil::assertEquals(reader->getNormValues(f->name()) != nullptr,
                             !f->fieldType()->omitNorms());
      TestUtil::assertEquals(reader->getNormValues(f->name()) != nullptr,
                             DocHelper::noNorms.find(f->name()) ==
                                 DocHelper::noNorms.end());
      if (reader->getNormValues(f->name()) == nullptr) {
        // test for norms of null
        shared_ptr<NumericDocValues> norms =
            MultiDocValues::getNormValues(reader, f->name());
        assertNull(norms);
      }
    }
  }
}

void TestSegmentReader::testTermVectors() 
{
  shared_ptr<Terms> result =
      reader->getTermVectors(0)->terms(DocHelper::TEXT_FIELD_2_KEY);
  assertNotNull(result);
  TestUtil::assertEquals(3, result->size());
  shared_ptr<TermsEnum> termsEnum = result->begin();
  while (termsEnum->next() != nullptr) {
    wstring term = termsEnum->term()->utf8ToString();
    int freq = static_cast<int>(termsEnum->totalTermFreq());
    assertTrue(DocHelper::FIELD_2_TEXT.find(term) != wstring::npos);
    assertTrue(freq > 0);
  }

  shared_ptr<Fields> results = reader->getTermVectors(0);
  assertTrue(results->size() > 0);
  assertEquals(L"We do not have 3 term freq vectors", 3, results->size());
}

void TestSegmentReader::testOutOfBoundsAccess() 
{
  int numDocs = reader->maxDoc();

  expectThrows(out_of_range::typeid, [&]() { reader->document(-1); });

  expectThrows(out_of_range::typeid, [&]() { reader->getTermVectors(-1); });

  expectThrows(out_of_range::typeid, [&]() { reader->document(numDocs); });

  expectThrows(out_of_range::typeid,
               [&]() { reader->getTermVectors(numDocs); });
}
} // namespace org::apache::lucene::index