#pragma once
#include "shell_view.hpp"
#include <fcitx/inputcontext.h>
#include <fcitx/inputpanel.h>
#include <fcitx-utils/utf8.h>
#include <algorithm>
#include <functional>
#include <memory>
#include <stdexcept>

namespace sogo::direct {
class Word final:public fcitx::CandidateWord {
public:
    Word(std::string value,std::function<void(fcitx::InputContext*)> select):CandidateWord(fcitx::Text(std::move(value))),select_(std::move(select)){}
    void select(fcitx::InputContext* ic)const override{select_(ic);}
private:std::function<void(fcitx::InputContext*)> select_;
};
class Page final:public fcitx::CandidateList,public fcitx::PageableCandidateList {
public:
    Page(const std::vector<std::string>& values,int page,std::function<void(int,fcitx::InputContext*)> select,
         std::function<void(bool)> flip):page_(page),flip_(std::move(flip)){
        setPageable(this);
        for(std::size_t i=0;i<values.size();++i){
            words_.push_back(std::make_unique<Word>(values[i],[select,i](fcitx::InputContext* ic){select(i,ic);}));
            const auto number=std::to_string(i+1)+".";
            // Some PC Shell candidates already carry their display number.
            // Keep the engine text intact and avoid drawing that number twice.
            if(values[i].compare(0,number.size(),number)==0)labels_.emplace_back();
            else labels_.emplace_back(number);
        }
    }
    const fcitx::Text& label(int i)const override{return labels_.at(i);}
    const fcitx::CandidateWord& candidate(int i)const override{return *words_.at(i);}
    int size()const override{return words_.size();}
    int cursorIndex()const override{return words_.empty()?-1:0;}
    fcitx::CandidateLayoutHint layoutHint()const override{return fcitx::CandidateLayoutHint::Horizontal;}
    bool hasPrev()const override{return page_>0;}
    bool hasNext()const override{return words_.size()>=5;}
    void prev()override{flip_(false);}
    void next()override{flip_(true);}
    bool usedNextBefore()const override{return page_>0;}
private:
    int page_;std::function<void(bool)> flip_;
    std::vector<std::unique_ptr<Word>> words_;std::vector<fcitx::Text> labels_;
};
inline void validate(const fc4::ShellView::Snapshot& value){
    if(!fcitx::utf8::validate(value.composition)||!fcitx::utf8::validate(value.result))throw std::runtime_error("invalid engine UTF-8");
    for(const auto& word:value.candidates)if(!fcitx::utf8::validate(word))throw std::runtime_error("invalid candidate UTF-8");
    if(std::any_of(value.result.begin(),value.result.end(),[](unsigned char c){return c<32&&c!='\t'&&c!='\n';}))
        throw std::runtime_error("unsupported engine control code");
}
class Presentation {
public:
    struct Updates {bool preedit=false,candidates=false,ui=false;};
    void reset(){valid_=false;context_=nullptr;composition_.clear();candidates_.clear();}
    Updates present(fcitx::InputContext* ic,const fc4::ShellView::Snapshot& value,int page,
                    std::function<void(int,fcitx::InputContext*)> select,std::function<void(bool)> flip){
        validate(value);
        const auto cursor=value.hasCursor&&value.cursor>=0&&value.cursor<=static_cast<int>(value.composition.size())
            ?value.cursor:static_cast<int>(value.composition.size());
        const auto capabilities=ic->capabilityFlags();
        const bool fresh=!valid_||context_!=ic||capabilities_!=capabilities;
        Updates changed;
        // A commit can clear the client's preedit. Re-send it even if the
        // remaining composition happens to have the same text and cursor.
        changed.preedit=fresh||composition_!=value.composition||cursor_!=cursor||!value.result.empty();
        changed.candidates=fresh||candidates_!=value.candidates||page_!=page;
        changed.ui=changed.preedit||changed.candidates;
        fcitx::Text preedit;
        std::unique_ptr<Page> list;
        if(changed.preedit){preedit=fcitx::Text(value.composition,fcitx::TextFormatFlag::Underline);preedit.setCursor(cursor);}
        if(changed.candidates&&!value.candidates.empty())
            list=std::make_unique<Page>(value.candidates,page,std::move(select),std::move(flip));
        // Complete allocations before committing text to the application.
        if(changed.preedit)composition_=value.composition;
        if(changed.candidates)candidates_=value.candidates;
        cursor_=cursor;page_=page;context_=ic;capabilities_=capabilities;valid_=true;
        if(!value.result.empty())ic->commitString(value.result);
        if(changed.preedit){
            ic->inputPanel().setPreedit(preedit);ic->inputPanel().setClientPreedit(preedit);
        }
        if(changed.candidates)ic->inputPanel().setCandidateList(std::move(list));
        if(changed.preedit)ic->updatePreedit();
        if(changed.ui)ic->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);
        return changed;
    }
private:
    bool valid_=false;fcitx::InputContext* context_=nullptr;int cursor_=0,page_=0;
    fcitx::CapabilityFlags capabilities_;
    std::string composition_;std::vector<std::string> candidates_;
};
}
