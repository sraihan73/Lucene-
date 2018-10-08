using namespace std;

#include "CompositeBreakIterator.h"
#include "BreakIteratorWrapper.h"
#include "ICUTokenizerConfig.h"
#include "ScriptIterator.h"

namespace org::apache::lucene::analysis::icu::segmentation
{
using com::ibm::icu::lang::UCharacter;
using com::ibm::icu::lang::UProperty;
using com::ibm::icu::lang::UScript;
using com::ibm::icu::text::BreakIterator;

CompositeBreakIterator::CompositeBreakIterator(
    shared_ptr<ICUTokenizerConfig> config)
    : config(config),
      scriptIterator(make_shared<ScriptIterator>(config->combineCJ()))
{
}

int CompositeBreakIterator::next()
{
  int next = rbbi->next();
  while (next == BreakIterator::DONE && scriptIterator->next()) {
    rbbi = getBreakIterator(scriptIterator->getScriptCode());
    rbbi->setText(text, scriptIterator->getScriptStart(),
                  scriptIterator->getScriptLimit() -
                      scriptIterator->getScriptStart());
    next = rbbi->next();
  }
  return (next == BreakIterator::DONE)
             ? BreakIterator::DONE
             : next + scriptIterator->getScriptStart();
}

int CompositeBreakIterator::current()
{
  constexpr int current = rbbi->current();
  return (current == BreakIterator::DONE)
             ? BreakIterator::DONE
             : current + scriptIterator->getScriptStart();
}

int CompositeBreakIterator::getRuleStatus() { return rbbi->getRuleStatus(); }

int CompositeBreakIterator::getScriptCode()
{
  return scriptIterator->getScriptCode();
}

void CompositeBreakIterator::setText(std::deque<wchar_t> &text, int start,
                                     int length)
{
  this->text = text;
  scriptIterator->setText(text, start, length);
  if (scriptIterator->next()) {
    rbbi = getBreakIterator(scriptIterator->getScriptCode());
    rbbi->setText(text, scriptIterator->getScriptStart(),
                  scriptIterator->getScriptLimit() -
                      scriptIterator->getScriptStart());
  } else {
    rbbi = getBreakIterator(UScript::COMMON);
    rbbi->setText(text, 0, 0);
  }
}

shared_ptr<BreakIteratorWrapper>
CompositeBreakIterator::getBreakIterator(int scriptCode)
{
  if (wordBreakers[scriptCode] == nullptr) {
    wordBreakers[scriptCode] =
        make_shared<BreakIteratorWrapper>(config->getBreakIterator(scriptCode));
  }
  return wordBreakers[scriptCode];
}
} // namespace org::apache::lucene::analysis::icu::segmentation