using namespace std;

#include "TestAllFilesHaveChecksumFooter.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using Directory = org::apache::lucene::store::Directory;
using IndexInput = org::apache::lucene::store::IndexInput;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestAllFilesHaveChecksumFooter::test() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  conf->setCodec(TestUtil::getDefaultCodec());
  shared_ptr<RandomIndexWriter> riw =
      make_shared<RandomIndexWriter>(random(), dir, conf);
  // Use LineFileDocs so we (hopefully) get most Lucene features
  // tested, e.g. IntPoint was recently added to it:
  shared_ptr<LineFileDocs> docs = make_shared<LineFileDocs>(random());
  for (int i = 0; i < 100; i++) {
    riw->addDocument(docs->nextDoc());
    if (random()->nextInt(7) == 0) {
      riw->commit();
    }
    if (random()->nextInt(20) == 0) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      riw->deleteDocuments(make_shared<Term>(L"docid", Integer::toString(i)));
    }
    if (random()->nextInt(15) == 0) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      riw->updateNumericDocValue(
          make_shared<Term>(L"docid", Integer::toString(i)), L"docid_intDV",
          static_cast<int64_t>(i));
    }
  }
  delete riw;
  checkFooters(dir);
  delete dir;
}

void TestAllFilesHaveChecksumFooter::checkFooters(
    shared_ptr<Directory> dir) 
{
  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
  checkFooter(dir, sis->getSegmentsFileName());

  for (auto si : sis) {
    for (auto file : si->files()) {
      checkFooter(dir, file);
    }
    if (si->info->getUseCompoundFile()) {
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory
      // cfsDir = si.info.getCodec().compoundFormat().getCompoundReader(dir,
      // si.info, newIOContext(random())))
      {
        org::apache::lucene::store::Directory cfsDir =
            si->info->getCodec()->compoundFormat()->getCompoundReader(
                dir, si->info, newIOContext(random()));
        for (auto cfsFile : cfsDir->listAll()) {
          checkFooter(cfsDir, cfsFile);
        }
      }
    }
  }
}

void TestAllFilesHaveChecksumFooter::checkFooter(
    shared_ptr<Directory> dir, const wstring &file) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexInput in =
  // dir.openInput(file, newIOContext(random())))
  {
    org::apache::lucene::store::IndexInput in_ =
        dir->openInput(file, newIOContext(random()));
    CodecUtil::checksumEntireFile(in_);
  }
}
} // namespace org::apache::lucene::index