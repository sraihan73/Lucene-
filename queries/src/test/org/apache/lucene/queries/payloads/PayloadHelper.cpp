using namespace std;

#include "PayloadHelper.h"

namespace org::apache::lucene::queries::payloads
{
using namespace org::apache::lucene::analysis;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using TextField = org::apache::lucene::document::TextField;
using BytesRef = org::apache::lucene::util::BytesRef;
using English = org::apache::lucene::util::English;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
const wstring PayloadHelper::NO_PAYLOAD_FIELD = L"noPayloadField";
const wstring PayloadHelper::MULTI_FIELD = L"multiField";
const wstring PayloadHelper::FIELD = L"field";

PayloadHelper::PayloadAnalyzer::PayloadAnalyzer(
    shared_ptr<PayloadHelper> outerInstance)
    : Analyzer(PER_FIELD_REUSE_STRATEGY), outerInstance(outerInstance)
{
}

shared_ptr<TokenStreamComponents>
PayloadHelper::PayloadAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> result =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);
  return make_shared<TokenStreamComponents>(
      result, make_shared<PayloadFilter>(outerInstance, result, fieldName));
}

PayloadHelper::PayloadFilter::PayloadFilter(
    shared_ptr<PayloadHelper> outerInstance, shared_ptr<TokenStream> input,
    const wstring &fieldName)
    : TokenFilter(input), fieldName(fieldName),
      payloadAtt(addAttribute(PayloadAttribute::typeid)),
      outerInstance(outerInstance)
{
}

bool PayloadHelper::PayloadFilter::incrementToken() 
{

  if (input->incrementToken()) {
    if (fieldName == FIELD) {
      payloadAtt->setPayload(
          make_shared<BytesRef>(outerInstance->payloadField));
    } else if (fieldName == MULTI_FIELD) {
      if (numSeen % 2 == 0) {
        payloadAtt->setPayload(
            make_shared<BytesRef>(outerInstance->payloadMultiField1));
      } else {
        payloadAtt->setPayload(
            make_shared<BytesRef>(outerInstance->payloadMultiField2));
      }
      numSeen++;
    }
    return true;
  }
  return false;
}

void PayloadHelper::PayloadFilter::reset() 
{
  TokenFilter::reset();
  this->numSeen = 0;
}

shared_ptr<IndexSearcher>
PayloadHelper::setUp(shared_ptr<Random> random,
                     shared_ptr<Similarity> similarity,
                     int numDocs) 
{
  shared_ptr<Directory> directory =
      make_shared<MockDirectoryWrapper>(random, make_shared<RAMDirectory>());
  shared_ptr<PayloadAnalyzer> analyzer =
      make_shared<PayloadAnalyzer>(shared_from_this());

  // TODO randomize this
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      directory,
      (make_shared<IndexWriterConfig>(analyzer))->setSimilarity(similarity));
  // writer.infoStream = System.out;
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<TextField>(FIELD, English::intToEnglish(i),
                                          Field::Store::YES));
    doc->push_back(make_shared<TextField>(MULTI_FIELD,
                                          English::intToEnglish(i) + L"  " +
                                              English::intToEnglish(i),
                                          Field::Store::YES));
    doc->push_back(make_shared<TextField>(
        NO_PAYLOAD_FIELD, English::intToEnglish(i), Field::Store::YES));
    writer->addDocument(doc);
  }
  writer->forceMerge(1);
  reader = DirectoryReader::open(writer);
  delete writer;

  shared_ptr<IndexSearcher> searcher =
      LuceneTestCase::newSearcher(LuceneTestCase::getOnlyLeafReader(reader));
  searcher->setSimilarity(similarity);
  return searcher;
}

void PayloadHelper::tearDown()  { delete reader; }
} // namespace org::apache::lucene::queries::payloads