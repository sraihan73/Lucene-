using namespace std;

#include "Test2BPostingsBytes.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using Codec = org::apache::lucene::codecs::Codec;
using CompressingCodec =
    org::apache::lucene::codecs::compressing::CompressingCodec;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::apache::lucene::util::LuceneTestCase::Monster;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;

void Test2BPostingsBytes::test() 
{
  shared_ptr<IndexWriterConfig> defaultConfig =
      make_shared<IndexWriterConfig>(nullptr);
  shared_ptr<Codec> defaultCodec = defaultConfig->getCodec();
  if (std::dynamic_pointer_cast<CompressingCodec>(
          (make_shared<IndexWriterConfig>(nullptr))->getCodec()) != nullptr) {
    shared_ptr<Pattern> regex =
        Pattern::compile(L"maxDocsPerChunk=(\\d+), blockSize=(\\d+)");
    // C++ TODO: There is no native C++ equivalent to 'toString':
    shared_ptr<Matcher> matcher = regex->matcher(defaultCodec->toString());
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertTrue(L"Unexpected CompressingCodec toString() output: " +
                   defaultCodec->toString(),
               matcher->find());
    int maxDocsPerChunk = static_cast<Integer>(matcher->group(1));
    int blockSize = static_cast<Integer>(matcher->group(2));
    int product = maxDocsPerChunk * blockSize;
    assumeTrue(defaultCodec->getName() + L" maxDocsPerChunk (" +
                   to_wstring(maxDocsPerChunk) + L") * blockSize (" +
                   to_wstring(blockSize) +
                   L") < 16 - this can trigger OOM with -Dtests.heapsize=30g",
               product >= 16);
  }

  shared_ptr<BaseDirectoryWrapper> dir =
      newFSDirectory(createTempDir(L"2BPostingsBytes1"));
  if (std::dynamic_pointer_cast<MockDirectoryWrapper>(dir) != nullptr) {
    (std::static_pointer_cast<MockDirectoryWrapper>(dir))
        ->setThrottling(MockDirectoryWrapper::Throttling::NEVER);
  }

  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()))
               .setMaxBufferedDocs(IndexWriterConfig::DISABLE_AUTO_FLUSH)
               .setRAMBufferSizeMB(256.0)
               .setMergeScheduler(make_shared<ConcurrentMergeScheduler>())
               .setMergePolicy(newLogMergePolicy(false, 10))
               .setOpenMode(IndexWriterConfig::OpenMode::CREATE)
               .setCodec(TestUtil::getDefaultCodec()));

  shared_ptr<MergePolicy> mp = w->getConfig()->getMergePolicy();
  if (std::dynamic_pointer_cast<LogByteSizeMergePolicy>(mp) != nullptr) {
    // 1 petabyte:
    (std::static_pointer_cast<LogByteSizeMergePolicy>(mp))
        ->setMaxMergeMB(1024 * 1024 * 1024);
  }

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setIndexOptions(IndexOptions::DOCS_AND_FREQS);
  ft->setOmitNorms(true);
  shared_ptr<MyTokenStream> tokenStream = make_shared<MyTokenStream>();
  shared_ptr<Field> field = make_shared<Field>(L"field", tokenStream, ft);
  doc->push_back(field);

  constexpr int numDocs = 1000;
  for (int i = 0; i < numDocs; i++) {
    if (i % 2 == 1) { // trick blockPF's little optimization
      tokenStream->n = 65536;
    } else {
      tokenStream->n = 65537;
    }
    w->addDocument(doc);
  }
  w->forceMerge(1);
  delete w;

  shared_ptr<DirectoryReader> oneThousand = DirectoryReader::open(dir);
  std::deque<std::shared_ptr<DirectoryReader>> subReaders(1000);
  Arrays::fill(subReaders, oneThousand);
  shared_ptr<BaseDirectoryWrapper> dir2 =
      newFSDirectory(createTempDir(L"2BPostingsBytes2"));
  if (std::dynamic_pointer_cast<MockDirectoryWrapper>(dir2) != nullptr) {
    (std::static_pointer_cast<MockDirectoryWrapper>(dir2))
        ->setThrottling(MockDirectoryWrapper::Throttling::NEVER);
  }
  shared_ptr<IndexWriter> w2 =
      make_shared<IndexWriter>(dir2, make_shared<IndexWriterConfig>(nullptr));
  TestUtil::addIndexesSlowly(w2, subReaders);
  w2->forceMerge(1);
  delete w2;
  oneThousand->close();

  shared_ptr<DirectoryReader> oneMillion = DirectoryReader::open(dir2);
  subReaders = std::deque<std::shared_ptr<DirectoryReader>>(2000);
  Arrays::fill(subReaders, oneMillion);
  shared_ptr<BaseDirectoryWrapper> dir3 =
      newFSDirectory(createTempDir(L"2BPostingsBytes3"));
  if (std::dynamic_pointer_cast<MockDirectoryWrapper>(dir3) != nullptr) {
    (std::static_pointer_cast<MockDirectoryWrapper>(dir3))
        ->setThrottling(MockDirectoryWrapper::Throttling::NEVER);
  }
  shared_ptr<IndexWriter> w3 =
      make_shared<IndexWriter>(dir3, make_shared<IndexWriterConfig>(nullptr));
  TestUtil::addIndexesSlowly(w3, subReaders);
  w3->forceMerge(1);
  delete w3;
  oneMillion->close();

  delete dir;
  delete dir2;
  delete dir3;
}

bool Test2BPostingsBytes::MyTokenStream::incrementToken()
{
  if (index < n) {
    clearAttributes();
    termAtt->buffer()[0] = L'a';
    termAtt->setLength(1);
    index++;
    return true;
  }
  return false;
}

void Test2BPostingsBytes::MyTokenStream::reset() { index = 0; }
} // namespace org::apache::lucene::index