#include "shell_view.hpp"

#include <array>
#include <cassert>
#include <memory>
#include <string>

// A C++ compiler emits the Itanium ABI vtables; the fixture does not assemble a
// table by hand. Only the slots used by ShellView have typed implementations.
struct Text {
    virtual const char* data() { return value.c_str(); }
    std::string value;
};
struct List {
    virtual Text* at(std::uint64_t index) { return &items.at(index); }
    virtual std::int32_t size() { return count; }
    std::array<Text, 3> items;
    std::int32_t count = 3;
};
struct Shell {
    virtual ~Shell() = default;
    virtual void opaque2() {}
    virtual void opaque3() {}
    virtual void opaque4() {}
    virtual void opaque5() {}
    virtual bool insert(std::int32_t code) { lastCode = code; return accepted; }
    virtual bool back() { ++backs; return true; }
    virtual void clear() { composition.value.clear(); result.value.clear(); }
    virtual void select(std::int32_t index) { selected = index; }
    virtual void opaque10() {}
    virtual void opaque11() {}
    virtual void opaque12() {}
    virtual bool pageDown() { ++page; return true; }
    virtual bool pageUp() { --page; return true; }
    virtual void opaque15() {}
    virtual List* candidates() { return &list; }
    virtual Text* preedit() { return &composition; }
    virtual void opaque18() {}
    virtual Text* output() { return &result; }
    virtual void opaque20() {}
    virtual void opaque21() {}
    virtual void opaque22() {}
    virtual void opaque23() {}
    virtual void opaque24() {}
    virtual bool setCursor(bool flag, std::int32_t n) { cursorFlag=flag; cursor=n; return true; }
    virtual bool getCursor(std::int32_t* n) { *n=cursor; return true; }
    virtual void opaque27() {}
    virtual std::int32_t info(std::uint32_t n) { return static_cast<std::int32_t>(n + 100); }
    bool accepted = true, cursorFlag = false;
    int lastCode=0, backs=0, selected=-1, page=0, cursor=4;
    Text composition, result;
    List list;
};

int main() {
    auto fixture = std::make_unique<Shell>();
    fixture->composition.value = "ni'hao";
    fixture->result.value = "你好";
    fixture->list.items[0].value = "你好";
    fixture->list.items[1].value = "拟好";
    fixture->list.items[2].value = "你号";
    sogo::fc4::ShellView engine(fixture.get());
    auto snapshot = engine.snapshot();
    assert(snapshot.composition=="ni'hao" && snapshot.result=="你好");
    assert(snapshot.candidates.size()==3 && snapshot.candidates[1]=="拟好");
    assert(snapshot.hasCursor && snapshot.cursor==4);
    fixture->composition.value = "changed";
    fixture->list.items[0].value = "changed";
    assert(snapshot.composition=="ni'hao" && snapshot.candidates[0]=="你好");
    for (int i=0;i<20000;++i) {
        assert(engine.insertChar(97+i%26));
        assert(fixture->lastCode==97+i%26);
        assert(engine.pageDown() && engine.pageUp());
        assert(fixture->page==0);
    }
    fixture->accepted=false;
    assert(!engine.insertChar(42));
    assert(engine.back() && fixture->backs==1);
    engine.select(2);
    assert(fixture->selected==2);
    assert(engine.setCursor(true, 7) && fixture->cursorFlag && fixture->cursor==7);
    assert(engine.integerInfo(102)==202);
    bool rejected=false;
    try { engine.select(-1); } catch(const std::out_of_range&) { rejected=true; }
    assert(rejected);
    fixture->list.count=257;
    rejected=false;
    try { engine.snapshot(); } catch(const std::length_error&) { rejected=true; }
    assert(rejected);
    fixture->list.count=0;
    fixture->result.value.assign(65536, 'a');
    rejected=false;
    try { engine.snapshot(); } catch(const std::length_error&) { rejected=true; }
    assert(rejected);
    engine.clear();
    assert(engine.snapshot().result.empty());
    rejected=false;
    try { sogo::fc4::ShellView bad(nullptr); } catch(const std::invalid_argument&) { rejected=true; }
    assert(rejected);
}
