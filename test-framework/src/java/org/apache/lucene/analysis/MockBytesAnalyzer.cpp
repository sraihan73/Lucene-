using namespace std;

#include "MockBytesAnalyzer.h"

namespace org::apache::lucene::analysis
{
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

shared_ptr<TokenStreamComponents>
MockBytesAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> t = make_shared<MockTokenizer>(
      MockUTF16TermAttributeImpl::UTF16_TERM_ATTRIBUTE_FACTORY,
      MockTokenizer::KEYWORD, false, MockTokenizer::DEFAULT_MAX_TOKEN_LENGTH);
  return make_shared<TokenStreamComponents>(t);
}

shared_ptr<AttributeFactory>
MockBytesAnalyzer::attributeFactory(const wstring &fieldName)
{
  return MockUTF16TermAttributeImpl::UTF16_TERM_ATTRIBUTE_FACTORY;
}
} // namespace org::apache::lucene::analysis