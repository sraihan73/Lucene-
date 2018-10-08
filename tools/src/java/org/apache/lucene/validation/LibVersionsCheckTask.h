#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/validation/Dependency.h"

#include  "core/src/java/org/apache/lucene/validation/ivyde/IvyNodeElement.h"

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::validation
{

using org::apache::ivy::Ivy;
using IvyNodeElement = org::apache::lucene::validation::ivyde::IvyNodeElement;
using org::apache::tools::ant::BuildException;
using org::apache::tools::ant::Project;
using org::apache::tools::ant::Task;
using org::apache::tools::ant::types::ResourceCollection;
using org::apache::tools::ant::types::resources::Resources;
using org::xml::sax::Attributes;
using org::xml::sax::SAXException;
using org::xml::sax::helpers::DefaultHandler;

/**
 * An Ant task to verify that the '/org/name' keys in ivy-versions.properties
 * are sorted lexically and are neither duplicates nor orphans, and that all
 * dependencies in all ivy.xml files use rev="${/org/name}" format.
 */
class LibVersionsCheckTask : public Task
{
  GET_CLASS_NAME(LibVersionsCheckTask)

private:
  static const std::wstring IVY_XML_FILENAME;
  static const std::shared_ptr<Pattern> COORDINATE_KEY_PATTERN;
  static const std::shared_ptr<Pattern> BLANK_OR_COMMENT_LINE_PATTERN;
  static const std::shared_ptr<Pattern> TRAILING_BACKSLASH_PATTERN;
  static const std::shared_ptr<Pattern> LEADING_WHITESPACE_PATTERN;
  static const std::shared_ptr<Pattern>
      WHITESPACE_GOODSTUFF_WHITESPACE_BACKSLASH_PATTERN;
  static const std::shared_ptr<Pattern> TRAILING_WHITESPACE_BACKSLASH_PATTERN;
  static const std::shared_ptr<Pattern> MODULE_NAME_PATTERN;
  static const std::shared_ptr<Pattern> MODULE_DIRECTORY_PATTERN;
  std::shared_ptr<Ivy> ivy;

  /**
   * All ivy.xml files to check.
   */
  std::shared_ptr<Resources> ivyXmlResources = std::make_shared<Resources>();

  /**
   * Centralized Ivy versions properties file: ivy-versions.properties
   */
  std::shared_ptr<File> centralizedVersionsFile;

  /**
   * Centralized Ivy ignore conflicts file: ivy-ignore-conflicts.properties
   */
  std::shared_ptr<File> ignoreConflictsFile;

  /**
   * Ivy settings file: top-level-ivy-settings.xml
   */
  std::shared_ptr<File> topLevelIvySettingsFile;

  /**
   * Location of common build dir: lucene/build/
   */
  std::shared_ptr<File> commonBuildDir;

  /**
   * Location of ivy cache resolution directory.
   */
  std::shared_ptr<File> ivyResolutionCacheDir;

  /**
   * Artifact lock strategy that Ivy should use.
   */
  std::wstring ivyLockStrategy;

  /**
   * A logging level associated with verbose logging.
   */
  int verboseLevel = Project::MSG_VERBOSE;

  /**
   * All /org/name keys found in ivy-versions.properties,
   * mapped to info about direct dependence and what would
   * be conflicting indirect dependencies if Lucene/Solr
   * were to use transitive dependencies.
   */
  std::unordered_map<std::wstring, std::shared_ptr<Dependency>>
      directDependencies = std::make_shared<
          LinkedHashMap<std::wstring, std::shared_ptr<Dependency>>>();

  /**
   * All /org/name keys found in ivy-ignore-conflicts.properties,
   * mapped to the set of indirect dependency versions that will
   * be ignored, i.e. not trigger a conflict.
   */
  std::unordered_map<std::wstring, std::unordered_set<std::wstring>>
      ignoreConflictVersions =
          std::unordered_map<std::wstring, std::unordered_set<std::wstring>>();

private:
  class Dependency : public std::enable_shared_from_this<Dependency>
  {
    GET_CLASS_NAME(Dependency)
  public:
    std::wstring org;
    std::wstring name;
    std::wstring directVersion;
    std::wstring latestVersion;
    bool directlyReferenced = false;
    std::shared_ptr<
        LinkedHashMap<std::shared_ptr<IvyNodeElement>, Set<std::wstring>>>
        conflictLocations = std::make_shared<LinkedHashMap<
            std::shared_ptr<IvyNodeElement>,
            Set<std::wstring>>>(); // dependency path -> moduleNames

    Dependency(const std::wstring &org, const std::wstring &name,
               const std::wstring &directVersion);
  };

  /**
   * Adds a set of ivy.xml resources to check.
   */
public:
  virtual void add(std::shared_ptr<ResourceCollection> rc);

  virtual void setVerbose(bool verbose);

  virtual void setCentralizedVersionsFile(std::shared_ptr<File> file);

  virtual void setTopLevelIvySettingsFile(std::shared_ptr<File> file);

  virtual void setIvyResolutionCacheDir(std::shared_ptr<File> dir);

  virtual void setIvyLockStrategy(const std::wstring &strategy);

  virtual void setCommonBuildDir(std::shared_ptr<File> file);

  virtual void setIgnoreConflictsFile(std::shared_ptr<File> file);

  /**
   * Execute the task.
   */
  void execute()  override;

private:
  bool findLatestConflictVersions();

  /**
   * Collects indirect dependency version conflicts to ignore
   * in ivy-ignore-conflicts.properties, and also checks for orphans
   * (coordinates not included in ivy-versions.properties).
   *
   * Returns true if no orphans are found.
   */
  bool collectVersionConflictsToIgnore();

  void collectDirectDependencies();

  /**
   * Transitively resolves all dependencies in the given ivy.xml file,
   * looking for indirect dependencies with versions that conflict
   * with those of direct dependencies.  Dependency conflict when a
   * direct dependency's version is older than that of an indirect
   * dependency with the same /org/name.
   *
   * Returns true if no version conflicts are found and no resolution
   * errors occurred, false otherwise.
   */
  bool resolveTransitively(std::shared_ptr<File> ivyXmlFile);

  /**
   * Recursively finds indirect dependencies that have a version conflict with a
   * direct dependency. Returns true if one or more conflicts are found, false
   * otherwise
   */
  bool collectConflicts(std::shared_ptr<IvyNodeElement> root,
                        std::shared_ptr<IvyNodeElement> parent,
                        const std::wstring &moduleName);

  /**
   * Copy-pasted from Ivy's
   * org.apache.ivy.plugins.latest.LatestRevisionStrategy
   * with minor modifications
   */
  static const std::unordered_map<std::wstring, int> SPECIAL_MEANINGS;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static LibVersionsCheckTask::StaticConstructor staticConstructor;

  /**
   * Copy-pasted from Ivy's
   * org.apache.ivy.plugins.latest.LatestRevisionStrategy.MridComparator
   * with minor modifications
   */
private:
  class LatestVersionComparator
      : public std::enable_shared_from_this<LatestVersionComparator>,
        public Comparator<std::wstring>
  {
    GET_CLASS_NAME(LatestVersionComparator)
  public:
    int compare(const std::wstring &rev1, const std::wstring &rev2) override;

  private:
    static const std::shared_ptr<Pattern> IS_NUMBER;
    static bool isNumber(const std::wstring &str);
  };

private:
  static std::shared_ptr<LatestVersionComparator> LATEST_VERSION_COMPARATOR;

  /**
   * Returns true if directVersion is less than indirectVersion, and
   * coordinate=indirectVersion is not present in
   * ivy-ignore-conflicts.properties.
   */
  bool isConflict(const std::wstring &coordinate,
                  const std::wstring &directVersion,
                  const std::wstring &indirectVersion);

  /**
   * Returns the number of direct dependencies in conflict with indirect
   * dependencies.
   */
  int emitConflicts();

  bool emitConflict(std::shared_ptr<StringBuilder> builder,
                    const std::wstring &conflictCoordinate,
                    std::shared_ptr<IvyNodeElement> parent, int depth);

  bool hasConflicts(const std::wstring &conflictCoordinate,
                    std::shared_ptr<IvyNodeElement> parent);

  std::wstring xmlToString(std::shared_ptr<File> ivyXmlFile);

  void setupIvy();

  /**
   * Returns true if the "/org/name" coordinate keys in the given
   * properties file are lexically sorted and are not duplicates.
   */
  bool verifySortedCoordinatesPropertiesFile(
      std::shared_ptr<File> coordinatePropertiesFile);

  /**
   * Builds up logical {@link java.util.Properties} lines, composed of one
   * non-blank, non-comment initial line, either:
   *
   * 1. without a non-escaped trailing slash; or
   * 2. with a non-escaped trailing slash, followed by
   *    zero or more lines with a non-escaped trailing slash, followed by
   *    one or more lines without a non-escaped trailing slash
   *
   * All leading non-escaped whitespace and trailing non-escaped whitespace +
   * non-escaped slash are trimmed from each line before concatenating.
   *
   * After composing the logical line, escaped characters are un-escaped.
   *
   * null is returned if there are no lines left to read.
   */
  std::wstring readLogicalPropertiesLine(
      std::shared_ptr<BufferedReader> reader) ;

  /**
   * Check a single ivy.xml file for dependencies' versions in
   * rev="${/org/name}" format.  Returns false if problems are found, true
   * otherwise.
   */
  bool checkIvyXmlFile(std::shared_ptr<File> ivyXmlFile) throw(
      ParserConfigurationException, SAXException, IOException);

private:
  class DependencyRevChecker : public DefaultHandler
  {
    GET_CLASS_NAME(DependencyRevChecker)
  private:
    std::shared_ptr<LibVersionsCheckTask> outerInstance;

    const std::shared_ptr<File> ivyXmlFile;
    const std::stack<std::wstring> tags = std::stack<std::wstring>();

  public:
    bool fail = false;

    DependencyRevChecker(std::shared_ptr<LibVersionsCheckTask> outerInstance,
                         std::shared_ptr<File> ivyXmlFile);

    void startElement(
        const std::wstring &uri, const std::wstring &localName,
        const std::wstring &qName,
        std::shared_ptr<Attributes> attributes)  override;

    void endElement(const std::wstring &uri, const std::wstring &localName,
                    const std::wstring &qName)  override;

  private:
    bool insideDependenciesTag();

  protected:
    std::shared_ptr<DependencyRevChecker> shared_from_this()
    {
      return std::static_pointer_cast<DependencyRevChecker>(
          org.xml.sax.helpers.DefaultHandler::shared_from_this());
    }
  };

protected:
  std::shared_ptr<LibVersionsCheckTask> shared_from_this()
  {
    return std::static_pointer_cast<LibVersionsCheckTask>(
        org.apache.tools.ant.Task::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/validation/
