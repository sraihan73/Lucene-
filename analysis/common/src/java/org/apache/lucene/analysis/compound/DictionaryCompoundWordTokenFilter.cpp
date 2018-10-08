using namespace std;

#include "DictionaryCompoundWordTokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"

namespace org::apache::lucene::analysis::compound
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

DictionaryCompoundWordTokenFilter::DictionaryCompoundWordTokenFilter(
    shared_ptr<TokenStream> input, shared_ptr<CharArraySet> dictionary)
    : CompoundWordTokenFilterBase(input, dictionary)
{
  if (dictionary == nullptr) {
    throw invalid_argument(L"dictionary must not be null");
  }
}

DictionaryCompoundWordTokenFilter::DictionaryCompoundWordTokenFilter(
    shared_ptr<TokenStream> input, shared_ptr<CharArraySet> dictionary,
    int minWordSize, int minSubwordSize, int maxSubwordSize,
    bool onlyLongestMatch)
    : CompoundWordTokenFilterBase(input, dictionary, minWordSize,
                                  minSubwordSize, maxSubwordSize,
                                  onlyLongestMatch)
{
  if (dictionary == nullptr) {
    throw invalid_argument(L"dictionary must not be null");
  }
}

void DictionaryCompoundWordTokenFilter::decompose()
{
  constexpr int len = termAtt->length();
  for (int i = 0; i <= len - this->minSubwordSize; ++i) {
    shared_ptr<CompoundToken> longestMatchToken = nullptr;
    for (int j = this->minSubwordSize; j <= this->maxSubwordSize; ++j) {
      if (i + j > len) {
        break;
      }
      if (dictionary->contains(termAtt->buffer(), i, j)) {
        if (this->onlyLongestMatch) {
          if (longestMatchToken != nullptr) {
            if (longestMatchToken->txt->length() < j) {
              longestMatchToken =
                  make_shared<CompoundToken>(shared_from_this(), i, j);
            }
          } else {
            longestMatchToken =
                make_shared<CompoundToken>(shared_from_this(), i, j);
          }
        } else {
          tokens.push_back(
              make_shared<CompoundToken>(shared_from_this(), i, j));
        }
      }
    }
    if (this->onlyLongestMatch && longestMatchToken != nullptr) {
      tokens.push_back(longestMatchToken);
    }
  }
}
} // namespace org::apache::lucene::analysis::compound