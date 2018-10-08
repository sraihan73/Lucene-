using namespace std;

#include "TextFragment.h"

namespace org::apache::lucene::search::highlight
{

TextFragment::TextFragment(shared_ptr<std::wstring> markedUpText,
                           int textStartPos, int fragNum)
{
  this->markedUpText = markedUpText;
  this->textStartPos = textStartPos;
  this->fragNum = fragNum;
}

void TextFragment::setScore(float score) { this->score = score; }

float TextFragment::getScore() { return score; }

void TextFragment::merge(shared_ptr<TextFragment> frag2)
{
  textEndPos = frag2->textEndPos;
  score = max(score, frag2->score);
}

bool TextFragment::follows(shared_ptr<TextFragment> fragment)
{
  return textStartPos == fragment->textEndPos;
}

int TextFragment::getFragNum() { return fragNum; }

wstring TextFragment::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return markedUpText->substr(textStartPos, textEndPos - textStartPos)
      ->toString();
}
} // namespace org::apache::lucene::search::highlight