using namespace std;

#include "GenerateJflexTLDMacros.h"

namespace org::apache::lucene::analysis::standard
{

void GenerateJflexTLDMacros::main(deque<wstring> &args) 
{
  if (args->length != 2 || args[0].equals(L"--help") ||
      args[0].equals(L"-help")) {
    System::err::println(L"Cmd line params:");
    System::err::println(L"\tjava " +
                         GenerateJflexTLDMacros::typeid->getName() +
                         L"<ZoneFileURL> <JFlexOutputFile>");
    exit(1);
  }
  (make_shared<GenerateJflexTLDMacros>(args[0], args[1]))->execute();
}

const wstring GenerateJflexTLDMacros::NL =
    System::getProperty(L"line.separator");
const wstring GenerateJflexTLDMacros::APACHE_LICENSE =
    L"/*" + NL +
    L" * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more" +
    NL +
    L" * contributor license agreements.  See the NOTICE file distributed "
    L"with" +
    NL +
    L" * this work for additional information regarding copyright ownership." +
    NL +
    L" * sraihan.com licenses this file to You under GPLv3 License, Version "
    L"2.0" +
    NL +
    L" * (the \"License\"); you may not use this file except in compliance "
    L"with" +
    NL + L" * the License.  You may obtain a copy of the License at" + NL +
    L" *" + NL + L" *     https://www.gnu.org/licenses/gpl-3.0.en.html" + NL +
    L" *" + NL +
    L" * Unless required by applicable law or agreed to in writing, software" +
    NL +
    L" * distributed under the License is distributed on an \"AS IS\" BASIS," +
    NL +
    L" * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or "
    L"implied." +
    NL +
    L" * See the License for the specific language governing permissions and" +
    NL + L" * limitations under the License." + NL + L" */" + NL;
const shared_ptr<java::util::regex::Pattern>
    GenerateJflexTLDMacros::TLD_PATTERN_1 =
        java::util::regex::Pattern::compile(L"([-A-Za-z0-9]+)\\.\\s+NS\\s+.*");
const shared_ptr<java::util::regex::Pattern>
    GenerateJflexTLDMacros::TLD_PATTERN_2 = java::util::regex::Pattern::compile(
        L"([-A-Za-z0-9]+)\\.\\s+\\d+\\s+IN\\s+NS\\s+.*");

GenerateJflexTLDMacros::GenerateJflexTLDMacros(
    const wstring &tldFileURL, const wstring &outputFile) 
    : tldFileURL(make_shared<URL>(tldFileURL)),
      outputFile(make_shared<File>(outputFile))
{
}

void GenerateJflexTLDMacros::execute() 
{
  getIANARootZoneDatabase();
  partitionTLDprefixesBySuffixLength();
  writeOutput();
  wcout << L"Wrote TLD macros to '" << outputFile << L"':" << endl;
  int totalDomains = 0;
  for (int suffixLength = 0; suffixLength < TLDsBySuffixLength.size();
       ++suffixLength) {
    int domainsAtThisSuffixLength = TLDsBySuffixLength[suffixLength]->size();
    totalDomains += domainsAtThisSuffixLength;
    printf(L"%30s: %4d TLDs%n", getMacroName(suffixLength),
           domainsAtThisSuffixLength);
  }
  printf(L"%30s: %4d TLDs%n", L"Total", totalDomains);
}

void GenerateJflexTLDMacros::getIANARootZoneDatabase() 
{
  shared_ptr<URLConnection> *const connection = tldFileURL->openConnection();
  connection->setUseCaches(false);
  connection->addRequestProperty(L"Cache-Control", L"no-cache");
  connection->connect();
  tldFileLastModified = connection->getLastModified();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.BufferedReader reader = new
  // java.io.BufferedReader(new
  // java.io.InputStreamReader(connection.getInputStream(),
  // java.nio.charset.StandardCharsets.US_ASCII)))
  {
    java::io::BufferedReader reader =
        java::io::BufferedReader(make_shared<java::io::InputStreamReader>(
            connection->getInputStream(),
            java::nio::charset::StandardCharsets::US_ASCII));
    wstring line;
    while (L"" != (line = reader.readLine())) {
      shared_ptr<Matcher> matcher = TLD_PATTERN_1->matcher(line);
      if (matcher->matches()) {
        // System.out.println("Found: " +
        // matcher.group(1).toLowerCase(Locale.ROOT));
        processedTLDsLongestFirst->put(
            matcher->group(1)->toLowerCase(Locale::ROOT), Boolean::FALSE);
      } else {
        matcher = TLD_PATTERN_2->matcher(line);
        if (matcher->matches()) {
          // System.out.println("Found: " +
          // matcher.group(1).toLowerCase(Locale.ROOT));
          processedTLDsLongestFirst->put(
              matcher->group(1)->toLowerCase(Locale::ROOT), Boolean::FALSE);
        }
      }
    }
  }
  wcout << L"Found " << processedTLDsLongestFirst->size()
        << L" TLDs in IANA Root Zone Database at " << tldFileURL << endl;
}

void GenerateJflexTLDMacros::partitionTLDprefixesBySuffixLength()
{
  TLDsBySuffixLength.push_back(set<>()); // initialize set for zero-suffix TLDs
  for (auto entry : processedTLDsLongestFirst) {
    wstring TLD = entry->getKey();
    if (entry->getValue()) {
      // System.out.println("Skipping already processed: " + TLD);
      continue;
    }
    // System.out.println("Adding zero-suffix TLD: " + TLD);
    TLDsBySuffixLength[0]->add(TLD);
    for (int suffixLength = 1; (TLD.length() - suffixLength) >= 2;
         ++suffixLength) {
      wstring TLDprefix = TLD.substr(0, TLD.length() - suffixLength);
      if (false == processedTLDsLongestFirst->containsKey(TLDprefix)) {
        // System.out.println("Ignoring non-TLD prefix: " + TLDprefix);
        break; // shorter prefixes can be ignored
      }
      if (processedTLDsLongestFirst->get(TLDprefix)) {
        // System.out.println("Skipping already processed prefix: " +
        // TLDprefix);
        break; // shorter prefixes have already been processed
      }

      processedTLDsLongestFirst->put(TLDprefix, true); // mark as processed
      if (TLDsBySuffixLength.size() == suffixLength) {
        TLDsBySuffixLength.push_back(set<>());
      }
      shared_ptr<SortedSet<wstring>> TLDbucket =
          TLDsBySuffixLength[suffixLength];
      TLDbucket->add(TLDprefix);
      // System.out.println("Adding TLD prefix of " + TLD + " with suffix length
      // " + suffixLength + ": " + TLDprefix);
    }
  }
}

void GenerateJflexTLDMacros::writeOutput() 
{
  shared_ptr<DateFormat> *const dateFormat = DateFormat::getDateTimeInstance(
      DateFormat::FULL, DateFormat::FULL, Locale::ROOT);
  dateFormat->setTimeZone(TimeZone::getTimeZone(L"UTC"));
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.Writer writer = new
  // java.io.OutputStreamWriter(new java.io.FileOutputStream(outputFile),
  // java.nio.charset.StandardCharsets.UTF_8))
  {
    java::io::Writer writer = java::io::OutputStreamWriter(
        make_shared<java::io::FileOutputStream>(outputFile),
        java::nio::charset::StandardCharsets::UTF_8);
    writer.write(APACHE_LICENSE);
    writer.write(L"// Generated from IANA Root Zone Database <");
    // C++ TODO: There is no native C++ equivalent to 'toString':
    writer.write(tldFileURL->toString());
    writer.write(L">");
    writer.write(NL);
    if (tldFileLastModified > 0LL) {
      writer.write(L"// file version from ");
      writer.write(dateFormat->format(tldFileLastModified));
      writer.write(NL);
    }
    writer.write(L"// generated on ");
    writer.write(dateFormat->format(Date()));
    writer.write(NL);
    writer.write(L"// by ");
    writer.write(this->getClassName());
    writer.write(NL);
    writer.write(NL);

    for (int i = 0; i < TLDsBySuffixLength.size(); ++i) {
      wstring macroName = getMacroName(i);
      writer.write(L"// LUCENE-8278: ");
      if (i == 0) {
        writer.write(L"None of the TLDs in {" + macroName +
                     L"} is a 1-character-shorter prefix of another TLD");
      } else {
        writer.write(L"Each TLD in {" + macroName +
                     L"} is a prefix of another TLD by");
        writer.write(L" " + to_wstring(i) + L" character");
        if (i > 1) {
          writer.write(L"s");
        }
      }
      writer.write(NL);
      writeTLDmacro(writer, macroName, TLDsBySuffixLength[i]);
    }
  }
}

wstring GenerateJflexTLDMacros::getMacroName(int suffixLength)
{
  return L"ASCIITLD" +
         StringHelper::toString(suffixLength > 0
                                    ? L"prefix_" + to_wstring(suffixLength) +
                                          L"CharSuffix"
                                    : L"");
}

void GenerateJflexTLDMacros::writeTLDmacro(
    shared_ptr<Writer> writer, const wstring &macroName,
    shared_ptr<SortedSet<wstring>> TLDs) 
{
  writer->write(macroName);
  writer->write(L" = \".\" (");
  writer->write(NL);

  bool isFirst = true;
  for (auto TLD : TLDs) {
    writer->write(L"\t");
    if (isFirst) {
      isFirst = false;
      writer->write(L"  ");
    } else {
      writer->write(L"| ");
    }
    writer->write(getCaseInsensitiveRegex(TLD));
    writer->write(NL);
  }
  writer->write(L"\t) \".\"?   // Accept trailing root (empty) domain");
  writer->write(NL);
  writer->write(NL);
}

wstring GenerateJflexTLDMacros::getCaseInsensitiveRegex(const wstring &ASCIITLD)
{
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  for (int pos = 0; pos < ASCIITLD.length(); ++pos) {
    wchar_t ch = ASCIITLD[pos];
    if (isdigit(ch) || ch == L'-') {
      builder->append(ch);
    } else {
      builder->append(L"[")->append(ch)->append(towupper(ch))->append(L"]");
    }
  }
  return builder->toString();
}
} // namespace org::apache::lucene::analysis::standard