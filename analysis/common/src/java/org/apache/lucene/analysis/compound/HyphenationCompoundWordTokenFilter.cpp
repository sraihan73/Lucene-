using namespace std;

#include "HyphenationCompoundWordTokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "hyphenation/Hyphenation.h"
#include "hyphenation/HyphenationTree.h"

namespace org::apache::lucene::analysis::compound
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Hyphenation =
    org::apache::lucene::analysis::compound::hyphenation::Hyphenation;
using HyphenationTree =
    org::apache::lucene::analysis::compound::hyphenation::HyphenationTree;
using org::xml::sax::InputSource;

HyphenationCompoundWordTokenFilter::HyphenationCompoundWordTokenFilter(
    shared_ptr<TokenStream> input, shared_ptr<HyphenationTree> hyphenator,
    shared_ptr<CharArraySet> dictionary)
    : HyphenationCompoundWordTokenFilter(
          input, hyphenator, dictionary, DEFAULT_MIN_WORD_SIZE,
          DEFAULT_MIN_SUBWORD_SIZE, DEFAULT_MAX_SUBWORD_SIZE, false)
{
}

HyphenationCompoundWordTokenFilter::HyphenationCompoundWordTokenFilter(
    shared_ptr<TokenStream> input, shared_ptr<HyphenationTree> hyphenator,
    shared_ptr<CharArraySet> dictionary, int minWordSize, int minSubwordSize,
    int maxSubwordSize, bool onlyLongestMatch)
    : CompoundWordTokenFilterBase(input, dictionary, minWordSize,
                                  minSubwordSize, maxSubwordSize,
                                  onlyLongestMatch)
{

  this->hyphenator = hyphenator;
}

HyphenationCompoundWordTokenFilter::HyphenationCompoundWordTokenFilter(
    shared_ptr<TokenStream> input, shared_ptr<HyphenationTree> hyphenator,
    int minWordSize, int minSubwordSize, int maxSubwordSize)
    : HyphenationCompoundWordTokenFilter(input, hyphenator, nullptr,
                                         minWordSize, minSubwordSize,
                                         maxSubwordSize, false)
{
}

HyphenationCompoundWordTokenFilter::HyphenationCompoundWordTokenFilter(
    shared_ptr<TokenStream> input, shared_ptr<HyphenationTree> hyphenator)
    : HyphenationCompoundWordTokenFilter(
          input, hyphenator, DEFAULT_MIN_WORD_SIZE, DEFAULT_MIN_SUBWORD_SIZE,
          DEFAULT_MAX_SUBWORD_SIZE)
{
}

shared_ptr<HyphenationTree>
HyphenationCompoundWordTokenFilter::getHyphenationTree(
    const wstring &hyphenationFilename) 
{
  return getHyphenationTree(make_shared<InputSource>(hyphenationFilename));
}

shared_ptr<HyphenationTree>
HyphenationCompoundWordTokenFilter::getHyphenationTree(
    shared_ptr<InputSource> hyphenationSource) 
{
  shared_ptr<HyphenationTree> tree = make_shared<HyphenationTree>();
  tree->loadPatterns(hyphenationSource);
  return tree;
}

void HyphenationCompoundWordTokenFilter::decompose()
{
  // get the hyphenation points
  shared_ptr<Hyphenation> hyphens =
      hyphenator->hyphenate(termAtt->buffer(), 0, termAtt->length(), 1, 1);
  // No hyphen points found -> exit
  if (hyphens == nullptr) {
    return;
  }

  const std::deque<int> hyp = hyphens->getHyphenationPoints();

  for (int i = 0; i < hyp.size(); ++i) {
    int remaining = hyp.size() - i;
    int start = hyp[i];
    shared_ptr<CompoundToken> longestMatchToken = nullptr;
    for (int j = 1; j < remaining; j++) {
      int partLength = hyp[i + j] - start;

      // if the part is longer than maxSubwordSize we
      // are done with this round
      if (partLength > this->maxSubwordSize) {
        break;
      }

      // we only put subwords to the token stream
      // that are longer than minPartSize
      if (partLength < this->minSubwordSize) {
        // BOGUS/BROKEN/FUNKY/WACKO: somehow we have negative 'parts' according
        // to the calculation above, and we rely upon minSubwordSize being >=0
        // to filter them out...
        continue;
      }

      // check the dictionary
      if (dictionary == nullptr ||
          dictionary->contains(termAtt->buffer(), start, partLength)) {
        if (this->onlyLongestMatch) {
          if (longestMatchToken != nullptr) {
            if (longestMatchToken->txt->length() < partLength) {
              longestMatchToken = make_shared<CompoundToken>(shared_from_this(),
                                                             start, partLength);
            }
          } else {
            longestMatchToken = make_shared<CompoundToken>(shared_from_this(),
                                                           start, partLength);
          }
        } else {
          tokens.push_back(make_shared<CompoundToken>(shared_from_this(), start,
                                                      partLength));
        }
      } else if (dictionary->contains(termAtt->buffer(), start,
                                      partLength - 1)) {
        // check the dictionary again with a word that is one character
        // shorter
        // to avoid problems with genitive 's characters and other binding
        // characters
        if (this->onlyLongestMatch) {
          if (longestMatchToken != nullptr) {
            if (longestMatchToken->txt->length() < partLength - 1) {
              longestMatchToken = make_shared<CompoundToken>(
                  shared_from_this(), start, partLength - 1);
            }
          } else {
            longestMatchToken = make_shared<CompoundToken>(
                shared_from_this(), start, partLength - 1);
          }
        } else {
          tokens.push_back(make_shared<CompoundToken>(shared_from_this(), start,
                                                      partLength - 1));
        }
      }
    }
    if (this->onlyLongestMatch && longestMatchToken != nullptr) {
      tokens.push_back(longestMatchToken);
    }
  }
}
} // namespace org::apache::lucene::analysis::compound