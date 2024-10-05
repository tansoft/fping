
#include "config.h"
#include "central_func.h"

#include <sys/wait.h> 

#ifdef DEBUG
#define CRTDEBUG
#endif
#include "crtlib.h"
#include "crthttp.h"
#include "crtjson.h"

extern "C" int real_main(int argc, char **argv);

using namespace crtfun;

struct thread_data {
    int fdstdout;
    int fdstderr;
    string jobid;
    string outbuf;
    string errbuf;
};

#define DEFAULT_INTERVAL    600
#define RETRY_CONCESSION    60

int central_func(int argc, char **argv) {
    int ret;
    char useragent[256];
    int sleepsec = DEFAULT_INTERVAL;
    char central_request_url[4096];
    //char central_commitjob_url[4096];
    string next;
    string postdata;
    if (argc != 1) {
        return real_main(argc, argv);
    }
    crtfun::set_debugformat(CRTDEBUG_PROCESSID);
    snprintf(useragent, 256, "%s/%s", PACKAGE, VERSION);
    crtfun::setup_http_user_agent(useragent);
    crtfun::setup_http_post_json(true);

    //snprintf(central_commitjob_url, 4096, "%sjob", CENTRAL_MODE);

    while(1) {
        sleepsec += RETRY_CONCESSION;
        snprintf(central_request_url, 4096, "%s?next=%s", CENTRAL_MODE, next.c_str());
        string json = crtfun::http_download_to_str(CENTRAL_MODE, postdata.empty()?0:postdata.c_str());
        crtdebug("[HTTP]ret:%s\n",json.c_str());
        if (!json.empty()) {
            crtjsonparser parser;
            crtjson *obj = parser.parse(json.c_str());
            pid_t fpid;
            if (obj) {
                if (parser.findint(obj, "status") == 200) {
                    string command = parser.findstr(obj, "command");
                    next = parser.findstr(obj, "next");
                    postdata = "";
                    sleepsec = parser.findint(obj, "interval");
                    if (sleepsec == 0) sleepsec = DEFAULT_INTERVAL;
                    crtjson *job = parser.find(obj, "job");
                    job = parser.firstitem(job);
                    map<pid_t, struct thread_data *> subpids;
                    pair<map<pid_t, struct thread_data *>::iterator, bool> mapret;
                    while(job) {
                        string jobid = parser.findstr(job, "jobid");
                        string command = parser.findstr(job, "command");
                        crtdebug("start job %s with cmd: %s\n", jobid.c_str(), command.c_str());
                        int fd_stdout[2];
                        int fd_stderr[2];
                        pipe(fd_stdout);
                        pipe(fd_stderr);
                        int flags = fcntl(fd_stdout[0],F_GETFL);
                        flags = flags | O_NONBLOCK;
                        fcntl(fd_stdout[0],F_SETFL,flags);
                        flags = fcntl(fd_stderr[0],F_GETFL);
                        flags = flags | O_NONBLOCK;
                        fcntl(fd_stderr[0],F_SETFL,flags);
                        fpid = fork();
                        if (fpid < 0) {
                            printf("fork error!");
                            exit(1);
                        } else if (fpid == 0) {
                            crtdebug("sub process %lu fork job %s with cmd: %s\n", getpid(), jobid.c_str(), command.c_str());
                            close(fd_stdout[0]);
                            dup2(fd_stdout[1], STDOUT_FILENO);
                            close(fd_stderr[0]);
                            dup2(fd_stderr[1], STDERR_FILENO);
                            crtstringtoken token(command.c_str(), " ");
                            int fakeargc = 0;
                            char *fakeargv[64];
                            while(token.ismore()) {
                                fakeargv[fakeargc++] = strdup(token.nexttoken().c_str());
                            }
                            int ret = real_main(fakeargc, fakeargv);
                            for(int i=0;i<fakeargc;i++) {
                                //printf("param[%d]=%s",i, fakeargv[i]);
                                free(fakeargv[i]);
                            }
                            return ret;
                        } else if (fpid > 0) {
                            // main process
                            struct thread_data *data = new struct thread_data;
                            close(fd_stdout[1]);
                            data->fdstdout = fd_stdout[0];
                            close(fd_stderr[1]);
                            data->fdstderr = fd_stderr[0];
                            data->jobid = jobid;
                            mapret = subpids.insert(map<pid_t, struct thread_data *>::value_type(fpid, data));
                            if (!mapret.second) {
                                printf("insert map error!");
                                exit(1);
                            }
                        }
                        job = parser.nextitem(job);
                    }
                    int status;
                    pid_t subpid;
                    int len;
                    map<pid_t, struct thread_data *>::const_iterator it;
                    char buf[4097];
                    crtjson *root=parser.createarray();
                    while(subpids.size() > 0) {
                        it = subpids.begin();
                        while(it!=subpids.end()) {
                            len = read(it->second->fdstdout, buf, 4096);
                            if (len > 0) {
                                buf[len] = '\0';
                                it->second->outbuf += buf;
                            }
                            len = read(it->second->fdstderr, buf, 4096);
                            if (len > 0) {
                                buf[len] = '\0';
                                it->second->errbuf += buf;
                            }
                            subpid = waitpid(it->first, &status, WNOHANG); // not use -1 to check all subprocess
                            if (subpid>0) {
                                //commit result, it->second
                                crtjson *obj=parser.createobject();
                                parser.objectadd_string(obj, "jobid", it->second->jobid.c_str());
                                parser.objectadd_string(obj, "stdout", it->second->outbuf.c_str());
                                parser.objectadd_string(obj, "stderr", it->second->errbuf.c_str());
                                parser.arrayadd(root, obj);
                                crtdebug("commit result pid:%lu jobid:%s\n", it->first, it->second->jobid.c_str());
                                close(it->second->fdstdout);
                                close(it->second->fdstderr);
                                delete it->second;
                                subpids.erase(it++);
                            } else {
                                it++;
                            }
                        }
                        usleep(100);
                    }
                    postdata = parser.tostring(root, 0);
                    crtdebug(postdata.c_str());
                    parser.delete_json(root);
                }
                parser.delete_json(obj);
                //ret = real_main(argc, argv);
            }
        }
        sleep(sleepsec);
    }
    return 0;
}
