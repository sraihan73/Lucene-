using namespace std;

#include "Version.h"

namespace org::apache::lucene::util
{

const shared_ptr<Version> Version::LUCENE_6_0_0 = make_shared<Version>(6, 0, 0);
const shared_ptr<Version> Version::LUCENE_6_0_1 = make_shared<Version>(6, 0, 1);
const shared_ptr<Version> Version::LUCENE_6_1_0 = make_shared<Version>(6, 1, 0);
const shared_ptr<Version> Version::LUCENE_6_2_0 = make_shared<Version>(6, 2, 0);
const shared_ptr<Version> Version::LUCENE_6_2_1 = make_shared<Version>(6, 2, 1);
const shared_ptr<Version> Version::LUCENE_6_3_0 = make_shared<Version>(6, 3, 0);
const shared_ptr<Version> Version::LUCENE_6_4_0 = make_shared<Version>(6, 4, 0);
const shared_ptr<Version> Version::LUCENE_6_4_1 = make_shared<Version>(6, 4, 1);
const shared_ptr<Version> Version::LUCENE_6_4_2 = make_shared<Version>(6, 4, 2);
const shared_ptr<Version> Version::LUCENE_6_5_0 = make_shared<Version>(6, 5, 0);
const shared_ptr<Version> Version::LUCENE_6_5_1 = make_shared<Version>(6, 5, 1);
const shared_ptr<Version> Version::LUCENE_6_6_0 = make_shared<Version>(6, 6, 0);
const shared_ptr<Version> Version::LUCENE_6_6_1 = make_shared<Version>(6, 6, 1);
const shared_ptr<Version> Version::LUCENE_6_6_2 = make_shared<Version>(6, 6, 2);
const shared_ptr<Version> Version::LUCENE_6_6_3 = make_shared<Version>(6, 6, 3);
const shared_ptr<Version> Version::LUCENE_6_6_4 = make_shared<Version>(6, 6, 4);
const shared_ptr<Version> Version::LUCENE_7_0_0 = make_shared<Version>(7, 0, 0);
const shared_ptr<Version> Version::LUCENE_7_0_1 = make_shared<Version>(7, 0, 1);
const shared_ptr<Version> Version::LUCENE_7_1_0 = make_shared<Version>(7, 1, 0);
const shared_ptr<Version> Version::LUCENE_7_2_0 = make_shared<Version>(7, 2, 0);
const shared_ptr<Version> Version::LUCENE_7_2_1 = make_shared<Version>(7, 2, 1);
const shared_ptr<Version> Version::LUCENE_7_3_0 = make_shared<Version>(7, 3, 0);
const shared_ptr<Version> Version::LUCENE_7_3_1 = make_shared<Version>(7, 3, 1);
const shared_ptr<Version> Version::LUCENE_7_4_0 = make_shared<Version>(7, 4, 0);
const shared_ptr<Version> Version::LATEST = LUCENE_7_4_0;
const shared_ptr<Version> Version::LUCENE_CURRENT = LATEST;

shared_ptr<Version> Version::parse(const wstring &version) 
{

  shared_ptr<StrictStringTokenizer> tokens =
      make_shared<StrictStringTokenizer>(version, L'.');
  if (tokens->hasMoreTokens() == false) {
    throw make_shared<ParseException>(
        L"Version is not in form major.minor.bugfix(.prerelease) (got: " +
            version + L")",
        0);
  }

  int major;
  wstring token = tokens->nextToken();
  try {
    major = stoi(token);
  } catch (const NumberFormatException &nfe) {
    shared_ptr<ParseException> p =
        make_shared<ParseException>(L"Failed to parse major version from \"" +
                                        token + L"\" (got: " + version + L")",
                                    0);
    p->initCause(nfe);
    throw p;
  }

  if (tokens->hasMoreTokens() == false) {
    throw make_shared<ParseException>(
        L"Version is not in form major.minor.bugfix(.prerelease) (got: " +
            version + L")",
        0);
  }

  int minor;
  token = tokens->nextToken();
  try {
    minor = stoi(token);
  } catch (const NumberFormatException &nfe) {
    shared_ptr<ParseException> p =
        make_shared<ParseException>(L"Failed to parse minor version from \"" +
                                        token + L"\" (got: " + version + L")",
                                    0);
    p->initCause(nfe);
    throw p;
  }

  int bugfix = 0;
  int prerelease = 0;
  if (tokens->hasMoreTokens()) {

    token = tokens->nextToken();
    try {
      bugfix = stoi(token);
    } catch (const NumberFormatException &nfe) {
      shared_ptr<ParseException> p = make_shared<ParseException>(
          L"Failed to parse bugfix version from \"" + token + L"\" (got: " +
              version + L")",
          0);
      p->initCause(nfe);
      throw p;
    }

    if (tokens->hasMoreTokens()) {
      token = tokens->nextToken();
      try {
        prerelease = stoi(token);
      } catch (const NumberFormatException &nfe) {
        shared_ptr<ParseException> p = make_shared<ParseException>(
            L"Failed to parse prerelease version from \"" + token +
                L"\" (got: " + version + L")",
            0);
        p->initCause(nfe);
        throw p;
      }
      if (prerelease == 0) {
        throw make_shared<ParseException>(
            L"Invalid value " + to_wstring(prerelease) +
                L" for prerelease; should be 1 or 2 (got: " + version + L")",
            0);
      }

      if (tokens->hasMoreTokens()) {
        // Too many tokens!
        throw make_shared<ParseException>(
            L"Version is not in form major.minor.bugfix(.prerelease) (got: " +
                version + L")",
            0);
      }
    }
  }

  try {
    return make_shared<Version>(major, minor, bugfix, prerelease);
  } catch (const invalid_argument &iae) {
    shared_ptr<ParseException> pe = make_shared<ParseException>(
        L"failed to parse version string \"" + version + L"\": " + iae.what(),
        0);
    pe->initCause(iae);
    throw pe;
  }
}

shared_ptr<Version>
Version::parseLeniently(const wstring &version) 
{
  wstring versionOrig = version;
  version = version.toUpperCase(Locale::ROOT);
  switch (version) {
  case L"LATEST":
  case L"LUCENE_CURRENT":
    return LATEST;
  default:
    version =
        version.replaceFirst(L"^LUCENE_(\\d+)_(\\d+)_(\\d+)$", L"$1.$2.$3")
            .replaceFirst(L"^LUCENE_(\\d+)_(\\d+)$", L"$1.$2.0")
            .replaceFirst(L"^LUCENE_(\\d)(\\d)$", L"$1.$2.0");
    try {
      return parse(version);
    } catch (const ParseException &pe) {
      shared_ptr<ParseException> pe2 = make_shared<ParseException>(
          L"failed to parse lenient version string \"" + versionOrig + L"\": " +
              pe->getMessage(),
          0);
      pe2->initCause(pe);
      throw pe2;
    }
  }
}

shared_ptr<Version> Version::fromBits(int major, int minor, int bugfix)
{
  return make_shared<Version>(major, minor, bugfix);
}

Version::Version(int major, int minor, int bugfix)
    : Version(major, minor, bugfix, 0)
{
}

Version::Version(int major, int minor, int bugfix, int prerelease)
    : major(major), minor(minor), bugfix(bugfix), prerelease(prerelease),
      encodedValue(major << 18 | minor << 10 | bugfix << 2 | prerelease)
{
  // NOTE: do not enforce major version so we remain future proof, except to
  // make sure it fits in the 8 bits we encode it into:
  if (major > 255 || major < 0) {
    throw invalid_argument(L"Illegal major version: " + to_wstring(major));
  }
  if (minor > 255 || minor < 0) {
    throw invalid_argument(L"Illegal minor version: " + to_wstring(minor));
  }
  if (bugfix > 255 || bugfix < 0) {
    throw invalid_argument(L"Illegal bugfix version: " + to_wstring(bugfix));
  }
  if (prerelease > 2 || prerelease < 0) {
    throw invalid_argument(L"Illegal prerelease version: " +
                           to_wstring(prerelease));
  }
  if (prerelease != 0 && (minor != 0 || bugfix != 0)) {
    throw invalid_argument(L"Prerelease version only supported with major "
                           L"release (got prerelease: " +
                           to_wstring(prerelease) + L", minor: " +
                           to_wstring(minor) + L", bugfix: " +
                           to_wstring(bugfix) + L")");
  }

  assert(encodedIsValid());
}

bool Version::onOrAfter(shared_ptr<Version> other)
{
  return encodedValue >= other->encodedValue;
}

wstring Version::toString()
{
  if (prerelease == 0) {
    return L"" + to_wstring(major) + L"." + to_wstring(minor) + L"." +
           to_wstring(bugfix);
  }
  return L"" + to_wstring(major) + L"." + to_wstring(minor) + L"." +
         to_wstring(bugfix) + L"." + to_wstring(prerelease);
}

bool Version::equals(any o)
{
  return o != nullptr && std::dynamic_pointer_cast<Version>(o) != nullptr &&
         (any_cast<std::shared_ptr<Version>>(o))->encodedValue == encodedValue;
}

bool Version::encodedIsValid()
{
  assert(major ==
         ((static_cast<int>(static_cast<unsigned int>(encodedValue) >> 18)) &
          0xFF));
  assert(minor ==
         ((static_cast<int>(static_cast<unsigned int>(encodedValue) >> 10)) &
          0xFF));
  assert(bugfix ==
         ((static_cast<int>(static_cast<unsigned int>(encodedValue) >> 2)) &
          0xFF));
  assert(prerelease == (encodedValue & 0x03));
  return true;
}

int Version::hashCode() { return encodedValue; }
} // namespace org::apache::lucene::util