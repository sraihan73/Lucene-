using namespace std;

#include "BaseIndexFileFormatTestCase.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Codec = org::apache::lucene::codecs::Codec;
using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using NormsConsumer = org::apache::lucene::codecs::NormsConsumer;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using StoredFieldsWriter = org::apache::lucene::codecs::StoredFieldsWriter;
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using TermVectorsWriter = org::apache::lucene::codecs::TermVectorsWriter;
using MockRandomPostingsFormat =
    org::apache::lucene::codecs::mockrandom::MockRandomPostingsFormat;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using TextField = org::apache::lucene::document::TextField;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using FlushInfo = org::apache::lucene::store::FlushInfo;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using BytesRef = org::apache::lucene::util::BytesRef;
using CloseableThreadLocal = org::apache::lucene::util::CloseableThreadLocal;
using IOUtils = org::apache::lucene::util::IOUtils;
using InfoStream = org::apache::lucene::util::InfoStream;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using RamUsageTester = org::apache::lucene::util::RamUsageTester;
using Rethrow = org::apache::lucene::util::Rethrow;
using StringHelper = org::apache::lucene::util::StringHelper;
using TestUtil = org::apache::lucene::util::TestUtil;
using Version = org::apache::lucene::util::Version;
const shared_ptr<java::util::Set<type_info>>
    BaseIndexFileFormatTestCase::EXCLUDED_CLASSES =
        java::util::Collections::newSetFromMap(
            make_shared<java::util::IdentityHashMap<type_info, bool>>());

BaseIndexFileFormatTestCase::StaticConstructor::StaticConstructor()
{
  // Directory objects, don't take into account eg. the NIO buffers
  EXCLUDED_CLASSES->add(Directory::typeid);
  EXCLUDED_CLASSES->add(IndexInput::typeid);

  // used for thread management, not by the index
  EXCLUDED_CLASSES->add(CloseableThreadLocal::typeid);
  EXCLUDED_CLASSES->add(ThreadLocal::typeid);

  // don't follow references to the top-level reader
  EXCLUDED_CLASSES->add(IndexReader::typeid);
  EXCLUDED_CLASSES->add(IndexReaderContext::typeid);

  // usually small but can bump memory usage for
  // memory-efficient things like stored fields
  EXCLUDED_CLASSES->add(FieldInfos::typeid);
  EXCLUDED_CLASSES->add(SegmentInfo::typeid);
  EXCLUDED_CLASSES->add(SegmentCommitInfo::typeid);
  EXCLUDED_CLASSES->add(FieldInfo::typeid);

  // constant overhead is typically due to strings
  // TODO: can we remove this and still pass the test consistently
  EXCLUDED_CLASSES->add(wstring::typeid);
}

BaseIndexFileFormatTestCase::StaticConstructor
    BaseIndexFileFormatTestCase::staticConstructor;

BaseIndexFileFormatTestCase::Accumulator::Accumulator(any root) : root(root) {}

int64_t BaseIndexFileFormatTestCase::Accumulator::accumulateObject(
    any o, int64_t shallowSize,
    unordered_map<std::shared_ptr<java::lang::reflect::Field>, any>
        &fieldValues,
    shared_ptr<deque<any>> queue)
{
  for (type_info clazz = o.type(); clazz != nullptr;
       clazz = clazz.getSuperclass()) {
    if (EXCLUDED_CLASSES->contains(clazz) && o != root) {
      return 0;
    }
  }
  // we have no way to estimate the size of these things in codecs although
  // something like a Collections.newSetFromMap(new HashMap<>()) uses quite
  // some memory... So for now the test ignores the overhead of such
  // collections but can we do better?
  int64_t v;
  if (std::dynamic_pointer_cast<deque>(o) != nullptr) {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: java.util.deque<?> coll = (java.util.deque<?>)
    // o;
    shared_ptr < deque < ? >> coll = any_cast < deque < ? >> (o);
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: queue.addAll((java.util.deque<?>) o);
        queue->addAll(any_cast<deque<?>>(o));
        v = static_cast<int64_t>(coll->size()) *
            RamUsageEstimator::NUM_BYTES_OBJECT_REF;
  } else if (dynamic_cast<unordered_map>(o) != nullptr) {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: final java.util.Map<?, ?> map_obj = (java.util.Map<?,?>) o;
    const unordered_map < ?, ? > map_obj = any_cast < unordered_map < ?, ? >> (o);
    queue->addAll(map_obj.keySet());
    queue->addAll(map_obj.values());
    v = 2LL * map_obj.size() * RamUsageEstimator::NUM_BYTES_OBJECT_REF;
  } else {
    v = RamUsageTester::Accumulator::accumulateObject(o, shallowSize,
                                                      fieldValues, queue);
  }
  // System.out.println(o.getClass() + "=" + v);
  return v;
}

int64_t BaseIndexFileFormatTestCase::Accumulator::accumulateArray(
    any array_, int64_t shallowSize, deque<any> &values,
    shared_ptr<deque<any>> queue)
{
  int64_t v = RamUsageTester::Accumulator::accumulateArray(
      array_, shallowSize, values, queue);
  // System.out.println(array.getClass() + "=" + v);
  return v;
}

int BaseIndexFileFormatTestCase::getCreatedVersionMajor()
{
  return Version::LATEST->major;
}

template <typename D>
D BaseIndexFileFormatTestCase::applyCreatedVersionMajor(D d) 
{
  static_assert(is_base_of<org.apache.lucene.store.Directory, D>::value,
                L"D must inherit from org.apache.lucene.store.Directory");

  if (SegmentInfos::getLastCommitGeneration(d) != -1) {
    throw invalid_argument(L"Cannot set the created version on a Directory "
                           L"that already has segments");
  }
  if (getCreatedVersionMajor() != Version::LATEST->major ||
      random()->nextBoolean()) {
    (make_shared<SegmentInfos>(getCreatedVersionMajor()))->commit(d);
  }
  return d;
}

void BaseIndexFileFormatTestCase::setUp() 
{
  LuceneTestCase::setUp();
  // set the default codec, so adding test cases to this isn't fragile
  savedCodec = Codec::getDefault();
  Codec::setDefault(getCodec());
}

void BaseIndexFileFormatTestCase::tearDown() 
{
  Codec::setDefault(savedCodec); // restore
  LuceneTestCase::tearDown();
}

unordered_map<wstring, int64_t>
BaseIndexFileFormatTestCase::bytesUsedByExtension(
    shared_ptr<Directory> d) 
{
  unordered_map<wstring, int64_t> bytesUsedByExtension =
      unordered_map<wstring, int64_t>();
  for (auto file : d->listAll()) {
    if (IndexFileNames::CODEC_FILE_PATTERN->matcher(file).matches()) {
      const wstring ext = IndexFileNames::getExtension(file);
      constexpr int64_t previousLength =
          bytesUsedByExtension.find(ext) != bytesUsedByExtension.end()
              ? bytesUsedByExtension[ext]
              : 0;
      bytesUsedByExtension.emplace(ext, previousLength + d->fileLength(file));
    }
  }
  bytesUsedByExtension.keySet().removeAll(excludedExtensionsFromByteCounts());

  return bytesUsedByExtension;
}

shared_ptr<deque<wstring>>
BaseIndexFileFormatTestCase::excludedExtensionsFromByteCounts()
{
  return unordered_set<wstring>(
      Arrays::asList(std::deque<wstring>{L"si", L"lock"}));
}

void BaseIndexFileFormatTestCase::testMergeStability() 
{
  assumeTrue(L"merge is not stable", mergeIsStable());
  shared_ptr<Directory> dir = applyCreatedVersionMajor(newDirectory());

  // do not use newMergePolicy that might return a MockMergePolicy that ignores
  // the no-CFS ratio do not use RIW which will change things up!
  shared_ptr<MergePolicy> mp = newTieredMergePolicy();
  mp->setNoCFSRatio(0);
  shared_ptr<IndexWriterConfig> cfg =
      (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
          ->setUseCompoundFile(false)
          ->setMergePolicy(mp);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, cfg);
  constexpr int numDocs = atLeast(500);
  for (int i = 0; i < numDocs; ++i) {
    shared_ptr<Document> d = make_shared<Document>();
    addRandomFields(d);
    w->addDocument(d);
  }
  w->forceMerge(1);
  w->commit();
  delete w;
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);

  shared_ptr<Directory> dir2 = applyCreatedVersionMajor(newDirectory());
  mp = newTieredMergePolicy();
  mp->setNoCFSRatio(0);
  cfg = (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
            ->setUseCompoundFile(false)
            ->setMergePolicy(mp);
  w = make_shared<IndexWriter>(dir2, cfg);
  TestUtil::addIndexesSlowly(w, {reader});

  w->commit();
  delete w;

  TestUtil::assertEquals(bytesUsedByExtension(dir), bytesUsedByExtension(dir2));

  reader->close();
  delete dir;
  delete dir2;
}

bool BaseIndexFileFormatTestCase::mergeIsStable() { return true; }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Slow public void testRamBytesUsed() throws
// java.io.IOException
void BaseIndexFileFormatTestCase::testRamBytesUsed() 
{
  if (std::dynamic_pointer_cast<RandomCodec>(Codec::getDefault()) != nullptr) {
    // this test relies on the fact that two segments will be written with
    // the same codec so we need to disable MockRandomPF
    shared_ptr<Set<wstring>> *const avoidCodecs = unordered_set<wstring>(
        (std::static_pointer_cast<RandomCodec>(Codec::getDefault()))
            ->avoidCodecs);
    avoidCodecs->add((make_shared<MockRandomPostingsFormat>())->getName());
    Codec::setDefault(make_shared<RandomCodec>(random(), avoidCodecs));
  }
  shared_ptr<Directory> dir = applyCreatedVersionMajor(newDirectory());
  shared_ptr<IndexWriterConfig> cfg =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, cfg);
  // we need to index enough documents so that constant overhead doesn't
  // dominate
  constexpr int numDocs = atLeast(10000);
  shared_ptr<LeafReader> reader1 = nullptr;
  for (int i = 0; i < numDocs; ++i) {
    shared_ptr<Document> d = make_shared<Document>();
    addRandomFields(d);
    w->addDocument(d);
    if (i == 100) {
      w->forceMerge(1);
      w->commit();
      reader1 = getOnlyLeafReader(DirectoryReader::open(dir));
    }
  }
  w->forceMerge(1);
  w->commit();
  delete w;

  shared_ptr<LeafReader> reader2 =
      getOnlyLeafReader(DirectoryReader::open(dir));

  for (shared_ptr<LeafReader> reader : Arrays::asList(reader1, reader2)) {
    (make_shared<SimpleMergedSegmentWarmer>(InfoStream::NO_OUTPUT))
        ->warm(reader);
  }

  constexpr int64_t actualBytes =
      RamUsageTester::sizeOf(
          reader2, make_shared<RamUsageTester::Accumulator>(reader2)) -
      RamUsageTester::sizeOf(reader1,
                             make_shared<RamUsageTester::Accumulator>(reader1));
  constexpr int64_t expectedBytes =
      (std::static_pointer_cast<SegmentReader>(reader2))->ramBytesUsed() -
      (std::static_pointer_cast<SegmentReader>(reader1))->ramBytesUsed();
  constexpr int64_t absoluteError = actualBytes - expectedBytes;
  constexpr double relativeError =
      static_cast<double>(absoluteError) / actualBytes;
  const wstring message = L"Actual RAM usage " + to_wstring(actualBytes) +
                          L", but got " + to_wstring(expectedBytes) + L", " +
                          to_wstring(100 * relativeError) + L"% error";
  assertTrue(message, abs(relativeError) < 0.20 || abs(absoluteError) < 1000);

  delete reader1;
  delete reader2;
  delete dir;
}

void BaseIndexFileFormatTestCase::testMultiClose() 
{
  // first make a one doc index
  shared_ptr<Directory> oneDocIndex = applyCreatedVersionMajor(newDirectory());
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      oneDocIndex,
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> oneDoc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType->setStoreTermVectors(true);
  shared_ptr<Field> customField =
      make_shared<Field>(L"field", L"contents", customType);
  oneDoc->push_back(customField);
  oneDoc->push_back(make_shared<NumericDocValuesField>(L"field", 5));
  iw->addDocument(oneDoc);
  shared_ptr<LeafReader> oneDocReader =
      getOnlyLeafReader(DirectoryReader::open(iw));
  delete iw;

  // now feed to codec apis manually
  // we use FSDir, things like ramdir are not guaranteed to cause fails if you
  // write to them after close(), etc
  shared_ptr<Directory> dir =
      newFSDirectory(createTempDir(L"justSoYouGetSomeChannelErrors"));
  shared_ptr<Codec> codec = getCodec();

  shared_ptr<SegmentInfo> segmentInfo = make_shared<SegmentInfo>(
      dir, Version::LATEST, Version::LATEST, L"_0", 1, false, codec,
      Collections::emptyMap(), StringHelper::randomId(), unordered_map<>(),
      nullptr);
  shared_ptr<FieldInfo> proto =
      oneDocReader->getFieldInfos()->fieldInfo(L"field");
  shared_ptr<FieldInfo> field = make_shared<FieldInfo>(
      proto->name, proto->number, proto->hasVectors(), proto->omitsNorms(),
      proto->hasPayloads(), proto->getIndexOptions(), proto->getDocValuesType(),
      proto->getDocValuesGen(), unordered_map<>(),
      proto->getPointDimensionCount(), proto->getPointNumBytes(),
      proto->isSoftDeletesField());

  shared_ptr<FieldInfos> fieldInfos =
      make_shared<FieldInfos>(std::deque<std::shared_ptr<FieldInfo>>{field});

  shared_ptr<SegmentWriteState> writeState = make_shared<SegmentWriteState>(
      nullptr, dir, segmentInfo, fieldInfos, nullptr,
      make_shared<IOContext>(make_shared<FlushInfo>(1, 20)));

  shared_ptr<SegmentReadState> readState = make_shared<SegmentReadState>(
      dir, segmentInfo, fieldInfos, IOContext::READ);

  // PostingsFormat
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.codecs.FieldsConsumer
  // consumer = codec.postingsFormat().fieldsConsumer(writeState))
  {
    org::apache::lucene::codecs::FieldsConsumer consumer =
        codec->postingsFormat()->fieldsConsumer(writeState);
    consumer->write(MultiFields::getFields(oneDocReader));
    IOUtils::close({consumer});
    IOUtils::close({consumer});
  }
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.codecs.FieldsProducer
  // producer = codec.postingsFormat().fieldsProducer(readState))
  {
    org::apache::lucene::codecs::FieldsProducer producer =
        codec->postingsFormat()->fieldsProducer(readState);
    IOUtils::close({producer});
    IOUtils::close({producer});
  }

  // DocValuesFormat
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.codecs.DocValuesConsumer
  // consumer = codec.docValuesFormat().fieldsConsumer(writeState))
  {
    org::apache::lucene::codecs::DocValuesConsumer consumer =
        codec->docValuesFormat()->fieldsConsumer(writeState);
    consumer.addNumericField(
        field, make_shared<EmptyDocValuesProducerAnonymousInnerClass>(
                   shared_from_this(), field));
    IOUtils::close({consumer});
    IOUtils::close({consumer});
  }
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.codecs.DocValuesProducer
  // producer = codec.docValuesFormat().fieldsProducer(readState))
  {
    org::apache::lucene::codecs::DocValuesProducer producer =
        codec->docValuesFormat()->fieldsProducer(readState);
    IOUtils::close({producer});
    IOUtils::close({producer});
  }

  // NormsFormat
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.codecs.NormsConsumer
  // consumer = codec.normsFormat().normsConsumer(writeState))
  {
    org::apache::lucene::codecs::NormsConsumer consumer =
        codec->normsFormat()->normsConsumer(writeState);
    consumer.addNormsField(field, make_shared<NormsProducerAnonymousInnerClass>(
                                      shared_from_this(), field));
    IOUtils::close({consumer});
    IOUtils::close({consumer});
  }
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.codecs.NormsProducer
  // producer = codec.normsFormat().normsProducer(readState))
  {
    org::apache::lucene::codecs::NormsProducer producer =
        codec->normsFormat()->normsProducer(readState);
    IOUtils::close({producer});
    IOUtils::close({producer});
  }

  // TermVectorsFormat
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.codecs.TermVectorsWriter
  // consumer = codec.termVectorsFormat().vectorsWriter(dir, segmentInfo,
  // writeState.context))
  {
    org::apache::lucene::codecs::TermVectorsWriter consumer =
        codec->termVectorsFormat()->vectorsWriter(dir, segmentInfo,
                                                  writeState->context);
    consumer.startDocument(1);
    consumer.startField(field, 1, false, false, false);
    consumer.startTerm(make_shared<BytesRef>(L"testing"), 2);
    consumer.finishTerm();
    consumer.finishField();
    consumer.finishDocument();
    consumer.finish(fieldInfos, 1);
    IOUtils::close({consumer});
    IOUtils::close({consumer});
  }
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.codecs.TermVectorsReader
  // producer = codec.termVectorsFormat().vectorsReader(dir, segmentInfo,
  // fieldInfos, readState.context))
  {
    org::apache::lucene::codecs::TermVectorsReader producer =
        codec->termVectorsFormat()->vectorsReader(dir, segmentInfo, fieldInfos,
                                                  readState->context);
    IOUtils::close({producer});
    IOUtils::close({producer});
  }

  // StoredFieldsFormat
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.codecs.StoredFieldsWriter
  // consumer = codec.storedFieldsFormat().fieldsWriter(dir, segmentInfo,
  // writeState.context))
  {
    org::apache::lucene::codecs::StoredFieldsWriter consumer =
        codec->storedFieldsFormat()->fieldsWriter(dir, segmentInfo,
                                                  writeState->context);
    consumer.startDocument();
    consumer.writeField(field, customField);
    consumer.finishDocument();
    consumer.finish(fieldInfos, 1);
    IOUtils::close({consumer});
    IOUtils::close({consumer});
  }
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.codecs.StoredFieldsReader
  // producer = codec.storedFieldsFormat().fieldsReader(dir, segmentInfo,
  // fieldInfos, readState.context))
  {
    org::apache::lucene::codecs::StoredFieldsReader producer =
        codec->storedFieldsFormat()->fieldsReader(dir, segmentInfo, fieldInfos,
                                                  readState->context);
    IOUtils::close({producer});
    IOUtils::close({producer});
  }

  IOUtils::close({oneDocReader, oneDocIndex, dir});
}

BaseIndexFileFormatTestCase::EmptyDocValuesProducerAnonymousInnerClass::
    EmptyDocValuesProducerAnonymousInnerClass(
        shared_ptr<BaseIndexFileFormatTestCase> outerInstance,
        shared_ptr<org::apache::lucene::index::FieldInfo> field)
{
  this->outerInstance = outerInstance;
  this->field = field;
}

shared_ptr<NumericDocValues>
BaseIndexFileFormatTestCase::EmptyDocValuesProducerAnonymousInnerClass::
    getNumeric(shared_ptr<FieldInfo> field)
{
  return make_shared<NumericDocValuesAnonymousInnerClass>(shared_from_this());
}

BaseIndexFileFormatTestCase::EmptyDocValuesProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass::NumericDocValuesAnonymousInnerClass(
        shared_ptr<EmptyDocValuesProducerAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
  docID = -1;
}

int BaseIndexFileFormatTestCase::EmptyDocValuesProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass::docID()
{
  return docID;
}

int BaseIndexFileFormatTestCase::EmptyDocValuesProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass::nextDoc()
{
  docID++;
  if (docID == 1) {
    docID = NO_MORE_DOCS;
  }
  return docID;
}

int BaseIndexFileFormatTestCase::EmptyDocValuesProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass::advance(int target)
{
  if (docID <= 0 && target == 0) {
    docID = 0;
  } else {
    docID = NO_MORE_DOCS;
  }
  return docID;
}

bool BaseIndexFileFormatTestCase::EmptyDocValuesProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass::advanceExact(int target) throw(
        IOException)
{
  docID = target;
  return target == 0;
}

int64_t
BaseIndexFileFormatTestCase::EmptyDocValuesProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass::cost()
{
  return 1;
}

int64_t
BaseIndexFileFormatTestCase::EmptyDocValuesProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass::longValue()
{
  return 5;
}

BaseIndexFileFormatTestCase::NormsProducerAnonymousInnerClass::
    NormsProducerAnonymousInnerClass(
        shared_ptr<BaseIndexFileFormatTestCase> outerInstance,
        shared_ptr<org::apache::lucene::index::FieldInfo> field)
{
  this->outerInstance = outerInstance;
  this->field = field;
}

shared_ptr<NumericDocValues>
BaseIndexFileFormatTestCase::NormsProducerAnonymousInnerClass::getNorms(
    shared_ptr<FieldInfo> field)
{
  return make_shared<NumericDocValuesAnonymousInnerClass2>(shared_from_this());
}

BaseIndexFileFormatTestCase::NormsProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass2::NumericDocValuesAnonymousInnerClass2(
        shared_ptr<NormsProducerAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
  docID = -1;
}

int BaseIndexFileFormatTestCase::NormsProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass2::docID()
{
  return docID;
}

int BaseIndexFileFormatTestCase::NormsProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass2::nextDoc()
{
  docID++;
  if (docID == 1) {
    docID = NO_MORE_DOCS;
  }
  return docID;
}

int BaseIndexFileFormatTestCase::NormsProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass2::advance(int target)
{
  if (docID <= 0 && target == 0) {
    docID = 0;
  } else {
    docID = NO_MORE_DOCS;
  }
  return docID;
}

bool BaseIndexFileFormatTestCase::NormsProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass2::advanceExact(int target) throw(
        IOException)
{
  docID = target;
  return target == 0;
}

int64_t BaseIndexFileFormatTestCase::NormsProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass2::cost()
{
  return 1;
}

int64_t BaseIndexFileFormatTestCase::NormsProducerAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass2::longValue()
{
  return 5;
}

void BaseIndexFileFormatTestCase::NormsProducerAnonymousInnerClass::
    checkIntegrity()
{
}

BaseIndexFileFormatTestCase::NormsProducerAnonymousInnerClass::
    ~NormsProducerAnonymousInnerClass()
{
}

int64_t
BaseIndexFileFormatTestCase::NormsProducerAnonymousInnerClass::ramBytesUsed()
{
  return 0;
}

void BaseIndexFileFormatTestCase::testRandomExceptions() 
{
  // disable slow things: we don't rely upon sleeps here.
  shared_ptr<MockDirectoryWrapper> dir =
      applyCreatedVersionMajor(newMockDirectory());
  dir->setThrottling(MockDirectoryWrapper::Throttling::NEVER);
  dir->setUseSlowOpenClosers(false);
  dir->setRandomIOExceptionRate(0.001); // more rare

  // log all exceptions we hit, in case we fail (for debugging)
  shared_ptr<ByteArrayOutputStream> exceptionLog =
      make_shared<ByteArrayOutputStream>();
  shared_ptr<PrintStream> exceptionStream =
      make_shared<PrintStream>(exceptionLog, true, L"UTF-8");
  // PrintStream exceptionStream = System.out;

  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  // just for now, try to keep this test reproducible
  conf->setMergeScheduler(make_shared<SerialMergeScheduler>());
  conf->setCodec(getCodec());

  int numDocs = atLeast(500);

  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, conf);
  try {
    bool allowAlreadyClosed = false;
    for (int i = 0; i < numDocs; i++) {
      dir->setRandomIOExceptionRateOnOpen(
          0.02); // turn on exceptions for openInput/createOutput

      shared_ptr<Document> doc = make_shared<Document>();
      // C++ TODO: There is no native C++ equivalent to 'toString':
      doc->push_back(
          newStringField(L"id", Integer::toString(i), Field::Store::NO));
      addRandomFields(doc);

      // single doc
      try {
        iw->addDocument(doc);
        // we made it, sometimes delete our doc
        // C++ TODO: There is no native C++ equivalent to 'toString':
        iw->deleteDocuments({make_shared<Term>(L"id", Integer::toString(i))});
      } catch (const AlreadyClosedException &ace) {
        // OK: writer was closed by abort; we just reopen now:
        dir->setRandomIOExceptionRateOnOpen(
            0.0); // disable exceptions on openInput until next iteration
        assertTrue(iw->deleter->isClosed());
        assertTrue(allowAlreadyClosed);
        allowAlreadyClosed = false;
        conf = newIndexWriterConfig(analyzer);
        // just for now, try to keep this test reproducible
        conf->setMergeScheduler(make_shared<SerialMergeScheduler>());
        conf->setCodec(getCodec());
        iw = make_shared<IndexWriter>(dir, conf);
      } catch (const IOException &e) {
        handleFakeIOException(e, exceptionStream);
        allowAlreadyClosed = true;
      }

      if (random()->nextInt(10) == 0) {
        // trigger flush:
        try {
          if (random()->nextBoolean()) {
            shared_ptr<DirectoryReader> ir = nullptr;
            try {
              ir = DirectoryReader::open(iw, random()->nextBoolean(), false);
              dir->setRandomIOExceptionRateOnOpen(
                  0.0); // disable exceptions on openInput until next iteration
              TestUtil::checkReader(ir);
            }
            // C++ TODO: There is no native C++ equivalent to the exception
            // 'finally' clause:
            finally {
              IOUtils::closeWhileHandlingException({ir});
            }
          } else {
            dir->setRandomIOExceptionRateOnOpen(
                0.0); // disable exceptions on openInput until next iteration:
                      // or we make slowExists angry and trip a scarier assert!
            iw->commit();
          }
          if (DirectoryReader::indexExists(dir)) {
            TestUtil::checkIndex(dir);
          }
        } catch (const AlreadyClosedException &ace) {
          // OK: writer was closed by abort; we just reopen now:
          dir->setRandomIOExceptionRateOnOpen(
              0.0); // disable exceptions on openInput until next iteration
          assertTrue(iw->deleter->isClosed());
          assertTrue(allowAlreadyClosed);
          allowAlreadyClosed = false;
          conf = newIndexWriterConfig(analyzer);
          // just for now, try to keep this test reproducible
          conf->setMergeScheduler(make_shared<SerialMergeScheduler>());
          conf->setCodec(getCodec());
          iw = make_shared<IndexWriter>(dir, conf);
        } catch (const IOException &e) {
          handleFakeIOException(e, exceptionStream);
          allowAlreadyClosed = true;
        }
      }
    }

    try {
      dir->setRandomIOExceptionRateOnOpen(
          0.0); // disable exceptions on openInput until next iteration:
                // or we make slowExists angry and trip a scarier assert!
      delete iw;
    } catch (const IOException &e) {
      handleFakeIOException(e, exceptionStream);
      try {
        iw->rollback();
      } catch (const runtime_error &t) {
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

void BaseIndexFileFormatTestCase::handleFakeIOException(
    shared_ptr<IOException> e, shared_ptr<PrintStream> exceptionStream)
{
  runtime_error ex = e;
  while (ex != nullptr) {
    if (ex.what() != nullptr &&
        ex.what()->startsWith(L"a random IOException")) {
      exceptionStream->println(L"\nTEST: got expected fake exc:" + ex.what());
      ex.printStackTrace(exceptionStream);
      return;
    }
    ex = ex.getCause();
  }

  Rethrow::rethrow(e);
}
} // namespace org::apache::lucene::index