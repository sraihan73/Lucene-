using namespace std;

#include "LicenseCheckTask.h"

namespace org::apache::lucene::validation
{
using org::apache::tools::ant::BuildException;
using org::apache::tools::ant::Project;
using org::apache::tools::ant::Task;
using org::apache::tools::ant::types::Mapper;
using org::apache::tools::ant::types::Resource;
using org::apache::tools::ant::types::ResourceCollection;
using org::apache::tools::ant::types::resources::FileResource;
using org::apache::tools::ant::types::resources::Resources;
using org::apache::tools::ant::util::FileNameMapper;
const wstring LicenseCheckTask::CHECKSUM_TYPE = L"sha1";
const wstring LicenseCheckTask::FAILURE_MESSAGE =
    wstring(L"License check failed. Check the logs.\n") +
    L"If you recently modified ivy-versions.properties or any module's "
    L"ivy.xml,\n" +
    L"make sure you run \"ant clean-jars jar-checksums\" before running "
    L"precommit.";

void LicenseCheckTask::add(shared_ptr<ResourceCollection> rc)
{
  jarResources->add(rc);
}

void LicenseCheckTask::addConfiguredLicenseMapper(shared_ptr<Mapper> mapper)
{
  if (licenseMapper != nullptr) {
    // C++ TODO: The following line could not be converted:
    throw org.apache.tools.ant.BuildException(
        L"Only one license mapper is allowed.");
  }
  this->licenseMapper = mapper->getImplementation();
}

void LicenseCheckTask::setVerbose(bool verbose)
{
  verboseLevel = (verbose ? Project::MSG_INFO : Project::MSG_VERBOSE);
}

void LicenseCheckTask::setLicenseDirectory(shared_ptr<File> file)
{
  licenseDirectory = file;
}

void LicenseCheckTask::setSkipSnapshotsChecksum(bool skipSnapshotsChecksum)
{
  this->skipSnapshotsChecksum = skipSnapshotsChecksum;
}

void LicenseCheckTask::setSkipChecksum(bool skipChecksum)
{
  this->skipChecksum = skipChecksum;
}

void LicenseCheckTask::setSkipRegexChecksum(const wstring &skipRegexChecksum)
{
  try {
    if (skipRegexChecksum != L"" && skipRegexChecksum.length() > 0) {
      this->skipRegexChecksum = Pattern::compile(skipRegexChecksum);
    }
  } catch (const PatternSyntaxException &e) {
    throw make_shared<BuildException>(
        L"Unable to compile skipRegexChecksum pattern.  Reason: " +
            e->getMessage() + L" " + skipRegexChecksum,
        e);
  }
}

void LicenseCheckTask::execute() 
{
  if (licenseMapper == nullptr) {
    // C++ TODO: The following line could not be converted:
    throw org.apache.tools.ant.BuildException(
        L"Expected an embedded <licenseMapper>.");
  }

  if (skipChecksum) {
    log(L"Skipping checksum verification for dependencies", Project::MSG_INFO);
  } else {
    if (skipSnapshotsChecksum) {
      log(L"Skipping checksum for SNAPSHOT dependencies", Project::MSG_INFO);
    }

    if (skipRegexChecksum != nullptr) {
      log(L"Skipping checksum for dependencies matching regex: " +
              skipRegexChecksum->pattern(),
          Project::MSG_INFO);
    }
  }

  jarResources->setProject(getProject());
  processJars();

  if (failures) {
    // C++ TODO: The following line could not be converted:
    throw org.apache.tools.ant.BuildException(FAILURE_MESSAGE);
  }
}

void LicenseCheckTask::processJars()
{
  log(L"Starting scan.", verboseLevel);
  int64_t start = System::currentTimeMillis();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unchecked")
  // java.util.Iterator<org.apache.tools.ant.types.Resource> iter =
  // (java.util.Iterator<org.apache.tools.ant.types.Resource>)
  // jarResources.iterator();
  Resources::const_iterator iter =
      std::static_pointer_cast<Iterator<std::shared_ptr<Resource>>>(
          jarResources->begin());
  int checked = 0;
  int errors = 0;
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  while (iter->hasNext()) {
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    shared_ptr<Resource> *const r = iter->next();
    if (!r->isExists()) {
      // C++ TODO: The following line could not be converted:
      throw org.apache.tools.ant.BuildException(
          L"JAR resource does not exist: " + r.getName());
    }
    if (!(std::dynamic_pointer_cast<FileResource>(r) != nullptr)) {
      // C++ TODO: The following line could not be converted:
      throw org.apache.tools.ant.BuildException(
          L"Only filesystem resource are supported: " + r.getName() +
          L", was: " + r.getClassName());
    }

    shared_ptr<File> jarFile =
        (std::static_pointer_cast<FileResource>(r))->getFile();
    if (!checkJarFile(jarFile)) {
      errors++;
    }
    checked++;
    iter++;
  }

  log(wstring::format(
          Locale::ROOT,
          L"Scanned %d JAR file(s) for licenses (in %.2fs.), %d error(s).",
          checked, (System::currentTimeMillis() - start) / 1000.0, errors),
      errors > 0 ? Project::MSG_ERR : Project::MSG_INFO);
}

bool LicenseCheckTask::checkJarFile(shared_ptr<File> jarFile)
{
  log(L"Scanning: " + jarFile->getPath(), verboseLevel);

  if (!skipChecksum) {
    bool skipDueToSnapshot =
        skipSnapshotsChecksum && jarFile->getName()->contains(L"-SNAPSHOT");
    if (!skipDueToSnapshot &&
        !matchesRegexChecksum(jarFile, skipRegexChecksum)) {
      // validate the jar matches against our expected hash
      shared_ptr<File> *const checksumFile = make_shared<File>(
          licenseDirectory, jarFile->getName() + L"." + CHECKSUM_TYPE);
      if (!(checksumFile->exists() && checksumFile->canRead())) {
        log(L"MISSING " + CHECKSUM_TYPE + L" checksum file for: " +
                jarFile->getPath(),
            Project::MSG_ERR);
        log(L"EXPECTED " + CHECKSUM_TYPE + L" checksum file : " +
                checksumFile->getPath(),
            Project::MSG_ERR);
        this->failures = true;
        return false;
      } else {
        const wstring expectedChecksum = readChecksumFile(checksumFile);
        try {
          shared_ptr<MessageDigest> *const md =
              MessageDigest::getInstance(CHECKSUM_TYPE);
          std::deque<char> buf(CHECKSUM_BUFFER_SIZE);
          try {
            shared_ptr<FileInputStream> fis =
                make_shared<FileInputStream>(jarFile);
            try {
              shared_ptr<DigestInputStream> dis =
                  make_shared<DigestInputStream>(fis, md);
              try {
                while (dis->read(buf, 0, CHECKSUM_BUFFER_SIZE) != -1) {
                  // NOOP
                }
              }
              // C++ TODO: There is no native C++ equivalent to the exception
              // 'finally' clause:
              finally {
                dis->close();
              }
            }
            // C++ TODO: There is no native C++ equivalent to the exception
            // 'finally' clause:
            finally {
              fis->close();
            }
          } catch (const IOException &ioe) {
            throw make_shared<BuildException>(
                L"IO error computing checksum of file: " + jarFile, ioe);
          }
          const std::deque<char> checksumBytes = md->digest();
          const wstring checksum = createChecksumString(checksumBytes);
          if (checksum != expectedChecksum) {
            log(L"CHECKSUM FAILED for " + jarFile->getPath() +
                    L" (expected: \"" + expectedChecksum + L"\" was: \"" +
                    checksum + L"\")",
                Project::MSG_ERR);
            this->failures = true;
            return false;
          }

        } catch (const NoSuchAlgorithmException &ae) {
          throw make_shared<BuildException>(L"Digest type " + CHECKSUM_TYPE +
                                                L" not supported by your JVM",
                                            ae);
        }
      }
    } else if (skipDueToSnapshot) {
      log(L"Skipping jar because it is a SNAPSHOT : " +
              jarFile->getAbsolutePath(),
          Project::MSG_INFO);
    } else {
      log(L"Skipping jar because it matches regex pattern: " +
              jarFile->getAbsolutePath() + L" pattern: " +
              skipRegexChecksum->pattern(),
          Project::MSG_INFO);
    }
  }

  // Get the expected license path base from the mapper and search for license
  // files.
  unordered_map<std::shared_ptr<File>, LicenseType> foundLicenses =
      make_shared<LinkedHashMap<std::shared_ptr<File>, LicenseType>>();
  deque<std::shared_ptr<File>> expectedLocations =
      deque<std::shared_ptr<File>>();
  for (wstring mappedPath : licenseMapper->mapFileName(jarFile->getName())) {
    for (LicenseType licenseType : LicenseType::values()) {
      shared_ptr<File> licensePath = make_shared<File>(
          licenseDirectory, mappedPath + licenseType.licenseFileSuffix());
      if (licensePath->exists()) {
        foundLicenses.emplace(licensePath, licenseType);
        log(L" FOUND " + licenseType.name() + L" license at " +
                licensePath->getPath(),
            verboseLevel);
        // We could continue scanning here to detect duplicate associations?
        goto outerBreak;
      } else {
        expectedLocations.push_back(licensePath);
      }
    }
  outerContinue:;
  }
outerBreak:

  // Check for NOTICE files.
  for (auto e : foundLicenses) {
    LicenseType license = e.second;
    wstring licensePath = e.first::getName();
    wstring baseName = licensePath.substr(
        0, licensePath.length() - license.licenseFileSuffix()->length());
    shared_ptr<File> noticeFile = make_shared<File>(
        licenseDirectory, baseName + license.noticeFileSuffix());

    if (noticeFile->exists()) {
      log(L" FOUND NOTICE file at " + noticeFile->getAbsolutePath(),
          verboseLevel);
    } else {
      if (license.isNoticeRequired()) {
        this->failures = true;
        log(L"MISSING NOTICE for the license file:\n  " + licensePath +
                L"\n  Expected location below:\n  " +
                noticeFile->getAbsolutePath(),
            Project::MSG_ERR);
      }
    }
  }

  // In case there is something missing, complain.
  if (foundLicenses.empty()) {
    this->failures = true;
    shared_ptr<StringBuilder> message = make_shared<StringBuilder>();
    message->append(L"MISSING LICENSE for the following file:\n  " +
                    jarFile->getAbsolutePath() +
                    L"\n  Expected locations below:\n");
    for (auto location : expectedLocations) {
      message->append(L"  => ")
          ->append(location->getAbsolutePath())
          ->append(L"\n");
    }
    log(message->toString(), Project::MSG_ERR);
    return false;
  }

  return true;
}

wstring LicenseCheckTask::createChecksumString(std::deque<char> &digest)
{
  shared_ptr<StringBuilder> checksum = make_shared<StringBuilder>();
  for (int i = 0; i < digest.size(); i++) {
    checksum->append(
        wstring::format(Locale::ROOT, L"%02x", CHECKSUM_BYTE_MASK & digest[i]));
  }
  return checksum->toString();
}

wstring LicenseCheckTask::readChecksumFile(shared_ptr<File> f)
{
  shared_ptr<BufferedReader> reader = nullptr;
  try {
    reader = make_shared<BufferedReader>(make_shared<InputStreamReader>(
        make_shared<FileInputStream>(f), StandardCharsets::UTF_8));
    try {
      wstring checksum = reader->readLine();
      if (L"" == checksum || 0 == checksum.length()) {
        // C++ TODO: The following line could not be converted:
        throw org.apache.tools.ant.BuildException(
            L"Failed to find checksum in file: " + f);
      }
      return checksum;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      reader->close();
    }
  } catch (const IOException &e) {
    throw make_shared<BuildException>(L"IO error reading checksum file: " + f,
                                      e);
  }
}

bool LicenseCheckTask::matchesRegexChecksum(
    shared_ptr<File> jarFile, shared_ptr<Pattern> skipRegexChecksum)
{
  if (skipRegexChecksum == nullptr) {
    return false;
  }
  shared_ptr<Matcher> m = skipRegexChecksum->matcher(jarFile->getName());
  return m->matches();
}
} // namespace org::apache::lucene::validation