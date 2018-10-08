using namespace std;

#include "TestSegmentCacheables.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using DocValues = org::apache::lucene::index::DocValues;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NoMergePolicy = org::apache::lucene::index::NoMergePolicy;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

bool TestSegmentCacheables::isCacheable(shared_ptr<LeafReaderContext> ctx,
                                        deque<SegmentCacheable> &ss)
{
  for (shared_ptr<SegmentCacheable> s : ss) {
    if (s->isCacheable(ctx) == false) {
      return false;
    }
  }
  return true;
}

void TestSegmentCacheables::testMultipleDocValuesDelegates() 
{

  shared_ptr<SegmentCacheable> seg = [&](ctx) { true; };
  shared_ptr<SegmentCacheable> non = [&](ctx) { false; };
  shared_ptr<SegmentCacheable> dv1 = [&](ctx) {
    org::apache::lucene::index::DocValues::isCacheable(ctx, {L"field1"});
  };
  shared_ptr<SegmentCacheable> dv2 = [&](ctx) {
    org::apache::lucene::index::DocValues::isCacheable(ctx, {L"field2"});
  };
  shared_ptr<SegmentCacheable> dv3 = [&](ctx) {
    org::apache::lucene::index::DocValues::isCacheable(ctx, {L"field3"});
  };
  shared_ptr<SegmentCacheable> dv34 = [&](ctx) {
    org::apache::lucene::index::DocValues::isCacheable(ctx,
                                                       {L"field3", L"field4"});
  };
  shared_ptr<SegmentCacheable> dv12 = [&](ctx) {
    org::apache::lucene::index::DocValues::isCacheable(ctx,
                                                       {L"field1", L"field2"});
  };

  shared_ptr<SegmentCacheable> seg_dv1 = [&](ctx) {
    isCacheable(ctx, {seg, dv1});
  };
  shared_ptr<SegmentCacheable> dv2_dv34 = [&](ctx) {
    isCacheable(ctx, {dv2, dv34});
  };
  shared_ptr<SegmentCacheable> dv2_non = [&](ctx) {
    isCacheable(ctx, {dv2, non});
  };

  shared_ptr<SegmentCacheable> seg_dv1_dv2_dv34 = [&](ctx) {
    isCacheable(ctx, {seg_dv1, dv2_dv34});
  };

  shared_ptr<SegmentCacheable> dv1_dv3 = [&](ctx) {
    isCacheable(ctx, {dv1, dv3});
  };
  shared_ptr<SegmentCacheable> dv12_dv1_dv3 = [&](ctx) {
    isCacheable(ctx, {dv12, dv1_dv3});
  };

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig()->setMergePolicy(NoMergePolicy::INSTANCE);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"field3", 1));
  doc->push_back(newTextField(L"text", L"text", Field::Store::NO));
  w->addDocument(doc);
  w->commit();
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(w);

  shared_ptr<LeafReaderContext> ctx = reader->leaves()->get(0);

  assertTrue(seg_dv1->isCacheable(ctx));
  assertTrue(dv2_dv34->isCacheable(ctx));
  assertTrue(seg_dv1_dv2_dv34->isCacheable(ctx));
  assertFalse(dv2_non->isCacheable(ctx));

  w->updateNumericDocValue(make_shared<Term>(L"text", L"text"), L"field3", 2LL);
  w->commit();
  reader->close();
  reader = DirectoryReader::open(dir);

  // after field3 is updated, all composites referring to it should be
  // uncacheable

  ctx = reader->leaves()->get(0);
  assertTrue(seg_dv1->isCacheable(ctx));
  assertFalse(dv34->isCacheable(ctx));
  assertFalse(dv2_dv34->isCacheable(ctx));
  assertFalse(dv1_dv3->isCacheable(ctx));
  assertFalse(seg_dv1_dv2_dv34->isCacheable(ctx));
  assertFalse(dv12_dv1_dv3->isCacheable(ctx));

  reader->close();
  delete w;
  delete dir;
}
} // namespace org::apache::lucene::search