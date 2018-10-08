using namespace std;

#include "Config.h"
#include "Format.h"

namespace org::apache::lucene::benchmark::byTask::utils
{

const wstring Config::DEFAULT_PRINT_PROPS =
    System::getProperty(L"tests.verbose", L"true");
const wstring Config::NEW_LINE = System::getProperty(L"line.separator");

Config::Config(shared_ptr<Reader> algReader) 
{
  // read alg file to array of lines
  deque<wstring> lines = deque<wstring>();
  shared_ptr<BufferedReader> r = make_shared<BufferedReader>(algReader);
  int lastConfigLine = 0;
  for (wstring line = r->readLine(); line != L""; line = r->readLine()) {
    lines.push_back(line);
    if ((int)line.find(L'=') > 0) {
      lastConfigLine = lines.size();
    }
  }
  r->close();
  // copy props lines to string
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (int i = 0; i < lastConfigLine; i++) {
    sb->append(lines[i]);
    sb->append(NEW_LINE);
  }
  // read props from string
  this->props = make_shared<Properties>();
  props->load(make_shared<StringReader>(sb->toString()));

  // make sure work dir is set properly
  if (props->get(L"work.dir") == nullptr) {
    props->setProperty(L"work.dir",
                       System::getProperty(L"benchmark.work.dir", L"work"));
  }

  if (static_cast<Boolean>(
          props->getProperty(L"print.props", DEFAULT_PRINT_PROPS))
          .booleanValue()) {
    printProps();
  }

  // copy algorithm lines
  sb = make_shared<StringBuilder>();
  for (int i = lastConfigLine; i < lines.size(); i++) {
    sb->append(lines[i]);
    sb->append(NEW_LINE);
  }
  algorithmText = sb->toString();
}

Config::Config(shared_ptr<Properties> props)
{
  this->props = props;
  if (static_cast<Boolean>(
          props->getProperty(L"print.props", DEFAULT_PRINT_PROPS))
          .booleanValue()) {
    printProps();
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings({"unchecked", "rawtypes"}) private void
// printProps()
void Config::printProps()
{
  wcout << L"------------> config properties:" << endl;
  deque<wstring> propKeys = deque(props->keySet());
  sort(propKeys.begin(), propKeys.end());
  for (auto propName : propKeys) {
    wcout << propName << L" = " << props->getProperty(propName) << endl;
  }
  wcout << L"-------------------------------" << endl;
}

wstring Config::get(const wstring &name, const wstring &dflt)
{
  std::deque<wstring> vals = any_cast<std::deque<wstring>>(valByRound[name]);
  if (vals.size() > 0) {
    return vals[roundNumber % vals.size()];
  }
  // done if not by round
  wstring sval = props->getProperty(name, dflt);
  if (sval == L"") {
    return L"";
  }
  if (sval.find(L":") == wstring::npos) {
    return sval;
  } else if (sval.find(L":\\") != wstring::npos ||
             sval.find(L":/") != wstring::npos) {
    // this previously messed up absolute path names on Windows. Assuming
    // there is no real value that starts with \ or /
    return sval;
  }
  // first time this prop is extracted by round
  int k = (int)sval.find(L":");
  wstring colName = sval.substr(0, k);
  sval = sval.substr(k + 1);
  colForValByRound.emplace(name, colName);
  vals = propToStringArray(sval);
  valByRound.emplace(name, vals);
  return vals[roundNumber % vals.size()];
}

void Config::set(const wstring &name, const wstring &value) 
{
  if (valByRound[name] != nullptr) {
    throw runtime_error(L"Cannot modify a multi value property!");
  }
  props->setProperty(name, value);
}

int Config::get(const wstring &name, int dflt)
{
  // use value by round if already parsed
  std::deque<int> vals = any_cast<std::deque<int>>(valByRound[name]);
  if (vals.size() > 0) {
    return vals[roundNumber % vals.size()];
  }
  // done if not by round
  wstring sval = props->getProperty(name, L"" + to_wstring(dflt));
  if (sval.find(L":") == wstring::npos) {
    return stoi(sval);
  }
  // first time this prop is extracted by round
  int k = (int)sval.find(L":");
  wstring colName = sval.substr(0, k);
  sval = sval.substr(k + 1);
  colForValByRound.emplace(name, colName);
  vals = propToIntArray(sval);
  valByRound.emplace(name, vals);
  return vals[roundNumber % vals.size()];
}

double Config::get(const wstring &name, double dflt)
{
  // use value by round if already parsed
  std::deque<double> vals = any_cast<std::deque<double>>(valByRound[name]);
  if (vals.size() > 0) {
    return vals[roundNumber % vals.size()];
  }
  // done if not by round
  wstring sval = props->getProperty(name, L"" + to_wstring(dflt));
  if (sval.find(L":") == wstring::npos) {
    return stod(sval);
  }
  // first time this prop is extracted by round
  int k = (int)sval.find(L":");
  wstring colName = sval.substr(0, k);
  sval = sval.substr(k + 1);
  colForValByRound.emplace(name, colName);
  vals = propToDoubleArray(sval);
  valByRound.emplace(name, vals);
  return vals[roundNumber % vals.size()];
}

bool Config::get(const wstring &name, bool dflt)
{
  // use value by round if already parsed
  std::deque<bool> vals = any_cast<std::deque<bool>>(valByRound[name]);
  if (vals.size() > 0) {
    return vals[roundNumber % vals.size()];
  }
  // done if not by round
  wstring sval = props->getProperty(name, L"" + StringHelper::toString(dflt));
  if (sval.find(L":") == wstring::npos) {
    return StringHelper::fromString<bool>(sval).booleanValue();
  }
  // first time this prop is extracted by round
  int k = (int)sval.find(L":");
  wstring colName = sval.substr(0, k);
  sval = sval.substr(k + 1);
  colForValByRound.emplace(name, colName);
  vals = propToBooleanArray(sval);
  valByRound.emplace(name, vals);
  return vals[roundNumber % vals.size()];
}

int Config::newRound()
{
  roundNumber++;

  shared_ptr<StringBuilder> sb = (make_shared<StringBuilder>(L"--> Round "))
                                     ->append(roundNumber - 1)
                                     ->append(L"-->")
                                     ->append(roundNumber);

  // log changes in values
  if (valByRound.size() > 0) {
    sb->append(L": ");
    for (auto name : valByRound) {
      any a = valByRound[name.first];
      if (dynamic_cast<std::deque<int>>(a) != nullptr) {
        std::deque<int> ai = any_cast<std::deque<int>>(a);
        int n1 = (roundNumber - 1) % ai.size();
        int n2 = roundNumber % ai.size();
        sb->append(L"  ")
            ->append(name.first)
            ->append(L":")
            ->append(ai[n1])
            ->append(L"-->")
            ->append(ai[n2]);
      } else if (dynamic_cast<std::deque<double>>(a) != nullptr) {
        std::deque<double> ad = any_cast<std::deque<double>>(a);
        int n1 = (roundNumber - 1) % ad.size();
        int n2 = roundNumber % ad.size();
        sb->append(L"  ")
            ->append(name.first)
            ->append(L":")
            ->append(ad[n1])
            ->append(L"-->")
            ->append(ad[n2]);
      } else if (dynamic_cast<std::deque<wstring>>(a) != nullptr) {
        std::deque<wstring> ad = any_cast<std::deque<wstring>>(a);
        int n1 = (roundNumber - 1) % ad.size();
        int n2 = roundNumber % ad.size();
        sb->append(L"  ")
            ->append(name.first)
            ->append(L":")
            ->append(ad[n1])
            ->append(L"-->")
            ->append(ad[n2]);
      } else {
        std::deque<bool> ab = any_cast<std::deque<bool>>(a);
        int n1 = (roundNumber - 1) % ab.size();
        int n2 = roundNumber % ab.size();
        sb->append(L"  ")
            ->append(name.first)
            ->append(L":")
            ->append(ab[n1])
            ->append(L"-->")
            ->append(ab[n2]);
      }
    }
  }

  wcout << endl;
  wcout << sb->toString() << endl;
  wcout << endl;

  return roundNumber;
}

std::deque<wstring> Config::propToStringArray(const wstring &s)
{
  if (s.find(L":") == wstring::npos) {
    return std::deque<wstring>{s};
  }

  deque<wstring> a = deque<wstring>();
  shared_ptr<StringTokenizer> st = make_shared<StringTokenizer>(s, L":");
  while (st->hasMoreTokens()) {
    wstring t = st->nextToken();
    a.push_back(t);
  }
  return a.toArray(std::deque<wstring>(a.size()));
}

std::deque<int> Config::propToIntArray(const wstring &s)
{
  if (s.find(L":") == wstring::npos) {
    return std::deque<int>{stoi(s)};
  }

  deque<int> a = deque<int>();
  shared_ptr<StringTokenizer> st = make_shared<StringTokenizer>(s, L":");
  while (st->hasMoreTokens()) {
    wstring t = st->nextToken();
    a.push_back(stoi(t));
  }
  std::deque<int> res(a.size());
  for (int i = 0; i < a.size(); i++) {
    res[i] = a[i];
  }
  return res;
}

std::deque<double> Config::propToDoubleArray(const wstring &s)
{
  if (s.find(L":") == wstring::npos) {
    return std::deque<double>{stod(s)};
  }

  deque<double> a = deque<double>();
  shared_ptr<StringTokenizer> st = make_shared<StringTokenizer>(s, L":");
  while (st->hasMoreTokens()) {
    wstring t = st->nextToken();
    a.push_back(stod(t));
  }
  std::deque<double> res(a.size());
  for (int i = 0; i < a.size(); i++) {
    res[i] = a[i];
  }
  return res;
}

std::deque<bool> Config::propToBooleanArray(const wstring &s)
{
  if (s.find(L":") == wstring::npos) {
    return std::deque<bool>{StringHelper::fromString<bool>(s).booleanValue()};
  }

  deque<bool> a = deque<bool>();
  shared_ptr<StringTokenizer> st = make_shared<StringTokenizer>(s, L":");
  while (st->hasMoreTokens()) {
    wstring t = st->nextToken();
    a.push_back(optional<bool>(t));
  }
  std::deque<bool> res(a.size());
  for (int i = 0; i < a.size(); i++) {
    res[i] = a[i];
  }
  return res;
}

wstring Config::getColsNamesForValsByRound()
{
  if (colForValByRound.empty()) {
    return L"";
  }
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (auto name : colForValByRound) {
    wstring colName = colForValByRound[name.first];
    sb->append(L" ")->append(colName);
  }
  return sb->toString();
}

wstring Config::getColsValuesForValsByRound(int roundNum)
{
  if (colForValByRound.empty()) {
    return L"";
  }
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (auto name : colForValByRound) {
    wstring colName = colForValByRound[name.first];
    wstring template_ = L" " + colName;
    if (roundNum < 0) {
      // just append blanks
      sb->append(Format::formatPaddLeft(L"-", template_));
    } else {
      // append actual values, for that round
      any a = valByRound[name.first];
      if (dynamic_cast<std::deque<int>>(a) != nullptr) {
        std::deque<int> ai = any_cast<std::deque<int>>(a);
        int n = roundNum % ai.size();
        sb->append(Format::format(ai[n], template_));
      } else if (dynamic_cast<std::deque<double>>(a) != nullptr) {
        std::deque<double> ad = any_cast<std::deque<double>>(a);
        int n = roundNum % ad.size();
        sb->append(Format::format(2, ad[n], template_));
      } else if (dynamic_cast<std::deque<wstring>>(a) != nullptr) {
        std::deque<wstring> ad = any_cast<std::deque<wstring>>(a);
        int n = roundNum % ad.size();
        sb->append(ad[n]);
      } else {
        std::deque<bool> ab = any_cast<std::deque<bool>>(a);
        int n = roundNum % ab.size();
        sb->append(Format::formatPaddLeft(L"" + StringHelper::toString(ab[n]),
                                          template_));
      }
    }
  }
  return sb->toString();
}

int Config::getRoundNumber() { return roundNumber; }

wstring Config::getAlgorithmText() { return algorithmText; }
} // namespace org::apache::lucene::benchmark::byTask::utils