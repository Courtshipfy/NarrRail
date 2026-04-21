// Unity build file for yaml-cpp library
// This file includes all yaml-cpp source files to compile them as part of NarrRailEditor module

#pragma warning(push)
#pragma warning(disable: 4996) // deprecated functions
#pragma warning(disable: 4244) // conversion warnings
#pragma warning(disable: 4267) // size_t conversion
#pragma warning(disable: 4100) // unreferenced formal parameter
#pragma warning(disable: 4702) // unreachable code

#include "../../ThirdParty/yaml-cpp/src/binary.cpp"
#include "../../ThirdParty/yaml-cpp/src/convert.cpp"
#include "../../ThirdParty/yaml-cpp/src/depthguard.cpp"
#include "../../ThirdParty/yaml-cpp/src/directives.cpp"
#include "../../ThirdParty/yaml-cpp/src/emit.cpp"
#include "../../ThirdParty/yaml-cpp/src/emitfromevents.cpp"
#include "../../ThirdParty/yaml-cpp/src/emitter.cpp"
#include "../../ThirdParty/yaml-cpp/src/emitterstate.cpp"
#include "../../ThirdParty/yaml-cpp/src/emitterutils.cpp"
#include "../../ThirdParty/yaml-cpp/src/exceptions.cpp"
#include "../../ThirdParty/yaml-cpp/src/exp.cpp"
#include "../../ThirdParty/yaml-cpp/src/memory.cpp"
#include "../../ThirdParty/yaml-cpp/src/node.cpp"
#include "../../ThirdParty/yaml-cpp/src/nodebuilder.cpp"
#include "../../ThirdParty/yaml-cpp/src/nodeevents.cpp"
#include "../../ThirdParty/yaml-cpp/src/node_data.cpp"
#include "../../ThirdParty/yaml-cpp/src/null.cpp"
#include "../../ThirdParty/yaml-cpp/src/ostream_wrapper.cpp"
#include "../../ThirdParty/yaml-cpp/src/parse.cpp"
#include "../../ThirdParty/yaml-cpp/src/parser.cpp"
#include "../../ThirdParty/yaml-cpp/src/regex_yaml.cpp"
#include "../../ThirdParty/yaml-cpp/src/scanner.cpp"
#include "../../ThirdParty/yaml-cpp/src/scanscalar.cpp"
#include "../../ThirdParty/yaml-cpp/src/scantag.cpp"
#include "../../ThirdParty/yaml-cpp/src/scantoken.cpp"
#include "../../ThirdParty/yaml-cpp/src/simplekey.cpp"
#include "../../ThirdParty/yaml-cpp/src/singledocparser.cpp"
#include "../../ThirdParty/yaml-cpp/src/stream.cpp"
#include "../../ThirdParty/yaml-cpp/src/tag.cpp"
#include "../../ThirdParty/yaml-cpp/src/contrib/graphbuilder.cpp"
#include "../../ThirdParty/yaml-cpp/src/contrib/graphbuilderadapter.cpp"

#pragma warning(pop)
