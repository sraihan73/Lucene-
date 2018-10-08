using namespace std;

#include "TestQueryBitSetProducer.h"

namespace org::apache::lucene::search::join
{
using Document = org::apache::lucene::document::Document;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using FilterDirectoryReader = org::apache::lucene::index::FilterDirectoryReader;
using FilterLeafReader = org::apache::lucene::index::FilterLeafReader;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReader = org::apache::lucene::index::LeafReader;
using NoMergePolicy = org::apache::lucene::index::NoMergePolicy;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using MatchNoDocsQuery = org::apache::lucene::search::MatchNoDocsQuery;
using Directory = org::apache::lucene::store::Directory;
using BitSet = org::apache::lucene::util::BitSet;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestQueryBitSetProducer::testSimple() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig()->setMergePolicy(NoMergePolicy::INSTANCE);
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  w->addDocument(make_shared<Document>());
  shared_ptr<DirectoryReader> reader = w->getReader();

  shared_ptr<QueryBitSetProducer> producer =
      make_shared<QueryBitSetProducer>(make_shared<MatchNoDocsQuery>());
  assertNull(producer->getBitSet(reader->leaves()->get(0)));
  assertEquals(1, producer->cache.size());

  producer = make_shared<QueryBitSetProducer>(make_shared<MatchAllDocsQuery>());
  shared_ptr<BitSet> bitSet = producer->getBitSet(reader->leaves()->get(0));
  assertEquals(1, bitSet->length());
  assertEquals(true, bitSet->get(0));
  assertEquals(1, producer->cache.size());

  IOUtils::close({reader, w, dir});
}

void TestQueryBitSetProducer::testReaderNotSuitedForCaching() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig()->setMergePolicy(NoMergePolicy::INSTANCE);
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  w->addDocument(make_shared<Document>());
  shared_ptr<DirectoryReader> reader =
      make_shared<DummyDirectoryReader>(w->getReader());

  shared_ptr<QueryBitSetProducer> producer =
      make_shared<QueryBitSetProducer>(make_shared<MatchNoDocsQuery>());
  assertNull(producer->getBitSet(reader->leaves()->get(0)));
  assertEquals(0, producer->cache.size());

  producer = make_shared<QueryBitSetProducer>(make_shared<MatchAllDocsQuery>());
  shared_ptr<BitSet> bitSet = producer->getBitSet(reader->leaves()->get(0));
  assertEquals(1, bitSet->length());
  assertEquals(true, bitSet->get(0));
  assertEquals(0, producer->cache.size());

  IOUtils::close({reader, w, dir});
}

TestQueryBitSetProducer::DummyDirectoryReader::DummyDirectoryReader(
    shared_ptr<DirectoryReader> in_) 
{
  FilterDirectoryReader(in_,
                        make_shared<SubReaderWrapperAnonymousInnerClass>());
}

TestQueryBitSetProducer::DummyDirectoryReader::
    SubReaderWrapperAnonymousInnerClass::SubReaderWrapperAnonymousInnerClass()
{
}

shared_ptr<LeafReader> TestQueryBitSetProducer::DummyDirectoryReader::
    SubReaderWrapperAnonymousInnerClass::wrap(shared_ptr<LeafReader> reader)
{
  return make_shared<FilterLeafReaderAnonymousInnerClass>(shared_from_this(),
                                                          reader);
}

TestQueryBitSetProducer::DummyDirectoryReader::
    SubReaderWrapperAnonymousInnerClass::FilterLeafReaderAnonymousInnerClass::
        FilterLeafReaderAnonymousInnerClass(
            shared_ptr<SubReaderWrapperAnonymousInnerClass> outerInstance,
            shared_ptr<LeafReader> reader)
    : org::apache::lucene::index::FilterLeafReader(reader)
{
  this->outerInstance = outerInstance;
}

shared_ptr<CacheHelper> TestQueryBitSetProducer::DummyDirectoryReader::
    SubReaderWrapperAnonymousInnerClass::FilterLeafReaderAnonymousInnerClass::
        getCoreCacheHelper()
{
  return nullptr;
}

shared_ptr<CacheHelper> TestQueryBitSetProducer::DummyDirectoryReader::
    SubReaderWrapperAnonymousInnerClass::FilterLeafReaderAnonymousInnerClass::
        getReaderCacheHelper()
{
  return nullptr;
}

shared_ptr<DirectoryReader>
TestQueryBitSetProducer::DummyDirectoryReader::doWrapDirectoryReader(
    shared_ptr<DirectoryReader> in_) 
{
  return make_shared<DummyDirectoryReader>(in_);
}

shared_ptr<CacheHelper>
TestQueryBitSetProducer::DummyDirectoryReader::getReaderCacheHelper()
{
  return nullptr;
}
} // namespace org::apache::lucene::search::join