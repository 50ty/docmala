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
#include <iostream>

#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <docmala/Docmala.h>

using namespace std;
namespace po = boost::program_options;

int main(int argc, char* argv[]) {
    po::options_description desc("Documentation Markup Language");
    desc.add_options()("help", "produce this help message") //
        ("input,i", po::value<string>(), "input file") //
        ("outputdir,o", po::value<string>(), "output directory") //
        ("outputplugins,p", po::value<vector<string>>(), "plugins for output generation") //
        ("parameters",
         po::value<vector<string>>()->multitoken(),
         "parameters for plugins in form [key]=[value] or [key] for flags")("listoutputplugins,l", "print a list of output plugins");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help") != 0u) {
        cout << desc << "\n";
        return 0;
    }

    docmala::Docmala       docmala;
    docmala::ParameterList parameters;

    if (vm.count("listoutputplugins") != 0u) {
        for (const auto& plugin : docmala.listOutputPlugins()) {
            cout << plugin << "\n";
}
        return 0;
    }

    string outputDir;
    string inputFile;

    if (vm.count("input") != 0u) {
        inputFile = vm["input"].as<string>();
    } else {
        cout << "An input file has to be specified\n";
        return 1;
    }

    if (vm.count("outputdir") != 0u) {
        outputDir = vm["outputdir"].as<string>();
    } else {
        outputDir = inputFile.substr(0, inputFile.find_last_of("\\/"));
    }

    if (vm.count("parameters") != 0u) {
        for (auto parameter : vm["parameters"].as<vector<string>>()) {
            auto equalsLocation = parameter.find_first_of('=');
            if (equalsLocation != string::npos) {
                string key   = parameter.substr(0, equalsLocation);
                string value = parameter.substr(equalsLocation + 1);
                parameters.insert(make_pair(key, docmala::Parameter{key, value, docmala::FileLocation()}));
            } else {
                parameters.insert(make_pair(parameter, docmala::Parameter{parameter, "", docmala::FileLocation()}));
            }
        }
    }

    parameters.emplace(make_pair("outputdir", docmala::Parameter{"outputdir", outputDir, docmala::FileLocation()}));
    docmala.setParameters(parameters);
    docmala.parseFile(inputFile);

    for (const auto& error : docmala.errors()) {
        cout << error.location.fileName << "(" << error.location.line << ":" << error.location.column << "): " << error.message << "\n";
    }

    if (vm.count("outputplugins") != 0u) {
        for (const auto& plugin : vm["outputplugins"].as<vector<string>>()) {
            if (!docmala.produceOutput(plugin)) {
                cout << "Unable to create output for plugin: " << plugin << "\n";
                return -1;
            }
        }
    } else {
        cout << "No output plugin specified. No output is generated.\n";
        return -1;
    }

    return 0;
}
