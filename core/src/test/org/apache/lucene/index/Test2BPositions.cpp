using namespace std;

#include "Test2BPositions.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
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

void Test2BPositions::test() 
{
  shared_ptr<BaseDirectoryWrapper> dir =
      newFSDirectory(createTempDir(L"2BPositions"));
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
  ft->setOmitNorms(true);
  shared_ptr<Field> field =
      make_shared<Field>(L"field", make_shared<MyTokenStream>(), ft);
  doc->push_back(field);

  constexpr int numDocs = (numeric_limits<int>::max() / 26) + 1;
  for (int i = 0; i < numDocs; i++) {
    w->addDocument(doc);
    if (VERBOSE && i % 100000 == 0) {
      wcout << i << L" of " << numDocs << L"..." << endl;
    }
  }
  w->forceMerge(1);
  delete w;
  delete dir;
}

bool Test2BPositions::MyTokenStream::incrementToken()
{
  if (index < 52) {
    clearAttributes();
    termAtt->setLength(1);
    termAtt->buffer()[0] = L'a';
    posIncAtt->setPositionIncrement(1 + index);
    index++;
    return true;
  }
  return false;
}

void Test2BPositions::MyTokenStream::reset() { index = 0; }
} // namespace org::apache::lucene::index