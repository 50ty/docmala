#include "DetectOS.h"
#ifdef CURRENT_OS_LINUX
#include <extension_system/Extension.hpp>
#include <docmala/DocmaPlugin.h>
#include <fstream>
#include <spawn.h>
#include <sys/wait.h>
#include <sstream>
#include <unordered_map>
#include <fcntl.h>

using namespace docmala;

class PlantUMLPlugin : public DocumentPlugin {
    // DocmaPlugin interface
public:

    ~PlantUMLPlugin() override;

    BlockProcessing blockProcessing() const override;
    bool process( const ParameterList &parameters, const FileLocation &location, Document &document, const std::string &block) override;
    const std::vector<Error> lastErrors() const override { return _errors; }
    bool initHost(const ParameterList &parameters, const FileLocation &location);

    std::vector<Error> _errors;
    std::unordered_map<std::string, DocumentPart::Image> _cache;

    int _stdInPipe[2] = {0};
    int _stdOutPipe[2] = {0};
    int _stdErrPipe[2] = {0};

    bool _hostInitialized = false;
};

void split(const std::string& s, char c, std::vector<std::string>& v) {
   std::string::size_type i = 0;
   std::string::size_type j = s.find(c);

   while (j != std::string::npos) {
      v.push_back(s.substr(i, j-i));
      i = ++j;
      j = s.find(c, j);

      if (j == std::string::npos)
         v.push_back(s.substr(i, s.length()));
   }
}

PlantUMLPlugin::~PlantUMLPlugin()
{
    if( _hostInitialized ) {
        close(_stdInPipe[1]);
        close(_stdOutPipe[0]);
        close(_stdErrPipe[0]);
    }
}

DocumentPlugin::BlockProcessing PlantUMLPlugin::blockProcessing() const {
    return BlockProcessing::Required;
}


bool PlantUMLPlugin::initHost(const ParameterList &parameters, const FileLocation &location)
{
    if( _hostInitialized ) {
        return true;
    }

    std::string pluginDir;

    auto pluginDirIter = parameters.find("pluginDir");
    if( pluginDirIter != parameters.end() ) {
        pluginDir =  pluginDirIter->second.value + '/';
    }

    const std::string commandLine = "java -splash:no -jar " + pluginDir + "PlantUMLHost.jar";
    std::string stringargs [] = {"java", "-splash:no", "-jar", pluginDir + "PlantUMLHost.jar"};
    char *args [] = {&stringargs[0][0], &stringargs[1][0], &stringargs[2][0], &stringargs[3][0], nullptr};


    posix_spawn_file_actions_t action;
    pid_t pid = 0;

    if( pipe(_stdInPipe) || pipe(_stdOutPipe) || pipe(_stdErrPipe) ) {
        _errors.push_back({location, "Internal: Unable to create pipes"});
        return false;
    }

    posix_spawn_file_actions_init(&action);

    posix_spawn_file_actions_adddup2(&action, _stdInPipe[0], 0);
    posix_spawn_file_actions_addclose(&action, _stdInPipe[1]);

    posix_spawn_file_actions_adddup2(&action, _stdOutPipe[1], 1);
    posix_spawn_file_actions_addclose(&action, _stdOutPipe[0]);

    posix_spawn_file_actions_adddup2(&action, _stdErrPipe[1], 2);
    posix_spawn_file_actions_addclose(&action, _stdErrPipe[0]);

    if(posix_spawnp(&pid, args[0] , &action, NULL, &args[0], NULL) != 0) {
        _errors.push_back({location, "Internal: Unable start plantuml host."});
        return false;
    }

    close(_stdInPipe[0]);
    close(_stdOutPipe[1]);
    close(_stdErrPipe[1]);

    int retval = fcntl( _stdErrPipe[0], F_SETFL, fcntl(_stdErrPipe[0], F_GETFL) | O_NONBLOCK);
    if( retval == -1 ) {
        _errors.push_back({location, "Internal: Unable to configure stderr pipe"});
        return false;
    }

    _hostInitialized = true;
    return true;
}

bool PlantUMLPlugin::process(const ParameterList &parameters, const FileLocation &location, Document &document, const std::string &block)
{
    _errors.clear();
    if( !initHost(parameters, location) ) {
        return false;
    }

    auto cachePosition = _cache.find(block);

    if( cachePosition != _cache.end() ) {
        DocumentPart::Image image = cachePosition->second;
        image.location = location;
        document.addPart( image );
        return true;
    }

    std::string outFile = "@startuml\n";
    outFile += block + "\n";
    outFile += "@enduml \n";

    write(_stdInPipe[1], outFile.data(), outFile.length() );

    std::string imageData;
    int32_t length = 0;

    ssize_t readBytes = read( _stdOutPipe[0], &length, 4);

    if( readBytes != 4 ) {
        return false;
    }

    size_t pos = 0;
    imageData.resize(static_cast<size_t>(length));

    while( true )
    {
        readBytes = read( _stdOutPipe[0], &imageData[pos], static_cast<size_t>(length) - pos);

        if( readBytes <= 0 )
            break;

        pos += static_cast<size_t>(readBytes);

        if( pos >= static_cast<size_t>(length) )
            break;
    }

    DocumentPart::Text text(location);
    DocumentPart::Image image("svg+xml", "svg", imageData, text);
    document.addPart( image );

    std::string error;
    char buffer[8192] = {0};
    while( true ) {
        readBytes = read( _stdErrPipe[0], buffer, 8192);
        if( readBytes <= 0 ) {
            break;
        }
        error.append(buffer, static_cast<size_t>(readBytes));
    }

    if( !error.empty() ) {
        std::vector<std::string> errorInfo;
        split(error, '\n', errorInfo);
        int lineNumber = 0;
        std::string errorText;

        if( errorInfo.size() > 0 && errorInfo[0] == "ERROR" ) {
            if( errorInfo.size() > 1 ) {
                try {
                    lineNumber = std::stoi(errorInfo[1]);
                } catch(...) {
                    return false;
                }
            }

            for( std::vector<std::string>::size_type i = 2; i < errorInfo.size(); i++ ) {
                errorText += errorInfo[i];
            }
            FileLocation l = location;
            l.line += lineNumber + 1;
            _errors.push_back({l, errorText});
            return false;
        }
    }

    _cache.insert(std::make_pair(block, image));

    return true;
}

EXTENSION_SYSTEM_EXTENSION(docmala::DocumentPlugin, PlantUMLPlugin, "plantuml", 1, "Creates an uml diagram form the subsequent block", EXTENSION_SYSTEM_NO_USER_DATA )
#endif
