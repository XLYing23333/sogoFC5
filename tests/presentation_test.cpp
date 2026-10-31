#include "presentation.hpp"
#include <fcitx/inputcontextmanager.h>
#include <cassert>
using namespace fcitx;
class Client final:public InputContext {
public:
    explicit Client(InputContextManager& manager):InputContext(manager,"fixture"){created();}
    ~Client()override{destroy();}
    const char* frontend()const override{return "fixture";}
    std::vector<std::string> commits;int preedits=0,forwards=0;
protected:
    void commitStringImpl(const std::string& text)override{commits.push_back(text);}
    void deleteSurroundingTextImpl(int,unsigned)override{}
    void forwardKeyImpl(const ForwardKeyEvent&)override{++forwards;}
    void updatePreeditImpl()override{++preedits;}
};
int main(){
    // InputContextManager has no desktop frontend, Instance, X11 connection,
    // D-Bus connection or event injection in this test.
    InputContextManager manager;Client client(manager);
    sogo::direct::Presentation renderer;
    client.setCapabilityFlags(CapabilityFlag::Preedit);
    sogo::fc4::ShellView::Snapshot value;value.composition="ni'hao";value.cursor=2;value.hasCursor=true;
    value.candidates={"你好","拟好","你号","泥好","倪好"};
    int selected=-1,flipped=0;
    auto select=[&](int index,InputContext* ic){assert(ic==&client);selected=index;};
    auto flip=[&](bool next){flipped=next?1:-1;};
    renderer.present(&client,value,0,select,flip);
    assert(client.commits.empty()&&client.forwards==0);
    const auto preedits=client.preedits;
    const auto original=client.inputPanel().candidateList();
    for(int i=0;i<100;++i){
        const auto changed=renderer.present(&client,value,0,select,flip);
        assert(!changed.preedit&&!changed.candidates&&!changed.ui);
        assert(client.inputPanel().candidateList()==original);
    }
    assert(client.preedits==preedits);
    value.cursor=3;
    auto changed=renderer.present(&client,value,0,select,flip);
    assert(changed.preedit&&!changed.candidates&&changed.ui&&client.inputPanel().candidateList()==original);
    value.cursor=2;renderer.present(&client,value,0,select,flip);
    changed=renderer.present(&client,value,1,select,flip);
    assert(!changed.preedit&&changed.candidates&&changed.ui);
    renderer.present(&client,value,0,select,flip);
    assert(client.inputPanel().clientPreedit().toString()=="ni'hao");
    assert(client.inputPanel().preedit().cursor()==2);
    auto list=client.inputPanel().candidateList();assert(list&&list->size()==5);
    list->candidate(1).select(&client);assert(selected==1);
    assert(list->label(0).toString()=="1.");
    assert(list->toPageable()->hasNext()&&!list->toPageable()->hasPrev());
    list->toPageable()->next();assert(flipped==1);
    value.candidates={"1.你好","2.拟好","3.你号","4.泥好","5.倪好"};
    renderer.present(&client,value,1,select,flip);
    list=client.inputPanel().candidateList();
    for(int i=0;i<list->size();++i){
        assert(list->label(i).empty());
        assert(list->label(i).toString()+list->candidate(i).text().toString()==value.candidates[i]);
    }
    list->candidate(3).select(&client);assert(selected==3);
    assert(list->toPageable()->hasPrev());list->toPageable()->prev();assert(flipped==-1);
    value.candidates={"3.14159","1.2版本","2026.10"};
    renderer.present(&client,value,0,select,flip);
    list=client.inputPanel().candidateList();
    for(int i=0;i<list->size();++i){
        assert(list->label(i).toString()==std::to_string(i+1)+".");
        assert(list->candidate(i).text().toString()==value.candidates[i]);
    }
    value.result="你好";value.composition.clear();value.candidates.clear();
    renderer.present(&client,value,0,select,flip);
    assert(client.commits.size()==1&&client.commits[0]=="你好");
    assert(client.inputPanel().clientPreedit().empty()&&!client.inputPanel().candidateList());
    value.result.clear();renderer.present(&client,value,0,select,flip);assert(client.commits.size()==1);
    value.result="valid";value.candidates={std::string(1,'\xff')};bool rejected=false;
    try{renderer.present(&client,value,0,select,flip);}catch(const std::runtime_error&){rejected=true;}
    assert(rejected&&client.commits.size()==1);
    value.candidates.clear();value.result="()\x19";rejected=false;
    try{renderer.present(&client,value,0,select,flip);}catch(const std::runtime_error&){rejected=true;}
    assert(rejected&&client.commits.size()==1&&client.forwards==0);
    value.result="甲词";value.composition="abc";value.cursor=1;
    value.candidates={"甲词","乙词"};
    renderer.present(&client,value,0,select,flip);
    const auto withCommit=client.preedits;
    changed=renderer.present(&client,value,0,select,flip);
    assert(changed.preedit&&!changed.candidates&&changed.ui&&client.preedits==withCommit+1);
    assert(client.commits.size()==3&&client.commits[1]=="甲词"&&client.commits[2]=="甲词");
    value.result.clear();
    changed=renderer.present(&client,value,0,select,flip);assert(!changed.ui);
    renderer.reset();
    changed=renderer.present(&client,value,0,select,flip);assert(changed.preedit&&changed.candidates&&changed.ui);
    Client other(manager);other.setCapabilityFlags(CapabilityFlag::Preedit);
    int otherSelected=-1;
    changed=renderer.present(&other,value,0,[&](int index,InputContext* ic){assert(ic==&other);otherSelected=index;},flip);
    assert(changed.preedit&&changed.candidates&&changed.ui);
    other.inputPanel().candidateList()->candidate(0).select(&other);assert(otherSelected==0);
    other.setCapabilityFlags(CapabilityFlags{});
    changed=renderer.present(&other,value,0,[](int,InputContext*){},flip);
    assert(changed.preedit&&changed.candidates&&changed.ui);
}
