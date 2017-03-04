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
    BlockProcessing blockProcessing() const override;
    bool process( const ParameterList &parameters, const FileLocation &location, Document &document, const std::string &block) override;

    const std::vector<Error> lastErrors() const { return _errors; }

    std::vector<Error> _errors;
    std::unordered_map<std::string, DocumentPart::Image> _cache;
};


DocumentPlugin::BlockProcessing PlantUMLPlugin::blockProcessing() const {
    return BlockProcessing::Required;
}

bool PlantUMLPlugin::process(const ParameterList &parameters, const FileLocation &location, Document &document, const std::string &block)
{
    std::stringstream outputFileName;

    auto cachePosition = _cache.find(block);
    std::string pluginDir;

    if( cachePosition != _cache.end() ) {
        DocumentPart::Image image = cachePosition->second;
        image.line = location.line;
        document.addPart( image );
        return true;
    }

    auto outPathIter = parameters.find("outputDir");
    if( outPathIter != parameters.end() ) {
        outputFileName << outPathIter->second.value << "/";
    }

    auto pluginDirIter = parameters.find("pluginDir");
    if( pluginDirIter != parameters.end() ) {
        pluginDir =  pluginDirIter->second.value + '/';
    }

    outputFileName << "uml_" << "_" << location.line <<".png";

    std::string outFile = "@startuml\n";
    outFile += block + "\n";
    outFile += "@enduml \n";

    int exit_code = 0;

    HANDLE g_hChildStd_IN_Rd = NULL;
    HANDLE g_hChildStd_IN_Wr = NULL;
    HANDLE g_hChildStd_OUT_Rd = NULL;
    HANDLE g_hChildStd_OUT_Wr = NULL;

    //std::string stringargs [] = {"java", "-jar", pluginDir + "plantuml.jar", "-p"};
    std::string commandline = "javaw -splash:no -jar " + pluginDir + "plantuml.jar -p -nbthreads 4";

    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;
    SECURITY_ATTRIBUTES saAttr;

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    BOOL bSuccess;

    ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
    ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );

    // Create a pipe for the child process's STDOUT.

    if ( ! CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0) ) {
        _errors.push_back({location, "StdoutRd CreatePipe"});
        return false;
    }

    // Ensure the read handle to the pipe for STDOUT is not inherited.

    if ( ! SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0) ) {
        _errors.push_back({location, "StdoutRd SetHandleInformation"});
        return false;
    }

    // Create a pipe for the child process's STDIN.

    if (! CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0)) {
        _errors.push_back({location, "Stdin CreatePipe"});
        return false;
    }

    // Ensure the write handle to the pipe for STDIN is not inherited.

    if ( ! SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0) ) {
        _errors.push_back({location, "Stdin SetHandleInformation"});
        return false;
    }

    siStartInfo.cb = sizeof(STARTUPINFO);
    //siStartInfo.hStdError = g_hChildStd_OUT_Wr;
    siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
    siStartInfo.hStdInput = g_hChildStd_IN_Rd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    bSuccess = CreateProcessA(
                NULL,
                &commandline[0],     // command line
            NULL,          // process security attributes
            NULL,          // primary thread security attributes
            TRUE,          // handles are inherited
            0,             // creation flags
            NULL,          // use parent's environment
            NULL,          // use parent's current directory
            &siStartInfo,  // STARTUPINFO pointer
            &piProcInfo);  // receives PROCESS_INFORMATION

    if( !bSuccess ) {
        _errors.push_back({location, "Unable to execute: '" + commandline + "'"});
        return false;
    }

    DWORD bytesWritten = 0;
    std::size_t bytesToWrite = outFile.length();

    while( bytesToWrite > 0 ) {
        DWORD thisTimeWritten = 0;
        bSuccess = WriteFile(g_hChildStd_IN_Wr, outFile.data() + bytesWritten, bytesToWrite, &thisTimeWritten, NULL);
        if ( ! bSuccess )
            break;
        bytesToWrite -= thisTimeWritten;
        bytesWritten += thisTimeWritten;
    }

    CloseHandle(g_hChildStd_IN_Wr);
    CloseHandle(g_hChildStd_IN_Rd);
    CloseHandle(g_hChildStd_OUT_Wr);

    std::string imageData;

//    DWORD bytesAvail = 0;
//    if (!PeekNamedPipe(g_hChildStd_OUT_Rd, NULL, 0, NULL, &bytesAvail, NULL)) {
//        _errors.push_back({location, "Failed to call PeekNamedPipe" });
//        return false;
//    }

//    imageData.resize( static_cast<std::string::size_type>(bytesAvail) );

//    DWORD readBytes = 0;
//    bSuccess = ReadFile( g_hChildStd_OUT_Rd, &imageData[0], imageData.size(), &readBytes, NULL);

//    if( readBytes != bytesAvail ) {
//        _errors.push_back({location, "Not all data was read." });
//        return false;
//    }
    while( true )
    {
        char buffer[1024];

        DWORD readBytes = 0;
        bSuccess = ReadFile( g_hChildStd_OUT_Rd, buffer, 1024, &readBytes, NULL);

        if( ! bSuccess || readBytes == 0 )
            break;

        if( readBytes > 0 )
        {
            imageData.append(buffer, static_cast<size_t>(readBytes));
        } else {
            break;
        }
    }

    WaitForSingleObject(piProcInfo.hProcess, INFINITE);

    DocumentPart::Text text(location.line);
    DocumentPart::Image image("png", "png", imageData, text);
    _cache.insert(std::make_pair(block, image));
    document.addPart( image );
    return true;
}

EXTENSION_SYSTEM_EXTENSION(docmala::DocumentPlugin, PlantUMLPlugin, "plantuml", 1, "Creates an uml diagram form the subsequent block", EXTENSION_SYSTEM_NO_USER_DATA )
#endif
