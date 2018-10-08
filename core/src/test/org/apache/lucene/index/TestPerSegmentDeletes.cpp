using namespace std;

#include "TestPerSegmentDeletes.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestPerSegmentDeletes::testDeletes1() 
{
  // IndexWriter.debug2 = System.out;
  shared_ptr<Directory> dir = make_shared<MockDirectoryWrapper>(
      make_shared<Random>(random()->nextLong()), make_shared<RAMDirectory>());
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  iwc->setMergeScheduler(make_shared<SerialMergeScheduler>());
  iwc->setMaxBufferedDocs(5000);
  iwc->setRAMBufferSizeMB(100);
  shared_ptr<RangeMergePolicy> fsmp = make_shared<RangeMergePolicy>(false);
  iwc->setMergePolicy(fsmp);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, iwc);
  for (int x = 0; x < 5; x++) {
    writer->addDocument(DocHelper::createDocument(x, L"1", 2));
    // System.out.println("numRamDocs(" + x + ")" + writer.numRamDocs());
  }
  // System.out.println("commit1");
  writer->commit();
  TestUtil::assertEquals(1, writer->segmentInfos->size());
  for (int x = 5; x < 10; x++) {
    writer->addDocument(DocHelper::createDocument(x, L"2", 2));
    // System.out.println("numRamDocs(" + x + ")" + writer.numRamDocs());
  }
  // System.out.println("commit2");
  writer->commit();
  TestUtil::assertEquals(2, writer->segmentInfos->size());

  for (int x = 10; x < 15; x++) {
    writer->addDocument(DocHelper::createDocument(x, L"3", 2));
    // System.out.println("numRamDocs(" + x + ")" + writer.numRamDocs());
  }

  writer->deleteDocuments({make_shared<Term>(L"id", L"1")});

  writer->deleteDocuments({make_shared<Term>(L"id", L"11")});

  writer->flush(false, false);

  // deletes are now resolved on flush, so there shouldn't be
  // any deletes after flush
  assertFalse(writer->bufferedUpdatesStream->any());

  // get reader flushes pending deletes
  // so there should not be anymore
  shared_ptr<IndexReader> r1 = writer->getReader();
  assertFalse(writer->bufferedUpdatesStream->any());
  delete r1;

  // delete id:2 from the first segment
  // merge segments 0 and 1
  // which should apply the delete id:2
  writer->deleteDocuments({make_shared<Term>(L"id", L"2")});
  writer->flush(false, false);
  fsmp = std::static_pointer_cast<RangeMergePolicy>(
      writer->getConfig()->getMergePolicy());
  fsmp->doMerge = true;
  fsmp->start = 0;
  fsmp->length = 2;
  writer->maybeMerge();

  TestUtil::assertEquals(2, writer->segmentInfos->size());

  // id:2 shouldn't exist anymore because
  // it's been applied in the merge and now it's gone
  shared_ptr<IndexReader> r2 = writer->getReader();
  std::deque<int> id2docs =
      toDocsArray(make_shared<Term>(L"id", L"2"), nullptr, r2);
  assertTrue(id2docs.empty());
  delete r2;

  /**
  // added docs are in the ram buffer
  for (int x = 15; x < 20; x++) {
    writer.addDocument(TestIndexWriterReader.createDocument(x, "4", 2));
    System.out.println("numRamDocs(" + x + ")" + writer.numRamDocs());
  }
  assertTrue(writer.numRamDocs() > 0);
  // delete from the ram buffer
  writer.deleteDocuments(new Term("id", Integer.toString(13)));

  Term id3 = new Term("id", Integer.toString(3));

  // delete from the 1st segment
  writer.deleteDocuments(id3);

  assertTrue(writer.numRamDocs() > 0);

  //System.out
  //    .println("segdels1:" + writer.docWriter.deletesToString());

  //assertTrue(writer.docWriter.segmentDeletes.size() > 0);

  // we cause a merge to happen
  fsmp.doMerge = true;
  fsmp.start = 0;
  fsmp.length = 2;
  System.out.println("maybeMerge "+writer.segmentInfos);

  SegmentInfo info0 = writer.segmentInfos.info(0);
  SegmentInfo info1 = writer.segmentInfos.info(1);

  writer.maybeMerge();
  System.out.println("maybeMerge after "+writer.segmentInfos);
  // there should be docs in RAM
  assertTrue(writer.numRamDocs() > 0);

  // assert we've merged the 1 and 2 segments
  // and still have a segment leftover == 2
  assertEquals(2, writer.segmentInfos.size());
  assertFalse(segThere(info0, writer.segmentInfos));
  assertFalse(segThere(info1, writer.segmentInfos));

  //System.out.println("segdels2:" + writer.docWriter.deletesToString());

  //assertTrue(writer.docWriter.segmentDeletes.size() > 0);

  IndexReader r = writer.getReader();
  IndexReader r1 = r.getSequentialSubReaders()[0];
  printDelDocs(r1.getLiveDocs());
  int[] docs = toDocsArray(id3, null, r);
  System.out.println("id3 docs:"+Arrays.toString(docs));
  // there shouldn't be any docs for id:3
  assertTrue(docs == null);
  r.close();

  part2(writer, fsmp);
  **/
  // System.out.println("segdels2:"+writer.docWriter.segmentDeletes.toString());
  // System.out.println("close");
  delete writer;
  delete dir;
}

void TestPerSegmentDeletes::part2(
    shared_ptr<IndexWriter> writer,
    shared_ptr<RangeMergePolicy> fsmp) 
{
  for (int x = 20; x < 25; x++) {
    writer->addDocument(DocHelper::createDocument(x, L"5", 2));
    // System.out.println("numRamDocs(" + x + ")" + writer.numRamDocs());
  }
  writer->flush(false, false);
  for (int x = 25; x < 30; x++) {
    writer->addDocument(DocHelper::createDocument(x, L"5", 2));
    // System.out.println("numRamDocs(" + x + ")" + writer.numRamDocs());
  }
  writer->flush(false, false);

  // System.out.println("infos3:"+writer.segmentInfos);

  shared_ptr<Term> delterm = make_shared<Term>(L"id", L"8");
  writer->deleteDocuments({delterm});
  // System.out.println("segdels3:" + writer.docWriter.deletesToString());

  fsmp->doMerge = true;
  fsmp->start = 1;
  fsmp->length = 2;
  writer->maybeMerge();

  // deletes for info1, the newly created segment from the
  // merge should have no deletes because they were applied in
  // the merge
  // SegmentInfo info1 = writer.segmentInfos.info(1);
  // assertFalse(exists(info1, writer.docWriter.segmentDeletes));

  // System.out.println("infos4:"+writer.segmentInfos);
  // System.out.println("segdels4:" + writer.docWriter.deletesToString());
}

bool TestPerSegmentDeletes::segThere(shared_ptr<SegmentCommitInfo> info,
                                     shared_ptr<SegmentInfos> infos)
{
  for (auto si : infos) {
    if (si->info->name == info->info->name) {
      return true;
    }
  }
  return false;
}

void TestPerSegmentDeletes::printDelDocs(shared_ptr<Bits> bits)
{
  if (bits == nullptr) {
    return;
  }
  for (int x = 0; x < bits->length(); x++) {
    wcout << x << L":" << bits->get(x) << endl;
  }
}

std::deque<int> TestPerSegmentDeletes::toDocsArray(
    shared_ptr<Term> term, shared_ptr<Bits> bits,
    shared_ptr<IndexReader> reader) 
{
  shared_ptr<TermsEnum> ctermsEnum =
      MultiFields::getTerms(reader, term->field_)->begin();
  if (ctermsEnum->seekExact(make_shared<BytesRef>(term->text()))) {
    shared_ptr<PostingsEnum> postingsEnum =
        TestUtil::docs(random(), ctermsEnum, nullptr, PostingsEnum::NONE);
    return toArray(postingsEnum);
  }
  return nullptr;
}

std::deque<int> TestPerSegmentDeletes::toArray(
    shared_ptr<PostingsEnum> postingsEnum) 
{
  std::deque<int> docs(0);
  int numDocs = 0;
  while (postingsEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
    int docID = postingsEnum->docID();
    docs = ArrayUtil::grow(docs, numDocs + 1);
    docs[numDocs + 1] = docID;
  }
  return Arrays::copyOf(docs, numDocs);
}

TestPerSegmentDeletes::RangeMergePolicy::RangeMergePolicy(bool useCompoundFile)
    : useCompoundFile(useCompoundFile)
{
}

shared_ptr<MergeSpecification>
TestPerSegmentDeletes::RangeMergePolicy::findMerges(
    MergeTrigger mergeTrigger, shared_ptr<SegmentInfos> segmentInfos,
    shared_ptr<MergeContext> mergeContext) 
{
  shared_ptr<MergeSpecification> ms = make_shared<MergeSpecification>();
  if (doMerge) {
    shared_ptr<OneMerge> om = make_shared<OneMerge>(
        segmentInfos->asList().subList(start, start + length));
    ms->add(om);
    doMerge = false;
    return ms;
  }
  return nullptr;
}

shared_ptr<MergeSpecification>
TestPerSegmentDeletes::RangeMergePolicy::findForcedMerges(
    shared_ptr<SegmentInfos> segmentInfos, int maxSegmentCount,
    unordered_map<std::shared_ptr<SegmentCommitInfo>, bool> &segmentsToMerge,
    shared_ptr<MergeContext> mergeContext) 
{
  return nullptr;
}

shared_ptr<MergeSpecification>
TestPerSegmentDeletes::RangeMergePolicy::findForcedDeletesMerges(
    shared_ptr<SegmentInfos> segmentInfos,
    shared_ptr<MergeContext> mergeContext) 
{
  return nullptr;
}

bool TestPerSegmentDeletes::RangeMergePolicy::useCompoundFile(
    shared_ptr<SegmentInfos> segments, shared_ptr<SegmentCommitInfo> newSegment,
    shared_ptr<MergeContext> mergeContext)
{
  return useCompoundFile_;
}
} // namespace org::apache::lucene::index