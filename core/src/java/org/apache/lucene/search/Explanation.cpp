using namespace std;

#include "Explanation.h"

namespace org::apache::lucene::search
{

shared_ptr<Explanation>
Explanation::match(float value, const wstring &description,
                   shared_ptr<deque<std::shared_ptr<Explanation>>> details)
{
  return make_shared<Explanation>(true, value, description, details);
}

shared_ptr<Explanation> Explanation::match(float value,
                                           const wstring &description,
                                           deque<Explanation> &details)
{
  return make_shared<Explanation>(true, value, description,
                                  Arrays::asList(details));
}

shared_ptr<Explanation> Explanation::noMatch(
    const wstring &description,
    shared_ptr<deque<std::shared_ptr<Explanation>>> details)
{
  return make_shared<Explanation>(false, 0.0f, description, details);
}

shared_ptr<Explanation> Explanation::noMatch(const wstring &description,
                                             deque<Explanation> &details)
{
  return make_shared<Explanation>(false, 0.0f, description,
                                  Arrays::asList(details));
}

Explanation::Explanation(
    bool match, float value, const wstring &description,
    shared_ptr<deque<std::shared_ptr<Explanation>>> details)
    : match(match), value(value),
      description(Objects::requireNonNull(description)),
      details(Collections::unmodifiableList(deque<>(details)))
{
  for (auto detail : details) {
    Objects::requireNonNull(detail);
  }
}

bool Explanation::isMatch() { return match_; }

float Explanation::getValue() { return value; }

wstring Explanation::getDescription() { return description; }

wstring Explanation::getSummary()
{
  return to_wstring(getValue()) + L" = " + getDescription();
}

std::deque<std::shared_ptr<Explanation>> Explanation::getDetails()
{
  return details.toArray(std::deque<std::shared_ptr<Explanation>>(0));
}

wstring Explanation::toString() { return toString(0); }

wstring Explanation::toString(int depth)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  for (int i = 0; i < depth; i++) {
    buffer->append(L"  ");
  }
  buffer->append(getSummary());
  buffer->append(L"\n");

  std::deque<std::shared_ptr<Explanation>> details = getDetails();
  for (int i = 0; i < details.size(); i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    buffer->append(details[i]->toString(depth + 1));
  }

  return buffer->toString();
}

bool Explanation::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<Explanation> that = any_cast<std::shared_ptr<Explanation>>(o);
  return match_ == that->match_ && Float::compare(that->value, value) == 0 &&
         Objects::equals(description, that->description) &&
         Objects::equals(details, that->details);
}

int Explanation::hashCode()
{
  return Objects::hash(match_, value, description, details);
}
} // namespace org::apache::lucene::search