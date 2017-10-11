/**
    @file
    @copyright
        Copyright (C) 2017 Michael Adam
        Copyright (C) 2017 Bernd Amend
        Copyright (C) 2017 Stefan Rommel

        This program is free software: you can redistribute it and/or modify
        it under the terms of the GNU Lesser General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU Lesser General Public License
        along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "DetectOS.h"
#ifdef CURRENT_OS_LINUX
#include <docmala/DocmaPlugin.h>
#include <extension_system/Extension.hpp>
#include <fcntl.h>
#include <fstream>
#include <spawn.h>
#include <sstream>
#include <sys/wait.h>
#include <unordered_map>

using namespace docmala;

namespace {
void split(const std::string& s, char c, std::vector<std::string>& v) {
    std::string::size_type i = 0;
    std::string::size_type j = s.find(c);

    while (j != std::string::npos) {
        v.push_back(s.substr(i, j - i));
        i = ++j;
        j = s.find(c, j);

        if (j == std::string::npos) {
            v.push_back(s.substr(i, s.length()));
        }
    }
}
} // namespace

class PlantUMLPlugin : public DocumentPlugin {
    // DocmaPlugin interface
public:
    ~PlantUMLPlugin() override;

    BlockProcessing blockProcessing() const override;
    std::vector<Error> process(const ParameterList& parameters, const FileLocation& location, Document& document, const std::string& block) override;
    std::vector<Error> initHost(const ParameterList& parameters, const FileLocation& location);

    std::unordered_map<std::string, DocumentPart::Image> _cache;

    int _stdInPipe[2]  = {0};
    int _stdOutPipe[2] = {0};
    int _stdErrPipe[2] = {0};

    bool _hostInitialized = false;
};

PlantUMLPlugin::~PlantUMLPlugin() {
    if (_hostInitialized) {
        close(_stdInPipe[1]);
        close(_stdOutPipe[0]);
        close(_stdErrPipe[0]);
    }
}

DocumentPlugin::BlockProcessing PlantUMLPlugin::blockProcessing() const {
    return BlockProcessing::Required;
}

std::vector<Error> PlantUMLPlugin::initHost(const ParameterList& parameters, const FileLocation& location) {
    if (_hostInitialized) {
        return {};
    }

    std::string pluginDir;

    auto pluginDirIter = parameters.find("pluginDir");
    if (pluginDirIter != parameters.end()) {
        pluginDir = pluginDirIter->second.value + '/';
    }

    const std::string commandLine  = "java -splash:no -jar " + pluginDir + "PlantUMLHost.jar";
    std::string       stringargs[] = {"java", "-splash:no", "-jar", pluginDir + "PlantUMLHost.jar"};
    char*             args[]       = {&stringargs[0][0], &stringargs[1][0], &stringargs[2][0], &stringargs[3][0], nullptr};

    posix_spawn_file_actions_t action{};
    pid_t                      pid = 0;

    if ((pipe(_stdInPipe) != 0) || (pipe(_stdOutPipe) != 0) || (pipe(_stdErrPipe) != 0)) {
        return {{location, "Internal: Unable to create pipes"}};
    }

    posix_spawn_file_actions_init(&action);

    posix_spawn_file_actions_adddup2(&action, _stdInPipe[0], 0);
    posix_spawn_file_actions_addclose(&action, _stdInPipe[1]);

    posix_spawn_file_actions_adddup2(&action, _stdOutPipe[1], 1);
    posix_spawn_file_actions_addclose(&action, _stdOutPipe[0]);

    posix_spawn_file_actions_adddup2(&action, _stdErrPipe[1], 2);
    posix_spawn_file_actions_addclose(&action, _stdErrPipe[0]);

    if (posix_spawnp(&pid, args[0], &action, nullptr, &args[0], nullptr) != 0) {
        return {{location, "Internal: Unable start plantuml host."}};
    }

    close(_stdInPipe[0]);
    close(_stdOutPipe[1]);
    close(_stdErrPipe[1]);

    int retval = fcntl(_stdErrPipe[0], F_SETFL, fcntl(_stdErrPipe[0], F_GETFL) | O_NONBLOCK);
    if (retval == -1) {
        return {{location, "Internal: Unable to configure stderr pipe"}};
    }

    _hostInitialized = true;
    return {};
}

std::vector<Error> PlantUMLPlugin::process(const ParameterList& parameters, const FileLocation& location, Document& document, const std::string& block) {
    auto initHostErrors = initHost(parameters, location);

    if( !initHostErrors.empty() ){
        return initHostErrors;
    }

    auto cachePosition = _cache.find(block);

    if (cachePosition != _cache.end()) {
        DocumentPart::Image image = cachePosition->second;
        image.location            = location;
        document.addPart(image);
        return {};
    }

    std::string outFile = "@startuml\n";
    outFile += block + "\n";
    outFile += "@enduml \n";

    write(_stdInPipe[1], outFile.data(), outFile.length());

    std::string imageData;
    int32_t     length = 0;

    ssize_t readBytes = read(_stdOutPipe[0], &length, 4);

    if (readBytes != 4) {
        return {{location, "Protocol error while communicating with plantUml host"}};
    }

    size_t pos = 0;
    imageData.resize(static_cast<size_t>(length));

    while (true) {
        readBytes = read(_stdOutPipe[0], &imageData[pos], static_cast<size_t>(length) - pos);

        if (readBytes <= 0) {
            break;
        }

        pos += static_cast<size_t>(readBytes);

        if (pos >= static_cast<size_t>(length)) {
            break;
        }
    }

    DocumentPart::Text  text(location);
    DocumentPart::Image image("svg+xml", "svg", imageData, text);
    document.addPart(image);

    std::string error;
    char        buffer[8192] = {0};
    while (true) {
        readBytes = read(_stdErrPipe[0], buffer, 8192);
        if (readBytes <= 0) {
            break;
        }
        error.append(buffer, static_cast<size_t>(readBytes));
    }

    if (!error.empty()) {
        std::vector<std::string> errorInfo;
        split(error, '\n', errorInfo);
        int         lineNumber = 0;
        std::string errorText;

        if (!errorInfo.empty() && errorInfo[0] == "ERROR") {
            if (errorInfo.size() > 1) {
                try {
                    lineNumber = std::stoi(errorInfo[1]);
                } catch (...) {
                    return {{location, "Protocol error while reading an error from plantUml host"}};
                }
            }

            for (std::vector<std::string>::size_type i = 2; i < errorInfo.size(); i++) {
                errorText += errorInfo[i];
            }
            FileLocation l = location;
            l.line += lineNumber + 1;
            return {{l, errorText}};
        }
    }

    _cache.insert(std::make_pair(block, image));

    return {};
}

EXTENSION_SYSTEM_EXTENSION(
    docmala::DocumentPlugin, PlantUMLPlugin, "plantuml", 1, "Creates an uml diagram form the subsequent block", EXTENSION_SYSTEM_NO_USER_DATA)
#endif
