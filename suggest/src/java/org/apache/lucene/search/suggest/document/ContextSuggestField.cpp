using namespace std;

#include "ContextSuggestField.h"

namespace org::apache::lucene::search::suggest::document
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

ContextSuggestField::ContextSuggestField(const wstring &name,
                                         const wstring &value, int weight,
                                         deque<std::wstring> &contexts)
    : SuggestField(name, value, weight),
      contexts(unordered_set<>((contexts_ != nullptr) ? contexts_->length : 0))
{
  validate(value);
  if (contexts_ != nullptr) {
    Collections::addAll(this->contexts_, contexts_);
  }
}

deque<std::shared_ptr<std::wstring>> ContextSuggestField::contexts()
{
  return contexts_;
}

shared_ptr<CompletionTokenStream>
ContextSuggestField::wrapTokenStream(shared_ptr<TokenStream> stream)
{
  const deque<std::shared_ptr<std::wstring>> contexts = this->contexts();
  for (auto context : contexts) {
    validate(context);
  }
  shared_ptr<CompletionTokenStream> completionTokenStream;
  if (std::dynamic_pointer_cast<CompletionTokenStream>(stream) != nullptr) {
    // TODO this is awkward; is there a better way avoiding re-creating the
    // chain?
    completionTokenStream =
        std::static_pointer_cast<CompletionTokenStream>(stream);
    shared_ptr<PrefixTokenFilter> prefixTokenFilter =
        make_shared<PrefixTokenFilter>(completionTokenStream->inputTokenStream,
                                       static_cast<wchar_t>(CONTEXT_SEPARATOR),
                                       contexts);
    completionTokenStream = make_shared<CompletionTokenStream>(
        prefixTokenFilter, completionTokenStream->preserveSep,
        completionTokenStream->preservePositionIncrements,
        completionTokenStream->maxGraphExpansions);
  } else {
    completionTokenStream =
        make_shared<CompletionTokenStream>(make_shared<PrefixTokenFilter>(
            stream, static_cast<wchar_t>(CONTEXT_SEPARATOR), contexts));
  }
  return completionTokenStream;
}

char ContextSuggestField::type() { return TYPE; }

ContextSuggestField::PrefixTokenFilter::PrefixTokenFilter(
    shared_ptr<TokenStream> input, wchar_t separator,
    deque<std::shared_ptr<std::wstring>> &prefixes)
    : org::apache::lucene::analysis::TokenFilter(input), separator(separator),
      prefixes(prefixes)
{
  this->currentPrefix.reset();
}

bool ContextSuggestField::PrefixTokenFilter::incrementToken() 
{
  if (currentPrefix != nullptr) {
    if (!currentPrefix->hasNext()) {
      return input->incrementToken();
    } else {
      posAttr->setPositionIncrement(0);
    }
  } else {
    currentPrefix = prefixes.begin();
    termAttr->setEmpty();
    posAttr->setPositionIncrement(1);
  }
  termAttr->setEmpty();
  if (currentPrefix->hasNext()) {
    termAttr->append(currentPrefix->next());
  }
  termAttr->append(separator);
  return true;
}

void ContextSuggestField::PrefixTokenFilter::reset() 
{
  TokenFilter::reset();
  currentPrefix.reset();
}

void ContextSuggestField::validate(shared_ptr<std::wstring> value)
{
  for (int i = 0; i < value->length(); i++) {
    if (CONTEXT_SEPARATOR == value->charAt(i)) {
      throw invalid_argument(
          L"Illegal value [" + value + L"] UTF-16 codepoint [0x" +
          Integer::toHexString(static_cast<int>(value->charAt(i))) +
          L"] at position " + to_wstring(i) + L" is a reserved character");
    }
  }
}
} // namespace org::apache::lucene::search::suggest::document