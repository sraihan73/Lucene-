using namespace std;

#include "Test2BPostings.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TimeUnits = org::apache::lucene::util::TimeUnits;
using com::carrotsearch::randomizedtesting::annotations::TimeoutSuite;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void test() throws Exception
void Test2BPostings::test() 
{
  shared_ptr<BaseDirectoryWrapper> dir =
      newFSDirectory(createTempDir(L"2BPostings"));
  if (std::dynamic_pointer_cast<MockDirectoryWrapper>(dir) != nullptr) {
    (std::static_pointer_cast<MockDirectoryWrapper>(dir))
        ->setThrottling(MockDirectoryWrapper::Throttling::NEVER);
  }

  shared_ptr<IndexWriterConfig> iwc =
      (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
          ->setMaxBufferedDocs(IndexWriterConfig::DISABLE_AUTO_FLUSH)
          ->setRAMBufferSizeMB(256.0)
          ->setMergeScheduler(make_shared<ConcurrentMergeScheduler>())
          ->setMergePolicy(newLogMergePolicy(false, 10))
          ->setOpenMode(IndexWriterConfig::OpenMode::CREATE);

  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);

  shared_ptr<MergePolicy> mp = w->getConfig()->getMergePolicy();
  if (std::dynamic_pointer_cast<LogByteSizeMergePolicy>(mp) != nullptr) {
    // 1 petabyte:
    (std::static_pointer_cast<LogByteSizeMergePolicy>(mp))
        ->setMaxMergeMB(1024 * 1024 * 1024);
  }

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setOmitNorms(true);
  ft->setIndexOptions(IndexOptions::DOCS);
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

bool Test2BPostings::MyTokenStream::incrementToken()
{
  if (index <= L'z') {
    clearAttributes();
    termAtt->setLength(1);
    termAtt->buffer()[0] = static_cast<wchar_t>(index)++;
    return true;
  }
  return false;
}

void Test2BPostings::MyTokenStream::reset() { index = L'a'; }
} // namespace org::apache::lucene::index