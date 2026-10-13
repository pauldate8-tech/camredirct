#include <sys/socket.h>
#include <sys/un.h>
#include <mutex>
#define CONFIG_PATH "/data/adb/camredirect/config"
#define HITS_PATH   "/data/adb/camredirect/hits.log"

static void append_file(const char *path,const std::string &s){
    int fd=open(path,O_WRONLY|O_CREAT|O_APPEND,0600); if(fd<0)return; write(fd,s.data(),s.size()); close(fd); }

static bool valid_pkg(const std::string &s){ if(s.empty()||s.size()>128)return false;
    for(char c:s) if(!(isalnum((unsigned char)c)||c=='.'||c=='_'))return false; return true; }
static bool valid_id(const std::string &s){ if(s.empty()||s.size()>32)return false;
    for(char c:s) if(!isalnum((unsigned char)c))return false; return true; }

// Rewrite config: drop any existing line for `key`, keep comments/other keys,
// then (if val non-empty) append key=val. Atomic via temp+rename.
static void rewrite_config(const std::string &key,const std::string &val){
    std::string in=read_file(CONFIG_PATH), out; size_t pos=0;
    while(pos<in.size()){ size_t eol=in.find('\n',pos); std::string line=in.substr(pos,eol-pos);
        pos=(eol==std::string::npos)?in.size():eol+1;
        std::string k=line; auto eq=line.find('='); if(eq!=std::string::npos)k=line.substr(0,eq);
        if(k==key) continue;                 // drop old entry
        out+=line; out+='\n'; }
    if(!val.empty()) out+=key+"="+val+"\n";
    std::string tmp=std::string(CONFIG_PATH)+".tmp";
    int fd=open(tmp.c_str(),O_WRONLY|O_CREAT|O_TRUNC,0600);
    if(fd<0)return; write(fd,out.data(),out.size()); fsync(fd); close(fd);
    rename(tmp.c_str(),CONFIG_PATH);
}

static void handle_manager_client(int c){
    char buf[512]; ssize_t r=read(c,buf,sizeof buf-1); if(r<=0)return; buf[r]='\0';
    std::string cmd(buf); while(!cmd.empty()&&(cmd.back()=='\n'||cmd.back()=='\r'))cmd.pop_back();
    auto reply=[&](const char*s){ write(c,s,strlen(s)); };
    if(cmd=="GET"){ std::string cfg=read_file(CONFIG_PATH); write(c,cfg.data(),cfg.size()); return; }
    // SET <pkg> <id>   |   DEL <pkg>
    std::string op,a,b; { std::istringstream ss(cmd); ss>>op>>a>>b; }
    if(op=="SET"&&valid_pkg(a)&&valid_id(b)){ rewrite_config(a,b); reply("OK\n"); }
    else if(op=="DEL"&&valid_pkg(a)){ rewrite_config(a,""); reply("OK\n"); }
    else reply("ERR\n");
}

static void start_manager_listener(){
    int s=socket(AF_UNIX,SOCK_STREAM,0); if(s<0)return;
    sockaddr_un addr{}; addr.sun_family=AF_UNIX;
    const char *name="camredirect"; addr.sun_path[0]='\0';          // abstract namespace
    memcpy(addr.sun_path+1,name,strlen(name));
    socklen_t len=offsetof(sockaddr_un,sun_path)+1+strlen(name);
    if(bind(s,(sockaddr*)&addr,len)<0){ close(s); return; }
    listen(s,4);
    for(;;){ int c=accept(s,nullptr,nullptr); if(c<0)continue; handle_manager_client(c); close(c); }
}

static void companion_handler(int fd){
    static std::once_flag once;
    std::call_once(once,[]{ std::thread(start_manager_listener).detach(); });  // persist for process life
    char op=0; if(read(fd,&op,1)<=0) return;
    if(op=='R'){ std::string cfg=read_file(CONFIG_PATH); uint32_t len=cfg.size();
        write(fd,&len,sizeof len); write(fd,cfg.data(),len); }
    else if(op=='L'){ uint32_t len=0; if(read_full(fd,&len,sizeof len)!=sizeof len||len>4096)return;
        std::string line(len,'\0'); if(read_full(fd,line.data(),len)!=(ssize_t)len)return;
        append_file(HITS_PATH,line); }
}
REGISTER_ZYGISK_COMPANION(companion_handler)
