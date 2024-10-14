
#include "central_func.h"
#include "config.h"

#include <sys/wait.h>

#ifdef DEBUG
#define CRTDEBUG
#endif
#include "crthttp.h"
#include "crtjson.h"
#include "crtlib.h"

extern "C" {
void clean_up();
int real_main(int argc, char **argv);
}

using namespace crtfun;

struct thread_data {
    int fdstdout;
    int fdstderr;
    string jobid;
    string outbuf;
    string errbuf;
};

#ifdef DEBUG
#define DEFAULT_INTERVAL 30
#define RETRY_CONCESSION 5
#else
#define DEFAULT_INTERVAL 600
#define RETRY_CONCESSION 60
#endif
#define MAX_CPU 128

typedef struct _cpu_info {
    char name[20];
    unsigned int user;
    unsigned int nice;
    unsigned int system;
    unsigned int idle;
    unsigned int iowait;
    unsigned int irq;
    unsigned int softirq;
} cpu_info_t;

// Get the CPU stat
int get_cpu_occupy(int cpu_index, cpu_info_t *info)
{
    FILE *fp = NULL;
    char key[10] = { 0 };
    char buf[256] = { 0 };
    int ret = 1;
    snprintf(key, 10, "cpu%d", cpu_index);
    fp = fopen("/proc/stat", "r");
    if (fp) {
        while (fgets(buf, sizeof(buf), fp)) {
            sscanf(buf, "%s %u %u %u %u %u %u %u", info->name, &info->user, &info->nice,
                &info->system, &info->idle, &info->iowait, &info->irq, &info->softirq);
            if (strcmp(info->name, key) == 0) {
                ret = 0;
                break;
            }
        }
        fclose(fp);
    }
    return ret;
}

double calc_cpu_rate(cpu_info_t *old_info, cpu_info_t *new_info)
{
    double od, nd;
    double usr_dif, sys_dif, nice_dif;
    double user_cpu_rate;
    double kernel_cpu_rate;

    od = (double)(old_info->user + old_info->nice + old_info->system + old_info->idle + old_info->iowait + old_info->irq + old_info->softirq);
    nd = (double)(new_info->user + new_info->nice + new_info->system + new_info->idle + new_info->iowait + new_info->irq + new_info->softirq);

    if (nd - od) {
        user_cpu_rate = (new_info->user - old_info->user) / (nd - od) * 100;
        kernel_cpu_rate = (new_info->system - old_info->system) / (nd - od) * 100;
        return user_cpu_rate + kernel_cpu_rate;
    }
    return 0;
}

#define VMRSS_LINE 22

double get_process_memory_use()
{
    char file_name[64] = { 0 };
    FILE *fd;
    char line_buff[512] = { 0 };
    snprintf(file_name, 64, "/proc/%jd/status", (intmax_t)getpid());
    fd = fopen(file_name, "r");
    if (nullptr == fd)
        return 0;
    char name[64];
    int vmrss = 0;
    for (int i = 0; i < VMRSS_LINE - 1; i++)
        fgets(line_buff, sizeof(line_buff), fd);
    fgets(line_buff, sizeof(line_buff), fd);
    sscanf(line_buff, "%s %d", name, &vmrss);
    fclose(fd);
    // cnvert VmRSS from KB to MB
    return vmrss / 1024.0;
}

static cpu_info_t sinfo[MAX_CPU];
static cpu_info_t einfo[MAX_CPU];
static int use_cpu = 0;

double get_cpu()
{
    string cpu;
    double cpurate = 0.0;
    int i;
    if (use_cpu == 0) {
        use_cpu = min((int)sysconf(_SC_NPROCESSORS_ONLN), MAX_CPU);
        for (i = 0; i < use_cpu; i++) {
            get_cpu_occupy(i, &sinfo[i]);
        }
    }
    else {
        for (i = 0; i < use_cpu; i++) {
            get_cpu_occupy(i, &einfo[i]);
            cpurate += calc_cpu_rate(&sinfo[i], &einfo[i]);
        }
        cpurate /= use_cpu;
    }
    return cpurate;
}

int central_func(int argc, char **argv)
{
    int ret;
    char useragent[256];
    int sleepsec = DEFAULT_INTERVAL;
    char central_request_url[4096];
    // char central_commitjob_url[4096];
    string next;
    string postdata;
    if (argc != 1) {
        return real_main(argc, argv);
    }
    // signal(SIGINT, SIG_IGN);
    //  daemon(1, 0);
    crtfun::set_debugformat(CRTDEBUG_PROCESSID);
    char *program = strrchr(argv[0], '/');
    if (!program) {
        program = argv[0];
    }
    else {
        program++;
    }
    snprintf(useragent, 256, "%s/%s", program, VERSION);
    crtfun::setup_http_user_agent(useragent);
    crtfun::setup_http_post_json(true);

    // snprintf(central_commitjob_url, 4096, "%sjob", CENTRAL_MODE);

    while (1) {
        sleepsec += RETRY_CONCESSION;
        snprintf(central_request_url, 4096, "%s?next=%s&cpu=%.2f&mem=%.2f", CENTRAL_MODE, next.c_str(), get_cpu(), get_process_memory_use());
        string json = crtfun::http_download_to_str(central_request_url, postdata.empty() ? 0 : postdata.c_str());
        crtdebug("[HTTP]ret:%s\n", json.c_str());
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
                    crtjson *job = parser.find(obj, "job");
                    job = parser.firstitem(job);
                    map<pid_t, struct thread_data *> subpids;
                    pair<map<pid_t, struct thread_data *>::iterator, bool> mapret;
                    while (job) {
                        string jobid = parser.findstr(job, "jobid");
                        string command = parser.findstr(job, "command");
                        // if interval is 0 and job is empty, it mean's need exit
                        sleepsec++;
                        // crtdebug("start job %s with cmd: %s\n", jobid.c_str(), command.c_str());
                        int fd_stdout[2];
                        int fd_stderr[2];
                        pipe(fd_stdout);
                        pipe(fd_stderr);
                        int flags = fcntl(fd_stdout[0], F_GETFL);
                        flags = flags | O_NONBLOCK;
                        fcntl(fd_stdout[0], F_SETFL, flags);
                        flags = fcntl(fd_stderr[0], F_GETFL);
                        flags = flags | O_NONBLOCK;
                        fcntl(fd_stderr[0], F_SETFL, flags);
                        fpid = fork();
                        if (fpid < 0) {
                            crtdebug("fork error!");
                            exit(1);
                        }
                        else if (fpid == 0) {
                            crtdebug("sub process %jd fork job %s with cmd: %s\n", (intmax_t)getpid(), jobid.c_str(), command.c_str());
                            close(fd_stdout[0]);
                            dup2(fd_stdout[1], STDOUT_FILENO);
                            close(fd_stderr[0]);
                            dup2(fd_stderr[1], STDERR_FILENO);
                            crtstringtoken token(command.c_str(), " ");
                            int fakeargc = 0;
                            char *fakeargv[64];
                            while (token.ismore()) {
                                fakeargv[fakeargc++] = strdup(token.nexttoken().c_str());
                            }
                            // need set the end to 0 for optparse function
                            fakeargv[fakeargc] = 0;
                            // for (int i = 0; i < fakeargc; i++) {
                            //     crtdebug("param[%d] = %s\n", i, fakeargv[i]);
                            // }
                            int ret = real_main(fakeargc, fakeargv);
                            // crtdebug("real_main %d", ret);
                            close(fd_stdout[1]);
                            close(fd_stderr[1]);
                            for (int i = 0; i < fakeargc; i++) {
                                free(fakeargv[i]);
                            }
                            return ret;
                        }
                        else if (fpid > 0) {
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
                    crtjson *root = parser.createarray();
                    while (subpids.size() > 0) {
                        it = subpids.begin();
                        while (it != subpids.end()) {
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
                            if (subpid > 0) {
                                // commit result, it->second
                                crtjson *obj = parser.createobject();
                                parser.objectadd_string(obj, "jobid", it->second->jobid.c_str());
                                parser.objectadd_string(obj, "stdout", it->second->outbuf.c_str());
                                parser.objectadd_string(obj, "stderr", it->second->errbuf.c_str());
                                parser.objectadd_number(obj, "status", status);
                                parser.arrayadd(root, obj);
                                crtdebug("sub process %lu finish job %s\n", it->first, it->second->jobid.c_str());
                                close(it->second->fdstdout);
                                close(it->second->fdstderr);
                                delete it->second;
                                subpids.erase(it++);
                            }
                            else {
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
                // ret = real_main(argc, argv);
            }
        }
        // if interval is 0 and job is empty, it mean's need exit
        if (sleepsec == 0) {
            break;
        }
        sleep(sleepsec);
    }
    return 0;
}
