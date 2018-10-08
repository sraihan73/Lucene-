using namespace std;

#include "Input.h"

namespace org::apache::lucene::search::suggest
{
using BytesRef = org::apache::lucene::util::BytesRef;

Input::Input(shared_ptr<BytesRef> term, int64_t v,
             shared_ptr<BytesRef> payload)
    : Input(term, v, payload, true, nullptr, false)
{
}

Input::Input(const wstring &term, int64_t v, shared_ptr<BytesRef> payload)
    : Input(new BytesRef(term), v, payload)
{
}

Input::Input(shared_ptr<BytesRef> term, int64_t v,
             shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts)
    : Input(term, v, nullptr, false, contexts, true)
{
}

Input::Input(const wstring &term, int64_t v,
             shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts)
    : Input(new BytesRef(term), v, nullptr, false, contexts, true)
{
}

Input::Input(shared_ptr<BytesRef> term, int64_t v)
    : Input(term, v, nullptr, false, nullptr, false)
{
}

Input::Input(const wstring &term, int64_t v)
    : Input(new BytesRef(term), v, nullptr, false, nullptr, false)
{
}

Input::Input(const wstring &term, int v, shared_ptr<BytesRef> payload,
             shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts)
    : Input(new BytesRef(term), v, payload, true, contexts, true)
{
}

Input::Input(shared_ptr<BytesRef> term, int64_t v,
             shared_ptr<BytesRef> payload,
             shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts)
    : Input(term, v, payload, true, contexts, true)
{
}

Input::Input(shared_ptr<BytesRef> term, int64_t v,
             shared_ptr<BytesRef> payload, bool hasPayloads,
             shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts,
             bool hasContexts)
    : term(term), v(v), payload(payload), hasPayloads(hasPayloads),
      contexts(contexts), hasContexts(hasContexts)
{
}

bool Input::hasContexts() { return hasContexts_; }

bool Input::hasPayloads() { return hasPayloads_; }
} // namespace org::apache::lucene::search::suggest