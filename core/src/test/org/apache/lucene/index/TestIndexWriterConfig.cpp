using namespace std;

#include "TestIndexWriterConfig.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Codec = org::apache::lucene::codecs::Codec;
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using IndexingChain =
    org::apache::lucene::index::DocumentsWriterPerThread::IndexingChain;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using Directory = org::apache::lucene::store::Directory;
using InfoStream = org::apache::lucene::util::InfoStream;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;

shared_ptr<DocConsumer> TestIndexWriterConfig::MyIndexingChain::getChain(
    shared_ptr<DocumentsWriterPerThread> documentsWriter)
{
  return nullptr;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDefaults() throws Exception
void TestIndexWriterConfig::testDefaults() 
{
  shared_ptr<IndexWriterConfig> conf =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  assertEquals(MockAnalyzer::typeid, conf->getAnalyzer()->getClass());
  assertNull(conf->getIndexCommit());
  assertEquals(KeepOnlyLastCommitDeletionPolicy::typeid,
               conf->getIndexDeletionPolicy()->getClass());
  assertEquals(ConcurrentMergeScheduler::typeid,
               conf->getMergeScheduler()->getClass());
  assertEquals(OpenMode::CREATE_OR_APPEND, conf->getOpenMode());
  // we don't need to assert this, it should be unspecified
  assertTrue(IndexSearcher::getDefaultSimilarity() == conf->getSimilarity());
  assertEquals(IndexWriterConfig::DEFAULT_RAM_BUFFER_SIZE_MB,
               conf->getRAMBufferSizeMB(), 0.0);
  assertEquals(IndexWriterConfig::DEFAULT_MAX_BUFFERED_DOCS,
               conf->getMaxBufferedDocs());
  assertEquals(IndexWriterConfig::DEFAULT_READER_POOLING,
               conf->getReaderPooling());
  assertTrue(DocumentsWriterPerThread::defaultIndexingChain ==
             conf->getIndexingChain());
  assertNull(conf->getMergedSegmentWarmer());
  assertEquals(TieredMergePolicy::typeid, conf->getMergePolicy()->getClass());
  assertEquals(DocumentsWriterPerThreadPool::typeid,
               conf->getIndexerThreadPool()->getClass());
  assertEquals(FlushByRamOrCountsPolicy::typeid,
               conf->getFlushPolicy()->getClass());
  assertEquals(IndexWriterConfig::DEFAULT_RAM_PER_THREAD_HARD_LIMIT_MB,
               conf->getRAMPerThreadHardLimitMB());
  assertEquals(Codec::getDefault(), conf->getCodec());
  assertEquals(InfoStream::getDefault(), conf->getInfoStream());
  assertEquals(IndexWriterConfig::DEFAULT_USE_COMPOUND_FILE_SYSTEM,
               conf->getUseCompoundFile());
  assertTrue(conf->isCheckPendingFlushOnUpdate());
  // Sanity check - validate that all getters are covered.
  shared_ptr<Set<wstring>> getters = unordered_set<wstring>();
  getters->add(L"getAnalyzer");
  getters->add(L"getIndexCommit");
  getters->add(L"getIndexDeletionPolicy");
  getters->add(L"getMaxFieldLength");
  getters->add(L"getMergeScheduler");
  getters->add(L"getOpenMode");
  getters->add(L"getSimilarity");
  getters->add(L"getWriteLockTimeout");
  getters->add(L"getDefaultWriteLockTimeout");
  getters->add(L"getMaxBufferedDeleteTerms");
  getters->add(L"getRAMBufferSizeMB");
  getters->add(L"getMaxBufferedDocs");
  getters->add(L"getIndexingChain");
  getters->add(L"getMergedSegmentWarmer");
  getters->add(L"getMergePolicy");
  getters->add(L"getReaderPooling");
  getters->add(L"getIndexerThreadPool");
  getters->add(L"getFlushPolicy");
  getters->add(L"getRAMPerThreadHardLimitMB");
  getters->add(L"getCodec");
  getters->add(L"getInfoStream");
  getters->add(L"getUseCompoundFile");
  getters->add(L"isCheckPendingFlushOnUpdate");
  getters->add(L"getSoftDeletesField");

  for (shared_ptr<Method> m : IndexWriterConfig::typeid->getDeclaredMethods()) {
    if (m->getDeclaringClass() == IndexWriterConfig::typeid &&
        m->getName()->startsWith(L"get")) {
      assertTrue(L"method " + m->getName() + L" is not tested for defaults",
                 getters->contains(m->getName()));
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSettersChaining() throws Exception
void TestIndexWriterConfig::testSettersChaining() 
{
  // Ensures that every setter returns IndexWriterConfig to allow chaining.
  unordered_set<wstring> liveSetters = unordered_set<wstring>();
  unordered_set<wstring> allSetters = unordered_set<wstring>();
  for (shared_ptr<Method> m : IndexWriterConfig::typeid->getDeclaredMethods()) {
    if (m->getName()->startsWith(L"set") &&
        !Modifier::isStatic(m->getModifiers())) {
      allSetters.insert(m->getName());
      // setters overridden from LiveIndexWriterConfig are returned twice, once
      // with IndexWriterConfig return type and second with
      // LiveIndexWriterConfig. The ones from LiveIndexWriterConfig are marked
      // 'synthetic', so just collect them and assert in the end that we also
      // received them from IWC.
      if (m->isSynthetic()) {
        liveSetters.insert(m->getName());
      } else {
        assertEquals(L"method " + m->getName() +
                         L" does not return IndexWriterConfig",
                     IndexWriterConfig::typeid, m->getReturnType());
      }
    }
  }
  for (auto setter : liveSetters) {
    assertTrue(L"setter method not overridden by IndexWriterConfig: " + setter,
               find(allSetters.begin(), allSetters.end(), setter) !=
                   allSetters.end());
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testReuse() throws Exception
void TestIndexWriterConfig::testReuse() 
{
  shared_ptr<Directory> dir = newDirectory();
  // test that IWC cannot be reused across two IWs
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(nullptr);
  delete (make_shared<RandomIndexWriter>(random(), dir, conf));

  // this should fail
  expectThrows(IllegalStateException::typeid, [&]() {
    assertNotNull(make_shared<RandomIndexWriter>(random(), dir, conf));
  });

  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testOverrideGetters() throws Exception
void TestIndexWriterConfig::testOverrideGetters() 
{
  // Test that IndexWriterConfig overrides all getters, so that javadocs
  // contain all methods for the users. Also, ensures that IndexWriterConfig
  // doesn't declare getters that are not declared on LiveIWC.
  unordered_set<wstring> liveGetters = unordered_set<wstring>();
  for (shared_ptr<Method> m :
       LiveIndexWriterConfig::typeid->getDeclaredMethods()) {
    if (m->getName()->startsWith(L"get") &&
        !Modifier::isStatic(m->getModifiers())) {
      liveGetters.insert(m->getName());
    }
  }

  for (shared_ptr<Method> m : IndexWriterConfig::typeid->getDeclaredMethods()) {
    if (m->getName()->startsWith(L"get") &&
        !Modifier::isStatic(m->getModifiers())) {
      assertEquals(L"method " + m->getName() +
                       L" not overrided by IndexWriterConfig",
                   IndexWriterConfig::typeid, m->getDeclaringClass());
          assertTrue(L"method " + m->getName() + L" not declared on LiveIndexWriterConfig", find(liveGetters.begin(), liveGetters.end(), m->getName()) != liveGetters.end()));
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testConstants() throws Exception
void TestIndexWriterConfig::testConstants() 
{
  // Tests that the values of the constants does not change
  assertEquals(-1, IndexWriterConfig::DISABLE_AUTO_FLUSH);
  assertEquals(IndexWriterConfig::DISABLE_AUTO_FLUSH,
               IndexWriterConfig::DEFAULT_MAX_BUFFERED_DELETE_TERMS);
  assertEquals(IndexWriterConfig::DISABLE_AUTO_FLUSH,
               IndexWriterConfig::DEFAULT_MAX_BUFFERED_DOCS);
  assertEquals(16.0, IndexWriterConfig::DEFAULT_RAM_BUFFER_SIZE_MB, 0.0);
  assertEquals(true, IndexWriterConfig::DEFAULT_READER_POOLING);
  assertEquals(true, IndexWriterConfig::DEFAULT_USE_COMPOUND_FILE_SYSTEM);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testToString() throws Exception
void TestIndexWriterConfig::testToString() 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring str =
      (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
          ->toString();
  for (shared_ptr<Field> f : IndexWriterConfig::typeid->getDeclaredFields()) {
    int modifiers = f->getModifiers();
    if (Modifier::isStatic(modifiers) && Modifier::isFinal(modifiers)) {
      // Skip static final fields, they are only constants
      continue;
    } else if (L"indexingChain" == f->getName()) {
      // indexingChain is a package-private setting and thus is not output by
      // toString.
      continue;
    }
    if (f->getName().equals(L"inUseByIndexWriter")) {
      continue;
    }
    assertTrue(f->getName() + L" not found in toString",
               str.find(f->getName()) != wstring::npos);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testInvalidValues() throws Exception
void TestIndexWriterConfig::testInvalidValues() 
{
  shared_ptr<IndexWriterConfig> conf =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));

  // Test IndexDeletionPolicy
  assertEquals(KeepOnlyLastCommitDeletionPolicy::typeid,
               conf->getIndexDeletionPolicy()->getClass());
  conf->setIndexDeletionPolicy(make_shared<SnapshotDeletionPolicy>(nullptr));
  assertEquals(SnapshotDeletionPolicy::typeid,
               conf->getIndexDeletionPolicy()->getClass());
  expectThrows(invalid_argument::typeid,
               [&]() { conf->setIndexDeletionPolicy(nullptr); });

  // Test MergeScheduler
  assertEquals(ConcurrentMergeScheduler::typeid,
               conf->getMergeScheduler()->getClass());
  conf->setMergeScheduler(make_shared<SerialMergeScheduler>());
  assertEquals(SerialMergeScheduler::typeid,
               conf->getMergeScheduler()->getClass());
  expectThrows(invalid_argument::typeid,
               [&]() { conf->setMergeScheduler(nullptr); });

  // Test Similarity:
  // we shouldnt assert what the default is, just that it's not null.
  assertTrue(IndexSearcher::getDefaultSimilarity() == conf->getSimilarity());
  conf->setSimilarity(make_shared<MySimilarity>());
  assertEquals(MySimilarity::typeid, conf->getSimilarity()->getClass());
  expectThrows(invalid_argument::typeid,
               [&]() { conf->setSimilarity(nullptr); });

  // Test IndexingChain
  assertTrue(DocumentsWriterPerThread::defaultIndexingChain ==
             conf->getIndexingChain());

  expectThrows(invalid_argument::typeid,
               [&]() { conf->setMaxBufferedDocs(1); });

  expectThrows(invalid_argument::typeid, [&]() {
    conf->setMaxBufferedDocs(4);
    conf->setRAMBufferSizeMB(IndexWriterConfig::DISABLE_AUTO_FLUSH);
    conf->setMaxBufferedDocs(IndexWriterConfig::DISABLE_AUTO_FLUSH);
  });

  conf->setRAMBufferSizeMB(IndexWriterConfig::DEFAULT_RAM_BUFFER_SIZE_MB);
  conf->setMaxBufferedDocs(IndexWriterConfig::DEFAULT_MAX_BUFFERED_DOCS);
  expectThrows(invalid_argument::typeid, [&]() {
    conf->setRAMBufferSizeMB(IndexWriterConfig::DISABLE_AUTO_FLUSH);
  });

  expectThrows(invalid_argument::typeid,
               [&]() { conf->setRAMPerThreadHardLimitMB(2048); });

  expectThrows(invalid_argument::typeid,
               [&]() { conf->setRAMPerThreadHardLimitMB(0); });

  // Test MergePolicy
  assertEquals(TieredMergePolicy::typeid, conf->getMergePolicy()->getClass());
  conf->setMergePolicy(make_shared<LogDocMergePolicy>());
  assertEquals(LogDocMergePolicy::typeid, conf->getMergePolicy()->getClass());
  expectThrows(invalid_argument::typeid,
               [&]() { conf->setMergePolicy(nullptr); });
}

void TestIndexWriterConfig::testLiveChangeToCFS() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  iwc->setMergePolicy(newLogMergePolicy(true));
  // Start false:
  iwc->setUseCompoundFile(false);
  iwc->getMergePolicy()->setNoCFSRatio(0.0);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  // Change to true:
  w->getConfig()->setUseCompoundFile(true);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"field", L"foo", Store::NO));
  w->addDocument(doc);
  w->commit();
  assertTrue(L"Expected CFS after commit",
             w->newestSegment()->info->getUseCompoundFile());

  doc->push_back(newStringField(L"field", L"foo", Store::NO));
  w->addDocument(doc);
  w->commit();
  w->forceMerge(1);
  w->commit();

  // no compound files after merge
  assertFalse(L"Expected Non-CFS after merge",
              w->newestSegment()->info->getUseCompoundFile());

  shared_ptr<MergePolicy> lmp = w->getConfig()->getMergePolicy();
  lmp->setNoCFSRatio(1.0);
  lmp->setMaxCFSSegmentSizeMB(numeric_limits<double>::infinity());

  w->addDocument(doc);
  w->forceMerge(1);
  w->commit();
  assertTrue(L"Expected CFS after merge",
             w->newestSegment()->info->getUseCompoundFile());
  delete w;
  delete dir;
}
} // namespace org::apache::lucene::index