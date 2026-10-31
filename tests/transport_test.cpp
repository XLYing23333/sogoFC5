#include "protocol.hpp"
#include <cassert>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
using namespace sogo::direct;
int main(){
    int fds[2];assert(socketpair(AF_UNIX,SOCK_SEQPACKET|SOCK_NONBLOCK,0,fds)==0);
    const auto raw=[&](const void* buffer,std::size_t size){iovec v{const_cast<void*>(buffer),size};msghdr h{};h.msg_iov=&v;h.msg_iovlen=1;return sendmsg(fds[0],&h,MSG_NOSIGNAL);};
    auto h=packet(Command::Key,3,4,5);h.sym='a';assert(send(fds[0],h,"example"));
    Message m;assert(receive(fds[1],m)==1&&m.header.sequence==5&&m.body=="example");
    for(std::size_t n=1;n<sizeof(Header);++n){
        assert(raw(&h,n)==static_cast<ssize_t>(n));assert(receive(fds[1],m)==-1);
    }
    assert(send(fds[0],h,std::string(MaxPayload,'x')));
    assert(receive(fds[1],m)==1&&m.body==std::string(MaxPayload,'x'));
    assert(send(fds[0],h));assert(receive(fds[1],m)==1&&m.body.empty());
    assert(receive(fds[1],m)==0);
    auto malformed=h;malformed.size=100;assert(raw(&malformed,sizeof(malformed))==sizeof(malformed));assert(receive(fds[1],m)==-1);
    const std::string oversized(sizeof(Header)+MaxPayload+1,'x');assert(raw(oversized.data(),oversized.size())==static_cast<ssize_t>(oversized.size()));assert(receive(fds[1],m)==-1);
    pid_t child=fork();assert(child>=0);
    if(child==0){close(fds[0]);raise(SIGSTOP);_exit(0);}
    close(fds[1]);int status=0;assert(waitpid(child,&status,WUNTRACED)==child&&WIFSTOPPED(status));
    PendingQueue queue;queue.focus(1);auto request=packet(Command::Key);assert(queue.push(request,10));assert(send(fds[0],request));
    pollfd watch{fds[0],POLLIN,0};assert(poll(&watch,1,1)==0);
    assert(queue.expired(10+DeadlineUs));
    auto old=request;queue.focus(2);Pending stale;assert(queue.take(old,stale)==PendingQueue::Match::Stale);
    assert(kill(child,SIGKILL)==0);assert(waitpid(child,&status,0)==child&&WIFSIGNALED(status));
    assert(poll(&watch,1,100)==1);assert(receive(fds[0],m)==-1);close(fds[0]);
}
