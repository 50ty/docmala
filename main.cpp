#include <iostream>

#include <string>
#include <vector>

#include <docmala/Docmala.h>
#include <boost/program_options.hpp>

using namespace std;

int main(int argc, char *argv[])
{
    boost::program_options::options_description desc("Documentation Markup Language");
    desc.add_options()
        ("help", "produce this help message")
        ("input,i", boost::program_options::value<std::string>(), "input file")
        ("outputdir,o", boost::program_options::value<std::string>(), "output directory")
        ("outputplugins,p", boost::program_options::value<std::vector<std::string>>(), "plugins for output generation")
        ("listoutputplugins,l", "print a list of output plugins")
    ;

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    docmala::Docmala docmala;
    docmala::ParameterList parameters;

    if (vm.count("listoutputplugins")) {
        for( auto plugin : docmala.listOutputPlugins() )
            std::cout << plugin << "\n";
        return 1;
    }

    std::string outputDir;
    std::string inputFile;

    if (vm.count("input") ) {
        inputFile = vm["input"].as<std::string>();
    } else {
        std::cout << "An input file has to be specified\n";
        return 1;
    }

    if (vm.count("outputdir")) {
        outputDir = vm["outputdir"].as<std::string>();
    } else {
        outputDir = inputFile.substr(0, inputFile.find_last_of("\\/"));
    }

    parameters.insert(std::make_pair("outputdir", docmala::Parameter{"outputdir", outputDir, docmala::FileLocation()} ));
    docmala.parseFile(inputFile);

    for( const auto &error : docmala.errors() ) {
        std::cout << error.location.fileName << "(" << error.location.line << ":" << error.location.column << "): " <<
                     error.message << std::endl;
    }

    if (vm.count("outputplugins")) {
        for( auto plugin : vm["outputplugins"].as<std::vector<std::string>>() ) {
            if( !docmala.produceOutput(plugin) ) {
                std::cout << "Unable to create output for plugin: " << plugin << std::endl;
            }
        }
    } else {
        std::cout << "No output plugin specified. No output is generated." << std::endl;
    }

    return 0;
}
