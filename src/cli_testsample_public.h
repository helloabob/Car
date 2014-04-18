#ifndef CLI_TESTSAMPLE_PUBLIC_H
#define CLI_TESTSAMPLE_PUBLIC_H

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
//#include <string>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef TESTWITHGCC
#include <syslog.h>
#endif
#include <sys/timeb.h>
#include "f2fdefs.h"

#define USING_RSYSLOG


//---------------------------- definition of Terminal State-machine  --------------
enum eStateMachine{

    // 1) becomes SM_LOGINREQ immediately.
    SM_BOOT					= 0,		// Just boot up

    // 1) to get login server's ip (or domain) and port, sending CMD_LB_LOGINSERVERREQ every 2,2,4,4,8,8...secs,
    // 2) becomes SM_LOGINREQ when received CMD_LB_LOGINSERVERACK and succeed to resolve domain of login server
    SM_LOGINSVRIPREQ		= 1,		// Req for login_server_address

    // 1) to send CMD_NAT_LOGINREQ every 2,2,4,4,8,8...secs
    // 2) becomes SM_WAITINGFWACK when received CMD_NAT_LOGINACK
    // 3) becomes SM_LOGINSVRIPREQ after sending CMD_NAT_LOGINREQ 10 times
    SM_LOGINREQ				= 2,		// Req for login

    // 1) to send CMD_NAT_LOGINREQ every 2,2,2...secs
    // 2) becomes SM_NATDETECTREQ when received CMD_NAT_FWDETECTACK, and set firewall-flag to false
    // 3) becomes SM_NATDETECTREQ after sending CMD_NAT_LOGINREQ 5 times.
    SM_WAITINGFWACK			= 3,		// Waiting for firewall_detect ack

    // 1) to send CMD_NAT_NATDETECTREQ every 2,2,4,4,8,8...secs
    // 2) becomes SM_HEARTBEATING when CMD_NAT_NATDETECTACK received.
    // 3) becomes SM_BOOT after sending CMD_NAT_NATDETECTREQ 10 times.
    SM_NATDETECTREQ			= 4,		// Req for nat detect

    // 1) to send CMD_HEARTBEAT_REQ every 16 secs.
    // 2) becomes SM_BOOT if where's no CMD_HEARTBEAT_ACK received for 3*16 secs.
    SM_HEARTBEATING			= 5,		// Heart-beating
};

/*bob*/
typedef enum eCallStateMachine{
    // 1) becomes SMCALL_ADDRESSINGREQ when user takes a call.
    // 2) becomes SMCALL_CALLEEINFORM_LAN when CMD_CALL_CALLEEINFORM received and two terminals have the same ip.
    // 3) becomes SMCALL_CALLEEINFORM when CMD_CALL_CALLEEINFORM received.
    SMCALL_IDLE             = 0,        // Ready to-call or to-be-called.

    // 1) to send CMD_CALL_ADDRESSINGREQ every 2,2,2...secs,
    // 2) becomes SMCALL_CALLINGREQ_LAN when CMD_CALL_ADDRESSINGACK received and two terminals have the same ip.
    // 2) becomes SMCALL_CALLINGREQ when CMD_CALL_ADDRESSINGACK received.
    // 3) becomes SMCALL_LINEBUSY when CMD_CALL_ADDRESSINGACK received while callee offline, or after sending CMD_CALL_ADDRESSINGREQ 10 times.
    // 4) becomes SMCALL_CALLINGREQ when CALL_FWTRAVERSING received.
    // 5) If it receives CMD_CALL_FWTRAVERSING from callee, callee's ip/port should be saved for later use.
    SMCALL_ADDRESSINGREQ    = 1,        // Aaddressing request from caller

    // 1) to send CMD_CALL_CALLEEINFORMACK and CMD_CALL_FWTRAVERSING every 2,2,2...secs, to caller lan ip,
    // 2) becomes SMCALL_CALLEEINFORM after sending CMD_CALL_CALLEEINFORMACK and CMD_CALL_FWTRAVERSING 10 times.
    // 3) becomes SMCALL_RING when CMD_CALL_CALLINGREQ received.
    SMCALL_CALLEEINFORM_LAN = 2,

    // 1) to send CMD_CALL_CALLEEINFORMACK and CMD_CALL_FWTRAVERSING every 2,2,2...secs,
    // 2) becomes SMCALL_IDLE after sending CMD_CALL_CALLEEINFORMACK and CMD_CALL_FWTRAVERSING 10 times.
    // 3) becomes SMCALL_RING when CMD_CALL_CALLINGREQ received.
    SMCALL_CALLEEINFORM     = 3,        // Calleeinform from server

    // 1) to send CMD_CALL_CALLINGREQ every 2,2,2...secs, to callee lan ip,
    // 2) becomes SMCALL_CALLINGREQ  after sending CMD_CALL_CALLINGREQ 3 times.
    // 3) becomes SMCALL_RINGBACK when CMD_CALL_CALLINGACK or CMD_CALL_RING received.
    SMCALL_CALLINGREQ_LAN   = 4,        // Calling request from caller

    // 1) to send CMD_CALL_CALLINGREQ every 2,2,2...secs, to callee/mirror wan ip,
    // 2) becomes SMCALL_CALLEENOANSWER after sending CMD_CALL_CALLINGREQ 10 times.
    // 3) becomes SMCALL_RINGBACK when CMD_CALL_CALLINGACK or CMD_CALL_RING received.
    SMCALL_CALLINGREQ       = 5,        // Calling request from caller

    // 1) to send CMD_CALL_RING every 2,2,2...secs,
    // 2) becomes SMCALL_HANGUP after sending CMD_CALL_RING 10 times.
    // 3) becomes SMCALL_CALLEEAUDIO after user answering the call.
    SMCALL_RING             = 6,        // Callee ring

    // 1) to send CMD_CALL_RINGBACK every 2,2,2...secs,
    // 2) becomes SMCALL_CALLEENOANSWER after sending CMD_CALL_RINGBACK 10 times.
    // 3) becomes SMCALL_CALLERAUDIO when CMD_CALL_CALLEEAUDIO received.
    SMCALL_RINGBACK         = 7,        // Caller ringback

    // 1) to send CMD_CALL_CALLEEAUDIO whenever there are audio data to be sent
    // 2) becomes SMCALL_OTHERSIDE_HANGUP when CMD_CALL_HANGUP received.
    // 3) bedomes SMCALL_OTHERSIDE_HANGUP if there is no CMD_CALL_CALLERAUDIO received for 20 secs.
    // 4) becomes SMCALL_HANGUP when hangUp() is called.
    SMCALL_CALLEEAUDIO      = 8,        // audio from callee

    // 1) to send CMD_CALL_CALLERAUDIO whenever there are audio data to be sent
    // 2) becomes SMCALL_OTHERSIDE_HANGUP when CMD_CALL_HANGUP received.
    // 3) becomes SMCALL_OTHERSIDE_HANGUP if there is no CMD_CALL_CALLEEAUDIO received for 20 secs.
    // 4) becomes SMCALL_HANGUP when hangUp() is called.
    SMCALL_CALLERAUDIO      = 9,        // audio from caller

    // 1) notify user by display message or emitting busytone to indicate no callee's answer.
    // 2) becomes SMCALL_HANGUP 20 secs later.
    SMCALL_CALLEENOANSWER   = 10,        // Caller emits busytone stand for no answer.

    // 1) notify user by display message or emitting busytone to indicate line busy.
    // 2) becomes SMCALL_IDLE 20 secs later.
    SMCALL_LINEBUSY         = 11,        // Caller emits busytone stand for line busy.

    // 1) notify user by display message or emitting busytone to indicate hungup by the other side.
    // 2) becomes SMCALL_HANGUP_SENDING immediately.
    SMCALL_HANGUP           = 12,       // hangup.

    // 1) becomes SMCALL_IDLE later after sending 3 times of CMD_CALL_HUANGUP.
    SMCALL_HANGUP_SENDING   = 13,       // hangup.

    // 1) notify user by display message or emitting busytone to indicate hungup by the other side.
    // 2) becomes SMCALL_IDLE immediately.
    SMCALL_OTHERSIDE_HANGUP = 14,       // otherside huangup

    SMCALL_CALLEEAUDIO_TEST = 15,       // for test to send callee audio only.

    SMCALL_CALLERAUDIO_TEST = 16,       // for test to send caller audio only.
}eCallStateMachine;

enum eMirrorStateMachine{
    // 1) becomes SMMIRROR_FORWARDING when CMD_CALL_MIRRORINFORM
    SMMIRROR_STANDBY        = 0,        // Ready to mirror

    // 1) to forward all calling commands and media to other side
    // 2) becomes SMMIRROR_INFORMSTATE after forwarding CMD_CALL_HANGUP to other side
    // 3) becomes SMMIRROR_INFORMSTATE if where is no pkts to forward for 20 secs.
    SMMIRROR_FORWARDING     = 1,        // Forwarding pkts from caller to callee, and vice versa.

    // 1) to send CMD_HEARTBEAT_STAT every 2,2,2...secs.
    // 2) becomes SMMIRROR_STANDBY when CMD_HEARTBEAT_STATACK received.
    // 3) becomes SMMIRROR_STANDBY after sending CMD_HEARTBEAT_STAT 10 times.
    SMMIRROR_INFORMSTATE    = 2,        // Informing state to server.
};

enum eRegInfoGetStateMachine{
    // 1) becomes REGINFOGET_FETCHING when getRegInfo() called
    SMREGINFOGET_IDLE         = 0,        // Idle
    // 1) to send CMD_REGINFOGET_REQ every 1,1,1,2,2,2... secs.
    // 2) becomes REGINFOGET_NOTIFYAPP when CMD_REGINFOGET_ACK received.
    // 3) becomes REGINFOGET_NOTIFYAPP after sending CMD_REGINFOGET_REQ 5 tiems.
    SMREGINFOGET_FETCHING     = 1,        // Fetching reg info
    // 1) call OnGetRegInfoAck() to notify app the result
    // 2) becomes REGINFOGET_IDLE immediately
    SMREGINFOGET_NOTIFIED     = 2,        // Notify app the result
};

enum eRegInfoGetByMobileStateMachine{
    // 1) becomes REGINFOGETBYMOBILE_FETCHING when getRegInfoByMobile() called
    SMREGINFOGETBYMOBILE_IDLE         = 0,        // Idle
    // 1) to send CMD_REGINFOGETBYMOBILE_REQ every 1,1,1,2,2,2... secs.
    // 2) becomes REGINFOGETBYMOBILE_NOTIFYAPP when CMD_REGINFOGETBYMOBILE_ACK received.
    // 3) becomes REGINFOGETBYMOBILE_NOTIFYAPP after sending CMD_REGINFOGETBYMOBILE_REQ 5 tiems.
    SMREGINFOGETBYMOBILE_FETCHING     = 1,        // Fetching reg info
    // 1) call OnGetRegInfoAckByMobile() to notify app the result
    // 2) becomes REGINFOGETBYMOBILE_IDLE immediately
    SMREGINFOGETBYMOBILE_NOTIFIED     = 2,        // Notify app the result
};

enum eBindStateMachine{
    SMBIND_STANDBY          = 0,        //

};

enum eBDEBUGLEVEL{
    LV_CLOSE    = 0,                    // Display nothing
    LV_CRITICAL = 1,                    // Display critical messages
    LV_NORMAL   = 2,                    // Display critical & normal messages
    LV_INFO     = 3,                    // Display critical, normal & info messages
    LV_MEMORY   = 4,                    // Display critical, normal, info & pkt memory messages
};

//---------------------------- cmd/key string -------------------------------------
typedef struct tagCMDKEY_STRING
{
    const char *str;
    unsigned int v;
    const char *rem;
}CMDKEY_STRING;
static CMDKEY_STRING cmd_string_array[]={
    #include "protocol.cmd.txt"
};
static CMDKEY_STRING key_string_array[]={
    #include "protocol.key.txt"
};
static CMDKEY_STRING sm_string_array[]={
    #include "statemachine.txt"
};
static CMDKEY_STRING smgetreginfo_string_array[]={
    #include "statemachinegetreginfo.txt"
};
static CMDKEY_STRING smgetreginfobymobile_string_array[]={
    #include "statemachinegetreginfobymobile.txt"
};
static CMDKEY_STRING smmirror_string_array[]={
    #include "statemachinemirror.txt"
};
static CMDKEY_STRING smcall_string_array[]={
    #include "statemachinecall.txt"
};
const char* get_cmd_str(unsigned int v);
const char* get_cmd_rem(unsigned int v);
const char* get_key_str(unsigned int v);
const char* get_key_rem(unsigned int v);
const char* get_sm_str(unsigned int v);
const char* get_smreginfoget_str(unsigned int v);
const char* get_smreginfogetbymobile_str(unsigned int v);
const char* get_smmirror_str(unsigned int v);
const char* get_smcall_str(unsigned int v);



///*.........................................................................................*/
#ifndef __BDEBUGWHERE_BDEBUGINFO__
#define __BDEBUGWHERE_BDEBUGINFO__
//begin  broadcasting debugging macro.
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/file.h>
//#include <sys/vfs.h>
#include <sys/stat.h>
#include <ctype.h>

#include <pthread.h>

#ifndef TESTWITHGCC
//#include <JNIHelp.h>
//#include <jni.h>
//#include <android/log.h>
#endif

#if 1
static char szFilling[48];
#   define BDEBUG_ON
#ifdef TESTWITHGCC
#   ifdef BDEBUGLEVEL
#       define BDEBUGINFO(lv, fmt,args...) if (lv!=0 && BDEBUGLEVEL!=0 && BDEBUGLEVEL>=lv) broadcast_debug(__FUNCTION__,__LINE__,__FILE__,fmt,##args)
#	    define BDEBUGWHERE if (BDEBUGLEVEL>=3) broadcast_debug(__FUNCTION__,__LINE__,__FILE__,"")
#	    define BDEBUG BDEBUGINFO
#	    define BDEBUGINFO_ARGV(lv, argc,argv) if (lv!=0 && BDEBUGLEVEL!=0 && BDEBUGLEVEL>=lv) broadcast_debug_showargv(__FUNCTION__,__LINE__,__FILE__,argc,argv)
#	    define BDEBUGBT broadcast_bt(__FUNCTION__,__LINE__,__FILE__,"")
#	    define BDEBUGMEM(lv, addr,varname) if (lv!=0 && BDEBUGLEVEL!=0 && BDEBUGLEVEL>=lv) broadcast_mem(__FUNCTION__,__LINE__,__FILE__,addr,varname)
#   else
#   	define BDEBUGINFO
#	    define BDEBUGWHERE
#	    define BDEBUG
#	    define BDEBUGINFO_ARGV
#       define BDEBUGBT
#       define BDEBUGMEM
#   endif
#else
/*old
 #   define BDEBUGINFO(lv, format, ...) LOGD("[COM] "format"[L:%d, F:%s]", ##__VA_ARGS__, __LINE__ , __FUNCTION__);
 */
//#   define BDEBUGINFO(lv, format, ...) LOGD("[COM] " format"[L:%d, F:%s]", ##__VA_ARGS__, __LINE__ , __FUNCTION__);
//#   define BDEBUGINFO(lv, format, ...) printf("L:%d, F:%s, I:%", __LINE__ , __FUNCTION__, ##__VA_ARGS__)
/*Bob*/
//#   define BDEBUGINFO(lv, format, arg...) printf(format, ##arg)
#   define BDEBUGINFO(lv, format, arg...) {}

#   define BDEBUGMEM
#endif
#	define BDEBUG BDEBUGINFO
#else
#	define BDEBUGWHERE
#	define BDEBUGINFO
#	define BDEBUG
#	define BDEBUGINFO_ARGV
#	define BDEBUGBT
#	define BDEBUGMEM
#endif


#ifdef BDEBUG_ON
char* repstr(int len, char cFill);
void broadcast_debug (const char * funcname, const int line, const char * filename, const char* va, ...);
void broadcast_mem (const char * funcname, const int line, const char * filename, unsigned long addr, char* varname);
void broadcast_bt (const char * funcname, const int line, const char * filename);
void broadcast_debug_showargv(const char * funcname, const int line, const char * filename, int argc, char** argv);
#endif
//end broadcasting debugging macro.
#endif //__BDEBUGWHERE_BDEBUGINFO__
///*..........................................................................................*/


//---------------------------- retry controller -------------------------------------
//static unsigned int retry_interval_array0[6]={1,1,1,1,1,1};
//static unsigned int retry_interval_array0[6]={16,16,16,16,16,16};
static unsigned int retry_interval_array0[6]={8,8,8,8,8,8};
static unsigned int retry_interval_array1[6]={0,4,4,4,4,4};
//static unsigned int retry_interval_array2[6]={0,2,4,4,8,8};
static unsigned int retry_interval_array2[6]={0,1,1,1,1,1};
static unsigned int retry_interval_array3[6]={10,10,10,10,10,10};
static unsigned int retry_interval_array4[6]={0,2,2,4,4,4};
typedef struct tagRETRY_CTRL
{
    char* gettag(){return usertag;}
    int getmax(){return retrymax;}
    void reset(unsigned int max=0, char* tag=0){count=0;ftime(&last_time);retrymax=max;usertag=tag;}
    void refreshTime(){ftime(&last_time);}
    void inc(){++count;}
    bool isTimeout(){
        if (retrymax>0 && count>retrymax)
            return true;
        else
            return false;
    }
    bool isNextStep(){
        struct timeb t;
        ftime(&t);
        if ((t.time+t.millitm/1000 - (last_time.time+last_time.millitm/1000)) >= getInterval(count)) {
            BDEBUG(LV_INFO,"{array}%d secs elapsed, count: %d, t.time:%d \n",getInterval(count), count, t.time);
            inc();
            refreshTime();
            return true;
        }
        else
            return false;
    }
    unsigned int getInterval(unsigned n){
        if (n>=arrayLength)
            return array[arrayLength-1];
        else
            return array[n];
    }
    struct timeb last_time;
    unsigned int count;
    unsigned int retrymax;			// 0 : infinite
    char* usertag;                  // app's info passed by interface function and may be used in timer.
    tagRETRY_CTRL(unsigned int* a, int count){array=a;arrayLength=count; reset(0,0);}
    ~tagRETRY_CTRL(){if(usertag){free(usertag); usertag=0;}}
    unsigned int *array;
    unsigned int arrayLength;
}RETRY_CTRL;
#if 0
typedef struct tagRETRY_CTRL0
{
    char* gettag(){return usertag;}
    int getmax(){return retrymax;}
    void reset(unsigned int max=0, char* tag=0){count=0;ftime(&last_time);retrymax=max;usertag=tag;}
    void refreshTime(){ftime(&last_time);}
    void inc(){++count;}
    bool isTimeout(){
        if (retrymax>0 && count>retrymax)
            return true;
        else
            return false;
    }
    bool isNextStep(){
        struct timeb t;
        ftime(&t);
        if ((t.time - last_time.time) >= getInterval(count)) {
            BDEBUG(LV_INFO,"{array0}%d secs elapsed, count: %d, t.time:%d \n",getInterval(count), count, t.time);
            inc();
            refreshTime();
            return true;
        }
        else
            return false;
    }
    unsigned int getInterval(unsigned n){
        unsigned int m = sizeof(retry_interval_array0)/sizeof(retry_interval_array0[0]);
        if (n>=m)
            return retry_interval_array0[m-1];
        else
            return retry_interval_array0[n];
    }
    struct timeb last_time;
    unsigned int count;
    unsigned int retrymax;			// 0 : infinite
    char* usertag;                  // app's info passed by interface function and may be used in timer.
    tagRETRY_CTRL0(){reset(0,0);}
    ~tagRETRY_CTRL0(){if(usertag){free(usertag); usertag=0;}}
}RETRY_CTRL0;
typedef struct tagRETRY_CTRL1
{
    char* gettag(){return usertag;}
    int getmax(){return retrymax;}
    void reset(unsigned int max=0, char* tag=0){count=0;ftime(&last_time);retrymax=max;usertag=tag;}
    void refreshTime(){ftime(&last_time);}
    void inc(){++count;}
    bool isTimeout(){
        if (retrymax>0 && count>retrymax)
            return true;
        else
            return false;
    }
    bool isNextStep(){
        struct timeb t;
        ftime(&t);
        if ((t.time - last_time.time) >= getInterval(count)) {
            BDEBUG(LV_INFO, "{array1}%d secs elapsed, count: %d, t.time:%d \n",getInterval(count), count, t.time);
            inc();
            refreshTime();
            return true;
        }
        else
            return false;
    }
    unsigned int getInterval(unsigned n){
        unsigned int m = sizeof(retry_interval_array1)/sizeof(retry_interval_array1[0]);
        if (n>=m)
            return retry_interval_array1[m-1];
        else
            return retry_interval_array1[n];
    }
    struct timeb last_time;
    unsigned int count;
    unsigned int retrymax;			// 0 : infinite
    char* usertag;                  // app's info passed by interface function and may be used in timer.
    tagRETRY_CTRL1(){reset(0,0);}
    ~tagRETRY_CTRL1(){if(usertag){free(usertag); usertag=0;}}
}RETRY_CTRL1;
typedef struct tagRETRY_CTRL2
{
    char* gettag(){return usertag;}
    int getmax(){return retrymax;}
    void reset(unsigned int max=0, char* tag=0){count=0;ftime(&last_time);retrymax=max;usertag=tag;}
    void refreshTime(){ftime(&last_time);}
    void inc(){++count;}
    bool isTimeout(){
        if (retrymax>0 && count>retrymax)
            return true;
        else
            return false;
    }
    bool isNextStep(){
        struct timeb t;
        ftime(&t);
        if ((t.time - last_time.time) >= getInterval(count)) {
            BDEBUG(LV_INFO, "{array2}%d secs elapsed, count: %d, t.time:%d \n",getInterval(count), count, t.time);
            inc();
            refreshTime();
            return true;
        }
        else
            return false;
    }
    unsigned int getInterval(unsigned n){
        unsigned int m = sizeof(retry_interval_array2)/sizeof(retry_interval_array2[0]);
        if (n>=m)
            return retry_interval_array2[m-1];
        else
            return retry_interval_array2[n];
    }
    struct timeb last_time;
    unsigned int count;
    unsigned int retrymax;			// 0 : infinite
    char* usertag;                  // app's info passed by interface function and may be used in timer.
    tagRETRY_CTRL2(){reset(0,0);}
    ~tagRETRY_CTRL2(){if(usertag){free(usertag); usertag=0;}}
}RETRY_CTRL2;
typedef struct tagRETRY_CTRL3
{
    char* gettag(){return usertag;}
    int getmax(){return retrymax;}
    void reset(unsigned int max=0, char* tag=0){count=0;ftime(&last_time);retrymax=max;usertag=tag;}
    void refreshTime(){ftime(&last_time);}
    void inc(){++count;}
    bool isTimeout(){
        if (retrymax>0 && count>retrymax)
            return true;
        else
            return false;
    }
    bool isNextStep(){
        struct timeb t;
        ftime(&t);
        if ((t.time - last_time.time) >= getInterval(count)) {
            BDEBUG(LV_INFO, "{array3}%d secs elapsed, count: %d, t.time:%d \n",getInterval(count), count, t.time);
            inc();
            refreshTime();
            return true;
        }
        else
            return false;
    }
    unsigned int getInterval(unsigned n){
        unsigned int m = sizeof(retry_interval_array3)/sizeof(retry_interval_array3[0]);
        if (n>=m)
            return retry_interval_array3[m-1];
        else
            return retry_interval_array3[n];
    }
    struct timeb last_time;
    unsigned int count;
    unsigned int retrymax;			// 0 : infinite
    char* usertag;                  // app's info passed by interface function and may be used in timer.
    tagRETRY_CTRL3(){reset(0,0);}
    ~tagRETRY_CTRL3(){if(usertag){free(usertag); usertag=0;}}
}RETRY_CTRL3;


typedef struct tagRETRY_CTRL4
{
    char* gettag(){return usertag;}
    int getmax(){return retrymax;}
    void reset(unsigned int max=0, char* tag=0){count=0;ftime(&last_time);retrymax=max;usertag=tag;}
    void refreshTime(){ftime(&last_time);}
    void inc(){++count;}
    bool isTimeout(){
        if (retrymax>0 && count>retrymax)
            return true;
        else
            return false;
    }
    bool isNextStep(){
        struct timeb t;
        ftime(&t);
        if ((t.time - last_time.time) >= getInterval(count)) {
            BDEBUG(LV_INFO, "{array4}%d secs elapsed, count: %d, t.time:%d \n",getInterval(count), count, t.time);
            inc();
            refreshTime();
            return true;
        }
        else
            return false;
    }
    unsigned int getInterval(unsigned n){
        unsigned int m = sizeof(retry_interval_array4)/sizeof(retry_interval_array4[0]);
        if (n>=m)
            return retry_interval_array4[m-1];
        else
            return retry_interval_array4[n];
    }
    struct timeb last_time;
    unsigned int count;
    unsigned int retrymax;			// 0 : infinite
    char* usertag;                  // app's info passed by interface function and may be used in timer.
    tagRETRY_CTRL4(){reset(0,0);}
    ~tagRETRY_CTRL4(){if(usertag){free(usertag); usertag=0;}}
}RETRY_CTRL4;
#endif

#endif // CLI_TESTSAMPLE_PUBLIC_H
