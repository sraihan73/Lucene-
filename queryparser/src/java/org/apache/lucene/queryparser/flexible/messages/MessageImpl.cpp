using namespace std;

#include "MessageImpl.h"

namespace org::apache::lucene::queryparser::flexible::messages
{

MessageImpl::MessageImpl(const wstring &key) { this->key = key; }

MessageImpl::MessageImpl(const wstring &key, deque<any> &args)
    : MessageImpl(key)
{
  this->arguments = args;
}

std::deque<any> MessageImpl::getArguments() { return this->arguments; }

wstring MessageImpl::getKey() { return this->key; }

wstring MessageImpl::getLocalizedMessage()
{
  return getLocalizedMessage(Locale::getDefault());
}

wstring MessageImpl::getLocalizedMessage(shared_ptr<Locale> locale)
{
  return NLS::getLocalizedMessage(getKey(), locale, getArguments());
}

wstring MessageImpl::toString()
{
  std::deque<any> args = getArguments();
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>(getKey());
  if (args.size() > 0) {
    for (int i = 0; i < args.size(); i++) {
      sb->append(i == 0 ? L" " : L", ")->append(args[i]);
    }
  }
  return sb->toString();
}
} // namespace org::apache::lucene::queryparser::flexible::messages