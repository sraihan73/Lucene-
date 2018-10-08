using namespace std;

#include "TestTermdocPerf.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

RepeatingTokenizer::RepeatingTokenizer(const wstring &val,
                                       shared_ptr<Random> random,
                                       float percentDocs, int maxTF)
    : org::apache::lucene::analysis::Tokenizer(), random(random),
      percentDocs(percentDocs), maxTF(maxTF)
{
  this->value = val;
  this->termAtt = addAttribute(CharTermAttribute::typeid);
}

bool RepeatingTokenizer::incrementToken() 
{
  num--;
  if (num >= 0) {
    clearAttributes();
    termAtt->append(value);
    return true;
  }
  return false;
}

void RepeatingTokenizer::reset() 
{
  Tokenizer::reset();
  if (random->nextFloat() < percentDocs) {
    num = random->nextInt(maxTF) + 1;
  } else {
    num = 0;
  }
}

void TestTermdocPerf::addDocs(shared_ptr<Random> random,
                              shared_ptr<Directory> dir, int const ndocs,
                              const wstring &field, const wstring &val,
                              int const maxTF,
                              float const percentDocs) 
{

  shared_ptr<Analyzer> analyzer = make_shared<AnalyzerAnonymousInnerClass>(
      shared_from_this(), random, val, maxTF, percentDocs);

  shared_ptr<Document> doc = make_shared<Document>();

  doc->push_back(newStringField(field, val, Field::Store::NO));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(analyzer)
               ->setOpenMode(OpenMode::CREATE)
               ->setMaxBufferedDocs(100)
               ->setMergePolicy(newLogMergePolicy(100)));

  for (int i = 0; i < ndocs; i++) {
    writer->addDocument(doc);
  }

  writer->forceMerge(1);
  delete writer;
}

TestTermdocPerf::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestTermdocPerf> outerInstance, shared_ptr<Random> random,
    const wstring &val, int maxTF, float percentDocs)
{
  this->outerInstance = outerInstance;
  this->random = random;
  this->val = val;
  this->maxTF = maxTF;
  this->percentDocs = percentDocs;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestTermdocPerf::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<RepeatingTokenizer>(val, random, percentDocs, maxTF));
}

int TestTermdocPerf::doTest(int iter, int ndocs, int maxTF,
                            float percentDocs) 
{
  shared_ptr<Directory> dir = newDirectory();

  int64_t start = System::currentTimeMillis();
  addDocs(random(), dir, ndocs, L"foo", L"val", maxTF, percentDocs);
  int64_t end = System::currentTimeMillis();
  if (VERBOSE) {
    wcout << L"milliseconds for creation of " << ndocs << L" docs = "
          << (end - start) << endl;
  }

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);

  shared_ptr<TermsEnum> tenum = MultiFields::getTerms(reader, L"foo")->begin();

  start = System::currentTimeMillis();

  int ret = 0;
  shared_ptr<PostingsEnum> tdocs = nullptr;
  shared_ptr<Random> *const random =
      make_shared<Random>(TestTermdocPerf::random()->nextLong());
  for (int i = 0; i < iter; i++) {
    tenum->seekCeil(make_shared<BytesRef>(L"val"));
    tdocs = TestUtil::docs(random, tenum, tdocs, PostingsEnum::NONE);
    while (tdocs->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
      ret += tdocs->docID();
    }
  }

  end = System::currentTimeMillis();
  if (VERBOSE) {
    wcout << L"milliseconds for " << iter << L" TermDocs iteration: "
          << (end - start) << endl;
  }

  return ret;
}

void TestTermdocPerf::testTermDocPerf() 
{
  // performance test for 10% of documents containing a term
  // doTest(100000, 10000,3,.1f);
}
} // namespace org::apache::lucene::index