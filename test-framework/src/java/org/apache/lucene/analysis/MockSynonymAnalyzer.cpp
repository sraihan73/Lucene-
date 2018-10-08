using namespace std;

#include "MockSynonymAnalyzer.h"

namespace org::apache::lucene::analysis
{

shared_ptr<TokenStreamComponents>
MockSynonymAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<MockTokenizer> tokenizer = make_shared<MockTokenizer>();
  return make_shared<TokenStreamComponents>(
      tokenizer, make_shared<MockSynonymFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis