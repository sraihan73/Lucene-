using namespace std;

#include "TestDocValuesFieldSources.h"

namespace org::apache::lucene::queries::function
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using BinaryDocValuesField =
    org::apache::lucene::document::BinaryDocValuesField;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using SortedNumericDocValuesField =
    org::apache::lucene::document::SortedNumericDocValuesField;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using BytesRefFieldSource =
    org::apache::lucene::queries::function::valuesource::BytesRefFieldSource;
using LongFieldSource =
    org::apache::lucene::queries::function::valuesource::LongFieldSource;
using MultiValuedLongFieldSource = org::apache::lucene::queries::function::
    valuesource::MultiValuedLongFieldSource;
using Type = org::apache::lucene::search::SortedNumericSelector::Type;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
using com::carrotsearch::randomizedtesting::generators::RandomNumbers;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("fallthrough") public void
// test(org.apache.lucene.index.DocValuesType type) throws java.io.IOException
void TestDocValuesFieldSources::test(DocValuesType type) 
{
  shared_ptr<Directory> d = newDirectory();
  shared_ptr<IndexWriterConfig> iwConfig =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  constexpr int nDocs = atLeast(50);
  shared_ptr<Field> *const id = make_shared<NumericDocValuesField>(L"id", 0);
  shared_ptr<Field> *const f;
  switch (type) {
  case DocValuesType::BINARY:
    f = make_shared<BinaryDocValuesField>(L"dv", make_shared<BytesRef>());
    break;
  case DocValuesType::SORTED:
    f = make_shared<SortedDocValuesField>(L"dv", make_shared<BytesRef>());
    break;
  case DocValuesType::NUMERIC:
    f = make_shared<NumericDocValuesField>(L"dv", 0);
    break;
  case DocValuesType::SORTED_NUMERIC:
    f = make_shared<SortedNumericDocValuesField>(L"dv", 0);
    break;
  default:
    throw make_shared<AssertionError>();
  }
  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(id);
  document->push_back(f);

  const std::deque<any> vals = std::deque<any>(nDocs);

  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), d, iwConfig);
  for (int i = 0; i < nDocs; ++i) {
    id->setLongValue(i);
    switch (type) {
    case DocValuesType::SORTED:
    case DocValuesType::BINARY:
      do {
        vals[i] = TestUtil::randomSimpleString(random(), 20);
      } while ((any_cast<wstring>(vals[i]))->isEmpty());
      f->setBytesValue(make_shared<BytesRef>(any_cast<wstring>(vals[i])));
      break;
    case DocValuesType::NUMERIC:
    case DocValuesType::SORTED_NUMERIC: {
      constexpr int bitsPerValue =
          RandomNumbers::randomIntBetween(random(), 1, 31); // keep it an int
      vals[i] = static_cast<int64_t>(random()->nextInt(
          static_cast<int>(PackedInts::maxValue(bitsPerValue))));
      f->setLongValue(any_cast<optional<int64_t>>(vals[i]));
      break;
    }
    default:
      throw make_shared<AssertionError>();
    }
    iw->addDocument(document);
    if (random()->nextBoolean() && i % 10 == 9) {
      iw->commit();
    }
  }
  delete iw;

  shared_ptr<DirectoryReader> rd = DirectoryReader::open(d);
  for (shared_ptr<LeafReaderContext> leave : rd->leaves()) {
    shared_ptr<FunctionValues> *const ids =
        (make_shared<LongFieldSource>(L"id"))->getValues(nullptr, leave);
    shared_ptr<ValueSource> *const vs;
    switch (type) {
    case DocValuesType::BINARY:
    case DocValuesType::SORTED:
      vs = make_shared<BytesRefFieldSource>(L"dv");
      break;
    case DocValuesType::NUMERIC:
      vs = make_shared<LongFieldSource>(L"dv");
      break;
    case DocValuesType::SORTED_NUMERIC:
      // Since we are not indexing multiple values, MIN and MAX should work the
      // same way
      vs = random()->nextBoolean()
               ? make_shared<MultiValuedLongFieldSource>(L"dv", Type::MIN)
               : make_shared<MultiValuedLongFieldSource>(L"dv", Type::MAX);
      break;
    default:
      throw make_shared<AssertionError>();
    }
    shared_ptr<FunctionValues> *const values = vs->getValues(nullptr, leave);
    shared_ptr<BytesRefBuilder> bytes = make_shared<BytesRefBuilder>();
    for (int i = 0; i < leave->reader()->maxDoc(); ++i) {
      assertTrue(values->exists(i));
      if (std::dynamic_pointer_cast<BytesRefFieldSource>(vs) != nullptr) {
        assertTrue(dynamic_cast<wstring>(values->objectVal(i)) != nullptr);
      } else if (std::dynamic_pointer_cast<LongFieldSource>(vs) != nullptr) {
        assertTrue(dynamic_cast<optional<int64_t>>(values->objectVal(i)) !=
                   nullptr);
        assertTrue(values->bytesVal(i, bytes));
      } else {
        throw make_shared<AssertionError>();
      }

      any expected = vals[ids->intVal(i)];
      switch (type) {
      case DocValuesType::SORTED:
        values->ordVal(i); // no exception
        assertTrue(values->numOrd() >= 1);
        // fall-through
      case DocValuesType::BINARY:
        TestUtil::assertEquals(expected, values->objectVal(i));
        TestUtil::assertEquals(expected, values->strVal(i));
        TestUtil::assertEquals(expected, values->objectVal(i));
        TestUtil::assertEquals(expected, values->strVal(i));
        assertTrue(values->bytesVal(i, bytes));
        TestUtil::assertEquals(
            make_shared<BytesRef>(any_cast<wstring>(expected)), bytes->get());
        break;
      case DocValuesType::NUMERIC:
      case DocValuesType::SORTED_NUMERIC:
        TestUtil::assertEquals(
            (any_cast<std::shared_ptr<Number>>(expected)).longValue(),
            values->longVal(i));
        break;
      default:
        throw make_shared<AssertionError>();
      }
    }
  }
  rd->close();
  delete d;
}

void TestDocValuesFieldSources::test() 
{
  for (DocValuesType type : DocValuesType::values()) {
    if (type != DocValuesType::SORTED_SET && type != DocValuesType::NONE) {
      test(type);
    }
  }
}
} // namespace org::apache::lucene::queries::function