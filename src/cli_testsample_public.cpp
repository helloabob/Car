#include "cli_testsample_public.h"

#ifdef TESTWITHGCC
unsigned int LOG_FLAG = LOG_LOCAL0;
#endif

//bdebug_level: 0 -- close,  1 -- critical,  2 -- warning,  3 -- info
#ifdef BDEBUGLEVEL
#	define bdebug_level BDEBUGLEVEL
#else
#	define bdebug_level 0
#endif
//---------------------------- cmd/key string -------------------------------------
const char *str_notfount = "Unkown";
const char* get_cmd_str(unsigned int v)
{
    int n = sizeof(cmd_string_array) / sizeof(cmd_string_array[0]);
    for (int i=0; i<n; i++){
        if (cmd_string_array[i].v == v){
            return cmd_string_array[i].str;
        }
    }
    return str_notfount;
}

const char* get_cmd_rem(unsigned int v)
{
    int n = sizeof(cmd_string_array) / sizeof(cmd_string_array[0]);
    for (int i=0; i<n; i++){
        if (cmd_string_array[i].v == v){
            return cmd_string_array[i].rem;
        }
    }
    return str_notfount;
}

const char* get_key_str(unsigned int v)
{
    int n = sizeof(key_string_array) / sizeof(key_string_array[0]);
    for (int i=0; i<n; i++){
        if (key_string_array[i].v == v){
            return key_string_array[i].str;
        }
    }
    return str_notfount;
}

const char* get_key_rem(unsigned int v)
{
    int n = sizeof(key_string_array) / sizeof(key_string_array[0]);
    for (int i=0; i<n; i++){
        if (key_string_array[i].v == v){
            return key_string_array[i].rem;
        }
    }
    return str_notfount;
}

const char* get_sm_str(unsigned int v)
{
    int n = sizeof(sm_string_array) / sizeof(sm_string_array[0]);
    for (int i=0; i<n; i++){
        if (sm_string_array[i].v == v){
            return sm_string_array[i].str;
        }
    }
    return str_notfount;
}

const char* get_smreginfoget_str(unsigned int v)
{
    int n = sizeof(smgetreginfo_string_array) / sizeof(smgetreginfo_string_array[0]);
    for (int i=0; i<n; i++){
        if (smgetreginfo_string_array[i].v == v){
            return smgetreginfo_string_array[i].str;
        }
    }
    return str_notfount;
}

const char* get_smreginfogetbymobile_str(unsigned int v)
{
    int n = sizeof(smgetreginfobymobile_string_array) / sizeof(smgetreginfobymobile_string_array[0]);
    for (int i=0; i<n; i++){
        if (smgetreginfobymobile_string_array[i].v == v){
            return smgetreginfobymobile_string_array[i].str;
        }
    }
    return str_notfount;
}

const char* get_smmirror_str(unsigned int v)
{
    int n = sizeof(smmirror_string_array) / sizeof(smmirror_string_array[0]);
    for (int i=0; i<n; i++){
        if (smmirror_string_array[i].v == v){
            return smmirror_string_array[i].str;
        }
    }
    return str_notfount;
}

const char* get_smcall_str(unsigned int v)
{
    int n = sizeof(smcall_string_array) / sizeof(smcall_string_array[0]);
    for (int i=0; i<n; i++){
        if (smcall_string_array[i].v == v){
            return smcall_string_array[i].str;
        }
    }
    return str_notfount;
}


///--------------------------------------------------------------------------------
#ifdef USING_RSYSLOG
/* LOG_FLAG is used as #1 param when you call to syslog(), I recommand you set it to LOG_LOCAL0~LOG_LOCAL7,
 * and then append statements such as
    local1.* /var/log/f2f_lb.log
 * to /etc/rsyslog.conf, this will let the BDEBUG write syslog msgs to the related files besides /var/log/syslog.
 */
extern unsigned int LOG_FLAG;
#endif

#ifdef BDEBUG_ON
char* repstr(int len, char cFill)
{
    memset(szFilling, cFill, sizeof(szFilling));
    if (len<=0)
        szFilling[1]=0;
    else if (len<sizeof(szFilling))
        szFilling[len]=0;
    return szFilling;
}

void broadcast_debug (const char * funcname, const int line, const char * filename, const char* va, ...)
{
#ifdef USING_RSYSLOG
    char info[2048];
    char* p=strrchr((char*)filename,'/');
    if (!p) 
        p = (char *)filename;
    else
        p++;
    int n = sprintf(info," %s[%04d] : %s | ",p,line,funcname);
    va_list ap;
    va_start(ap, va);
    vsprintf(info+n,va, ap);
#ifdef TESTWITHGCC
//    syslog(LOG_FLAG, info);
#endif
    if (info[strlen(info)-1]!='\n')
        strcat(info, "\n");
    printf("%s",info);
    va_end(ap);
#else
    int info_udp_socket_fd=0;
    struct sockaddr_in info_udp_sout;
    int info_opt;
    info_opt = 1;
    memset(&info_udp_sout,0,sizeof(info_udp_sout));
    info_udp_sout.sin_family = AF_INET;
    info_udp_sout.sin_addr.s_addr=INADDR_BROADCAST;
    info_udp_sout.sin_port = htons(8888);

    pthread_mutex_t mutex_bdebug=PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&mutex_bdebug);

    info_udp_socket_fd=(int)socket(AF_INET,SOCK_DGRAM,0);
    if(info_udp_socket_fd != -1)
    {
        setsockopt(info_udp_socket_fd,SOL_SOCKET,SO_BROADCAST,(char*)&info_opt,sizeof(info_opt));
        char info[2048];
        int n = sprintf(info,"\t%s[%04d] : %s\n",filename,line,funcname);
        va_list ap;
        va_start(ap, va);
        vsprintf(info+n,va, ap);
        sendto(info_udp_socket_fd,info,strlen(info),0,(struct sockaddr *)&info_udp_sout,sizeof(info_udp_sout));
        va_end(ap);

        shutdown(info_udp_socket_fd,SHUT_RDWR);
        close(info_udp_socket_fd);
    }
    pthread_mutex_unlock(&mutex_bdebug);
#endif

}

void broadcast_mem (const char * funcname, const int line, const char * filename,
        unsigned long addr,char* varname)
{
    if (NULL==addr) return;
    char info[2048];
    int n;
    int i;
    unsigned char* p =(unsigned char*) addr;

    n = sprintf(info, "00-------03 04-------07 08-------0B 0C-------0F (0x%X, variable name: %s)\n",addr,varname);
    for (i=0; i<16; i++){
        n += sprintf(info+n, "%02X %02X %02X %02X %02X %02X %02X %02X-%02X %02X %02X %02X %02X %02X %02X %02X\n",
                p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],
                p[8],p[9],p[10],p[11],p[12],p[13],p[14],p[15]);
        p += 16;
    }
    broadcast_debug(funcname,line,filename,info);
}

void broadcast_bt (const char * funcname, const int line, const char * filename)
{
#if 0
    char info[1024];
    void *trace[36];
    char **messages = (char **)NULL;
    int i, trace_size = 0;

    trace_size = backtrace(trace, 36);
    messages = backtrace_symbols(trace, trace_size);
    for (i=0; i<trace_size; ++i)
    {
        sprintf(info,"[Call stack] %s\n", messages[i]);
        broadcast_debug(funcname,line,filename,info);
    }
#endif
}

void broadcast_debug_showargv(const char * funcname, const int line, const char * filename, int argc, char** argv)
{
    char info[1024];
    int j=0;
    int i;
    j+=sprintf(info,"****argv : ");
    for (i=0;i<argc;i++)
        j+=sprintf(info+j," %s",(NULL==argv[i])?"NULL":argv[i]);
    broadcast_debug(funcname,line,filename,info);
}
#endif //ifdef BDEBUG_ON
///--------------------------------------------------------------------------------

