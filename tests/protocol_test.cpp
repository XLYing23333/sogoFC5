#include "ImeClient.pb.h"
#include "ImeServer.pb.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/message.h>
#include <cassert>
#include <string>

int main() {
    const auto* input = SogouImeClientProto::descriptor();
    const auto* output = SogouImeServerRetProto::descriptor();
    assert(input->field_count()==40 && output->field_count()==33);
    for (int i=0;i<40;++i) {
        const auto* field=input->field(i);
        assert(field->number()==i+1 && field->is_optional());
    }
    assert(output->FindFieldByNumber(3)->name()=="szCompStr");
    assert(output->FindFieldByNumber(6)->name()=="szCandResStr");
    assert(output->FindFieldByNumber(11)->name()=="strCands");
    assert(output->FindFieldByNumber(11)->is_repeated());
    assert(output->FindFieldByNumber(13)->name()=="bPreOutput");

    SogouImeClientProto request;
    request.set_ninsertchar(97);
    request.set_nactive(0);
    request.set_bchinese(true);
    request.set_nparamval(-1);
    std::string wire;
    assert(request.SerializeToString(&wire));
    // These independent tags check field numbers, signed integer encoding and
    // explicit presence of an optional zero, rather than only round-tripping.
    const std::string expected = std::string("\x60\x61\xb8\x01\x00\xc0\x01\x01\xe0\x01", 10)
        + std::string(9, '\xff') + std::string(1, '\x01');
    assert(wire==expected);

    SogouImeServerRetProto reply;
    reply.set_binsertchar(false);
    reply.set_szcompstr("ni'hao");
    reply.set_ncursoridx(2);
    reply.set_szcandresstr("你好");
    reply.set_ncandcount(2);
    reply.add_strcands("你好");
    reply.add_strcands("拟好");
    reply.set_bpreoutput(false);
    assert(reply.SerializeToString(&wire));
    SogouImeServerRetProto parsed;
    assert(parsed.ParseFromString(wire));
    assert(parsed.has_binsertchar() && !parsed.binsertchar());
    assert(parsed.szcompstr()=="ni'hao" && parsed.ncursoridx()==2);
    assert(parsed.szcandresstr()=="你好" && parsed.strcands_size()==2);
    assert(parsed.strcands(1)=="拟好");
    parsed.clear_szcandresstr();
    assert(parsed.szcompstr()=="ni'hao" && parsed.szcandresstr().empty());
    assert(parsed.ParseFromString(""));
    assert(!parsed.has_binsertchar() && !parsed.has_szcompstr());
    assert(parsed.strcands_size()==0);
}
