
void uninit_ping_buffer_ipv4();
#ifdef IPV6
    void uninit_ping_buffer_ipv6();
#endif

// signal_handler is not problem

void clean_main_switches()
{
    int i;
    HOST_ENTRY *h;
    /* only loop table is ok? i see that event_queue_ping and event_queue_timeout is all reference it? */
    for (i = 0; i < num_hosts; i++) {
        h = table[i];
        free(h->name);
        free(h->host);
        free(h->resp_times);
        free(h->event_storage_ping);
        free(h->event_storage_timeout);
        free(h);
    }
    free(table);
    table = 0;
    memset(&event_queue_ping, 0, sizeof(struct event_queue));
    memset(&event_queue_timeout, 0, sizeof(struct event_queue));
    event_storage_count = 0;

    //will setup in main function, not need to parse
    //char *prog;
    //int ident4 = 0; /* our icmp identity field */
    //int ident6 = 0;
    //int socket4 = -1;
    //int socktype4 = -1;
    //int using_sock_dgram4 = 0;
    //int hints_ai_family = AF_INET;
    //int socket6 = -1;
    //int socktype6 = -1;
    //unsigned int trials;
    //struct in_addr src_addr;
    //struct in6_addr src_addr6;
    //struct timespec current_time; /* current time (pseudo) */
    //int64_t current_time_ns;
    //int64_t start_time;
    //int64_t end_time;
    //int64_t last_send_time; /* time last ping was sent */
    //int64_t next_report_time; /* time next -Q report is expected */
    //verbose_flag;
    //backoff_flag;

    status_snapshot = 0;
    finish_requested = 0;
    debugging = 0;

    retry = DEFAULT_RETRY;
    timeout = (int64_t)DEFAULT_TIMEOUT * 1000000;
    interval = (int64_t)DEFAULT_INTERVAL * 1000000;
    perhost_interval = (int64_t)DEFAULT_PERHOST_INTERVAL * 1000000;
    backoff = DEFAULT_BACKOFF_FACTOR;
    ping_data_size = DEFAULT_PING_DATA_SIZE;
    count = 1;
    min_reachable = 0;
    report_interval = 0;
    ttl = 0;
    src_addr_set = 0;
#ifdef IPV6
    src_addr6_set = 0;
#endif

    /* global stats */
    max_reply = 0;
    min_reply = 0;
    total_replies = 0;
    sum_replies = 0;
    max_hostname_len = 0;
    num_hosts = 0;
    num_alive = 0;
    num_unreachable = 0;
    num_noaddress = 0;
    num_timeout = 0;
    num_pingsent = 0;
    num_pingreceived = 0;
    num_othericmprcvd = 0;
    elapsed_flag = 0;

    /* switches */
    generate_flag = 0;
    quiet_flag = 0;
    stats_flag = 0;
    unreachable_flag = 0;
    alive_flag = 0;
    version_flag = 0;
    count_flag = 0;
    loop_flag = 0;
    netdata_flag = 0;
    per_recv_flag = 0;
    report_all_rtts_flag = 0;
    name_flag = 0;
    addr_flag = 0;
    rdns_flag = 0;
    multif_flag = 0;
    timeout_flag = 0;
    fast_reachable = 0;
    outage_flag = 0;
    timestamp_flag = 0;
    timestamp_format_flag = 0;
    random_data_flag = 0;
    cumulative_stats_flag = 0;
    check_source_flag = 0;
    print_tos_flag = 0;
    print_ttl_flag = 0;
#if defined(DEBUG) || defined(_DEBUG)
    randomly_lose_flag = 0;
    trace_flag = 0;
    print_per_system_flag = 0;
    lose_factor = 0;
#endif /* DEBUG || _DEBUG */
    fwmark = 0;
    filename = NULL;
}

void clean_up()
{
	seqmap_reset();
	uninit_ping_buffer_ipv4();
#ifdef IPV6
    uninit_ping_buffer_ipv6();
#endif
    clean_main_switches();
}

#include "http_download.inc"

int main(int argc, char **argv)
{
    int ret;
    while(1)
    {
        download_file("http://www.baidu.com/");
        //
        ret = real_main(argc, argv);
    }
}

#define main real_main