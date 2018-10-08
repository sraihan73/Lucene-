using namespace std;

#include "MorfologikFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRefBuilder.h"
#include "MorphosyntacticTagsAttribute.h"

namespace org::apache::lucene::analysis::morfologik
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using morfologik::stemming::Dictionary;
using morfologik::stemming::DictionaryLookup;
using morfologik::stemming::IStemmer;
using morfologik::stemming::WordData;
using morfologik::stemming::polish::PolishStemmer;

MorfologikFilter::MorfologikFilter(shared_ptr<TokenStream> in_)
    : MorfologikFilter(in_, new PolishStemmer().getDictionary())
{
}

MorfologikFilter::MorfologikFilter(shared_ptr<TokenStream> in_,
                                   shared_ptr<Dictionary> dict)
    : org::apache::lucene::analysis::TokenFilter(in_), input(in_),
      stemmer(make_shared<DictionaryLookup>(dict))
{
  this->lemmaList = Collections::emptyList();
}

const shared_ptr<java::util::regex::Pattern> MorfologikFilter::lemmaSplitter =
    java::util::regex::Pattern::compile(L"\\+|\\|");

void MorfologikFilter::popNextLemma()
{
  // One tag (concatenated) per lemma.
  shared_ptr<WordData> *const lemma = lemmaList[lemmaListIndex++];
  termAtt->setEmpty()->append(lemma->getStem());
  shared_ptr<std::wstring> tag = lemma->getTag();
  if (tag != nullptr) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    std::deque<wstring> tags = lemmaSplitter->split(tag->toString());
    for (int i = 0; i < tags.size(); i++) {
      if (tagsList.size() <= i) {
        tagsList.push_back(make_shared<StringBuilder>());
      }
      shared_ptr<StringBuilder> buffer = tagsList[i];
      buffer->setLength(0);
      buffer->append(tags[i]);
    }
    tagsAtt->setTags(tagsList.subList(0, tags.size()));
  } else {
    tagsAtt->setTags(Collections::emptyList<std::shared_ptr<StringBuilder>>());
  }
}

bool MorfologikFilter::lookupSurfaceForm(shared_ptr<std::wstring> token)
{
  lemmaList = this->stemmer->lookup(token);
  lemmaListIndex = 0;
  return lemmaList.size() > 0;
}

bool MorfologikFilter::incrementToken() 
{
  if (lemmaListIndex < lemmaList.size()) {
    restoreState(current);
    posIncrAtt->setPositionIncrement(0);
    popNextLemma();
    return true;
  } else if (this->input->incrementToken()) {
    if (!keywordAttr->isKeyword() &&
        (lookupSurfaceForm(termAtt) ||
         lookupSurfaceForm(toLowercase(termAtt)))) {
      current = captureState();
      popNextLemma();
    } else {
      tagsAtt->clear();
    }
    return true;
  } else {
    return false;
  }
}

shared_ptr<std::wstring>
MorfologikFilter::toLowercase(shared_ptr<std::wstring> chs)
{
  constexpr int length = chs->length();
  scratch->setLength(length);
  scratch->grow(length);

  std::deque<wchar_t> buffer = scratch->chars();
  for (int i = 0; i < length;) {
    i += Character::toChars(towlower(Character::codePointAt(chs, i)), buffer, i);
  }

  return scratch->get();
}

void MorfologikFilter::reset() 
{
  lemmaListIndex = 0;
  lemmaList = Collections::emptyList();
  tagsList.clear();
  TokenFilter::reset();
}
} // namespace org::apache::lucene::analysis::morfologik