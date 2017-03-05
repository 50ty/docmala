#include <extension_system/Extension.hpp>
#include <docmala/DocmaPlugin.h>
#include <fstream>
#include <spawn.h>
#include <sys/wait.h>
#include <sstream>

using namespace docmala;

class PlantUMLPlugin : public DocumentPlugin {
    // DocmaPlugin interface
public:
    PlantUMLPlugin();
    ~PlantUMLPlugin() override;

    BlockProcessing blockProcessing() const override;
    bool process( const ParameterList &parameters, const FileLocation &location, std::vector<DocumentPart> &document, const std::string &block) override;

private:
    int _inPipe[2] = {0};
    int _outPipe[2] = {0};
    pid_t _pid = 0;
    posix_spawn_file_actions_t _action;

};


PlantUMLPlugin::PlantUMLPlugin()
{

//    if( pipe(_inPipe) || pipe(_outPipe) ) {
//        // ERROR
//    }

//    posix_spawn_file_actions_init(&_action);

//    posix_spawn_file_actions_adddup2(&_action, _outPipe[0], 0);
//    posix_spawn_file_actions_addclose(&_action, _outPipe[1]);

//    posix_spawn_file_actions_adddup2(&_action, _inPipe[1], 1);
//    posix_spawn_file_actions_addclose(&_action, _inPipe[0]);

//    std::string stringargs [] = {"java", "-jar", "plantuml.jar", "-p"};
//    char *args [] = {&stringargs[0][0], &stringargs[1][0], &stringargs[2][0], &stringargs[3][0], nullptr};

//    if(posix_spawnp(&_pid, args[0] , &_action, NULL, &args[0], NULL) != 0) {
//        //cout << "posix_spawnp failed with error: " << strerror(errno) << "\n";
//    }

//    close(_outPipe[0]);
//    close(_inPipe[1]);

}

PlantUMLPlugin::~PlantUMLPlugin()
{
    int exit_code = 0;
    close(_outPipe[1]);
    waitpid(_pid,&exit_code,0);
    posix_spawn_file_actions_destroy(&_action);
}

DocumentPlugin::BlockProcessing PlantUMLPlugin::blockProcessing() const {
    return BlockProcessing::Required;
}

bool PlantUMLPlugin::process( const ParameterList &parameters, const FileLocation &location, std::vector<DocumentPart> &document, const std::string &block)
{
    std::string outFile = "@startuml\n";
    outFile += block + "\n";
    outFile += "@enduml \n";

    if( pipe(_inPipe) || pipe(_outPipe) ) {
        // ERROR
    }

    posix_spawn_file_actions_init(&_action);

    posix_spawn_file_actions_adddup2(&_action, _outPipe[0], 0);
    posix_spawn_file_actions_addclose(&_action, _outPipe[1]);

    posix_spawn_file_actions_adddup2(&_action, _inPipe[1], 1);
    posix_spawn_file_actions_addclose(&_action, _inPipe[0]);

    std::string stringargs [] = {"java", "-jar", "plantuml.jar", "-p"};
    char *args [] = {&stringargs[0][0], &stringargs[1][0], &stringargs[2][0], &stringargs[3][0], nullptr};

    if(posix_spawnp(&_pid, args[0] , &_action, NULL, &args[0], NULL) != 0) {
        //cout << "posix_spawnp failed with error: " << strerror(errno) << "\n";
    }

    close(_outPipe[0]);
    close(_inPipe[1]);

    write(_outPipe[1], outFile.data(), outFile.length() );

    std::string imageData;

    while( true )
    {
        char buffer[1024];
        ssize_t readData = read(_inPipe[0], buffer, 1024);
        if( readData > 0 )
        {
            imageData.append(buffer, static_cast<size_t>(readData));
        } else {
            break;
        }

    }

    DocumentPart::Text text;
    document.push_back( DocumentPart::Image("png", imageData, text) );
    return true;
}

EXTENSION_SYSTEM_EXTENSION(docmala::DocumentPlugin, PlantUMLPlugin, "plantuml", 1, "Creates an uml diagram form the subsequent block", EXTENSION_SYSTEM_NO_USER_DATA )
