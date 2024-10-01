
void uninit_ping_buffer_ipv4();
#ifdef IPV6
    void uninit_ping_buffer_ipv6();
#endif

void clean_main()
{
    struct event *cursor = event_queue_ping.first;
    int i = 0;
    table = (HOST_ENTRY **)calloc(num_hosts, sizeof(HOST_ENTRY *));
    if (!table)
        crash_and_burn("Can't malloc array of hosts");
    /* initialize table of hosts. we know that we have ping events scheduled
     * for each of them */
    for (cursor = event_queue_ping.first; cursor; cursor = cursor->ev_next) {
        table[i] = cursor->host;
        cursor->host->i = i;
        i++;
    }

    # already parse calloc malloc strdup functions
    num_hosts++;
    event_storage_count

struct event_queue event_queue_ping;
struct event_queue event_queue_timeout;


    p->name = strdup(name);
    p->host = strdup(host);
    p->resp_times = (int64_t *)malloc(trials * sizeof(int64_t));
    p->event_storage_ping = (struct event *)calloc(event_storage_count, sizeof(struct event));
    p->event_storage_timeout = (struct event *)calloc(event_storage_count, sizeof(struct event));

int event_storage_count;
p = (HOST_ENTRY *)calloc(1, sizeof(HOST_ENTRY));
    /* allocate event storage */
    p->event_storage_ping = (struct event *)calloc(event_storage_count, sizeof(struct event));
    p->event_storage_timeout = (struct event *)calloc(event_storage_count, sizeof(struct event));




/* basic algorithm to ensure that we have correct data at all times:
 *
 * 1. when a ping is sent:
 *    - two events get added into event_queue:
 *      - t+PERIOD: ping event
 *      - t+TIMEOUT: timeout event
 *
 * 2. when a ping is received:
 *    - record statistics (increase num_sent and num_received)
 *    - remove timeout event (we store the event in seqmap, so that we can retrieve it when the response is received)
 *
 * 3. when a timeout happens:
 *    - record statistics (increase num_sent only)
 */

#define EV_TYPE_PING 1
#define EV_TYPE_TIMEOUT 2

struct event {
    struct event *ev_prev;
    struct event *ev_next;
    int64_t ev_time;
    struct host_entry *host;
    int ping_index;
};

struct event_queue {
    struct event *first;
    struct event *last;
};

/*** globals ***/

HOST_ENTRY **table = NULL; /* array of pointers to items in the list */

/* we keep two separate queues: a ping queue, for when the next ping should be
 * sent, and a timeout queue. the reason for having two separate queues is that
 * the ping period and the timeout value are different, so if we put them in
 * the same event queue, we would need to scan many more entries when inserting
 * into the sorted list.
 */
struct event_queue event_queue_ping;
struct event_queue event_queue_timeout;

char *prog;
int ident4 = 0; /* our icmp identity field */
int ident6 = 0;
int socket4 = -1;
int socktype4 = -1;
int using_sock_dgram4 = 0;
#ifndef IPV6
int hints_ai_family = AF_INET;
#else
int socket6 = -1;
int socktype6 = -1;
int hints_ai_family = AF_UNSPEC;
#endif

volatile sig_atomic_t status_snapshot = 0;
volatile sig_atomic_t finish_requested = 0;

unsigned int debugging = 0;

/* all time-related values are int64_t nanoseconds */
unsigned int retry = DEFAULT_RETRY;
int64_t timeout = (int64_t)DEFAULT_TIMEOUT * 1000000;
int64_t interval = (int64_t)DEFAULT_INTERVAL * 1000000;
int64_t perhost_interval = (int64_t)DEFAULT_PERHOST_INTERVAL * 1000000;
float backoff = DEFAULT_BACKOFF_FACTOR;
unsigned int ping_data_size = DEFAULT_PING_DATA_SIZE;
unsigned int count = 1, min_reachable = 0;
unsigned int trials;
int64_t report_interval = 0;
unsigned int ttl = 0;
int src_addr_set = 0;
struct in_addr src_addr;
#ifdef IPV6
int src_addr6_set = 0;
struct in6_addr src_addr6;
#endif

/* global stats */
int64_t max_reply = 0;
int64_t min_reply = 0;
int64_t total_replies = 0;
int64_t sum_replies = 0;
int max_hostname_len = 0;
int num_hosts = 0; /* total number of hosts */
int num_alive = 0, /* total number alive */
    num_unreachable = 0, /* total number unreachable */
    num_noaddress = 0; /* total number of addresses not found */
int num_timeout = 0, /* number of times select timed out */
    num_pingsent = 0, /* total pings sent */
    num_pingreceived = 0, /* total pings received */
    num_othericmprcvd = 0; /* total non-echo-reply ICMP received */

struct timespec current_time; /* current time (pseudo) */
int64_t current_time_ns;
int64_t start_time;
int64_t end_time;
int64_t last_send_time; /* time last ping was sent */
int64_t next_report_time; /* time next -Q report is expected */

/* switches */
int generate_flag = 0; /* flag for IP list generation */
int verbose_flag, quiet_flag, stats_flag, unreachable_flag, alive_flag;
int elapsed_flag, version_flag, count_flag, loop_flag, netdata_flag;
int per_recv_flag, report_all_rtts_flag, name_flag, addr_flag, backoff_flag, rdns_flag;
int multif_flag, timeout_flag, fast_reachable;
int outage_flag = 0;
int timestamp_flag = 0;
int timestamp_format_flag = 0;
int random_data_flag = 0;
int cumulative_stats_flag = 0;
int check_source_flag = 0;
int print_tos_flag = 0;
int print_ttl_flag = 0;
#if defined(DEBUG) || defined(_DEBUG)
int randomly_lose_flag, trace_flag, print_per_system_flag;
int lose_factor;
#endif /* DEBUG || _DEBUG */

unsigned int fwmark = 0;

char *filename = NULL; /* file containing hosts to ping */

}

void clean_up()
{
	seqmap_reset();
	uninit_ping_buffer_ipv4();
    uninit_ping_buffer_ipv6();
}
