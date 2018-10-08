using namespace std;

#include "RSLPStemmerBase.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"

namespace org::apache::lucene::analysis::pt
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using namespace org::apache::lucene::analysis::util;
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;

RSLPStemmerBase::Rule::Rule(const wstring &suffix, int min,
                            const wstring &replacement)
    : suffix(suffix.toCharArray()), replacement(replacement.toCharArray()),
      min(min)
{
}

bool RSLPStemmerBase::Rule::matches(std::deque<wchar_t> &s, int len)
{
  return (len - suffix.size() >= min && StemmerUtil::endsWith(s, len, suffix));
}

int RSLPStemmerBase::Rule::replace(std::deque<wchar_t> &s, int len)
{
  if (replacement.size() > 0) {
    System::arraycopy(replacement, 0, s, len - suffix.size(),
                      replacement.size());
  }
  return len - suffix.size() + replacement.size();
}

RSLPStemmerBase::RuleWithSetExceptions::RuleWithSetExceptions(
    const wstring &suffix, int min, const wstring &replacement,
    std::deque<wstring> &exceptions)
    : Rule(suffix, min, replacement),
      exceptions(make_shared<CharArraySet>(Arrays::asList(exceptions), false))
{
  for (int i = 0; i < exceptions.size(); i++) {
    if (!StringHelper::endsWith(exceptions[i], suffix)) {
      throw runtime_error(L"useless exception '" + exceptions[i] +
                          L"' does not end with '" + suffix + L"'");
    }
  }
}

bool RSLPStemmerBase::RuleWithSetExceptions::matches(std::deque<wchar_t> &s,
                                                     int len)
{
  return Rule::matches(s, len) && !exceptions->contains(s, 0, len);
}

RSLPStemmerBase::RuleWithSuffixExceptions::RuleWithSuffixExceptions(
    const wstring &suffix, int min, const wstring &replacement,
    std::deque<wstring> &exceptions)
    : Rule(suffix, min, replacement),
      exceptions(std::deque<std::deque<wchar_t>>(exceptions.size()))
{
  for (int i = 0; i < exceptions.size(); i++) {
    if (!StringHelper::endsWith(exceptions[i], suffix)) {
      throw runtime_error(L"warning: useless exception '" + exceptions[i] +
                          L"' does not end with '" + suffix + L"'");
    }
  }
  for (int i = 0; i < exceptions.size(); i++) {
    this->exceptions[i] = exceptions[i].toCharArray();
  }
}

bool RSLPStemmerBase::RuleWithSuffixExceptions::matches(std::deque<wchar_t> &s,
                                                        int len)
{
  if (!Rule::matches(s, len)) {
    return false;
  }

  for (int i = 0; i < exceptions.size(); i++) {
    if (StemmerUtil::endsWith(s, len, exceptions[i])) {
      return false;
    }
  }

  return true;
}

RSLPStemmerBase::Step::Step(const wstring &name,
                            std::deque<std::shared_ptr<Rule>> &rules, int min,
                            std::deque<wstring> &suffixes)
    : name(name), rules(rules)
{
  if (min == 0) {
    min = numeric_limits<int>::max();
    for (auto r : rules) {
      min = min(min, r->min + r->suffix.size());
    }
  }
  this->min = min;

  if (suffixes.empty() || suffixes.empty()) {
    this->suffixes.clear();
  } else {
    this->suffixes = std::deque<std::deque<wchar_t>>(suffixes.size());
    for (int i = 0; i < suffixes.size(); i++) {
      this->suffixes[i] = suffixes[i].toCharArray();
    }
  }
}

int RSLPStemmerBase::Step::apply(std::deque<wchar_t> &s, int len)
{
  if (len < min) {
    return len;
  }

  if (suffixes.size() > 0) {
    bool found = false;

    for (int i = 0; i < suffixes.size(); i++) {
      if (StemmerUtil::endsWith(s, len, suffixes[i])) {
        found = true;
        break;
      }
    }

    if (!found) {
      return len;
    }
  }

  for (int i = 0; i < rules.size(); i++) {
    if (rules[i]->matches(s, len)) {
      return rules[i]->replace(s, len);
    }
  }

  return len;
}

unordered_map<wstring, std::shared_ptr<Step>>
RSLPStemmerBase::parse(type_info clazz, const wstring &resource)
{
  // TODO: this parser is ugly, but works. use a jflex grammar instead.
  try {
    shared_ptr<InputStream> is = clazz.getResourceAsStream(resource);
    shared_ptr<LineNumberReader> r = make_shared<LineNumberReader>(
        make_shared<InputStreamReader>(is, StandardCharsets::UTF_8));
    unordered_map<wstring, std::shared_ptr<Step>> steps =
        unordered_map<wstring, std::shared_ptr<Step>>();
    wstring step;
    while ((step = readLine(r)) != L"") {
      shared_ptr<Step> s = parseStep(r, step);
      steps.emplace(s->name, s);
    }
    r->close();
    return steps;
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

const shared_ptr<java::util::regex::Pattern> RSLPStemmerBase::headerPattern =
    java::util::regex::Pattern::compile(
        L"^\\{\\s*\"([^\"]*)\",\\s*([0-9]+),\\s*(0|1),\\s*\\{(.*)\\},\\s*$");
const shared_ptr<java::util::regex::Pattern> RSLPStemmerBase::stripPattern =
    java::util::regex::Pattern::compile(
        L"^\\{\\s*\"([^\"]*)\",\\s*([0-9]+)\\s*\\}\\s*(,|(\\}\\s*;))$");
const shared_ptr<java::util::regex::Pattern> RSLPStemmerBase::repPattern =
    java::util::regex::Pattern::compile(
        L"^\\{\\s*\"([^\"]*)\",\\s*([0-9]+),\\s*\"([^\"]*)\"\\}\\s*(,|(\\}\\s*;"
        L"))$");
const shared_ptr<java::util::regex::Pattern> RSLPStemmerBase::excPattern =
    java::util::regex::Pattern::compile(
        L"^\\{\\s*\"([^\"]*)\",\\s*([0-9]+),\\s*\"([^\"]*)\",\\s*\\{(.*)\\}\\s*"
        L"\\}\\s*(,|(\\}\\s*;))$");

shared_ptr<Step>
RSLPStemmerBase::parseStep(shared_ptr<LineNumberReader> r,
                           const wstring &header) 
{
  shared_ptr<Matcher> matcher = headerPattern->matcher(header);
  if (!matcher->find()) {
    throw runtime_error(L"Illegal Step header specified at line " +
                        r->getLineNumber());
  }
  assert(matcher->groupCount() == 4);
  wstring name = matcher->group(1);
  int min = static_cast<Integer>(matcher->group(2));
  int type = static_cast<Integer>(matcher->group(3));
  std::deque<wstring> suffixes = parseList(matcher->group(4));
  std::deque<std::shared_ptr<Rule>> rules = parseRules(r, type);
  return make_shared<Step>(name, rules, min, suffixes);
}

std::deque<std::shared_ptr<Rule>>
RSLPStemmerBase::parseRules(shared_ptr<LineNumberReader> r,
                            int type) 
{
  deque<std::shared_ptr<Rule>> rules = deque<std::shared_ptr<Rule>>();
  wstring line;
  while ((line = readLine(r)) != L"") {
    shared_ptr<Matcher> matcher = stripPattern->matcher(line);
    if (matcher->matches()) {
      rules.push_back(make_shared<Rule>(
          matcher->group(1), static_cast<Integer>(matcher->group(2)), L""));
    } else {
      matcher = repPattern->matcher(line);
      if (matcher->matches()) {
        rules.push_back(make_shared<Rule>(
            matcher->group(1), static_cast<Integer>(matcher->group(2)),
            matcher->group(3)));
      } else {
        matcher = excPattern->matcher(line);
        if (matcher->matches()) {
          if (type == 0) {
            rules.push_back(make_shared<RuleWithSuffixExceptions>(
                matcher->group(1), static_cast<Integer>(matcher->group(2)),
                matcher->group(3), parseList(matcher->group(4))));
          } else {
            rules.push_back(make_shared<RuleWithSetExceptions>(
                matcher->group(1), static_cast<Integer>(matcher->group(2)),
                matcher->group(3), parseList(matcher->group(4))));
          }
        } else {
          throw runtime_error(L"Illegal Step rule specified at line " +
                              r->getLineNumber());
        }
      }
    }
    if (StringHelper::endsWith(line, L";")) {
      return rules.toArray(std::deque<std::shared_ptr<Rule>>(rules.size()));
    }
  }
  return nullptr;
}

std::deque<wstring> RSLPStemmerBase::parseList(const wstring &s)
{
  if (s.length() == 0) {
    return nullptr;
  }
  std::deque<wstring> deque = s.split(L",");
  for (int i = 0; i < deque.size(); i++) {
    deque[i] = parseString(StringHelper::trim(deque[i]));
  }
  return deque;
}

wstring RSLPStemmerBase::parseString(const wstring &s)
{
  return s.substr(1, (s.length() - 1) - 1);
}

wstring
RSLPStemmerBase::readLine(shared_ptr<LineNumberReader> r) 
{
  wstring line = L"";
  while ((line = r->readLine()) != L"") {
    line = StringHelper::trim(line);
    if (line.length() > 0 && line[0] != L'#') {
      return line;
    }
  }
  return line;
}
} // namespace org::apache::lucene::analysis::pt