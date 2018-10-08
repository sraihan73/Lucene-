using namespace std;

#include "Highlighter.h"

namespace org::apache::lucene::search::highlight
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

Highlighter::Highlighter(shared_ptr<Scorer> fragmentScorer)
    : Highlighter(new SimpleHTMLFormatter(), fragmentScorer)
{
}

Highlighter::Highlighter(shared_ptr<Formatter> formatter,
                         shared_ptr<Scorer> fragmentScorer)
    : Highlighter(formatter, new DefaultEncoder(), fragmentScorer)
{
}

Highlighter::Highlighter(shared_ptr<Formatter> formatter,
                         shared_ptr<Encoder> encoder,
                         shared_ptr<Scorer> fragmentScorer)
{
  ensureArgumentNotNull(formatter, L"'formatter' must not be null");
  ensureArgumentNotNull(encoder, L"'encoder' must not be null");
  ensureArgumentNotNull(fragmentScorer, L"'fragmentScorer' must not be null");

  this->formatter = formatter;
  this->encoder = encoder;
  this->fragmentScorer = fragmentScorer;
}

wstring Highlighter::getBestFragment(
    shared_ptr<Analyzer> analyzer, const wstring &fieldName,
    const wstring &text) 
{
  shared_ptr<TokenStream> tokenStream = analyzer->tokenStream(fieldName, text);
  return getBestFragment(tokenStream, text);
}

wstring Highlighter::getBestFragment(
    shared_ptr<TokenStream> tokenStream,
    const wstring &text) 
{
  std::deque<wstring> results = getBestFragments(tokenStream, text, 1);
  if (results.size() > 0) {
    return results[0];
  }
  return L"";
}

std::deque<wstring> Highlighter::getBestFragments(
    shared_ptr<Analyzer> analyzer, const wstring &fieldName,
    const wstring &text,
    int maxNumFragments) 
{
  shared_ptr<TokenStream> tokenStream = analyzer->tokenStream(fieldName, text);
  return getBestFragments(tokenStream, text, maxNumFragments);
}

std::deque<wstring> Highlighter::getBestFragments(
    shared_ptr<TokenStream> tokenStream, const wstring &text,
    int maxNumFragments) 
{
  maxNumFragments = max(1, maxNumFragments); // sanity check

  std::deque<std::shared_ptr<TextFragment>> frag =
      getBestTextFragments(tokenStream, text, true, maxNumFragments);

  // Get text
  deque<wstring> fragTexts = deque<wstring>();
  for (int i = 0; i < frag.size(); i++) {
    if ((frag[i] != nullptr) && (frag[i]->getScore() > 0)) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      fragTexts.push_back(frag[i]->toString());
    }
  }
  return fragTexts.toArray(std::deque<wstring>(0));
}

std::deque<std::shared_ptr<TextFragment>> Highlighter::getBestTextFragments(
    shared_ptr<TokenStream> tokenStream, const wstring &text,
    bool mergeContiguousFragments,
    int maxNumFragments) 
{
  deque<std::shared_ptr<TextFragment>> docFrags =
      deque<std::shared_ptr<TextFragment>>();
  shared_ptr<StringBuilder> newText = make_shared<StringBuilder>();

  shared_ptr<CharTermAttribute> termAtt =
      tokenStream->addAttribute(CharTermAttribute::typeid);
  shared_ptr<OffsetAttribute> offsetAtt =
      tokenStream->addAttribute(OffsetAttribute::typeid);
  shared_ptr<TextFragment> currentFrag =
      make_shared<TextFragment>(newText, newText->length(), docFrags.size());

  if (std::dynamic_pointer_cast<QueryScorer>(fragmentScorer) != nullptr) {
    (std::static_pointer_cast<QueryScorer>(fragmentScorer))
        ->setMaxDocCharsToAnalyze(maxDocCharsToAnalyze);
  }

  shared_ptr<TokenStream> newStream = fragmentScorer->init(tokenStream);
  if (newStream != nullptr) {
    tokenStream = newStream;
  }
  fragmentScorer->startFragment(currentFrag);
  docFrags.push_back(currentFrag);

  shared_ptr<FragmentQueue> fragQueue =
      make_shared<FragmentQueue>(maxNumFragments);

  try {

    wstring tokenText;
    int startOffset;
    int endOffset;
    int lastEndOffset = 0;
    textFragmenter->start(text, tokenStream);

    shared_ptr<TokenGroup> tokenGroup = make_shared<TokenGroup>(tokenStream);

    tokenStream->reset();
    for (bool next = tokenStream->incrementToken();
         next && (offsetAtt->startOffset() < maxDocCharsToAnalyze);
         next = tokenStream->incrementToken()) {
      if ((offsetAtt->endOffset() > text.length()) ||
          (offsetAtt->startOffset() > text.length())) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        throw make_shared<InvalidTokenOffsetsException>(
            L"Token " + termAtt->toString() +
            L" exceeds length of provided text sized " +
            to_wstring(text.length()));
      }
      if ((tokenGroup->getNumTokens() > 0) && (tokenGroup->isDistinct())) {
        // the current token is distinct from previous tokens -
        // markup the cached token group info
        startOffset = tokenGroup->getStartOffset();
        endOffset = tokenGroup->getEndOffset();
        tokenText = text.substr(startOffset, endOffset - startOffset);
        wstring markedUpText = formatter->highlightTerm(
            encoder->encodeText(tokenText), tokenGroup);
        // store any whitespace etc from between this and last group
        if (startOffset > lastEndOffset) {
          newText->append(encoder->encodeText(
              text.substr(lastEndOffset, startOffset - lastEndOffset)));
        }
        newText->append(markedUpText);
        lastEndOffset = max(endOffset, lastEndOffset);
        tokenGroup->clear();

        // check if current token marks the start of a new fragment
        if (textFragmenter->isNewFragment()) {
          currentFrag->setScore(fragmentScorer->getFragmentScore());
          // record stats for a new fragment
          currentFrag->textEndPos = newText->length();
          currentFrag = make_shared<TextFragment>(newText, newText->length(),
                                                  docFrags.size());
          fragmentScorer->startFragment(currentFrag);
          docFrags.push_back(currentFrag);
        }
      }

      tokenGroup->addToken(fragmentScorer->getTokenScore());

      //        if(lastEndOffset>maxDocBytesToAnalyze)
      //        {
      //          break;
      //        }
    }
    currentFrag->setScore(fragmentScorer->getFragmentScore());

    if (tokenGroup->getNumTokens() > 0) {
      // flush the accumulated text (same code as in above loop)
      startOffset = tokenGroup->getStartOffset();
      endOffset = tokenGroup->getEndOffset();
      tokenText = text.substr(startOffset, endOffset - startOffset);
      wstring markedUpText =
          formatter->highlightTerm(encoder->encodeText(tokenText), tokenGroup);
      // store any whitespace etc from between this and last group
      if (startOffset > lastEndOffset) {
        newText->append(encoder->encodeText(
            text.substr(lastEndOffset, startOffset - lastEndOffset)));
      }
      newText->append(markedUpText);
      lastEndOffset = max(lastEndOffset, endOffset);
    }

    // Test what remains of the original text beyond the point where we stopped
    // analyzing
    if ((lastEndOffset < text.length()) &&
        (text.length() <= maxDocCharsToAnalyze)) {
      // append it to the last fragment
      newText->append(encoder->encodeText(text.substr(lastEndOffset)));
    }

    currentFrag->textEndPos = newText->length();

    // sort the most relevant sections of the text
    for (deque<std::shared_ptr<TextFragment>>::const_iterator i =
             docFrags.begin();
         i != docFrags.end(); ++i) {
      currentFrag = *i;

      // If you are running with a version of Lucene before 11th Sept 03
      // you do not have PriorityQueue.insert() - so uncomment the code below
      /*
                if (currentFrag.getScore() >= minScore)
                {
                  fragQueue.put(currentFrag);
                  if (fragQueue.size() > maxNumFragments)
                  { // if hit queue overfull
                    fragQueue.pop(); // remove lowest in hit queue
                    minScore = ((TextFragment) fragQueue.top()).getScore(); //
         reset minScore
                  }


                }
      */
      // The above code caused a problem as a result of Christoph Goller's 11th
      // Sept 03 fix to PriorityQueue. The correct method to use here is the new
      // "insert" method
      // USE ABOVE CODE IF THIS DOES NOT COMPILE!
      fragQueue->insertWithOverflow(currentFrag);
    }

    // return the most relevant fragments
    std::deque<std::shared_ptr<TextFragment>> frag(fragQueue->size());
    for (int i = frag.size() - 1; i >= 0; i--) {
      frag[i] = fragQueue->pop();
    }

    // merge any contiguous fragments to improve readability
    if (mergeContiguousFragments) {
      this->mergeContiguousFragments(frag);
      deque<std::shared_ptr<TextFragment>> fragTexts =
          deque<std::shared_ptr<TextFragment>>();
      for (int i = 0; i < frag.size(); i++) {
        if ((frag[i] != nullptr) && (frag[i]->getScore() > 0)) {
          fragTexts.push_back(frag[i]);
        }
      }
      frag = fragTexts.toArray(std::deque<std::shared_ptr<TextFragment>>(0));
    }

    return frag;

  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (tokenStream != nullptr) {
      try {
        tokenStream->end();
        delete tokenStream;
      } catch (const runtime_error &e) {
      }
    }
  }
}

void Highlighter::mergeContiguousFragments(
    std::deque<std::shared_ptr<TextFragment>> &frag)
{
  bool mergingStillBeingDone;
  if (frag.size() > 1) {
    do {
      mergingStillBeingDone = false; // initialise loop control flag
      // for each fragment, scan other frags looking for contiguous blocks
      for (int i = 0; i < frag.size(); i++) {
        if (frag[i] == nullptr) {
          continue;
        }
        // merge any contiguous blocks
        for (int x = 0; x < frag.size(); x++) {
          if (frag[x] == nullptr) {
            continue;
          }
          if (frag[i] == nullptr) {
            break;
          }
          shared_ptr<TextFragment> frag1 = nullptr;
          shared_ptr<TextFragment> frag2 = nullptr;
          int frag1Num = 0;
          int frag2Num = 0;
          int bestScoringFragNum;
          int worstScoringFragNum;
          // if blocks are contiguous....
          if (frag[i]->follows(frag[x])) {
            frag1 = frag[x];
            frag1Num = x;
            frag2 = frag[i];
            frag2Num = i;
          } else {
            if (frag[x]->follows(frag[i])) {
              frag1 = frag[i];
              frag1Num = i;
              frag2 = frag[x];
              frag2Num = x;
            }
          }
          // merging required..
          if (frag1 != nullptr) {
            if (frag1->getScore() > frag2->getScore()) {
              bestScoringFragNum = frag1Num;
              worstScoringFragNum = frag2Num;
            } else {
              bestScoringFragNum = frag2Num;
              worstScoringFragNum = frag1Num;
            }
            frag1->merge(frag2);
            frag[worstScoringFragNum].reset();
            mergingStillBeingDone = true;
            frag[bestScoringFragNum] = frag1;
          }
        }
      }
    } while (mergingStillBeingDone);
  }
}

wstring Highlighter::getBestFragments(
    shared_ptr<TokenStream> tokenStream, const wstring &text,
    int maxNumFragments,
    const wstring &separator) 
{
  std::deque<wstring> sections =
      getBestFragments(tokenStream, text, maxNumFragments);
  shared_ptr<StringBuilder> result = make_shared<StringBuilder>();
  for (int i = 0; i < sections.size(); i++) {
    if (i > 0) {
      result->append(separator);
    }
    result->append(sections[i]);
  }
  return result->toString();
}

int Highlighter::getMaxDocCharsToAnalyze() { return maxDocCharsToAnalyze; }

void Highlighter::setMaxDocCharsToAnalyze(int maxDocCharsToAnalyze)
{
  this->maxDocCharsToAnalyze = maxDocCharsToAnalyze;
}

shared_ptr<Fragmenter> Highlighter::getTextFragmenter()
{
  return textFragmenter;
}

void Highlighter::setTextFragmenter(shared_ptr<Fragmenter> fragmenter)
{
  textFragmenter = Objects::requireNonNull(fragmenter);
}

shared_ptr<Scorer> Highlighter::getFragmentScorer() { return fragmentScorer; }

void Highlighter::setFragmentScorer(shared_ptr<Scorer> scorer)
{
  fragmentScorer = Objects::requireNonNull(scorer);
}

shared_ptr<Encoder> Highlighter::getEncoder() { return encoder; }

void Highlighter::setEncoder(shared_ptr<Encoder> encoder)
{
  this->encoder = Objects::requireNonNull(encoder);
}

void Highlighter::ensureArgumentNotNull(any argument, const wstring &message)
{
  if (argument == nullptr) {
    throw invalid_argument(message);
  }
}

Highlighter::FragmentQueue::FragmentQueue(int size)
    : org::apache::lucene::util::PriorityQueue<TextFragment>(size)
{
}

bool Highlighter::FragmentQueue::lessThan(shared_ptr<TextFragment> fragA,
                                          shared_ptr<TextFragment> fragB)
{
  if (fragA->getScore() == fragB->getScore()) {
    return fragA->fragNum > fragB->fragNum;
  } else {
    return fragA->getScore() < fragB->getScore();
  }
}
} // namespace org::apache::lucene::search::highlight