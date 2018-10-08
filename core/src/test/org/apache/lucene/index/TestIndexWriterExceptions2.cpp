using namespace std;

#include "TestIndexWriterExceptions2.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CrankyTokenFilter = org::apache::lucene::analysis::CrankyTokenFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using MockVariableLengthPayloadFilter =
    org::apache::lucene::analysis::MockVariableLengthPayloadFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Codec = org::apache::lucene::codecs::Codec;
using AssertingCodec = org::apache::lucene::codecs::asserting::AssertingCodec;
using CrankyCodec = org::apache::lucene::codecs::cranky::CrankyCodec;
using BinaryDocValuesField =
    org::apache::lucene::document::BinaryDocValuesField;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using IntPoint = org::apache::lucene::document::IntPoint;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using SortedNumericDocValuesField =
    org::apache::lucene::document::SortedNumericDocValuesField;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using StoredField = org::apache::lucene::document::StoredField;
using TextField = org::apache::lucene::document::TextField;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using Rethrow = org::apache::lucene::util::Rethrow;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestIndexWriterExceptions2::testBasics() 
{
  // disable slow things: we don't rely upon sleeps here.
  shared_ptr<Directory> dir = newDirectory();
  if (std::dynamic_pointer_cast<MockDirectoryWrapper>(dir) != nullptr) {
    (std::static_pointer_cast<MockDirectoryWrapper>(dir))
        ->setThrottling(MockDirectoryWrapper::Throttling::NEVER);
    (std::static_pointer_cast<MockDirectoryWrapper>(dir))
        ->setUseSlowOpenClosers(false);
  }

  // log all exceptions we hit, in case we fail (for debugging)
  shared_ptr<ByteArrayOutputStream> exceptionLog =
      make_shared<ByteArrayOutputStream>();
  shared_ptr<PrintStream> exceptionStream =
      make_shared<PrintStream>(exceptionLog, true, L"UTF-8");
  // PrintStream exceptionStream = System.out;

  // create lots of non-aborting exceptions with a broken analyzer
  constexpr int64_t analyzerSeed = random()->nextLong();
  shared_ptr<Analyzer> analyzer = make_shared<AnalyzerAnonymousInnerClass>(
      shared_from_this(), analyzerSeed);

  // create lots of aborting exceptions with a broken codec
  // we don't need a random codec, as we aren't trying to find bugs in the codec
  // here.
  shared_ptr<Codec> inner = RANDOM_MULTIPLIER > 1
                                ? Codec::getDefault()
                                : make_shared<AssertingCodec>();
  shared_ptr<Codec> codec = make_shared<CrankyCodec>(
      inner, make_shared<Random>(random()->nextLong()));

  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  // just for now, try to keep this test reproducible
  conf->setMergeScheduler(make_shared<SerialMergeScheduler>());
  conf->setCodec(codec);

  int numDocs = atLeast(500);

  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, conf);
  try {
    bool allowAlreadyClosed = false;
    for (int i = 0; i < numDocs; i++) {
      // TODO: add crankyDocValuesFields, etc
      shared_ptr<Document> doc = make_shared<Document>();
      // C++ TODO: There is no native C++ equivalent to 'toString':
      doc->push_back(
          newStringField(L"id", Integer::toString(i), Field::Store::NO));
      doc->push_back(make_shared<NumericDocValuesField>(L"dv", i));
      // C++ TODO: There is no native C++ equivalent to 'toString':
      doc->push_back(make_shared<BinaryDocValuesField>(
          L"dv2", make_shared<BytesRef>(Integer::toString(i))));
      // C++ TODO: There is no native C++ equivalent to 'toString':
      doc->push_back(make_shared<SortedDocValuesField>(
          L"dv3", make_shared<BytesRef>(Integer::toString(i))));
      // C++ TODO: There is no native C++ equivalent to 'toString':
      doc->push_back(make_shared<SortedSetDocValuesField>(
          L"dv4", make_shared<BytesRef>(Integer::toString(i))));
      // C++ TODO: There is no native C++ equivalent to 'toString':
      doc->push_back(make_shared<SortedSetDocValuesField>(
          L"dv4", make_shared<BytesRef>(Integer::toString(i - 1))));
      doc->push_back(make_shared<SortedNumericDocValuesField>(L"dv5", i));
      doc->push_back(make_shared<SortedNumericDocValuesField>(L"dv5", i - 1));
      doc->push_back(newTextField(
          L"text1", TestUtil::randomAnalysisString(random(), 20, true),
          Field::Store::NO));
      // ensure we store something
      doc->push_back(make_shared<StoredField>(L"stored1", L"foo"));
      doc->push_back(make_shared<StoredField>(L"stored1", L"bar"));
      // ensure we get some payloads
      doc->push_back(newTextField(
          L"text_payloads", TestUtil::randomAnalysisString(random(), 6, true),
          Field::Store::NO));
      // ensure we get some vectors
      shared_ptr<FieldType> ft =
          make_shared<FieldType>(TextField::TYPE_NOT_STORED);
      ft->setStoreTermVectors(true);
      doc->push_back(newField(L"text_vectors",
                              TestUtil::randomAnalysisString(random(), 6, true),
                              ft));
      doc->push_back(make_shared<IntPoint>(L"point", random()->nextInt()));
      doc->push_back(make_shared<IntPoint>(L"point2d", random()->nextInt(),
                                           random()->nextInt()));

      if (random()->nextInt(10) > 0) {
        // single doc
        try {
          iw->addDocument(doc);
          // we made it, sometimes delete our doc, or update a dv
          int thingToDo = random()->nextInt(4);
          if (thingToDo == 0) {
            // C++ TODO: There is no native C++ equivalent to 'toString':
            iw->deleteDocuments(
                {make_shared<Term>(L"id", Integer::toString(i))});
          } else if (thingToDo == 1) {
            // C++ TODO: There is no native C++ equivalent to 'toString':
            iw->updateNumericDocValue(
                make_shared<Term>(L"id", Integer::toString(i)), L"dv", i + 1LL);
          } else if (thingToDo == 2) {
            // C++ TODO: There is no native C++ equivalent to 'toString':
            iw->updateBinaryDocValue(
                make_shared<Term>(L"id", Integer::toString(i)), L"dv2",
                make_shared<BytesRef>(Integer::toString(i + 1)));
          }
        } catch (const AlreadyClosedException &ace) {
          // OK: writer was closed by abort; we just reopen now:
          assertTrue(iw->deleter->isClosed());
          assertTrue(allowAlreadyClosed);
          allowAlreadyClosed = false;
          conf = newIndexWriterConfig(analyzer);
          // just for now, try to keep this test reproducible
          conf->setMergeScheduler(make_shared<SerialMergeScheduler>());
          conf->setCodec(codec);
          iw = make_shared<IndexWriter>(dir, conf);
        } catch (const runtime_error &e) {
          if (e.what() != nullptr &&
              e.what()->startsWith(L"Fake IOException")) {
            exceptionStream->println(L"\nTEST: got expected fake exc:" +
                                     e.what());
            e.printStackTrace(exceptionStream);
            allowAlreadyClosed = true;
          } else {
            Rethrow::rethrow(e);
          }
        }
      } else {
        // block docs
        shared_ptr<Document> doc2 = make_shared<Document>();
        // C++ TODO: There is no native C++ equivalent to 'toString':
        doc2->push_back(
            newStringField(L"id", Integer::toString(-i), Field::Store::NO));
        doc2->push_back(newTextField(
            L"text1", TestUtil::randomAnalysisString(random(), 20, true),
            Field::Store::NO));
        doc2->push_back(make_shared<StoredField>(L"stored1", L"foo"));
        doc2->push_back(make_shared<StoredField>(L"stored1", L"bar"));
        doc2->push_back(
            newField(L"text_vectors",
                     TestUtil::randomAnalysisString(random(), 6, true), ft));

        try {
          iw->addDocuments(Arrays::asList(doc, doc2));
          // we made it, sometimes delete our docs
          if (random()->nextBoolean()) {
            // C++ TODO: There is no native C++ equivalent to 'toString':
            iw->deleteDocuments(
                {make_shared<Term>(L"id", Integer::toString(i)),
                 make_shared<Term>(L"id", Integer::toString(-i))});
          }
        } catch (const AlreadyClosedException &ace) {
          // OK: writer was closed by abort; we just reopen now:
          assertTrue(iw->deleter->isClosed());
          assertTrue(allowAlreadyClosed);
          allowAlreadyClosed = false;
          conf = newIndexWriterConfig(analyzer);
          // just for now, try to keep this test reproducible
          conf->setMergeScheduler(make_shared<SerialMergeScheduler>());
          conf->setCodec(codec);
          iw = make_shared<IndexWriter>(dir, conf);
        } catch (const runtime_error &e) {
          if (e.what() != nullptr &&
              e.what()->startsWith(L"Fake IOException")) {
            exceptionStream->println(L"\nTEST: got expected fake exc:" +
                                     e.what());
            e.printStackTrace(exceptionStream);
            allowAlreadyClosed = true;
          } else {
            Rethrow::rethrow(e);
          }
        }
      }

      if (random()->nextInt(10) == 0) {
        // trigger flush:
        try {
          if (random()->nextBoolean()) {
            shared_ptr<DirectoryReader> ir = nullptr;
            try {
              ir = DirectoryReader::open(iw, random()->nextBoolean(), false);
              TestUtil::checkReader(ir);
            }
            // C++ TODO: There is no native C++ equivalent to the exception
            // 'finally' clause:
            finally {
              IOUtils::closeWhileHandlingException({ir});
            }
          } else {
            iw->commit();
          }
          if (DirectoryReader::indexExists(dir)) {
            TestUtil::checkIndex(dir);
          }
        } catch (const AlreadyClosedException &ace) {
          // OK: writer was closed by abort; we just reopen now:
          assertTrue(iw->deleter->isClosed());
          assertTrue(allowAlreadyClosed);
          allowAlreadyClosed = false;
          conf = newIndexWriterConfig(analyzer);
          // just for now, try to keep this test reproducible
          conf->setMergeScheduler(make_shared<SerialMergeScheduler>());
          conf->setCodec(codec);
          iw = make_shared<IndexWriter>(dir, conf);
        } catch (const runtime_error &e) {
          if (e.what() != nullptr &&
              e.what()->startsWith(L"Fake IOException")) {
            exceptionStream->println(L"\nTEST: got expected fake exc:" +
                                     e.what());
            e.printStackTrace(exceptionStream);
            allowAlreadyClosed = true;
          } else {
            Rethrow::rethrow(e);
          }
        }
      }
    }

    try {
      delete iw;
    } catch (const runtime_error &e) {
      if (e.what() != nullptr && e.what()->startsWith(L"Fake IOException")) {
        exceptionStream->println(L"\nTEST: got expected fake exc:" + e.what());
        e.printStackTrace(exceptionStream);
        try {
          iw->rollback();
        } catch (const runtime_error &t) {
        }
      } else {
        Rethrow::rethrow(e);
      }
    }
    delete dir;
  } catch (const runtime_error &t) {
    wcout << L"Unexpected exception: dumping fake-exception-log:..." << endl;
    exceptionStream->flush();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wcout << exceptionLog->toString(L"UTF-8") << endl;
    System::out::flush();
    Rethrow::rethrow(t);
  }

  if (VERBOSE) {
    wcout << L"TEST PASSED: dumping fake-exception-log:..." << endl;
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wcout << exceptionLog->toString(L"UTF-8") << endl;
  }
}

TestIndexWriterExceptions2::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestIndexWriterExceptions2> outerInstance,
        int64_t analyzerSeed)
{
  this->outerInstance = outerInstance;
  this->analyzerSeed = analyzerSeed;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestIndexWriterExceptions2::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<MockTokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, false);
  tokenizer->setEnableChecks(
      false); // TODO: can we turn this on? our filter is probably too evil
  shared_ptr<TokenStream> stream = tokenizer;
  // emit some payloads
  if (fieldName.find(L"payloads") != wstring::npos) {
    stream = make_shared<MockVariableLengthPayloadFilter>(
        make_shared<Random>(analyzerSeed), stream);
  }
  stream =
      make_shared<CrankyTokenFilter>(stream, make_shared<Random>(analyzerSeed));
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, stream);
}
} // namespace org::apache::lucene::index