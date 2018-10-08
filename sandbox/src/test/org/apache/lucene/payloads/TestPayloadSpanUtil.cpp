using namespace std;

#include "TestPayloadSpanUtil.h"

namespace org::apache::lucene::payloads
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermQuery = org::apache::lucene::search::TermQuery;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
const wstring TestPayloadSpanUtil::FIELD = L"f";

void TestPayloadSpanUtil::testPayloadSpanUtil() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), directory,
      newIndexWriterConfig(make_shared<PayloadAnalyzer>())
          ->setSimilarity(make_shared<ClassicSimilarity>()));

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(FIELD, L"xx rr yy mm  pp", Field::Store::YES));
  writer->addDocument(doc);

  shared_ptr<IndexReader> reader = writer->getReader();
  delete writer;
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  shared_ptr<PayloadSpanUtil> psu =
      make_shared<PayloadSpanUtil>(searcher->getTopReaderContext());

  shared_ptr<deque<std::deque<char>>> payloads = psu->getPayloadsForQuery(
      make_shared<TermQuery>(make_shared<Term>(FIELD, L"rr")));
  if (VERBOSE) {
    wcout << L"Num payloads:" << payloads->size() << endl;
    for (auto bytes : payloads) {
      wcout << wstring(bytes, StandardCharsets::UTF_8) << endl;
    }
  }
  delete reader;
  delete directory;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPayloadSpanUtil::PayloadAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> result =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);
  return make_shared<Analyzer::TokenStreamComponents>(
      result, make_shared<PayloadFilter>(result));
}

TestPayloadSpanUtil::PayloadFilter::PayloadFilter(shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
  pos = 0;
  entities->add(L"xx");
  entities->add(L"one");
  nopayload->add(L"nopayload");
  nopayload->add(L"np");
  termAtt = addAttribute(CharTermAttribute::typeid);
  posIncrAtt = addAttribute(PositionIncrementAttribute::typeid);
  payloadAtt = addAttribute(PayloadAttribute::typeid);
}

bool TestPayloadSpanUtil::PayloadFilter::incrementToken() 
{
  if (input->incrementToken()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring token = termAtt->toString();

    if (!nopayload->contains(token)) {
      if (entities->contains(token)) {
        payloadAtt->setPayload(
            make_shared<BytesRef>(token + L":Entity:" + to_wstring(pos)));
      } else {
        payloadAtt->setPayload(
            make_shared<BytesRef>(token + L":Noise:" + to_wstring(pos)));
      }
    }
    pos += posIncrAtt->getPositionIncrement();
    return true;
  }
  return false;
}

void TestPayloadSpanUtil::PayloadFilter::reset() 
{
  TokenFilter::reset();
  this->pos = 0;
}
} // namespace org::apache::lucene::payloads