using namespace std;

#include "SnowballFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "../../../../tartarus/snowball/SnowballProgram.h"

namespace org::apache::lucene::analysis::snowball
{
using LowerCaseFilter = org::apache::lucene::analysis::LowerCaseFilter;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;
using TurkishLowerCaseFilter =
    org::apache::lucene::analysis::tr::TurkishLowerCaseFilter;
using SnowballProgram = org::tartarus::snowball::SnowballProgram;

SnowballFilter::SnowballFilter(shared_ptr<TokenStream> input,
                               shared_ptr<SnowballProgram> stemmer)
    : org::apache::lucene::analysis::TokenFilter(input), stemmer(stemmer)
{
}

SnowballFilter::SnowballFilter(shared_ptr<TokenStream> in_, const wstring &name)
    : org::apache::lucene::analysis::TokenFilter(in_)
{
  // Class.forName is frowned upon in place of the ResourceLoader but in this
  // case,
  // the factory will use the other constructor so that the program is already
  // loaded.
  try {
    type_info stemClass =
        type_info::forName(L"org.tartarus.snowball.ext." + name + L"Stemmer")
            .asSubclass(SnowballProgram::typeid);
    stemmer = stemClass.newInstance();
  } catch (const runtime_error &e) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new IllegalArgumentException("Invalid stemmer class
    // specified: " + name, e);
    throw invalid_argument(L"Invalid stemmer class specified: " + name);
  }
}

bool SnowballFilter::incrementToken() 
{
  if (input->incrementToken()) {
    if (!keywordAttr->isKeyword()) {
      std::deque<wchar_t> termBuffer = termAtt->buffer();
      constexpr int length = termAtt->length();
      stemmer->setCurrent(termBuffer, length);
      stemmer->stem();
      const std::deque<wchar_t> finalTerm = stemmer->getCurrentBuffer();
      constexpr int newLength = stemmer->getCurrentBufferLength();
      if (finalTerm != termBuffer) {
        termAtt->copyBuffer(finalTerm, 0, newLength);
      } else {
        termAtt->setLength(newLength);
      }
    }
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis::snowball