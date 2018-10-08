using namespace std;

#include "TestAllFilesHaveCodecHeader.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using Directory = org::apache::lucene::store::Directory;
using IndexInput = org::apache::lucene::store::IndexInput;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestAllFilesHaveCodecHeader::test() 
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
  checkHeaders(dir, unordered_map<wstring, wstring>());
  delete dir;
}

void TestAllFilesHaveCodecHeader::checkHeaders(
    shared_ptr<Directory> dir,
    unordered_map<wstring, wstring> &namesToExtensions) 
{
  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
  checkHeader(dir, sis->getSegmentsFileName(), namesToExtensions, sis->getId());

  for (auto si : sis) {
    assertNotNull(si->info->getId());
    for (auto file : si->files()) {
      checkHeader(dir, file, namesToExtensions, si->info->getId());
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
          checkHeader(cfsDir, cfsFile, namesToExtensions, si->info->getId());
        }
      }
    }
  }
}

void TestAllFilesHaveCodecHeader::checkHeader(
    shared_ptr<Directory> dir, const wstring &file,
    unordered_map<wstring, wstring> &namesToExtensions,
    std::deque<char> &id) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexInput in =
  // dir.openInput(file, newIOContext(random())))
  {
    org::apache::lucene::store::IndexInput in_ =
        dir->openInput(file, newIOContext(random()));
    int val = in_->readInt();
    assertEquals(file + L" has no codec header, instead found: " +
                     to_wstring(val),
                 CodecUtil::CODEC_MAGIC, val);
    wstring codecName = in_->readString();
    assertFalse(codecName.isEmpty());
    wstring extension = IndexFileNames::getExtension(file);
    if (extension == L"") {
      assertTrue(StringHelper::startsWith(file, IndexFileNames::SEGMENTS));
      extension =
          L"<segments> (not a real extension, designates segments file)";
    }
    wstring previous = namesToExtensions.emplace(codecName, extension);
    if (previous != L"" && previous != extension) {
      fail(L"extensions " + previous + L" and " + extension +
           L" share same codecName " + codecName);
    }
    // read version
    in_->readInt();
    // read object id
    CodecUtil::checkIndexHeaderID(in_, id);
  }
}
} // namespace org::apache::lucene::index