using namespace std;

#include "LibVersionsCheckTask.h"

namespace org::apache::lucene::validation
{
using org::apache::ivy::Ivy;
using org::apache::ivy::core::LogOptions;
using org::apache::ivy::core::report::ResolveReport;
using org::apache::ivy::core::resolve::ResolveOptions;
using org::apache::ivy::core::settings::IvySettings;
using org::apache::ivy::plugins::conflict::NoConflictManager;
using InterpolatedProperties =
    org::apache::lucene::dependencies::InterpolatedProperties;
using IvyNodeElement = org::apache::lucene::validation::ivyde::IvyNodeElement;
using IvyNodeElementAdapter =
    org::apache::lucene::validation::ivyde::IvyNodeElementAdapter;
using org::apache::tools::ant::BuildException;
using org::apache::tools::ant::Project;
using org::apache::tools::ant::Task;
using org::apache::tools::ant::types::LogLevel;
using org::apache::tools::ant::types::Resource;
using org::apache::tools::ant::types::ResourceCollection;
using org::apache::tools::ant::types::resources::FileResource;
using org::apache::tools::ant::types::resources::Resources;
using org::xml::sax::Attributes;
using org::xml::sax::InputSource;
using org::xml::sax::SAXException;
using org::xml::sax::XMLReader;
using org::xml::sax::helpers::DefaultHandler;
using org::xml::sax::helpers::XMLReaderFactory;
const wstring LibVersionsCheckTask::IVY_XML_FILENAME = L"ivy.xml";
const shared_ptr<java::util::regex::Pattern>
    LibVersionsCheckTask::COORDINATE_KEY_PATTERN =
        java::util::regex::Pattern::compile(L"(/([^/ \t\f]+)/([^=:/ \t\f]+))");
const shared_ptr<java::util::regex::Pattern>
    LibVersionsCheckTask::BLANK_OR_COMMENT_LINE_PATTERN =
        java::util::regex::Pattern::compile(L"[ \t\f]*(?:[#!].*)?");
const shared_ptr<java::util::regex::Pattern>
    LibVersionsCheckTask::TRAILING_BACKSLASH_PATTERN =
        java::util::regex::Pattern::compile(L"[^\\\\]*(\\\\+)$");
const shared_ptr<java::util::regex::Pattern>
    LibVersionsCheckTask::LEADING_WHITESPACE_PATTERN =
        java::util::regex::Pattern::compile(L"[ \t\f]+(.*)");
const shared_ptr<java::util::regex::Pattern>
    LibVersionsCheckTask::WHITESPACE_GOODSTUFF_WHITESPACE_BACKSLASH_PATTERN =
        java::util::regex::Pattern::compile(
            L"[ \t\f]*(.*?)(?:(?<!\\\\)[ \t\f]*)?\\\\");
const shared_ptr<java::util::regex::Pattern>
    LibVersionsCheckTask::TRAILING_WHITESPACE_BACKSLASH_PATTERN =
        java::util::regex::Pattern::compile(L"(.*?)(?:(?<!\\\\)[ \t\f]*)?\\\\");
const shared_ptr<java::util::regex::Pattern>
    LibVersionsCheckTask::MODULE_NAME_PATTERN =
        java::util::regex::Pattern::compile(
            L"\\smodule\\s*=\\s*[\"']([^\"']+)[\"']");
const shared_ptr<java::util::regex::Pattern>
    LibVersionsCheckTask::MODULE_DIRECTORY_PATTERN =
        java::util::regex::Pattern::compile(
            L".*[/\\\\]((?:lucene|solr)[/\\\\].*)[/\\\\].*");

LibVersionsCheckTask::Dependency::Dependency(const wstring &org,
                                             const wstring &name,
                                             const wstring &directVersion)
{
  this->org = org;
  this->name = name;
  this->directVersion = directVersion;
}

void LibVersionsCheckTask::add(shared_ptr<ResourceCollection> rc)
{
  ivyXmlResources->add(rc);
}

void LibVersionsCheckTask::setVerbose(bool verbose)
{
  verboseLevel = (verbose ? Project::MSG_INFO : Project::MSG_VERBOSE);
}

void LibVersionsCheckTask::setCentralizedVersionsFile(shared_ptr<File> file)
{
  centralizedVersionsFile = file;
}

void LibVersionsCheckTask::setTopLevelIvySettingsFile(shared_ptr<File> file)
{
  topLevelIvySettingsFile = file;
}

void LibVersionsCheckTask::setIvyResolutionCacheDir(shared_ptr<File> dir)
{
  ivyResolutionCacheDir = dir;
}

void LibVersionsCheckTask::setIvyLockStrategy(const wstring &strategy)
{
  this->ivyLockStrategy = strategy;
}

void LibVersionsCheckTask::setCommonBuildDir(shared_ptr<File> file)
{
  commonBuildDir = file;
}

void LibVersionsCheckTask::setIgnoreConflictsFile(shared_ptr<File> file)
{
  ignoreConflictsFile = file;
}

void LibVersionsCheckTask::execute() 
{
  log(L"Starting scan.", verboseLevel);
  int64_t start = System::currentTimeMillis();

  setupIvy();

  int numErrors = 0;
  if (!verifySortedCoordinatesPropertiesFile(centralizedVersionsFile)) {
    ++numErrors;
  }
  if (!verifySortedCoordinatesPropertiesFile(ignoreConflictsFile)) {
    ++numErrors;
  }
  collectDirectDependencies();
  if (!collectVersionConflictsToIgnore()) {
    ++numErrors;
  }

  int numChecked = 0;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unchecked")
  // java.util.Iterator<org.apache.tools.ant.types.Resource> iter =
  // (java.util.Iterator<org.apache.tools.ant.types.Resource>)ivyXmlResources.iterator();
  Resources::const_iterator iter =
      std::static_pointer_cast<Iterator<std::shared_ptr<Resource>>>(
          ivyXmlResources->begin());
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  while (iter->hasNext()) {
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    shared_ptr<Resource> *const resource = iter->next();
    if (!resource->isExists()) {
      // C++ TODO: The following line could not be converted:
      throw org.apache.tools.ant.BuildException(L"Resource does not exist: " +
                                                resource.getName());
    }
    if (!(std::dynamic_pointer_cast<FileResource>(resource) != nullptr)) {
      // C++ TODO: The following line could not be converted:
      throw org.apache.tools.ant.BuildException(
          L"Only filesystem resources are supported: " + resource.getName() +
          L", was: " + resource.getClassName());
    }

    shared_ptr<File> ivyXmlFile =
        (std::static_pointer_cast<FileResource>(resource))->getFile();
    try {
      if (!checkIvyXmlFile(ivyXmlFile)) {
        ++numErrors;
      }
      if (!resolveTransitively(ivyXmlFile)) {
        ++numErrors;
      }
      if (!findLatestConflictVersions()) {
        ++numErrors;
      }
    } catch (const runtime_error &e) {
      throw make_shared<BuildException>(L"Exception reading file " +
                                            ivyXmlFile->getPath() + L" - " +
                                            e.what(),
                                        e);
    }
    ++numChecked;
    iter++;
  }

  log(L"Checking for orphans in " + centralizedVersionsFile->getName(),
      verboseLevel);
  for (auto entry : directDependencies) {
    wstring coordinateKey = entry.first;
    if (!entry.second::directlyReferenced) {
      log(L"ORPHAN coordinate key '" + coordinateKey + L"' in " +
              centralizedVersionsFile->getName() + L" is not found in any " +
              IVY_XML_FILENAME + L" file.",
          Project::MSG_ERR);
      ++numErrors;
    }
  }

  int numConflicts = emitConflicts();

  int messageLevel = numErrors > 0 ? Project::MSG_ERR : Project::MSG_INFO;
  log(L"Checked that " + centralizedVersionsFile->getName() + L" and " +
          ignoreConflictsFile->getName() +
          L" have lexically sorted '/org/name' keys and no duplicates or "
          L"orphans.",
      messageLevel);
  log(L"Scanned " + to_wstring(numChecked) + L" " + IVY_XML_FILENAME +
          L" files for rev=\"${/org/name}\" format.",
      messageLevel);
  log(L"Found " + to_wstring(numConflicts) +
      L" indirect dependency version conflicts.");
  log(wstring::format(Locale::ROOT, L"Completed in %.2fs., %d error(s).",
                      (System::currentTimeMillis() - start) / 1000.0,
                      numErrors),
      messageLevel);

  if (numConflicts > 0 || numErrors > 0) {
    // C++ TODO: The following line could not be converted:
    throw org.apache.tools.ant.BuildException(
        L"Lib versions check failed. Check the logs.");
  }
}

bool LibVersionsCheckTask::findLatestConflictVersions()
{
  bool success = true;
  shared_ptr<StringBuilder> latestIvyXml = make_shared<StringBuilder>();
  latestIvyXml->append(L"<ivy-module version=\"2.0\">\n");
  latestIvyXml->append(L"  <info organisation=\"org.apache.lucene\" "
                       L"module=\"core-tools-find-latest-revision\"/>\n");
  latestIvyXml->append(L"  <configurations>\n");
  latestIvyXml->append(L"    <conf name=\"default\" transitive=\"false\"/>\n");
  latestIvyXml->append(L"  </configurations>\n");
  latestIvyXml->append(L"  <dependencies>\n");
  for (auto directDependency : directDependencies) {
    shared_ptr<Dependency> dependency = directDependency.second;
    if (dependency->conflictLocations->entrySet()->isEmpty()) {
      continue;
    }
    latestIvyXml->append(L"    <dependency org=\"");
    latestIvyXml->append(dependency->org);
    latestIvyXml->append(L"\" name=\"");
    latestIvyXml->append(dependency->name);
    latestIvyXml->append(L"\" rev=\"latest.release\" conf=\"default->*\"/>\n");
  }
  latestIvyXml->append(L"  </dependencies>\n");
  latestIvyXml->append(L"</ivy-module>\n");
  shared_ptr<File> buildDir =
      make_shared<File>(commonBuildDir, L"ivy-transitive-resolve");
  if (!buildDir->exists() && !buildDir->mkdirs()) {
    // C++ TODO: The following line could not be converted:
    throw org.apache.tools.ant.BuildException(
        L"Could not create temp directory " + buildDir.getPath());
  }
  shared_ptr<File> findLatestIvyXmlFile =
      make_shared<File>(buildDir, L"find.latest.conflicts.ivy.xml");
  try {
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (java.io.Writer writer = new
    // java.io.OutputStreamWriter(new
    // java.io.FileOutputStream(findLatestIvyXmlFile),
    // java.nio.charset.StandardCharsets.UTF_8))
    {
      java::io::Writer writer = java::io::OutputStreamWriter(
          make_shared<java::io::FileOutputStream>(findLatestIvyXmlFile),
          java::nio::charset::StandardCharsets::UTF_8);
      writer.write(latestIvyXml->toString());
    }
    shared_ptr<ResolveOptions> options = make_shared<ResolveOptions>();
    options->setDownload(
        false); // Download only module descriptors, not artifacts
    options->setTransitive(false);          // Resolve only direct dependencies
    options->setUseCacheOnly(false);        // Download the internet!
    options->setOutputReport(false);        // Don't print to the console
    options->setLog(LogOptions::LOG_QUIET); // Don't log to the console
    options->setConfs(std::deque<wstring>{L"*"}); // Resolve all configurations
    shared_ptr<ResolveReport> resolveReport =
        ivy->resolve(findLatestIvyXmlFile->toURI().toURL(), options);
    shared_ptr<IvyNodeElement> root =
        IvyNodeElementAdapter::adapt(resolveReport);
    for (auto element : root->getDependencies()) {
      wstring coordinate =
          L"/" + element->getOrganization() + L"/" + element->getName();
      shared_ptr<Dependency> dependency = directDependencies[coordinate];
      if (nullptr == dependency) {
        log(L"ERROR: the following coordinate key does not appear in " +
                centralizedVersionsFile->getName() + L": " + coordinate,
            Project::MSG_ERR);
        success = false;
      } else {
        dependency->latestVersion = element->getRevision();
      }
    }
  } catch (const IOException &e) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    log(L"Exception writing to " + findLatestIvyXmlFile->getPath() + L": " +
            e->toString(),
        Project::MSG_ERR);
    success = false;
  } catch (const ParseException &e) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    log(L"Exception parsing filename " + findLatestIvyXmlFile->getPath() +
            L": " + e->toString(),
        Project::MSG_ERR);
    success = false;
  }
  return success;
}

bool LibVersionsCheckTask::collectVersionConflictsToIgnore()
{
  log(L"Checking for orphans in " + ignoreConflictsFile->getName(),
      verboseLevel);
  bool orphansFound = false;
  shared_ptr<InterpolatedProperties> properties =
      make_shared<InterpolatedProperties>();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.InputStream inputStream = new
  // java.io.FileInputStream(ignoreConflictsFile); java.io.Reader reader = new
  // java.io.InputStreamReader(inputStream,
  // java.nio.charset.StandardCharsets.UTF_8))
  {
    java::io::InputStream inputStream =
        java::io::FileInputStream(ignoreConflictsFile);
    java::io::Reader reader = java::io::InputStreamReader(
        inputStream, java::nio::charset::StandardCharsets::UTF_8);
    try {
      properties->load(reader);
    } catch (const IOException &e) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      throw make_shared<BuildException>(
          L"Exception reading " + ignoreConflictsFile + L": " + e->toString(),
          e);
    }
  }
  for (auto obj : properties) {
    wstring coordinate = static_cast<wstring>(obj.first);
    if (COORDINATE_KEY_PATTERN->matcher(coordinate).matches()) {
      if (directDependencies.find(coordinate) == directDependencies.end()) {
        orphansFound = true;
        log(L"ORPHAN coordinate key '" + coordinate + L"' in " +
                ignoreConflictsFile->getName() + L" is not found in " +
                centralizedVersionsFile->getName(),
            Project::MSG_ERR);
      } else {
        wstring versionsToIgnore = properties->getProperty(coordinate);
        deque<wstring> ignore = Arrays::asList(
            StringHelper::trim(versionsToIgnore)->split(L"\\s*,\\s*|\\s+"));
        ignoreConflictVersions.emplace(coordinate, unordered_set<>(ignore));
      }
    }
  }
  return !orphansFound;
}

void LibVersionsCheckTask::collectDirectDependencies()
{
  shared_ptr<InterpolatedProperties> properties =
      make_shared<InterpolatedProperties>();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.InputStream inputStream = new
  // java.io.FileInputStream(centralizedVersionsFile); java.io.Reader reader =
  // new java.io.InputStreamReader(inputStream,
  // java.nio.charset.StandardCharsets.UTF_8))
  {
    java::io::InputStream inputStream =
        java::io::FileInputStream(centralizedVersionsFile);
    java::io::Reader reader = java::io::InputStreamReader(
        inputStream, java::nio::charset::StandardCharsets::UTF_8);
    try {
      properties->load(reader);
    } catch (const IOException &e) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      throw make_shared<BuildException>(L"Exception reading " +
                                            centralizedVersionsFile + L": " +
                                            e->toString(),
                                        e);
    }
  }
  for (auto obj : properties) {
    wstring coordinate = static_cast<wstring>(obj.first);
    shared_ptr<Matcher> matcher = COORDINATE_KEY_PATTERN->matcher(coordinate);
    if (matcher->matches()) {
      wstring org = matcher->group(2);
      wstring name = matcher->group(3);
      wstring directVersion = properties->getProperty(coordinate);
      shared_ptr<Dependency> dependency =
          make_shared<Dependency>(org, name, directVersion);
      directDependencies.emplace(coordinate, dependency);
    }
  }
}

bool LibVersionsCheckTask::resolveTransitively(shared_ptr<File> ivyXmlFile)
{
  bool success = true;

  shared_ptr<ResolveOptions> options = make_shared<ResolveOptions>();
  options->setDownload(
      false); // Download only module descriptors, not artifacts
  options->setTransitive(true);    // Resolve transitively, if not already
                                   // specified in the ivy.xml file
  options->setUseCacheOnly(false); // Download the internet!
  options->setOutputReport(false); // Don't print to the console
  options->setLog(LogOptions::LOG_QUIET);        // Don't log to the console
  options->setConfs(std::deque<wstring>{L"*"}); // Resolve all configurations

  // Rewrite the ivy.xml, replacing all 'transitive="false"' with
  // 'transitive="true"' The Ivy API is file-based, so we have to write the
  // result to the filesystem.
  wstring moduleName = L"unknown";
  wstring ivyXmlContent = xmlToString(ivyXmlFile);
  shared_ptr<Matcher> matcher = MODULE_NAME_PATTERN->matcher(ivyXmlContent);
  if (matcher->find()) {
    moduleName = matcher->group(1);
  }
  ivyXmlContent = ivyXmlContent.replaceAll(
      L"\\btransitive\\s*=\\s*[\"']false[\"']", L"transitive=\"true\"");
  shared_ptr<File> transitiveIvyXmlFile = nullptr;
  try {
    shared_ptr<File> buildDir =
        make_shared<File>(commonBuildDir, L"ivy-transitive-resolve");
    if (!buildDir->exists() && !buildDir->mkdirs()) {
      // C++ TODO: The following line could not be converted:
      throw org.apache.tools.ant.BuildException(
          L"Could not create temp directory " + buildDir.getPath());
    }
    matcher = MODULE_DIRECTORY_PATTERN->matcher(ivyXmlFile->getCanonicalPath());
    if (!matcher->matches()) {
      // C++ TODO: The following line could not be converted:
      throw org.apache.tools.ant.BuildException(
          L"Unknown ivy.xml module directory: " +
          ivyXmlFile.getCanonicalPath());
    }
    wstring moduleDirPrefix = matcher->group(1).replaceAll(L"[/\\\\]", L".");
    transitiveIvyXmlFile = make_shared<File>(
        buildDir, L"transitive." + moduleDirPrefix + L".ivy.xml");
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (java.io.Writer writer = new
    // java.io.OutputStreamWriter(new
    // java.io.FileOutputStream(transitiveIvyXmlFile),
    // java.nio.charset.StandardCharsets.UTF_8))
    {
      java::io::Writer writer = java::io::OutputStreamWriter(
          make_shared<java::io::FileOutputStream>(transitiveIvyXmlFile),
          java::nio::charset::StandardCharsets::UTF_8);
      writer.write(ivyXmlContent);
    }
    shared_ptr<ResolveReport> resolveReport =
        ivy->resolve(transitiveIvyXmlFile->toURI().toURL(), options);
    shared_ptr<IvyNodeElement> root =
        IvyNodeElementAdapter::adapt(resolveReport);
    for (auto directDependency : root->getDependencies()) {
      wstring coordinate = L"/" + directDependency->getOrganization() + L"/" +
                           directDependency->getName();
      shared_ptr<Dependency> dependency = directDependencies[coordinate];
      if (nullptr == dependency) {
        log(L"ERROR: the following coordinate key does not appear in " +
            centralizedVersionsFile->getName() + L": " + coordinate);
        success = false;
      } else {
        dependency->directlyReferenced = true;
        if (collectConflicts(directDependency, directDependency, moduleName)) {
          success = false;
        }
      }
    }
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (ParseException | IOException e) {
    if (nullptr != transitiveIvyXmlFile) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      log(L"Exception reading " + transitiveIvyXmlFile->getPath() + L": " +
          e->toString());
    }
    success = false;
  }
  return success;
}

bool LibVersionsCheckTask::collectConflicts(shared_ptr<IvyNodeElement> root,
                                            shared_ptr<IvyNodeElement> parent,
                                            const wstring &moduleName)
{
  bool conflicts = false;
  for (auto child : parent->getDependencies()) {
    wstring coordinate =
        L"/" + child->getOrganization() + L"/" + child->getName();
    shared_ptr<Dependency> dependency = directDependencies[coordinate];
    if (nullptr != dependency) { // Ignore this indirect dependency if it's not
                                 // also a direct dependency
      wstring indirectVersion = child->getRevision();
      if (isConflict(coordinate, dependency->directVersion, indirectVersion)) {
        conflicts = true;
        shared_ptr<Set<wstring>> moduleNames =
            dependency->conflictLocations->get(root);
        if (nullptr == moduleNames) {
          moduleNames = unordered_set<>();
          dependency->conflictLocations->put(root, moduleNames);
        }
        moduleNames->add(moduleName);
      }
      conflicts |= collectConflicts(root, child, moduleName);
    }
  }
  return conflicts;
}

const unordered_map<wstring, int> LibVersionsCheckTask::SPECIAL_MEANINGS;

LibVersionsCheckTask::StaticConstructor::StaticConstructor()
{
  SPECIAL_MEANINGS = unordered_map<>();
  SPECIAL_MEANINGS.emplace(L"dev", -1);
  SPECIAL_MEANINGS.emplace(L"rc", 1);
  SPECIAL_MEANINGS.emplace(L"final", 2);
}

LibVersionsCheckTask::StaticConstructor LibVersionsCheckTask::staticConstructor;

int LibVersionsCheckTask::LatestVersionComparator::compare(const wstring &rev1,
                                                           const wstring &rev2)
{
  rev1 = rev1.replaceAll(L"([a-zA-Z])(\\d)", L"$1.$2");
  rev1 = rev1.replaceAll(L"(\\d)([a-zA-Z])", L"$1.$2");
  rev2 = rev2.replaceAll(L"([a-zA-Z])(\\d)", L"$1.$2");
  rev2 = rev2.replaceAll(L"(\\d)([a-zA-Z])", L"$1.$2");

  std::deque<wstring> parts1 = rev1.split(L"[-._+]");
  std::deque<wstring> parts2 = rev2.split(L"[-._+]");

  int i = 0;
  for (; i < parts1.size() && i < parts2.size(); i++) {
    if (parts1[i] == parts2[i]) {
      continue;
    }
    bool is1Number = isNumber(parts1[i]);
    bool is2Number = isNumber(parts2[i]);
    if (is1Number && !is2Number) {
      return 1;
    }
    if (is2Number && !is1Number) {
      return -1;
    }
    if (is1Number && is2Number) {
      return StringHelper::fromString<int64_t>(parts1[i]).compareTo(
          StringHelper::fromString<int64_t>(parts2[i]));
    }
    // both are strings, we compare them taking into account special meaning
    optional<int> sm1 = SPECIAL_MEANINGS[parts1[i].toLowerCase(Locale::ROOT)];
    optional<int> sm2 = SPECIAL_MEANINGS[parts2[i].toLowerCase(Locale::ROOT)];
    if (sm1) {
      sm2 = !sm2 ? optional<int>(0) : sm2;
      return sm1.compareTo(sm2);
    }
    if (sm2) {
      return (optional<int>(0))->compareTo(sm2);
    }
    return parts1[i].compare(parts2[i]);
  }
  if (i < parts1.size()) {
    return isNumber(parts1[i]) ? 1 : -1;
  }
  if (i < parts2.size()) {
    return isNumber(parts2[i]) ? -1 : 1;
  }
  return 0;
}

const shared_ptr<java::util::regex::Pattern>
    LibVersionsCheckTask::LatestVersionComparator::IS_NUMBER =
        java::util::regex::Pattern::compile(L"\\d+");

bool LibVersionsCheckTask::LatestVersionComparator::isNumber(const wstring &str)
{
  return IS_NUMBER->matcher(str).matches();
}

shared_ptr<LatestVersionComparator>
    LibVersionsCheckTask::LATEST_VERSION_COMPARATOR =
        make_shared<LatestVersionComparator>();

bool LibVersionsCheckTask::isConflict(const wstring &coordinate,
                                      const wstring &directVersion,
                                      const wstring &indirectVersion)
{
  bool isConflict =
      LATEST_VERSION_COMPARATOR->compare(directVersion, indirectVersion) < 0;
  if (isConflict) {
    shared_ptr<Set<wstring>> ignoredVersions =
        ignoreConflictVersions[coordinate];
    if (nullptr != ignoredVersions &&
        ignoredVersions->contains(indirectVersion)) {
      isConflict = false;
    }
  }
  return isConflict;
}

int LibVersionsCheckTask::emitConflicts()
{
  int conflicts = 0;
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  for (auto directDependency : directDependencies) {
    wstring coordinate = directDependency.first;
    shared_ptr<std::unordered_set<
        unordered_map::Entry<std::shared_ptr<IvyNodeElement>, Set<wstring>>>>
        entrySet = directDependency.second::conflictLocations::entrySet();
    if (entrySet->isEmpty()) {
      continue;
    }
    ++conflicts;
    unordered_map::Entry<std::shared_ptr<IvyNodeElement>, Set<wstring>> first =
        entrySet->begin()->next();
    int notPrinted = entrySet->size() - 1;
    builder->append(L"VERSION CONFLICT: transitive dependency in module(s) ");
    bool isFirst = true;
    for (wstring moduleName : first.getValue()) {
      if (isFirst) {
        isFirst = false;
      } else {
        builder->append(L", ");
      }
      builder->append(moduleName);
    }
    builder->append(L":\n");
    shared_ptr<IvyNodeElement> element = first.getKey();
    builder->append(L'/')
        ->append(element->getOrganization())
        ->append(L'/')
        ->append(element->getName())
        ->append(L'=')
        ->append(element->getRevision())
        ->append(L'\n');
    emitConflict(builder, coordinate, first.getKey(), 1);

    if (notPrinted > 0) {
      builder->append(L"... and ")->append(notPrinted)->append(L" more\n");
    }
    builder->append(L"\n");
  }
  if (builder->length() > 0) {
    log(builder->toString());
  }
  return conflicts;
}

bool LibVersionsCheckTask::emitConflict(shared_ptr<StringBuilder> builder,
                                        const wstring &conflictCoordinate,
                                        shared_ptr<IvyNodeElement> parent,
                                        int depth)
{
  for (auto child : parent->getDependencies()) {
    wstring indirectCoordinate =
        L"/" + child->getOrganization() + L"/" + child->getName();
    if (conflictCoordinate == indirectCoordinate) {
      shared_ptr<Dependency> dependency =
          directDependencies[conflictCoordinate];
      wstring directVersion = dependency->directVersion;
      if (isConflict(conflictCoordinate, directVersion, child->getRevision())) {
        for (int i = 0; i < depth - 1; ++i) {
          builder->append(L"    ");
        }
        builder->append(L"+-- ");
        builder->append(indirectCoordinate)
            ->append(L"=")
            ->append(child->getRevision());
        builder->append(L" <<< Conflict (direct=")->append(directVersion);
        builder->append(L", latest=")
            ->append(dependency->latestVersion)
            ->append(L")\n");
        return true;
      }
    } else if (hasConflicts(conflictCoordinate, child)) {
      for (int i = 0; i < depth - 1; ++i) {
        builder->append(L"    ");
      }
      builder->append(L"+-- ");
      builder->append(indirectCoordinate)
          ->append(L"=")
          ->append(child->getRevision())
          ->append(L"\n");
      if (emitConflict(builder, conflictCoordinate, child, depth + 1)) {
        return true;
      }
    }
  }
  return false;
}

bool LibVersionsCheckTask::hasConflicts(const wstring &conflictCoordinate,
                                        shared_ptr<IvyNodeElement> parent)
{
  // the element itself will never be in conflict, since its coordinate is
  // different
  for (auto child : parent->getDependencies()) {
    wstring indirectCoordinate =
        L"/" + child->getOrganization() + L"/" + child->getName();
    if (conflictCoordinate == indirectCoordinate) {
      shared_ptr<Dependency> dependency =
          directDependencies[conflictCoordinate];
      if (isConflict(conflictCoordinate, dependency->directVersion,
                     child->getRevision())) {
        return true;
      }
    } else if (hasConflicts(conflictCoordinate, child)) {
      return true;
    }
  }
  return false;
}

wstring LibVersionsCheckTask::xmlToString(shared_ptr<File> ivyXmlFile)
{
  shared_ptr<StringWriter> writer = make_shared<StringWriter>();
  try {
    shared_ptr<StreamSource> inputSource = make_shared<StreamSource>(
        make_shared<FileInputStream>(ivyXmlFile->getPath()));
    shared_ptr<Transformer> serializer =
        TransformerFactory::newInstance().newTransformer();
    serializer->transform(inputSource, make_shared<StreamResult>(writer));
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (TransformerException | IOException e) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    throw make_shared<BuildException>(
        L"Exception reading " + ivyXmlFile->getPath() + L": " + e->toString(),
        e);
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return writer->toString();
}

void LibVersionsCheckTask::setupIvy()
{
  shared_ptr<IvySettings> ivySettings = make_shared<IvySettings>();
  try {
    ivySettings->setVariable(L"common.build.dir",
                             commonBuildDir->getAbsolutePath());
    ivySettings->setVariable(L"ivy.exclude.types", L"source|javadoc");
    ivySettings->setVariable(L"ivy.resolution-cache.dir",
                             ivyResolutionCacheDir->getAbsolutePath());
    ivySettings->setVariable(L"ivy.lock-strategy", ivyLockStrategy);
    ivySettings->setVariable(
        L"ivysettings.xml",
        getProject().getProperty(L"ivysettings.xml")); // nested settings file
    ivySettings->setBaseDir(commonBuildDir);
    ivySettings->setDefaultConflictManager(make_shared<NoConflictManager>());
    ivy = Ivy::newInstance(ivySettings);
    ivy->configure(topLevelIvySettingsFile);
  } catch (const runtime_error &e) {
    throw make_shared<BuildException>(L"Exception reading " +
                                          topLevelIvySettingsFile->getPath() +
                                          L": " + e.what(),
                                      e);
  }
}

bool LibVersionsCheckTask::verifySortedCoordinatesPropertiesFile(
    shared_ptr<File> coordinatePropertiesFile)
{
  log(L"Checking for lexically sorted non-duplicated '/org/name' keys in: " +
          coordinatePropertiesFile,
      verboseLevel);
  bool success = true;
  wstring line = L"";
  wstring currentKey = L"";
  wstring previousKey = L"";
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.InputStream stream = new
  // java.io.FileInputStream(coordinatePropertiesFile); java.io.Reader reader =
  // new java.io.InputStreamReader(stream,
  // java.nio.charset.StandardCharsets.ISO_8859_1); java.io.BufferedReader
  // bufferedReader = new java.io.BufferedReader(reader))
  {
    java::io::InputStream stream =
        java::io::FileInputStream(coordinatePropertiesFile);
    java::io::Reader reader = java::io::InputStreamReader(
        stream, java::nio::charset::StandardCharsets::ISO_8859_1);
    java::io::BufferedReader bufferedReader = java::io::BufferedReader(reader);
    try {
      while (L"" != (line = readLogicalPropertiesLine(bufferedReader))) {
        shared_ptr<Matcher> *const keyMatcher =
            COORDINATE_KEY_PATTERN->matcher(line);
        if (!keyMatcher->lookingAt()) {
          continue; // Ignore keys that don't look like "/org/name"
        }
        currentKey = keyMatcher->group(1);
        if (L"" != previousKey) {
          int comparison = currentKey.compare(previousKey);
          if (0 == comparison) {
            log(L"DUPLICATE coordinate key '" + currentKey + L"' in " +
                    coordinatePropertiesFile->getName(),
                Project::MSG_ERR);
            success = false;
          } else if (comparison < 0) {
            log(L"OUT-OF-ORDER coordinate key '" + currentKey + L"' in " +
                    coordinatePropertiesFile->getName(),
                Project::MSG_ERR);
            success = false;
          }
        }
        previousKey = currentKey;
      }
    } catch (const IOException &e) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      throw make_shared<BuildException>(
          L"Exception reading " + coordinatePropertiesFile->getPath() + L": " +
              e->toString(),
          e);
    }
  }
  return success;
}

wstring LibVersionsCheckTask::readLogicalPropertiesLine(
    shared_ptr<BufferedReader> reader) 
{
  shared_ptr<StringBuilder> *const logicalLine = make_shared<StringBuilder>();
  wstring line;
  do {
    line = reader->readLine();
    if (L"" == line) {
      return L"";
    }
  } while (BLANK_OR_COMMENT_LINE_PATTERN->matcher(line).matches());

  shared_ptr<Matcher> backslashMatcher =
      TRAILING_BACKSLASH_PATTERN->matcher(line);
  // Check for a non-escaped backslash
  if (backslashMatcher->find() &&
      1 == (backslashMatcher->group(1)->length() % 2)) {
    shared_ptr<Matcher> *const firstLineMatcher =
        TRAILING_WHITESPACE_BACKSLASH_PATTERN->matcher(line);
    if (firstLineMatcher->matches()) {
      logicalLine->append(firstLineMatcher->group(
          1)); // trim trailing backslash and any preceding whitespace
    }
    line = reader->readLine();
    while (
        L"" != line &&
        (backslashMatcher = TRAILING_BACKSLASH_PATTERN->matcher(line)).find() &&
        1 == (backslashMatcher->group(1)->length() % 2)) {
      // Trim leading whitespace, the trailing backslash and any preceding
      // whitespace
      shared_ptr<Matcher> *const goodStuffMatcher =
          WHITESPACE_GOODSTUFF_WHITESPACE_BACKSLASH_PATTERN->matcher(line);
      if (goodStuffMatcher->matches()) {
        logicalLine->append(goodStuffMatcher->group(1));
      }
      line = reader->readLine();
    }
    if (L"" != line) {
      // line can't have a non-escaped trailing backslash
      shared_ptr<Matcher> *const leadingWhitespaceMatcher =
          LEADING_WHITESPACE_PATTERN->matcher(line);
      if (leadingWhitespaceMatcher->matches()) {
        line = leadingWhitespaceMatcher->group(1); // trim leading whitespace
      }
      logicalLine->append(line);
    }
  } else {
    logicalLine->append(line);
  }
  // trim non-escaped leading whitespace
  shared_ptr<Matcher> *const leadingWhitespaceMatcher =
      LEADING_WHITESPACE_PATTERN->matcher(logicalLine);
  shared_ptr<std::wstring> *const leadingWhitespaceStripped =
      leadingWhitespaceMatcher->matches() ? leadingWhitespaceMatcher->group(1)
                                          : logicalLine;

  // unescape all chars in the logical line
  shared_ptr<StringBuilder> output = make_shared<StringBuilder>();
  constexpr int numChars = leadingWhitespaceStripped->length();
  for (int pos = 0; pos < numChars - 1; ++pos) {
    wchar_t ch = leadingWhitespaceStripped->charAt(pos);
    if (ch == L'\\') {
      ch = leadingWhitespaceStripped->charAt(++pos);
    }
    output->append(ch);
  }
  if (numChars > 0) {
    output->append(leadingWhitespaceStripped->charAt(numChars - 1));
  }

  return output->toString();
}

bool LibVersionsCheckTask::checkIvyXmlFile(shared_ptr<File> ivyXmlFile) throw(
    ParserConfigurationException, SAXException, IOException)
{
  log(L"Scanning: " + ivyXmlFile->getPath(), verboseLevel);
  shared_ptr<XMLReader> xmlReader = XMLReaderFactory::createXMLReader();
  shared_ptr<DependencyRevChecker> revChecker =
      make_shared<DependencyRevChecker>(shared_from_this(), ivyXmlFile);
  xmlReader->setContentHandler(revChecker);
  xmlReader->setErrorHandler(revChecker);
  xmlReader->parse(make_shared<InputSource>(ivyXmlFile->getAbsolutePath()));
  return !revChecker->fail;
}

LibVersionsCheckTask::DependencyRevChecker::DependencyRevChecker(
    shared_ptr<LibVersionsCheckTask> outerInstance, shared_ptr<File> ivyXmlFile)
    : ivyXmlFile(ivyXmlFile), outerInstance(outerInstance)
{
}

void LibVersionsCheckTask::DependencyRevChecker::startElement(
    const wstring &uri, const wstring &localName, const wstring &qName,
    shared_ptr<Attributes> attributes) 
{
  if (localName == L"dependency" && insideDependenciesTag()) {
    wstring org = attributes->getValue(L"org");
    bool foundAllAttributes = true;
    if (L"" == org) {
      log(L"MISSING 'org' attribute on <dependency> in " +
              ivyXmlFile->getPath(),
          org.apache::tools::ant::Project::MSG_ERR);
      fail = true;
      foundAllAttributes = false;
    }
    wstring name = attributes->getValue(L"name");
    if (L"" == name) {
      log(L"MISSING 'name' attribute on <dependency> in " +
              ivyXmlFile->getPath(),
          org.apache::tools::ant::Project::MSG_ERR);
      fail = true;
      foundAllAttributes = false;
    }
    wstring rev = attributes->getValue(L"rev");
    if (L"" == rev) {
      log(L"MISSING 'rev' attribute on <dependency> in " +
              ivyXmlFile->getPath(),
          org.apache::tools::ant::Project::MSG_ERR);
      fail = true;
      foundAllAttributes = false;
    }
    if (foundAllAttributes) {
      wstring coordinateKey = L"/" + org + StringHelper::toString(L'/') + name;
      wstring expectedRev =
          L"${" + coordinateKey + StringHelper::toString(L'}');
      if (rev != expectedRev) {
        log(L"BAD <dependency> 'rev' attribute value '" + rev +
                L"' - expected '" + expectedRev + L"'" + L" in " +
                ivyXmlFile->getPath(),
            org.apache::tools::ant::Project::MSG_ERR);
        fail = true;
      }
      if (outerInstance->directDependencies.find(coordinateKey) ==
          outerInstance->directDependencies.end()) {
        log(L"MISSING key '" + coordinateKey + L"' in " +
                outerInstance->centralizedVersionsFile->getPath(),
            org.apache::tools::ant::Project::MSG_ERR);
        fail = true;
      }
    }
  }
  tags.push(localName);
}

void LibVersionsCheckTask::DependencyRevChecker::endElement(
    const wstring &uri, const wstring &localName,
    const wstring &qName) 
{
  tags.pop();
}

bool LibVersionsCheckTask::DependencyRevChecker::insideDependenciesTag()
{
  return tags.size() == 2 && tags.get(0).equals(L"ivy-module") &&
         tags.get(1).equals(L"dependencies");
}
} // namespace org::apache::lucene::validation