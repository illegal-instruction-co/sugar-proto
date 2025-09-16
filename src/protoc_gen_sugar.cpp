#include "emit_header.h"

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>

#include <memory>
#include <sstream>
#include <string>

using namespace std;

using google::protobuf::compiler::CodeGenerator;
using google::protobuf::compiler::GeneratorContext;
using google::protobuf::io::ZeroCopyOutputStream;

class SugarGenerator : public CodeGenerator {
public:
  bool Generate(const google::protobuf::FileDescriptor *file,
                const string &parameter, GeneratorContext *context,
                string *error) const override {
    ostringstream oss;
    emit_header_for_file(file, oss);

    const string content = oss.str();
    const string out_name = header_filename_for_file(file);

    unique_ptr<ZeroCopyOutputStream> output(context->Open(out_name));
    google::protobuf::io::CodedOutputStream cos(output.get());

    cos.WriteRaw(content.data(), static_cast<int>(content.size()));

    return true;
  }
};

int main(int argc, char *argv[]) {
  SugarGenerator gen;
  return google::protobuf::compiler::PluginMain(argc, argv, &gen);
}
