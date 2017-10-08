#include "DetectOS.h"
#ifdef CURRENT_OS_WINDOWS

#include <windows.h>

#include <extension_system/Extension.hpp>
#include <docmala/DocmaPlugin.h>
#include <fstream>
#include <sstream>
#include <unordered_map>

using namespace docmala;

class PlantUMLPlugin : public DocumentPlugin {
    // DocmaPlugin interface
public:
    bool initHost(const ParameterList& parameters, const FileLocation& location);
    ~PlantUMLPlugin() override;
    BlockProcessing blockProcessing() const override;
    bool process(const ParameterList& parameters, const FileLocation& location, Document& document, const std::string& block) override;

    const std::vector<Error> lastErrors() const {
        return _errors;
    }

    std::vector<Error>                                   _errors;
    std::unordered_map<std::string, DocumentPart::Image> _cache;

    HANDLE _stdInWriteHandle = NULL;
    HANDLE _stdOutReadHandle = NULL;
    HANDLE _stdErrReadHandle = NULL;

    bool _hostInitialized = false;
};

void split(const std::string& s, char c, std::vector<std::string>& v) {
    std::string::size_type i = 0;
    std::string::size_type j = s.find(c);

    while (j != std::string::npos) {
        v.push_back(s.substr(i, j - i - 1));
        i = ++j;
        j = s.find(c, j);

        if (j == std::string::npos)
            v.push_back(s.substr(i, s.length()));
    }
}

bool PlantUMLPlugin::initHost(const ParameterList& parameters, const FileLocation& location) {
    if (_hostInitialized)
        return true;

    std::string pluginDir;

    auto pluginDirIter = parameters.find("pluginDir");
    if (pluginDirIter != parameters.end()) {
        pluginDir = pluginDirIter->second.value + '/';
    }

    std::string commandLine = "javaw -splash:no -jar " + pluginDir + "PlantUMLHost.jar";

    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO         siStartInfo;
    SECURITY_ATTRIBUTES saAttr;

    saAttr.nLength              = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle       = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    HANDLE stdInReadHandle   = NULL;
    HANDLE stdOutWriteHandle = NULL;
    HANDLE stdErrWriteHandle = NULL;

    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));

    // Create a pipe for the child process's STDOUT.
    if (!CreatePipe(&_stdOutReadHandle, &stdOutWriteHandle, &saAttr, 0)) {
        _errors.push_back({location, "Unable to create pipe for stdOut"});
        return false;
    }

    // Ensure the read handle to the pipe for STDOUT is not inherited.
    if (!SetHandleInformation(_stdOutReadHandle, HANDLE_FLAG_INHERIT, 0)) {
        _errors.push_back({location, "Unable to configure pipe for stdOut"});
        return false;
    }

    // Create a pipe for the child process's STDIN.
    if (!CreatePipe(&stdInReadHandle, &_stdInWriteHandle, &saAttr, 0)) {
        _errors.push_back({location, "Unable to create pipe for stdIn"});
        return false;
    }

    // Ensure the write handle to the pipe for STDIN is not inherited.
    if (!SetHandleInformation(_stdInWriteHandle, HANDLE_FLAG_INHERIT, 0)) {
        _errors.push_back({location, "Unable to configure pipe for stdIn"});
        return false;
    }

    // Create a pipe for the child process's STDERR.
    if (!CreatePipe(&_stdErrReadHandle, &stdErrWriteHandle, &saAttr, 0)) {
        _errors.push_back({location, "Unable to create pipe for stdErr"});
        return false;
    }

    // Ensure the read handle to the pipe for STDERR is not inherited.
    if (!SetHandleInformation(_stdErrReadHandle, HANDLE_FLAG_INHERIT, 0)) {
        _errors.push_back({location, "Unable to configure pipe for stdErr"});
        return false;
    }

    siStartInfo.cb         = sizeof(STARTUPINFO);
    siStartInfo.hStdError  = stdErrWriteHandle;
    siStartInfo.hStdOutput = stdOutWriteHandle;
    siStartInfo.hStdInput  = stdInReadHandle;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    if (!CreateProcessA(NULL,
                        &commandLine[0], // command line
                        NULL, // process security attributes
                        NULL, // primary thread security attributes
                        TRUE, // handles are inherited
                        0, // creation flags
                        NULL, // use parent's environment
                        NULL, // use parent's current directory
                        &siStartInfo, // STARTUPINFO pointer
                        &piProcInfo)) { // receives PROCESS_INFORMATION
        _errors.push_back({location, "Unable to execute: '" + commandLine + "'"});
        return false;
    }

    CloseHandle(stdInReadHandle);
    CloseHandle(stdOutWriteHandle);
    CloseHandle(stdErrWriteHandle);

    _hostInitialized = true;
    return true;
}

PlantUMLPlugin::~PlantUMLPlugin() {
    CloseHandle(_stdInWriteHandle);
    CloseHandle(_stdOutReadHandle);
}

DocumentPlugin::BlockProcessing PlantUMLPlugin::blockProcessing() const {
    return BlockProcessing::Required;
}

bool PlantUMLPlugin::process(const ParameterList& parameters, const FileLocation& location, Document& document, const std::string& block) {
    _errors.clear();
    if (!initHost(parameters, location)) {
        return false;
    }

    auto cachePosition = _cache.find(block);

    if (cachePosition != _cache.end()) {
        DocumentPart::Image image = cachePosition->second;
        image.line                = location.line;
        document.addPart(image);
        return true;
    }

    std::string outFile = "@startuml\n";
    outFile += block + "\n";
    outFile += "@enduml \n";

    DWORD       bytesWritten = 0;
    std::size_t bytesToWrite = outFile.length();
    BOOL        bSuccess;

    while (bytesToWrite > 0) {
        DWORD thisTimeWritten = 0;
        bSuccess              = WriteFile(_stdInWriteHandle, outFile.data() + bytesWritten, bytesToWrite, &thisTimeWritten, NULL);
        if (!bSuccess)
            break;
        bytesToWrite -= thisTimeWritten;
        bytesWritten += thisTimeWritten;
    }

    std::string imageData;

    int32_t length    = 0;
    DWORD   readBytes = 0;

    bSuccess = ReadFile(_stdOutReadHandle, &length, 4, &readBytes, NULL);

    size_t pos = 0;
    imageData.resize(length);

    while (true) {
        DWORD readBytes = 0;
        bSuccess        = ReadFile(_stdOutReadHandle, &imageData[pos], length - pos, &readBytes, NULL);

        pos += readBytes;

        if (!bSuccess || readBytes == 0 || pos >= length)
            break;
    }

    DocumentPart::Text  text(location.line);
    DocumentPart::Image image("svg+xml", "svg", imageData, text);
    document.addPart(image);

    DWORD bytesAvail = 0;
    if (PeekNamedPipe(_stdErrReadHandle, NULL, 0, NULL, &bytesAvail, NULL)) {
        if (bytesAvail == 0)
            return true;
        std::string error;
        error.resize(bytesAvail);

        DWORD readBytes = 0;
        if (ReadFile(_stdErrReadHandle, &error[0], bytesAvail, &readBytes, NULL)) {
            std::vector<std::string> errorInfo;
            split(error, '\n', errorInfo);
            int         lineNumber = 0;
            std::string errorText;

            if (errorInfo.size() > 0 && errorInfo[0] == "ERROR") {
                if (errorInfo.size() > 1) {
                    try {
                        lineNumber = std::stoi(errorInfo[1]);
                    } catch (...) {
                        return false;
                    }
                }

                for (int i = 2; i < errorInfo.size(); i++) {
                    errorText += errorInfo[i];
                }
                FileLocation l = location;
                l.line += lineNumber + 1;
                _errors.push_back({l, errorText});
                return false;
            }
        }
    }

    _cache.insert(std::make_pair(block, image));

    return true;
}

EXTENSION_SYSTEM_EXTENSION(
    docmala::DocumentPlugin, PlantUMLPlugin, "plantuml", 1, "Creates an uml diagram form the subsequent block", EXTENSION_SYSTEM_NO_USER_DATA)
#endif
