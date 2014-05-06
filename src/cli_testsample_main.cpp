/**
 * @file cli_testsample_main.cpp
 * @brief this program is used to test F2F network protocol
 * @author fengyh@mele.cn
 * @version 0.9
 * @date 2012-06-13
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h> 
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include "cli_testsample_public.h"

/** ptotocol version */
#define F2FPROTOCOL_CODE 'SF2F'
#define F2FPROTOCOL_VERSTION 0x0100

/** server name and port */
#define F2FSVR_LB_UDPPORT 60001
#ifdef USING_LOCALHOST
#define F2FSVR_LB_DOMAIN "localhost"
#else
#define F2FSVR_LB_DOMAIN "f2flb.wwnav.com"
//#define F2FSVR_LB_DOMAIN "f2flb.stvbox.com"
#endif

F2FCBFUNCTIONS* cbfuncs=0;
pthread_mutex_t mutex_send=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_log=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_callstate=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_kv=PTHREAD_MUTEX_INITIALIZER;

/* if callback available */
bool bCanCallback = false;
bool bOnReadyCalled = false;

/** firewall and nat flag */
bool bTerminalHas_firewall = true;
bool bTerminal_isCone = false;
unsigned int session_id = 0;
char *terminal_id=0;
char *terminal_passwd=0;
char *terminal_lanip=0;
unsigned short terminal_lanport=0;
char terminal_wanip[32]={0};
unsigned short terminal_wanport=0;
char session_type[2]={0};



/** otherterminal info */
char *otherterminal_info_id=0;
char *otherterminal_info_ip=0;
unsigned short otherterminal_info_port=0;
char* otherterminal_info_lip=0;
unsigned short otherterminal_info_lport=0;
bool otherterminal_info_mirrorused = false;
char *otherterminal_info_mirroraddr = 0;

/** mirror info */
char *mirror_fwd_callerid=0;
char *mirror_fwd_callerip=0;
unsigned short mirror_fwd_callerport=0;
char *mirror_fwd_calleeid=0;
char *mirror_fwd_calleeip=0;
unsigned short mirror_fwd_calleeport=0;

/** callee id to test call */
char *test_calleeid = 0;

/** nat server addr */
char natdetectserver_ip[32]={0};
unsigned short natdetectserver_port=0;

/** loginserver addr */
char loginserver_ip[32]={0};
unsigned short loginserver_port=0;

/** udp socket */
int udp_sock=0;

/** local address */
struct sockaddr_in local_addr;

/** login_server address */
struct sockaddr_in login_svr_addr;

eStateMachine state_machine = SM_BOOT;
eMirrorStateMachine state_machine_mirror = SMMIRROR_STANDBY;
eCallStateMachine state_machine_call = SMCALL_IDLE;
eRegInfoGetStateMachine state_machine_reginfoget = SMREGINFOGET_IDLE;
eRegInfoGetByMobileStateMachine state_machine_reginfogetbymobile = SMREGINFOGETBYMOBILE_IDLE;

//RETRY_CTRL0 retry_ctrl_heatbeat;
//RETRY_CTRL1 retry_ctrl_222222;
//RETRY_CTRL1 retry_addresing_req;
//RETRY_CTRL1 retry_calling_req;
//RETRY_CTRL1 retry_fwtraversing;
//RETRY_CTRL1 retry_ringOrRingback;
//RETRY_CTRL2 retry_ctrl_224488;
//RETRY_CTRL3 retry_calleeaudio_timeout;
//RETRY_CTRL3 retry_calleraudio_timeout;
//RETRY_CTRL3 retry_mirrorfwding_timeout;
//RETRY_CTRL4 retry_reginfoget_timeout;
//RETRY_CTRL4 retry_reginfogetbymobile_timeout;
RETRY_CTRL retry_ctrl_heatbeat(retry_interval_array0, sizeof(retry_interval_array0)/sizeof(retry_interval_array0[0]));
RETRY_CTRL retry_ctrl_222222(retry_interval_array1, sizeof(retry_interval_array1)/sizeof(retry_interval_array1[0]));
RETRY_CTRL retry_addresing_req(retry_interval_array1, sizeof(retry_interval_array1)/sizeof(retry_interval_array1[0]));
RETRY_CTRL retry_calling_req(retry_interval_array1, sizeof(retry_interval_array1)/sizeof(retry_interval_array1[0]));
RETRY_CTRL retry_fwtraversing(retry_interval_array1, sizeof(retry_interval_array1)/sizeof(retry_interval_array1[0]));
RETRY_CTRL retry_ringOrRingback(retry_interval_array1, sizeof(retry_interval_array1)/sizeof(retry_interval_array1[0]));
RETRY_CTRL retry_ctrl_224488(retry_interval_array2, sizeof(retry_interval_array2)/sizeof(retry_interval_array2[0]));
RETRY_CTRL retry_calleeaudio_timeout(retry_interval_array3, sizeof(retry_interval_array3)/sizeof(retry_interval_array3[0]));
RETRY_CTRL retry_calleraudio_timeout(retry_interval_array3, sizeof(retry_interval_array3)/sizeof(retry_interval_array3[0]));
RETRY_CTRL retry_mirrorfwding_timeout(retry_interval_array3, sizeof(retry_interval_array3)/sizeof(retry_interval_array3[0]));
RETRY_CTRL retry_reginfoget_timeout(retry_interval_array4, sizeof(retry_interval_array4)/sizeof(retry_interval_array4[0]));
RETRY_CTRL retry_reginfogetbymobile_timeout(retry_interval_array4, sizeof(retry_interval_array4)/sizeof(retry_interval_array4[0]));

/** i/o buffer, one package should not exeed 1024 bytes. */
char recv_data[2048];					//recv_thread use it to receive data
char send_data[2048];					//timer_thread use it to send data
char send_data_threadudp[2048];			//used by recv_thread also
char send_data_codec[2048];				//codec use to send data


/** quiting */
bool quiting = false;
pthread_t thread_id = 0;
int count_sending_hangup = 0;

/** key-value pair */
#define KV_MAX 32
typedef struct _tagKEYVALUE
{
	int key;
	char *value;
    int len;
}KEYVALUE;
KEYVALUE* kv_pair[KV_MAX];

/** list used to store status notify info */
typedef struct _tagF2fList
{
    char* ptr[256];
    int nHead;
    int nTail;
    _tagF2fList(){
        nHead=nTail=0;
    }
    bool isEmpty(){
        return nTail==nHead;
    }
    bool isFull(){
        return ((nTail + 1) % (sizeof(ptr)/sizeof(*ptr)) == nHead);
    }
    void push_back(char* p){
        if (isFull()){
            free(p);
            return;
        }
        ptr[nTail++] = p;
        nTail = nTail % (sizeof(ptr)/sizeof(*ptr));
        //printf("************nHead: %d, nTail:%d*******************\n", nHead, nTail);
    }
    void pop_front(){
       if (isEmpty())
           return;
       free(ptr[nHead++]);
       nHead = nHead % (sizeof(ptr)/sizeof(*ptr));
       //printf("************nHead: %d, nTail:%d*******************\n", nHead, nTail);

    }
    char* front(){
       if (isEmpty()){
           return 0;
       }
       else {
           return ptr[nHead];
       }
    }
}F2FLIST;

F2FLIST list_logtoui;

/** pre definition */
char*getErrorMsg(F2FPKG* recv_data);
void getDataFromPkt(F2FPKG* pkg, int& data, int& len);
void handle_timer();
void handle_mirror_timer();
void handle_call_timer();
void handle_reginfo_get_timer();
void handle_reginfo_get_bymobile_timer();
void init_daemon(void);
void init_udp(unsigned short port);
bool isPktFromMirror(char* sourceIp, char* mirrorIps);
void kv_clear();
void kv_build(F2FPKG* pkg);
void kv_build_nodata(F2FPKG* pkg, int& data, int& len);
int  kv_empty_idx();
void kv_put(KEYVALUE* kv);
void onalterRegInfoAck(F2FPKG* recv_data);
void onCallAddressingAck(F2FPKG* recv_data);
void onCallCalleeinform(F2FPKG*recv_data);
void onCallFwTraversing(F2FPKG*recv_data, struct sockaddr_in* remoteaddr);
void onCallCallingReq(F2FPKG*recv_data, struct sockaddr_in* remoteaddr);
void onCallCallingAck(F2FPKG*recv_data);
void onCallRing(F2FPKG*recv_data, struct sockaddr_in* remoteaddr);
void onCallRingback(F2FPKG*recv_data, struct sockaddr_in* remoteaddr);
void onCallHangup(F2FPKG*recv_data);
void onCallerMedia(F2FPKG*recv_data);
void onCalleeMedia(F2FPKG*recv_data);
void onDataRead(char* recv_data, int bytes_read, struct sockaddr_in* remoteaddr);
void onHeartBeatAck(F2FPKG* recv_data);
void onLBLoginserverAck(F2FPKG* recv_data);
void onLoginAck(F2FPKG* recv_data);
void onMirrorinform(F2FPKG*recv_data);
void onNatDetectAck(F2FPKG* recv_data);
void onRegInfoGetAck(F2FPKG* recv_data);
void onRegInfoGetByMobileAck(F2FPKG* recv_data);
void pkg_init(F2FPKG* pkg, int cmd, int session);
void pkg_append(F2FPKG* pkg, int key, char* value);
int  pkg_len(F2FPKG* pkg);
void*recv_thread(void* ctx);
void sigroutine(int dunno);
void show_package(F2FPKG *pkg, int line);
void send_udp_pkg(char* ipOrDomain, unsigned short port, char *send_data, int len);
void send_udp_pkg_loginserver(char *send_data, int len);
void setStateMachine(eStateMachine e, const char* msg);
void setStateMachineCall(eCallStateMachine e, const char* msg, const char* func, int line);
void setStateMachineMirror(eMirrorStateMachine e, const char* msg);
void setStateMachineReginfoGet(eRegInfoGetStateMachine e, const char* msg, const char* func, int line);
void setStateMachineReginfoGetByMobile(eRegInfoGetByMobileStateMachine e, const char* msg, const char* func, int line);
void startCall_inner(char* callerid, char* calleeid, bool bCalledByTimer=false);
void sendCallingReq();
void sendFwTraversing();
void sendHangup();
void sendRing();
void sendRingback();
void sendCalleeAudio();
void sendCallerAudio();
void statusNotify(char* str);
void*timer_thread(void* ctx);
void uninit_udp();

void get_mac()
{
#if 1
    /*mark here for Darwin System Bob*/
//    struct ifreq tmp;
//    int sock_mac;
//    char mac_addr[30];
//    sock_mac = socket(AF_INET, SOCK_STREAM, 0);
//    if ( sock_mac == -1){
//        BDEBUG(LV_CRITICAL,"create socket fail\n");
//        return;
//    }
//    memset(&tmp,0,sizeof(tmp));
//    strncpy(tmp.ifr_name,"eth0",sizeof(tmp.ifr_name)-1 );
//    if ( (ioctl( sock_mac, SIOCGIFHWADDR, &tmp)) < 0 ){
//        BDEBUG(LV_CRITICAL,"eth0 mac ioctl error\n");
//
//        memset(&tmp,0,sizeof(tmp));
//        strncpy(tmp.ifr_name,"wlan0",sizeof(tmp.ifr_name)-1 );
//        if ( (ioctl( sock_mac, SIOCGIFHWADDR, &tmp)) < 0 ){
//            BDEBUG(LV_CRITICAL,"wlan0 mac ioctl error\n");
//            return;
//        }
//    }
//    sprintf(mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X",
//            (unsigned char)tmp.ifr_hwaddr.sa_data[0],
//            (unsigned char)tmp.ifr_hwaddr.sa_data[1],
//            (unsigned char)tmp.ifr_hwaddr.sa_data[2],
//            (unsigned char)tmp.ifr_hwaddr.sa_data[3],
//            (unsigned char)tmp.ifr_hwaddr.sa_data[4],
//            (unsigned char)tmp.ifr_hwaddr.sa_data[5]
//           );
//    BDEBUG(LV_NORMAL, "local mac:%s\n", mac_addr);
//    close(sock_mac);
    char mac_addr[30]="0A:0A:0A:0A:0A:0A";
    memcpy(terminal_id,mac_addr,strlen(mac_addr));

#else 
    FILE *fp;
    size_t len = 0;
    char *cmd="busybox ifconfig | grep HWaddr";
    char *line = NULL;

    if ( (fp=popen(cmd,"r"))==NULL){
        perror("popen");
        return;
    }
    if (getline(&line, &len, fp) != -1){
        if (*(line+strlen(line)-1)=='\n')
            *(line+strlen(line)-1) = 0;
        if (*(line+strlen(line)-1)=='\r')
            *(line+strlen(line)-1) = 0;
        if (*(line+strlen(line)-1)=='\n')
            *(line+strlen(line)-1) = 0;
        if (strlen(line)>19){
			if (terminal_id){free(terminal_id); terminal_id=0;}
			terminal_id = strdup(line + (strlen(line)-19));
            if (strlen(terminal_id)>0){
                for (int i=strlen(terminal_id)-1; i>=0; i--){
                    if (' '==terminal_id[i])
                        terminal_id[i] = 0;
                }
            }
            BDEBUG(LV_NORMAL, "get mac: \"%s\"", terminal_id);
            BDEBUGMEM((long unsigned int)terminal_id, "terminal_id");
        }
    }
    if (line)
        free(line);
    pclose(fp);
#endif
}

/** clear the kv pair array */
void kv_clear()
{
    pthread_mutex_lock(&mutex_kv);
    int i;
	for (i=0; i<KV_MAX; i++) {
        if (kv_pair[i]) {
            if (kv_pair[i]->value){
                free(kv_pair[i]->value);
                kv_pair[i]->value = 0;
            }
			free(kv_pair[i]);
            kv_pair[i] = 0;
		}
	}
    pthread_mutex_unlock(&mutex_kv);
}

int kv_empty_idx()
{
    int i=0;
	for (i=0; i<KV_MAX; i++) {
		if (0==kv_pair[i]) {
			break;
		}
	}
	return i;
}
void kv_put(KEYVALUE* kv)
{
    int idx = kv_empty_idx();
	if (idx<KV_MAX)
		kv_pair[idx] = kv;
}



void kv_build(F2FPKG* pkg)
{
    kv_clear();
    pthread_mutex_lock(&mutex_kv);
    F2FPKG_HEAD *hd = (F2FPKG_HEAD *)pkg;

	int flag=0;
	unsigned char info[1024];
	unsigned int key=0;
	unsigned char* q = info;
	unsigned char* p = pkg->payload;
	int i;
    for (i=0; (i < hd->payloadLen) && (p - pkg->payload < hd->payloadLen); i++) {
		if (*p == 0xA8 && *(p+1) == 0xF0){
			p += 2;
			if (flag==0){
				/** get key */
				key = atoi((const char*)info);
			}
			else {
				/** get value */
				KEYVALUE * kv = (KEYVALUE *)malloc(sizeof(KEYVALUE));
				kv->key = key;
				kv->value = strdup((const char*)info);
                kv->len = p - pkg->payload;
                kv->len = hd->payloadLen - kv->len;
				kv_put(kv);
			}
			q = info;
            *q = 0;
            flag ++;
			flag %= 2;
		}
		else {
			*q++ = *p++;
			*q = 0;
		}
	}
    pthread_mutex_unlock(&mutex_kv);
}

void kv_build_nodata(F2FPKG* pkg, int &data, int &len)
{
    kv_clear();
    pthread_mutex_lock(&mutex_kv);
    F2FPKG_HEAD *hd = (F2FPKG_HEAD *)pkg;

    int flag=0;
    unsigned char info[1024];
    unsigned int key=0;
    unsigned char* q = info;
    unsigned char* p = pkg->payload;
    unsigned int dataoffset=0;
    unsigned int ntmp = 0;
    int i;
    for (i=0; (i < hd->payloadLen) && (p - pkg->payload < hd->payloadLen) ; i++) {
        if (*p == 0xA8 && *(p+1) == 0xF0){
            p += 2;
            ntmp += 2;
            if (flag==0){
                /** get key */
                key = atoi((const char*)info);
                if (key==KEY_DATA){
                    dataoffset += ntmp;
                    data =(unsigned long) ((char*)pkg + sizeof(F2FPKG_HEAD) + dataoffset);
                    len = hd->payloadLen - dataoffset;
                    // we don't need key-value for data here.
                    break;
                }
            }
            else {
                /** get value */
                dataoffset += ntmp;
                ntmp = 0;
                KEYVALUE * kv = (KEYVALUE *)malloc(sizeof(KEYVALUE));
                kv->key = key;
                kv->value = strdup((const char*)info);
                kv->len = p - pkg->payload;
                kv->len = hd->payloadLen - kv->len;
                kv_put(kv);
            }
            q = info;
            *q = 0;
            flag ++;
            flag %= 2;
        }
        else {
            *q++ = *p++; ntmp++;
            *q = 0;
        }
    }
    pthread_mutex_unlock(&mutex_kv);

}

void setStateMachine(eStateMachine e, const char* msg)
{
    char szmsg[512];
    sprintf(szmsg,"SM:[(%d)%s--->(%d)%s]:%s", state_machine, get_sm_str(state_machine), e, get_sm_str(e), msg);
    BDEBUG(LV_NORMAL,"%s",szmsg);
    state_machine = e;
    statusNotify(szmsg);
}

void setStateMachineReginfoGet(eRegInfoGetStateMachine e, const char* msg, const char* func, int line)
{
    char szmsg[512];
    sprintf(szmsg,"SMREGINFOGET:[(%d)%s--->(%d)%s]:%s [%d@%s]", state_machine_reginfoget,
            get_smreginfoget_str(state_machine_reginfoget), e, get_smreginfoget_str(e), msg, line, func);
    BDEBUG(LV_NORMAL,"%s",szmsg);
    state_machine_reginfoget = e;

}

void setStateMachineReginfoGetByMobile(eRegInfoGetByMobileStateMachine e, const char* msg, const char* func, int line)
{
    char szmsg[512];
    sprintf(szmsg,"SMREGINFOGET:[(%d)%s--->(%d)%s]:%s [%d@%s]", state_machine_reginfogetbymobile,
            get_smreginfogetbymobile_str(state_machine_reginfogetbymobile), e, get_smreginfogetbymobile_str(e), msg, line, func);
    BDEBUG(LV_NORMAL,"%s",szmsg);
    state_machine_reginfogetbymobile = e;
}

void setStateMachineMirror(eMirrorStateMachine e, const char* msg)
{
    char szmsg[512];
    sprintf(szmsg,"SMMIRROR:[(%d)%s--->(%d)%s]:%s", state_machine_mirror, get_smmirror_str(state_machine_mirror), e, get_smmirror_str(e), msg);
    BDEBUG(LV_NORMAL,"%s",szmsg);
    state_machine_mirror = e;
    statusNotify(szmsg);
}

/*bob add default break to remove warnings.*/
void setStateMachineCall(eCallStateMachine e, const char* msg, const char *func, int line)
{
    pthread_mutex_lock(&mutex_callstate);
    //状态是否允许跳转
    bool bEnableChange = false;
    switch (state_machine_call){
    case SMCALL_IDLE:/*空闲状态*/
        switch(e){
        case SMCALL_ADDRESSINGREQ:     //呼叫请求
        case SMCALL_CALLEEINFORM_LAN:  //被叫通知，尝试局域网
        case SMCALL_CALLEEINFORM:      //被叫通知
            bEnableChange = true;
            break;
        default:
            break;
        }
        break;
    case SMCALL_ADDRESSINGREQ:
        switch(e){
        case SMCALL_CALLINGREQ_LAN:
        case SMCALL_CALLINGREQ:
        case SMCALL_LINEBUSY:
        case SMCALL_HANGUP:
        case SMCALL_OTHERSIDE_HANGUP:
            bEnableChange = true;
            break;
            default:
                break;
        }
        break;
    case SMCALL_CALLEEINFORM_LAN:
        switch(e){
        case SMCALL_CALLEEINFORM:
        case SMCALL_HANGUP:
        case SMCALL_OTHERSIDE_HANGUP:
        case SMCALL_RING:
            bEnableChange = true;
            break;
            default:
                break;
        }
        break;
    case SMCALL_CALLEEINFORM:
        switch(e){
        case SMCALL_IDLE:
        case SMCALL_HANGUP:
        case SMCALL_OTHERSIDE_HANGUP:
        case SMCALL_RING:
            bEnableChange = true;
            break;
            default:
                break;
        }
        break;
    case SMCALL_CALLINGREQ_LAN:
        switch(e){
        case SMCALL_CALLINGREQ:
        case SMCALL_CALLEENOANSWER:
        case SMCALL_RINGBACK:
        case SMCALL_HANGUP:
        case SMCALL_OTHERSIDE_HANGUP:
        case SMCALL_CALLERAUDIO:
            bEnableChange = true;
            break;
            default:
                break;
        }
        break;
    case SMCALL_CALLINGREQ:
        switch(e){
        case SMCALL_CALLEENOANSWER:
        case SMCALL_RINGBACK:
        case SMCALL_LINEBUSY:
        case SMCALL_HANGUP:
        case SMCALL_OTHERSIDE_HANGUP:
        case SMCALL_CALLERAUDIO:
            bEnableChange = true;
            break;
            default:
                break;
        }
        break;
    case SMCALL_RING:
        switch(e){
        case SMCALL_CALLEEAUDIO:
        case SMCALL_HANGUP:
        case SMCALL_OTHERSIDE_HANGUP:
            bEnableChange = true;
            break;
            default:
                break;
        }
        break;
    case SMCALL_RINGBACK:
        switch(e){
        case SMCALL_CALLERAUDIO:
        case SMCALL_CALLEENOANSWER:
        case SMCALL_HANGUP:
        case SMCALL_OTHERSIDE_HANGUP:
            bEnableChange = true;
            break;
            default:
                break;
        }
        break;
    case SMCALL_CALLEEAUDIO:
        switch(e){
        case SMCALL_HANGUP:
        case SMCALL_OTHERSIDE_HANGUP:
            bEnableChange = true;
            break;
            default:
                break;
        }
        break;
    case SMCALL_CALLERAUDIO:
        switch(e){
        case SMCALL_HANGUP:
        case SMCALL_OTHERSIDE_HANGUP:
            bEnableChange = true;
            break;
            default:
                break;
        }
        break;
    case SMCALL_CALLEENOANSWER:
        switch(e){
        case SMCALL_HANGUP:
        case SMCALL_OTHERSIDE_HANGUP:
            bEnableChange = true;
            break;
            default:
                break;
        }
        break;
    case SMCALL_LINEBUSY:
        switch(e){
        case SMCALL_HANGUP:
        case SMCALL_OTHERSIDE_HANGUP:
            bEnableChange = true;
            break;
            default:
                break;
        }
        break;
    case SMCALL_HANGUP:
        switch(e){
        case SMCALL_HANGUP_SENDING:
            bEnableChange = true;
            break;
            default:
                break;
        }
        break;
    case SMCALL_HANGUP_SENDING:
        switch(e){
        case SMCALL_IDLE:
            bEnableChange = true;
            break;
            default:
                break;
        }
        break;
    case SMCALL_OTHERSIDE_HANGUP:
    {
        switch(e){
        case SMCALL_IDLE:
            bEnableChange = true;
            break;
            default:
                break;
        }
        break;
        }

    default:
        break;
    }

    char szmsg[512];
    if (bEnableChange){
        sprintf(szmsg,"SMCALL:[(%d)%s--->(%d)%s]:%s [%d@%s]", state_machine_call, get_smcall_str(state_machine_call)
                , e, get_smcall_str(e), msg, line, func);
        state_machine_call = e;
    }
    else{
        sprintf(szmsg,"@Denied@ SMCALL:[(%d)%s--->(%d)%s]:%s [%d@%s]", state_machine_call, get_smcall_str(state_machine_call)
                , e, get_smcall_str(e), msg, line, func);
    }
    BDEBUG(LV_NORMAL, "%s",szmsg);
    statusNotify(szmsg);
    pthread_mutex_unlock(&mutex_callstate);
}

int alterRegInfo(char* nickname, char* mobileno)
{
	//只有在线才能更改注册信息
    if(SM_HEARTBEATING != state_machine) {
        BDEBUG(LV_CRITICAL,"Error: Cannot alter register infomation while terminal offline.");
        return -1;
    }
    F2FPKG *pkg = (F2FPKG*)send_data_codec;
    pkg_init(pkg, CMD_REGINFOALTER_REQ, session_id);
    pkg_append(pkg, KEY_ID, terminal_id);
    pkg_append(pkg, KEY_NICKNAME, (char*)nickname);
    if (mobileno){
        pkg_append(pkg, KEY_MOBILENO, (char*)mobileno);
    }
    send_udp_pkg_loginserver((char*)pkg, pkg_len(pkg));
    return 0;
}


int getRegInfo(char* mac)
{
    if (0==mac || *mac ==0)
        return -1;      // illegal paramater
    if (SM_HEARTBEATING!=state_machine){
        return -2;      // terminal is offline.
    }
    if (SMREGINFOGET_IDLE!=state_machine_reginfoget){
        return -3;      // statemachine does not permit this calling.
    }
    retry_reginfoget_timeout.reset(5,strdup(mac));
    setStateMachineReginfoGet(SMREGINFOGET_FETCHING,"getRegInfo() is called. And the timer is ticking.",__FUNCTION__,__LINE__);
    return 0;

}

int getRegInfoByMobile(char* mobileno)
{
    if (0==mobileno|| *mobileno ==0)
        return -1;      // illegal paramater
    if (SM_HEARTBEATING!=state_machine){
        return -2;      // terminal is offline.
    }
    if (SMREGINFOGETBYMOBILE_IDLE!=state_machine_reginfogetbymobile){
        return -3;      // statemachine does not permit this calling.
    }
    retry_reginfogetbymobile_timeout.reset(5,strdup(mobileno));
    setStateMachineReginfoGetByMobile(SMREGINFOGETBYMOBILE_FETCHING,"getRegInfoByMobile() is called. And the timer is ticking.",__FUNCTION__,__LINE__);
    return 0;
}


void init_udp(unsigned short port)
{

	/** create udp_sock */
	if ((udp_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("socket");
		exit(1);
	}
	int opt;
	setsockopt(udp_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);
	local_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(udp_sock,(struct sockaddr *)&local_addr, sizeof(struct sockaddr)) == -1)
	{
        perror("bind");
		exit(1);
	}

	/** create a thread to receive package from server */
	int ret = pthread_create(&thread_id, NULL, recv_thread, (void *)NULL);
    if (ret != 0) {
        perror("pthread_create");
        exit(1);
    }
    BDEBUG(LV_CRITICAL, "Thread 'recv_thread' starts...");

    if(fcntl(udp_sock, F_SETFL, O_NONBLOCK) == -1)
    {
        BDEBUG(LV_CRITICAL, "Error: %s", strerror(errno));
    }
#if 0
	signal(SIGKILL, sigroutine);
	signal(SIGINT, sigroutine);
	signal(SIGQUIT, sigroutine);
	signal(SIGHUP, sigroutine);
	signal(SIGTERM, sigroutine);
#endif
}

void uninit_udp()
{
	shutdown(udp_sock, SHUT_RDWR);
	close(udp_sock);
}


/* send pkt to ipOrDomain:port */
void send_udp_pkg(char* ipOrDomain, unsigned short port, char *send_data, int len)
{
    if (0==ipOrDomain || 0==send_data || len<=0)
        return;
    pthread_mutex_lock(&mutex_send);
    //--------------------------------------------------------
    F2FPKG_HEAD *hd = (F2FPKG_HEAD *)send_data;
#ifdef TESTWITHGCC
    //if ((CMD_CALL_CALLEEAUDIO!=hd->cmd) && (CMD_CALL_CALLERAUDIO!=hd->cmd))
        BDEBUG(LV_NORMAL, "  ====>  (%s) to %s:%d ", get_cmd_str(hd->cmd), ipOrDomain, port);
#endif
    //--------------------------------------------------------
    static int audioCount = 0;
    char sztmp[256];
    switch (hd->cmd){
    case CMD_CALL_CALLERAUDIO:
    case CMD_CALL_CALLEEAUDIO:
        audioCount ++;
        audioCount %= 100;
        if (0==audioCount){
            sprintf(sztmp,"        [%s]====>%s:%d, every 100 pkts being sent, it's shown once.", get_cmd_str(hd->cmd), ipOrDomain, port);
//            statusNotify(sztmp);
        }
        break;
    case CMD_HEARTBEAT_REQ:
        break;
    default:
        sprintf(sztmp,"        [%s]====>%s:%d", get_cmd_str(hd->cmd), ipOrDomain, port);
        statusNotify(sztmp);
        break;
    }

    struct hostent *host;
    host = (struct hostent*) gethostbyname(ipOrDomain);
    if (!host){
        BDEBUG(LV_CRITICAL,"error when call to gethostbyname(\"%s\")", ipOrDomain);
    }
    else {
        struct sockaddr_in dest_addr;
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(port);
        dest_addr.sin_addr = *((struct in_addr*)host->h_addr);
        sendto(udp_sock, send_data, len, 0, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));
    }
    pthread_mutex_unlock(&mutex_send);

}

/* send pkt to login server of the terminal*/
void send_udp_pkg_loginserver(char *send_data, int len)
{
    pthread_mutex_lock(&mutex_send);
    //--------------------------------------------------------
    F2FPKG_HEAD *hd = (F2FPKG_HEAD *)send_data;
    BDEBUG(LV_NORMAL, "  ====>loginserver (%s) to %s:%d ", get_cmd_str(hd->cmd), loginserver_ip, loginserver_port);
    //--------------------------------------------------------
    if ((CMD_CALL_CALLEEAUDIO!=hd->cmd) && (CMD_CALL_CALLERAUDIO!=hd->cmd)
            &&(CMD_HEARTBEAT_REQ!=hd->cmd)){
        char sztmp[256];
        sprintf(sztmp,"        [%s]====>%s:%d", get_cmd_str(hd->cmd), loginserver_ip, loginserver_port);
        statusNotify(sztmp);
        //list_logtoui.push_back(strdup(sztmp));
        //if (cbfuncs && cbfuncs->OnStatusNotify)
        //    cbfuncs->OnStatusNotify(strlen(sztmp), sztmp);
    }
    sendto(udp_sock, send_data, len, 0, (struct sockaddr *)&login_svr_addr, sizeof(struct sockaddr));
    pthread_mutex_unlock(&mutex_send);
}

void* recv_thread(void* ctx)
{

    int bytes_read;
    struct sockaddr_in saddr;
    //int len_addr = sizeof(struct sockaddr);
    socklen_t len_addr = sizeof(struct sockaddr);

    while (quiting == false)
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(udp_sock,&readfds);
        timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 1000;
        select(udp_sock+1,&readfds,NULL,NULL,&tv);
        bytes_read = 0;
        if(FD_ISSET(udp_sock,&readfds))
        {
            /* waiting for receive data */
            bytes_read = recvfrom(udp_sock,recv_data,2048,0,(struct sockaddr*)&saddr,&len_addr);
        }
        if (bytes_read<=0) {
            continue;
        }
        //------------------------------------------------
        F2FPKG_HEAD *hd = (F2FPKG_HEAD *)recv_data;
#ifdef TESTWITHGCC
        //if ((CMD_CALL_CALLEEAUDIO!=hd->cmd) && (CMD_CALL_CALLERAUDIO!=hd->cmd) )
            BDEBUG(LV_NORMAL, "  <==== (%s) form %s:%d ", get_cmd_str(hd->cmd), inet_ntoa(saddr.sin_addr), ntohs(saddr.sin_port));
#endif
        //------------------------------------------------
        
        recv_data[bytes_read] = '\0';
//printf("receive data=%s\n",recv_data);
        onDataRead(recv_data, bytes_read, &saddr);
    }
    BDEBUG(LV_CRITICAL, "Thread 'recv_thread' exit.");
    return NULL;
}


void onDataRead(char* recv_data, int bytes_read, struct sockaddr_in* remoteaddr)
{

    //ftime(&last_time_recv);
    show_package((F2FPKG*)recv_data, __LINE__);
    fflush(stdout);

    F2FPKG_HEAD* hd = (F2FPKG_HEAD*) recv_data;
    int cmd = hd->cmd;

    static int audioCount_in = 0;
    char sztmp[256];
    switch (hd->cmd){
    case CMD_CALL_CALLERAUDIO:
    case CMD_CALL_CALLEEAUDIO:
        audioCount_in ++;
        audioCount_in %= 100;
        if (0==audioCount_in){
            sprintf(sztmp,"        [%s]<====%s:%d, every 100 these pkts come in, it's shown once.", get_cmd_str(hd->cmd)
                    , inet_ntoa(remoteaddr->sin_addr), ntohs(remoteaddr->sin_port));
//            statusNotify(sztmp);
        }
        break;
    case CMD_HEARTBEAT_ACK:
        break;
    default:
        sprintf(sztmp,"        [%s]<====%s:%d.", get_cmd_str(hd->cmd)
                , inet_ntoa(remoteaddr->sin_addr), ntohs(remoteaddr->sin_port));
        statusNotify(sztmp);
        break;
    }

    switch (cmd) {
    case CMD_LB_LOGINSERVERACK: {
        onLBLoginserverAck((F2FPKG*)recv_data);
        break;
    }
    case CMD_NAT_LOGINACK: {
        onLoginAck((F2FPKG*)recv_data);
        break;
    }
    case CMD_NAT_FWDETECTACK: {
        if (SM_WAITINGFWACK==state_machine ){
            bTerminalHas_firewall = false;
            setStateMachine(SM_NATDETECTREQ, "Received fwdetect ack.");
            retry_ctrl_224488.reset(10);
        }
        break;
    }
    case CMD_NAT_NATDETECTACK: {
        onNatDetectAck((F2FPKG*)recv_data);
        break;
    }
    case CMD_HEARTBEAT_ACK: {
        retry_ctrl_heatbeat.reset(3);
        onHeartBeatAck((F2FPKG*)recv_data);
        break;
    }
    case CMD_REGINFOALTER_ACK: {
        onalterRegInfoAck((F2FPKG*)recv_data);
        break;
    }
    case CMD_REGINFOGET_ACK: {
        onRegInfoGetAck((F2FPKG*)recv_data);
        break;
    }
    case CMD_REGINFOGETBYMOBILE_ACK: {
        onRegInfoGetByMobileAck((F2FPKG*)recv_data);
        break;
    }
    case CMD_CALL_ADDRESSINGACK: {
        onCallAddressingAck((F2FPKG*)recv_data);
        break;
    }
    case CMD_CALL_CALLEEINFORM: {
        onCallCalleeinform((F2FPKG*)recv_data);
        break;
    }
    case CMD_CALL_FWTRAVERSING: {
        onCallFwTraversing((F2FPKG*)recv_data, remoteaddr);
        break;
    }
    case CMD_CALL_CALLINGREQ: {
        onCallCallingReq((F2FPKG*)recv_data, remoteaddr);
        break;
    }
    case CMD_CALL_CALLINGACK: {
        onCallCallingAck((F2FPKG*)recv_data);
        break;
    }
    case CMD_CALL_RING: {
        onCallRing((F2FPKG*)recv_data, remoteaddr);
        break;
    }
    case CMD_CALL_RINGBACK: {
        onCallRingback((F2FPKG*)recv_data, remoteaddr);
        break;
    }
    case CMD_CALL_HANGUP: {
        onCallHangup((F2FPKG*)recv_data);
        break;
    }
    case CMD_CALL_MIRRORINFORM: {
        onMirrorinform((F2FPKG*)recv_data);
        break;
    }
    case CMD_CALL_CALLEEAUDIO:
    case CMD_CALL_CALLEEVIDEO:
    case CMD_CALL_CALLEEPICTURE:
    case CMD_CALL_CALLEEFILE: {
        onCalleeMedia((F2FPKG*)recv_data);
        break;
    }
    case CMD_CALL_CALLERAUDIO:
    case CMD_CALL_CALLERVIDEO:
    case CMD_CALL_CALLERPICTURE:
    case CMD_CALL_CALLERFILE: {
        onCallerMedia((F2FPKG*)recv_data);
        break;
    }
    default:
        BDEBUG(LV_CRITICAL, " --------- unprocessed cmd %s -----------\n", get_cmd_str(cmd));
        break;
    }
#ifdef TESTWITHGCC
    #ifdef BDEBUGLEVEL
            BDEBUGWHERE;
    #endif
#endif
}

void pkg_init(F2FPKG* pkg, int cmd, int session)
{
    F2FPKG_HEAD *p = (F2FPKG_HEAD*)pkg;
    p->code = F2FPROTOCOL_CODE;
    p->version = F2FPROTOCOL_VERSTION;
    p->cmd = cmd;
    p->sessionId = session;
    p->errorCode = 0;
    p->payloadLen = 0;
}

void pkg_append(F2FPKG* pkg, int key, char* value)
{
    F2FPKG_HEAD *p = (F2FPKG_HEAD*)pkg;
    char *q = (char*)pkg->payload;
    p->payloadLen += sprintf(q + p->payloadLen, "%d\xA8\xF0%s\xA8\xF0", key, value);
}

void pkg_append_data(F2FPKG* pkg, int key, char* value, int len)
{
    F2FPKG_HEAD *p = (F2FPKG_HEAD*)pkg;
    char *q = (char*)pkg->payload;
    p->payloadLen += sprintf(q + p->payloadLen, "%d\xA8\xF0", key);
    memcpy(q + p->payloadLen, value, len);
    p->payloadLen += len;
}

int pkg_len(F2FPKG* pkg)
{
    return pkg->pkgHead.payloadLen + sizeof(F2FPKG_HEAD);
}

void f2fUnInit()
{
    quiting = true;
    uninit_udp();
}


/**
 * @brief Initial process to be a Daemon
 */
void init_daemon(void)
{
    int pid;
    int i;

    if((pid=fork()) > 0)
        exit(0);						//it's parent process, exit
    else if(pid< 0)
        exit(1);						//fork failed, quit

    /*it's the first sub-process, execute in backgroud*/
    setsid();							//first sub-process becomes the new session group leader and process leader

    /*serperate from console*/
    if((pid=fork()) > 0)
        exit(0);						//it's the first sub-process, terminate it
    else if(pid< 0)
        exit(1);						//fork failed, quit

    /** it's the second sub-process, continue...
  second sub-process is not the session group leader
 */
    for(i=0;i< 256;++i)				//close opened file description
        close(i);
    chdir("/tmp");						//change the work directory to /tmp
    umask(0);							//reset file create mask
    return;
}


void show_package(F2FPKG *pkg, int line)
{
    BDEBUGMEM(LV_MEMORY, (long unsigned )pkg, "pkt");
    unsigned char info[2048];
    F2FPKG_HEAD *hd = (F2FPKG_HEAD *)pkg;
    BDEBUG(LV_INFO, "package_data [line: %d]:\n", line);
    snprintf((char*)info, 5, "%s", &hd->code);
    BDEBUG(LV_INFO, "    protocal_code:%s\n", info);
    BDEBUG(LV_INFO, "          version:0x%04X\n", hd->version);
    BDEBUG(LV_INFO, "              cmd:0x%04X (%s)\n", hd->cmd, get_cmd_str(hd->cmd));
    BDEBUG(LV_INFO, "       session_id:%d\n", hd->sessionId);
    BDEBUG(LV_INFO, "         err_code:%d\n", hd->errorCode);
    BDEBUG(LV_INFO, "       payloadLen:%d\n", hd->payloadLen);

    int flag=0;
    memset(info,0,sizeof(info));
    unsigned char* q = info;
    unsigned char* p = pkg->payload;
    int i;
BDEBUG(LV_INFO, "       payloadData:%d\n", pkg->payload);
    for (i=0; (i < hd->payloadLen) && (p - pkg->payload < hd->payloadLen) ; i++) {
        if (*p == 0xA8 && *(p+1) == 0xF0){
            p += 2;
            if (flag==0){
                int n = atoi((const char*)info);
                if (n>0) {
                    q += sprintf((char*)q, "[%s] : ", get_key_str(n));
                }
                else {
                    q += sprintf((char*)q, " : ");
                }
            }
            else {
                BDEBUG(LV_INFO, "                %s\n",info);
                q = info;
                *q = 0;
            }
            flag ++;
            flag %= 2;
        }
        else {
            *q++ = *p++;
            *q = 0;
        }
    }
}

#if 0
void display_help(char *file)
{
    fprintf(stderr, "Usage: %s -i <id> -p <passwd> -I <lan_ip> -P <lan_port>\n", file);
    fprintf(stderr, "\n");
    fprintf(stderr, "\t           id : id of the F2F terminal.\n");
    fprintf(stderr, "\t       passwd : passwd of the F2F terminal.\n");
    fprintf(stderr, "\t       lan_ip : Ip address of terminal in the lan network.\n");
    fprintf(stderr, "\t     lan_port : Port of terminal used to receive incoming pkt in the lan network.\n");
}
#endif

bool isPktFromMirror(char* sourceIp, char* mirrorIps)
{
    if (0==otherterminal_info_mirroraddr)
        return false;
    return strstr(mirrorIps, sourceIp);
}

char* getErrorMsg(F2FPKG* recv_data)
{
    kv_build((F2FPKG*)recv_data);
    char* errormsg=0;
    int i = 0;
    pthread_mutex_lock(&mutex_kv);
    while (kv_pair[i] && i<KV_MAX){
        if (kv_pair[i]->key == KEY_ERRORMSG){
            errormsg = strdup(kv_pair[i]->value);
        }
        i++;
    }
    pthread_mutex_unlock(&mutex_kv);
    kv_clear();
    return errormsg;
}

void getDataFromPkt(F2FPKG* pkg, int& data, int& len)
{
    F2FPKG_HEAD *hd = (F2FPKG_HEAD *)pkg;

    int flag=0;
    unsigned char info[2048];
    unsigned int key=0;
    unsigned char* q = info;
    unsigned char* p = pkg->payload;
    unsigned int dataoffset=0;
    unsigned int ntmp = 0;
    int i;
    for (i=0; (i < hd->payloadLen) && (p - pkg->payload < hd->payloadLen) ; i++) {
        if (*p == 0xA8 && *(p+1) == 0xF0){
            p += 2;
            ntmp += 2;
            if (flag==0){
                /** get key */
                key = atoi((const char*)info);
                if (key==KEY_DATA) {
                    dataoffset += ntmp;
                    data =(unsigned long) ((char*)pkg + sizeof(F2FPKG_HEAD) + dataoffset);
                    len = hd->payloadLen - dataoffset;
#ifdef TESTWITHGCC
                        BDEBUG(LV_CLOSE,"data: %X, pkg: %X, sizeof(F2FPKG_HEAD): %d, dataoffset: %d", data, (char*)pkg, sizeof(F2FPKG_HEAD) , dataoffset);
#endif
                }
            }
            else {
                /** get value */
                dataoffset += ntmp;
                ntmp = 0;
            }
            q = info;
            *q = 0;
            flag ++;
            flag %= 2;
        }
        else {
            *q++ = *p++; ntmp++;
            *q = 0;
        }
    }

}

void f2fInit(char* lanip, unsigned short lanport, char* passwd, F2FCBFUNCTIONS* cb, char* mac, char* calleeid, char* terminalid)
{
    BDEBUG(LV_CRITICAL, "lanip:%s, lanport:%d, passwd:%s, cb:%X, mac: %s, calleeid:%s, terminalid:%s",
           lanip,lanport,passwd,cb,mac,calleeid?calleeid:"",terminalid?terminalid:"");
    quiting = false;
    cbfuncs = cb;
    terminal_lanip = strdup(lanip);
    terminal_lanport = lanport;
    terminal_passwd = strdup(passwd);
    if (calleeid) test_calleeid = strdup(calleeid);
	if (terminalid) terminal_id = strdup(terminalid);
    //get mac as terminal_id
#ifdef TESTWITHGCC
#if 0
	{
		if (calleeid)
			get_mac();
		else if (0==terminal_id )
			terminal_id = strdup("11:22:33:44:55:66");
    }
#endif
    terminal_id = strdup(mac);
#else
#if 0
    {
        struct ifreq tmp;
        int sock_mac;
        char mac_addr[30];
        sock_mac = socket(AF_INET, SOCK_STREAM, 0);
        if ( sock_mac == -1){
            BDEBUG(LV_CRITICAL, "create socket fail\n");
            return;
        }
        memset(&tmp,0,sizeof(tmp));
        strncpy(tmp.ifr_name,"eth0",sizeof(tmp.ifr_name)-1 );
        if ( (ioctl( sock_mac, SIOCGIFHWADDR, &tmp)) < 0 ){
            BDEBUG(LV_CRITICAL, "eth0 mac ioctl error\n");

            memset(&tmp,0,sizeof(tmp));
            strncpy(tmp.ifr_name,"wlan0",sizeof(tmp.ifr_name)-1 );
            if ( (ioctl( sock_mac, SIOCGIFHWADDR, &tmp)) < 0 ){
                BDEBUG(LV_CRITICAL, "wlan0 mac ioctl error\n");
                return;
            }
        }
        sprintf(mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X",
                (unsigned char)tmp.ifr_hwaddr.sa_data[0],
                (unsigned char)tmp.ifr_hwaddr.sa_data[1],
                (unsigned char)tmp.ifr_hwaddr.sa_data[2],
                (unsigned char)tmp.ifr_hwaddr.sa_data[3],
                (unsigned char)tmp.ifr_hwaddr.sa_data[4],
                (unsigned char)tmp.ifr_hwaddr.sa_data[5]
                );
        BDEBUG(LV_NORMAL, "local mac:%s\n", mac_addr);
        close(sock_mac);
        terminal_id = strdup(mac_addr);
    }
#else
    if(mac[0] != 0){
        /*bob*/
        terminal_id = strdup(&mac[0]);
//        terminal_id = strdup(&mac[1]);
    }
    else
        return;
#endif
#endif

    //init rsyslog
//    openlog("F2F_CLIENT", LOG_PID, LOG_USER);

    //init_daemon();

    init_udp(terminal_lanport);
    /** create a thread to receive package from server */
    int ret = pthread_create(&thread_id, NULL, timer_thread, (void *)NULL);
    if (ret != 0) {
        perror("pthread_create");
        return ;
//        exit(1);
    }
    BDEBUG(LV_CRITICAL, "Thread 'timer_thread' starts...");

}

#if 0
int terminal(int argc, char *argv[])
{
    openlog("F2F_LB", LOG_PID, LOG_USER);
    BDEBUG(LV_CRITICAL, "argc:%d %s", argc, argv[1]);
    /**  get arguments */
    int opt=0;
    //char *id=0, *passwd=0, *lan_ip=0, *lan_port=0, *calleeid=0;
    while ((opt = getopt(argc, argv, "i:p:I:P:c:h?")) != -1) {
        switch (opt) {
        case 'i':
            //id = strdup(optarg);
            terminal_id = strdup(optarg);
            break;
        case 'p':
            //passwd = strdup(optarg);
            terminal_passwd = strdup(optarg);
            break;
        case 'I':
            //lan_ip = strdup(optarg);
            terminal_lanip = strdup(optarg);
            break;
        case 'P':
            //lan_port = strdup(optarg);
            terminal_lanport = atoi(optarg);
            break;
        case 'c':
            //calleeid = strdup(optarg);
            test_calleeid = strdup(optarg);
            break;
        case 'h':
        case '?':
        default: /* '?' */
            display_help(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    /** to check the validation of the parameters */
    if (0==terminal_id || 0==terminal_passwd || 0==terminal_lanip || 0==terminal_lanport){
        display_help(argv[0]);
        exit(EXIT_FAILURE);
    }

    if (test_calleeid)
        get_mac();      // get MAC of eth1 or wlan0 and store it to terminal_id

    /** make this program a daemon process. */
    //init_daemon();

    init_udp(terminal_lanport);

    /** create a thread to receive package from server */
    int ret = pthread_create(&thread_id, NULL, timer_thread, (void *)NULL);
    if (ret != 0) {
        perror("pthread_create");
        exit(1);
    }
    BDEBUG(LV_CRITICAL, "Thread 'timer_thread' starts...");

    /** waiting for timer_thread to quit... */
    pthread_join(thread_id,NULL);

    return 0;
}
#endif

void* timer_thread(void* ctx)
{

    state_machine = SM_BOOT;
    while (quiting == false)
    {
        if (!list_logtoui.isEmpty()){
            pthread_mutex_lock(&mutex_log);
            char* p = list_logtoui.front();
            if (p){
                if ((0!=cbfuncs) && 0!=cbfuncs->OnStatusNotify){
                    cbfuncs->OnStatusNotify(strlen(p), p);
                }
            }
            list_logtoui.pop_front();
            pthread_mutex_unlock(&mutex_log);
        }
        usleep(200*1000);
        handle_timer();
        handle_mirror_timer();
        handle_call_timer();
        handle_reginfo_get_timer();
        handle_reginfo_get_bymobile_timer();
    }
    BDEBUG(LV_CRITICAL, "Thread 'timer_thread' exit.");

    /** wait 200 ms for the udp receiving thread to tear-down. */
    usleep(200*1000);

    /** free the variables */
    if (terminal_id!=0) { free(terminal_id); terminal_id=0;}
    if (terminal_passwd!=0) { free(terminal_passwd); terminal_passwd=0;}
    if (terminal_lanip!=0) { free(terminal_lanip); terminal_lanip=0;}
    if (test_calleeid!=0) { free(test_calleeid); test_calleeid=0;}

    return NULL;

}


void handle_timer()
{
    char sztmp[256];

    switch (state_machine) {
    case SM_BOOT:
    {
        retry_ctrl_224488.reset();			//reset retry controller
        setStateMachine(SM_LOGINSVRIPREQ,"Auto.");
        break;
    }
    case SM_LOGINSVRIPREQ:
    {
        /*send CMD_LB_LOGINSERVERREQ */
        if (retry_ctrl_224488.isNextStep()){
            F2FPKG *pkg = (F2FPKG*)send_data;
            pkg_init(pkg, CMD_LB_LOGINSERVERREQ, 0);
            pkg_append(pkg, KEY_ID, (char*)terminal_id);
            pkg_append(pkg, KEY_PASSWD, (char*)terminal_passwd);
            send_udp_pkg(F2FSVR_LB_DOMAIN, F2FSVR_LB_UDPPORT, (char*)pkg, pkg_len(pkg));
        }
        break;
    }
    case SM_LOGINREQ:
    {
        /*send CMD_NAT_LOGINREQ, retry 10 times*/
        if (retry_ctrl_224488.isTimeout()){
            retry_ctrl_224488.reset();			//reset retry controller
            setStateMachine(SM_LOGINSVRIPREQ,"timeout to send CMD_NAT_LOGINREQ.");
        }
        else if (retry_ctrl_224488.isNextStep()){
            F2FPKG *pkg = (F2FPKG*)send_data;
            pkg_init(pkg, CMD_NAT_LOGINREQ, 0);
            pkg_append(pkg, KEY_ID, terminal_id);
            pkg_append(pkg, KEY_PASSWD, terminal_passwd);
            pkg_append(pkg, KEY_LANIP, terminal_lanip);
            sprintf(sztmp,"%d", terminal_lanport);
            pkg_append(pkg, KEY_LANPORT, sztmp);
            pkg_append(pkg, KEY_CAPABILITY, "0");
            send_udp_pkg_loginserver((char*)pkg, pkg_len(pkg));
        }
        break;
    }
    case SM_WAITINGFWACK:
    {
        /*send CMD_NAT_LOGINREQ, retry max (defined in retry_ctrl_222222) times */
        if (retry_ctrl_222222.isTimeout()){
            //time out, it means the terminal  is behind a firewall
            bTerminalHas_firewall = true;
            setStateMachine(SM_NATDETECTREQ, "Time out in waiting for fwdetect ack.");
            retry_ctrl_224488.reset(10);
        }
        else if (retry_ctrl_222222.isNextStep()){
            F2FPKG *pkg = (F2FPKG*)send_data;
            pkg_init(pkg, CMD_NAT_LOGINREQ, 0);
            pkg_append(pkg, KEY_ID, (char*)terminal_id);
            pkg_append(pkg, KEY_PASSWD, (char*)terminal_passwd);
            pkg_append(pkg, KEY_LANIP, terminal_lanip);
            sprintf(sztmp, "%d", terminal_lanport);
            pkg_append(pkg, KEY_LANPORT, sztmp);
            pkg_append(pkg, KEY_CAPABILITY, "0");
            send_udp_pkg_loginserver((char*)pkg, pkg_len(pkg));
        }
        break;
    }
    case SM_NATDETECTREQ:
    {
        /*send CMD_NAT_NATDETECTREQ, retry unlimited times*/
        if (retry_ctrl_224488.isTimeout()){
            setStateMachine(SM_BOOT, "Time out in waiting for CMD_NAT_NATDETECTACK.");
        }
        else if (retry_ctrl_224488.isNextStep()){
            F2FPKG *pkg = (F2FPKG*)send_data;
            pkg_init(pkg, CMD_NAT_NATDETECTREQ, 0);
            pkg_append(pkg, KEY_ID, terminal_id);
            pkg_append(pkg, KEY_HASFW, (char*)((bTerminalHas_firewall)?"Y":"N"));
            pkg_append(pkg, KEY_WANIP, terminal_wanip);
            sprintf(sztmp, "%d", terminal_wanport);
            pkg_append(pkg, KEY_WANPORT, sztmp);
            pkg_append(pkg, KEY_LOGSVRIP, loginserver_ip);
            sprintf(sztmp, "%d", loginserver_port);
            pkg_append(pkg, KEY_LOGSVRPORT, sztmp);
            send_udp_pkg(natdetectserver_ip, natdetectserver_port, (char*)pkg, pkg_len(pkg));
        }
        break;
    }
    case SM_HEARTBEATING:
    {

        /*send CMD_HEARTBEAT_REQ, retry unlimited times*/
        if (retry_ctrl_heatbeat.isTimeout()){
            setStateMachine(SM_BOOT, "Time out in waiting for CMD_HEARTBEAT_ACK.");
        }
        else if (retry_ctrl_heatbeat.isNextStep()){
            F2FPKG *pkg = (F2FPKG*)send_data;
            pkg_init(pkg, CMD_HEARTBEAT_REQ, session_id);
            pkg_append(pkg, KEY_ID, terminal_id);
            pkg_append(pkg, KEY_FWDING, (SMMIRROR_STANDBY==state_machine_mirror)?(char*)"N":(char*)"Y");
            pkg_append(pkg, KEY_MEETING, "N");
            pkg_append(pkg, KEY_CALLING, (SMCALL_IDLE==state_machine_call)?(char*)"N":(char*)"Y");
            send_udp_pkg_loginserver((char*)pkg, pkg_len(pkg));
        }
        break;
    }
    default:
        break;
    }

}

int startCall(char* calleeid, SESSIONTYPE st)
{
    if (test_calleeid) {free(test_calleeid);test_calleeid=0;}
    test_calleeid = strdup(calleeid);
    snprintf(session_type,2,"%1d",st);

    if ((state_machine == SM_HEARTBEATING) && (state_machine_call == SMCALL_IDLE) ){
        //init variables about otherterminal
        if(otherterminal_info_id){free(otherterminal_info_id);otherterminal_info_id=0;}
        if(otherterminal_info_ip){free(otherterminal_info_ip);otherterminal_info_ip=0;}
        if(otherterminal_info_lip){free(otherterminal_info_lip);otherterminal_info_lip=0;}
        otherterminal_info_port=0;
        otherterminal_info_lport=0;
        otherterminal_info_mirrorused=false;
        if(otherterminal_info_mirroraddr){free(otherterminal_info_mirroraddr);otherterminal_info_mirroraddr=0;}

        //change statemachine to send addressing req.
        retry_addresing_req.reset(10);
        char sztmp[256];
        sprintf(sztmp, "Start calling %s-->%s", terminal_id, calleeid);
        setStateMachineCall(SMCALL_ADDRESSINGREQ, sztmp, __FUNCTION__, __LINE__);
        bCanCallback = true;
        bOnReadyCalled = false;
        return 0;
    }
    else
        return -1;
}

void handle_reginfo_get_timer()
{
    switch (state_machine_reginfoget){
    case SMREGINFOGET_IDLE:{
        break;
    }
    case SMREGINFOGET_FETCHING:{
        if (retry_reginfoget_timeout.isTimeout()) {
            char sztmp[64];
            sprintf(sztmp, "try %d times with no response", retry_reginfoget_timeout.getmax());
            setStateMachineReginfoGet(SMREGINFOGET_NOTIFIED, sztmp, __FUNCTION__,__LINE__);
            if (cbfuncs && cbfuncs->OnGetRegInfoAck)
                cbfuncs->OnGetRegInfoAck(-2,retry_reginfoget_timeout.gettag(),0,0);

        }
        else if (retry_reginfoget_timeout.isNextStep()) {
            F2FPKG *pkg = (F2FPKG*)send_data;
            pkg_init(pkg, CMD_REGINFOGET_REQ, 0);
            pkg_append(pkg, KEY_ID, retry_reginfoget_timeout.gettag());
            send_udp_pkg_loginserver((char*)pkg, pkg_len(pkg));
        }
        break;
    }
    case SMREGINFOGET_NOTIFIED:{
        setStateMachineReginfoGet(SMREGINFOGET_IDLE, "Notified app already.", __FUNCTION__,__LINE__);
        break;
    }
    default:
        break;
    }
}

void handle_reginfo_get_bymobile_timer()
{
    switch (state_machine_reginfogetbymobile){
    case SMREGINFOGETBYMOBILE_IDLE:{
        break;
    }
    case SMREGINFOGETBYMOBILE_FETCHING:{
        if (retry_reginfogetbymobile_timeout.isTimeout()) {
            char sztmp[64];
            sprintf(sztmp, "try %d times with no response", retry_reginfogetbymobile_timeout.getmax());
            setStateMachineReginfoGetByMobile(SMREGINFOGETBYMOBILE_NOTIFIED, sztmp, __FUNCTION__,__LINE__);
            if (cbfuncs && cbfuncs->OnGetRegInfoAck)
                cbfuncs->OnGetRegInfoAck(-2,0,0,retry_reginfoget_timeout.gettag());
        }
        else if (retry_reginfogetbymobile_timeout.isNextStep()) {
            F2FPKG *pkg = (F2FPKG*)send_data;
            pkg_init(pkg, CMD_REGINFOGETBYMOBILE_REQ, 0);
            pkg_append(pkg, KEY_MOBILENO, retry_reginfogetbymobile_timeout.gettag());
            send_udp_pkg_loginserver((char*)pkg, pkg_len(pkg));
        }
        break;
    }
    case SMREGINFOGETBYMOBILE_NOTIFIED:{
        setStateMachineReginfoGetByMobile(SMREGINFOGETBYMOBILE_IDLE, "Notified app already.", __FUNCTION__,__LINE__);
        break;
    }
    default:
        break;
    }


}

void handle_mirror_timer()
{
    switch (state_machine_mirror) {
    case SMMIRROR_STANDBY:
    {
        break;
    }
    case SMMIRROR_FORWARDING:
    {
        if (retry_mirrorfwding_timeout.isNextStep() && retry_mirrorfwding_timeout.isTimeout()) {
            setStateMachineMirror(SMMIRROR_STANDBY, "no data to forward for 30 secs");
        }
        break;

        break;
    }
    case SMMIRROR_INFORMSTATE:
    {

        break;
    }
    default:
        break;
    }
}

void handle_call_timer()
{
    switch (state_machine_call) {
    case SMCALL_IDLE:
    {
        break;
    }
    case SMCALL_ADDRESSINGREQ:
    {
        if (!retry_addresing_req.isTimeout()){
            if (retry_addresing_req.isNextStep())
                startCall_inner(terminal_id, test_calleeid, true);
        }
        else {
            setStateMachineCall(SMCALL_OTHERSIDE_HANGUP, "No reponse after sending 10 times of addresing request.", __FUNCTION__, __LINE__);
        }
        break;
    }
    case SMCALL_CALLEEINFORM_LAN:
    {
        if (!retry_fwtraversing.isTimeout()){
            if (retry_fwtraversing.isNextStep())
               sendFwTraversing();
        }
        else {
            retry_fwtraversing.reset(10);
            char sztmp[256];
            sprintf(sztmp, "No reponse after sending 3 times of fwtraversing. ==>%s:%d(lan)"
                    , otherterminal_info_lip, otherterminal_info_lport);
            setStateMachineCall(SMCALL_CALLEEINFORM, sztmp, __FUNCTION__, __LINE__);
        }
        break;
    }
    case SMCALL_CALLEEINFORM:
    {
        if (!retry_fwtraversing.isTimeout()){
            if (retry_fwtraversing.isNextStep())
               sendFwTraversing();
        }
        else {
            char sztmp[256];
            if (otherterminal_info_mirrorused){
                if (otherterminal_info_mirroraddr)
                    sprintf(sztmp, "No reponse after sending 3 times of fwtraversing. ==>%s(mirrors)"
                            , otherterminal_info_mirroraddr);
                else
                    sprintf(sztmp, "Need mirror, but no mirror is found.");
            }
            else
                sprintf(sztmp, "No reponse after sending 3 times of fwtraversing. ==>%s:%d(wan)"
                        , otherterminal_info_ip, otherterminal_info_port);
            setStateMachineCall(SMCALL_OTHERSIDE_HANGUP, sztmp, __FUNCTION__, __LINE__);
        }
        break;
    }
    case SMCALL_CALLINGREQ_LAN:
    {
        if (!retry_calling_req.isTimeout()){
            if (retry_calling_req.isNextStep())
                sendCallingReq();
        }
        else {
            retry_calling_req.reset(10);
            char sztmp[256];
            sprintf(sztmp, "No reponse after sending 3 times of calling-req. ==>%s:%d(lan)"
                    , otherterminal_info_lip, otherterminal_info_lport);
            setStateMachineCall(SMCALL_CALLINGREQ, sztmp, __FUNCTION__, __LINE__);
        }
        break;
    }
    case SMCALL_CALLINGREQ:
    {
        if (!retry_calling_req.isTimeout()){
            if (retry_calling_req.isNextStep())
                sendCallingReq();
        }
        else {
            char sztmp[256];
            if (otherterminal_info_mirrorused){
                if (otherterminal_info_mirroraddr)
                    sprintf(sztmp, "No reponse after sending 3 times of calling-req. ==>%s(mirrors)"
                            , otherterminal_info_mirroraddr);
                else
                    sprintf(sztmp, "Need mirror, but no mirror is found.");
            }
            else
                sprintf(sztmp, "No reponse after sending 3 times of calling-req. ==>%s:%d(wan)"
                        , otherterminal_info_ip, otherterminal_info_port);
            setStateMachineCall(SMCALL_LINEBUSY, sztmp, __FUNCTION__, __LINE__);
        }
        break;
    }

    case SMCALL_RING:
    {
        if (!retry_ringOrRingback.isTimeout()){
            if (retry_ringOrRingback.isNextStep()) {
                sendRing();
                retry_calleraudio_timeout.reset(1);
            }
		}
        else {
            char sztmp[256];
            if (otherterminal_info_mirrorused){
                if (otherterminal_info_mirroraddr)
                    sprintf(sztmp, "No answer, send busytone to caller. ==>%s(mirrors)"
                            , otherterminal_info_mirroraddr);
                else
                    sprintf(sztmp, "Need mirror, but no mirror is found.");
            }
            else
                sprintf(sztmp, "No answer, send busytone to caller. ==>%s:%d(wan)"
                        , otherterminal_info_ip, otherterminal_info_port);
            setStateMachineCall(SMCALL_HANGUP, sztmp, __FUNCTION__, __LINE__);
		}
		break;
	}

	case SMCALL_RINGBACK:
	{
		if (!retry_ringOrRingback.isTimeout()){
            if (retry_ringOrRingback.isNextStep()){
                sendRingback();
                retry_calleeaudio_timeout.reset(1);
            }
		}
		else {
            char sztmp[256];
            if (otherterminal_info_mirrorused){
                if (otherterminal_info_mirroraddr)
                    sprintf(sztmp, "Retry 20 times of sending ringback ==>%s(mirrors)"
                            , otherterminal_info_mirroraddr);
                else
                    sprintf(sztmp, "Need mirror, but no mirror is found.");
            }
            else
                sprintf(sztmp, "Retry 20 times of sending ringback ==>%s:%d(wan)"
                        , otherterminal_info_ip, otherterminal_info_port);
            setStateMachineCall(SMCALL_OTHERSIDE_HANGUP, sztmp, __FUNCTION__, __LINE__);
		}
		break;
	}
	case SMCALL_CALLEEAUDIO_TEST:
    {
        /*
		if (!retry_send_calleeaudio.isTimeout()){
			if (retry_send_calleeaudio.isNextStep())
				sendCalleeAudio();
		}
		else {
			setStateMachineCall(SMCALL_HANGUP, "test finished, hangup");
        }
        */

		break;
	}
	case SMCALL_CALLEEAUDIO:
	{
        /** no caller data incoming for 20 secs, change to SMCALL_HANGUP */
        if (retry_calleraudio_timeout.isNextStep() && retry_calleraudio_timeout.isTimeout()) {
            setStateMachineCall(SMCALL_OTHERSIDE_HANGUP, "No caller data incoming for 20 secs", __FUNCTION__, __LINE__);
        }
		break;
	}

	case SMCALL_CALLERAUDIO_TEST:
    {
        /*
		if (!retry_send_calleraudio.isTimeout()){
			if (retry_send_calleraudio.isNextStep())
				sendCallerAudio();
		}
		else {
			setStateMachineCall(SMCALL_HANGUP, "test finished, hangup");
        }
        */

		break;
	}
	case SMCALL_CALLERAUDIO:
	{
        /** no callee data incoming for 20 secs, change to SMCALL_HANGUP */
        if (retry_calleeaudio_timeout.isNextStep() && retry_calleeaudio_timeout.isTimeout()) {
            setStateMachineCall(SMCALL_OTHERSIDE_HANGUP, "No callee data incoming for 20 secs", __FUNCTION__, __LINE__);
        }
        break;
	}

	case SMCALL_CALLEENOANSWER:
	{
        setStateMachineCall(SMCALL_OTHERSIDE_HANGUP, "Callee no answer.", __FUNCTION__, __LINE__);
		break;
	}

	case SMCALL_LINEBUSY:
    {
        //setStateMachineCall(SMCALL_IDLE, "Linebusy.");
        setStateMachineCall(SMCALL_OTHERSIDE_HANGUP, "Linebusy.", __FUNCTION__, __LINE__);
        break;
	}

	case SMCALL_HANGUP:
    {
        if ((0!=cbfuncs) && 0!=cbfuncs->OnHangUp /*&& bCanCallback*/ ){
            cbfuncs->OnHangUp();
        }
        setStateMachineCall(SMCALL_HANGUP_SENDING, "Auto.", __FUNCTION__, __LINE__);
        count_sending_hangup = 0;
        break;
    }

    case SMCALL_HANGUP_SENDING:
    {
        if (count_sending_hangup++ > 5){
            setStateMachineCall(SMCALL_IDLE, "repeat sending hangup for 5 times.", __FUNCTION__, __LINE__);
        }
        else {
            sendHangup();
        }
        break;
    }

    case SMCALL_OTHERSIDE_HANGUP:
    {
        if ((0!=cbfuncs) && 0!=cbfuncs->OnHangUp /*&& bCanCallback*/ ){
            cbfuncs->OnHangUp();
        }
        setStateMachineCall(SMCALL_IDLE, "Auto.", __FUNCTION__, __LINE__);
        break;
    }

	default:
	break;
	}
}

void startCall_inner(char* callerid, char* calleeid, bool bCalledByTimer)
{
	F2FPKG *pkg=0;
	if (bCalledByTimer)
		pkg = (F2FPKG*)send_data;
	else
		pkg = (F2FPKG*)send_data_codec;
    pkg_init(pkg, CMD_CALL_ADDRESSINGREQ, 0);
    pkg_append(pkg, KEY_SESSIONTYPE, session_type);
	pkg_append(pkg, KEY_CALLERID, (char*)callerid);
	pkg_append(pkg, KEY_CALLEEID, (char*)calleeid);
	send_udp_pkg_loginserver((char*)pkg, pkg_len(pkg));
}

void sendCallingReq()
{
	F2FPKG *pkg = (F2FPKG*)send_data;
	pkg_init(pkg, CMD_CALL_CALLINGREQ, 0);
    pkg_append(pkg, KEY_SESSIONTYPE, session_type);
    pkg_append(pkg, KEY_SOURCEID, (char*)terminal_id);
	pkg_append(pkg, KEY_DESTID, (char*)otherterminal_info_id);
	if (SMCALL_CALLINGREQ_LAN==state_machine_call){
        //主被叫公网ip相同时，先尝试使用局域网ip来通讯
        send_udp_pkg(otherterminal_info_lip, otherterminal_info_lport, (char*)pkg, pkg_len(pkg));
    }
    else {
        //主被叫ip不相同，或者虽然相同，但是尝试局域网ip通讯失败后，执行此分支。
        if (otherterminal_info_mirrorused){
            //用到镜子终端，则“calling req包”发送到镜子终端
            if (otherterminal_info_mirroraddr){
                //有镜子列表，向镜子终端逐个发送“calling req包”
                char szMirrorIP[32];
                char szPort[16];
                bool bgetport = true;
                memset(szMirrorIP,0,sizeof(szMirrorIP));
                memset(szPort,0,sizeof(szPort));
                for (int i=0; i<strlen(otherterminal_info_mirroraddr); i++){
                    char c = otherterminal_info_mirroraddr[i];
                    if ('@'==c){
                        bgetport = false;
                    }
                    else if(';'==c){
                        bgetport = true;
                        send_udp_pkg(szMirrorIP, atoi(szPort), (char*)pkg, pkg_len(pkg));

                    }
                    else{
                        if (bgetport)
                            szPort[strlen(szPort)] = c;
                        else
                            szMirrorIP[strlen(szMirrorIP)] =c;
                    }
                }

            }
            else {
                //无镜子列表，出错
                BDEBUG(LV_NORMAL, "Error: No mirror to be used, cannot send CMD_CALL_CALLINGREQ pkt.");
            }
        }
        else{
            //没有用到镜子，“calling req 包”直接发送到主叫
            send_udp_pkg(otherterminal_info_ip, otherterminal_info_port, (char*)pkg, pkg_len(pkg));
        }
    }
}

void sendFwTraversing()
{
	F2FPKG *pkg = (F2FPKG*)send_data;
	pkg_init(pkg, CMD_CALL_FWTRAVERSING, 0);
	pkg_append(pkg, KEY_SOURCEID, (char*)terminal_id);
	pkg_append(pkg, KEY_DESTID, (char*)otherterminal_info_id);
    if (SMCALL_CALLEEINFORM_LAN==state_machine_call){
        //主被叫公网ip相同时，先尝试使用局域网ip来通讯
		send_udp_pkg(otherterminal_info_lip, otherterminal_info_lport, (char*)pkg, pkg_len(pkg));
	}
    else {
		//主被叫ip不相同，或者虽然相同，但是尝试局域网ip通讯失败后，执行此分支。
        if (otherterminal_info_mirrorused){
            //用到镜子终端，则“防火墙打通包”发送到镜子终端
            if (otherterminal_info_mirroraddr){
                //有镜子列表，向镜子终端逐个发送“防火墙打通包”
                char szMirrorIP[32];
                char szPort[16];
                bool bgetport = true;
                memset(szMirrorIP,0,sizeof(szMirrorIP));
                memset(szPort,0,sizeof(szPort));
                for (int i=0; i<strlen(otherterminal_info_mirroraddr); i++){
                    char c = otherterminal_info_mirroraddr[i];
                    if ('@'==c){
                        bgetport = false;
                    }
                    else if(';'==c){
                        bgetport = true;
                        send_udp_pkg(szMirrorIP, atoi(szPort), (char*)pkg, pkg_len(pkg));

                    }
                    else{
                        if (bgetport)
                            szPort[strlen(szPort)] = c;
                        else
                            szMirrorIP[strlen(szMirrorIP)] =c;
                    }
                }

            }
            else {
                //无镜子列表，出错
                BDEBUG(LV_NORMAL, "Error: No mirror to be used, cannot send CMD_CALL_FWTRAVERSING pkt.");
            }
        }
        else{
			//没有用到镜子，“防火墙打通包”直接发送到主叫
            send_udp_pkg(otherterminal_info_ip, otherterminal_info_port, (char*)pkg, pkg_len(pkg));
        }
	}

}

void sendHangup()
{
    F2FPKG *pkg = (F2FPKG*)send_data;
    pkg_init(pkg, CMD_CALL_HANGUP, 0);
    pkg_append(pkg, KEY_SESSIONTYPE, session_type);
    pkg_append(pkg, KEY_SOURCEID, (char*)terminal_id);
    pkg_append(pkg, KEY_DESTID, (char*)otherterminal_info_id);
    if (otherterminal_info_mirrorused){
        //ask mirror to forward the huangup
        if (otherterminal_info_mirroraddr){
            //send ring pkt to mirrors
            char szMirrorIP[32];
            char szPort[16];
            bool bgetport = true;
            memset(szMirrorIP,0,sizeof(szMirrorIP));
            memset(szPort,0,sizeof(szPort));
            for (int i=0; i<strlen(otherterminal_info_mirroraddr); i++){
                char c = otherterminal_info_mirroraddr[i];
                if ('@'==c){
                    bgetport = false;
                }
                else if(';'==c){
                    bgetport = true;
                    send_udp_pkg(szMirrorIP, atoi(szPort), (char*)pkg, pkg_len(pkg));
                }
                else{
                    if (bgetport)
                        szPort[strlen(szPort)] = c;
                    else
                        szMirrorIP[strlen(szMirrorIP)] =c;
                }
            }
        }
        else{
            BDEBUG(LV_NORMAL, "need mirror, but there is no mirror.");
        }

    }
    else {
        //send hangup pkt to caller
        send_udp_pkg(otherterminal_info_ip, otherterminal_info_port, (char*)pkg, pkg_len(pkg));
    }

}

void sendRing()
{
	F2FPKG *pkg = (F2FPKG*)send_data;
	pkg_init(pkg, CMD_CALL_RING, 0);
	pkg_append(pkg, KEY_SOURCEID, (char*)terminal_id);
    pkg_append(pkg, KEY_DESTID, (char*)otherterminal_info_id);
    if (otherterminal_info_mirrorused){
        //ask mirror to forward the ring pkt
        if (otherterminal_info_mirroraddr){
            //send ring pkt to mirrors
            char szMirrorIP[32];
            char szPort[16];
            bool bgetport = true;
            memset(szMirrorIP,0,sizeof(szMirrorIP));
            memset(szPort,0,sizeof(szPort));
            for (int i=0; i<strlen(otherterminal_info_mirroraddr); i++){
                char c = otherterminal_info_mirroraddr[i];
                if ('@'==c){
                    bgetport = false;
                }
                else if(';'==c){
                    bgetport = true;
                    send_udp_pkg(szMirrorIP, atoi(szPort), (char*)pkg, pkg_len(pkg));
                }
                else{
                    if (bgetport)
                        szPort[strlen(szPort)] = c;
                    else
                        szMirrorIP[strlen(szMirrorIP)] =c;
                }
            }
        }
        else{
            BDEBUG(LV_NORMAL, "need mirror, but there is no mirror.");
        }

    }
    else {
        //send ring pkt to caller
        send_udp_pkg(otherterminal_info_ip, otherterminal_info_port, (char*)pkg, pkg_len(pkg));
    }

}

void sendRingback()
{
	F2FPKG *pkg = (F2FPKG*)send_data;
	pkg_init(pkg, CMD_CALL_RINGBACK, 0);
	pkg_append(pkg, KEY_SOURCEID, (char*)terminal_id);
    pkg_append(pkg, KEY_DESTID, (char*)otherterminal_info_id);
    if (otherterminal_info_mirrorused){
        //ask mirror to forward the ring back pkt
        if (otherterminal_info_mirroraddr){
            //send ringback to mirrors
            char szMirrorIP[32];
            char szPort[16];
            bool bgetport = true;
            memset(szMirrorIP,0,sizeof(szMirrorIP));
            memset(szPort,0,sizeof(szPort));
            for (int i=0; i<strlen(otherterminal_info_mirroraddr); i++){
                char c = otherterminal_info_mirroraddr[i];
                if ('@'==c){
                    bgetport = false;
                }
                else if(';'==c){
                    bgetport = true;
                    send_udp_pkg(szMirrorIP, atoi(szPort), (char*)pkg, pkg_len(pkg));
                }
                else{
                    if (bgetport)
                        szPort[strlen(szPort)] = c;
                    else
                        szMirrorIP[strlen(szMirrorIP)] =c;
                }
            }

        }
        else{
            BDEBUG(LV_NORMAL, "need mirror, but there is no mirror.");
        }

    }
    else {
        //send ring back pkt to callee
        send_udp_pkg(otherterminal_info_ip, otherterminal_info_port, (char*)pkg, pkg_len(pkg));
    }
}

void sendCalleeAudio()
{
	F2FPKG *pkg = (F2FPKG*)send_data;
	pkg_init(pkg, CMD_CALL_CALLEEAUDIO, 0);
	pkg_append(pkg, KEY_SOURCEID, (char*)terminal_id);
	pkg_append(pkg, KEY_DESTID, (char*)otherterminal_info_id);
	send_udp_pkg(otherterminal_info_ip, otherterminal_info_port, (char*)pkg, pkg_len(pkg));
}

void sendCalleeData(char* data, int len)
{
	F2FPKG *pkg = (F2FPKG*)send_data_codec;
	pkg_init(pkg, CMD_CALL_CALLEEAUDIO, 0);
	pkg_append(pkg, KEY_SOURCEID, (char*)terminal_id);
	pkg_append(pkg, KEY_DESTID, (char*)otherterminal_info_id);
	pkg_append_data(pkg, KEY_DATA, data, len);
	send_udp_pkg(otherterminal_info_ip, otherterminal_info_port, (char*)pkg, pkg_len(pkg));
}


void sendCallerAudio()
{
	F2FPKG *pkg = (F2FPKG*)send_data;
	pkg_init(pkg, CMD_CALL_CALLERAUDIO, 0);
	pkg_append(pkg, KEY_SOURCEID, (char*)terminal_id);
	pkg_append(pkg, KEY_DESTID, (char*)otherterminal_info_id);
	send_udp_pkg(otherterminal_info_ip, otherterminal_info_port, (char*)pkg, pkg_len(pkg));

}

void sendData(unsigned char* data, int len)
{
    if (0==otherterminal_info_ip || 0==otherterminal_info_port){
        BDEBUG(LV_CRITICAL, "Error: otherterminal_info_ip or otherterminal_info_port is 0");
        return;
    }
    F2FPKG *pkg = (F2FPKG*)send_data_codec;
    if (SMCALL_CALLEEAUDIO == state_machine_call)
        pkg_init(pkg, CMD_CALL_CALLEEAUDIO, 0);
    else if (SMCALL_CALLERAUDIO == state_machine_call)
        pkg_init(pkg, CMD_CALL_CALLERAUDIO, 0);
    else{
        BDEBUG(LV_CRITICAL, "Error: cannot send data while in %s.", get_smcall_str(state_machine_call));
        return;
    }
    pkg_append(pkg, KEY_SOURCEID, (char*)terminal_id);
    pkg_append(pkg, KEY_DESTID, (char*)otherterminal_info_id);
    pkg_append_data(pkg, KEY_DATA, (char*)data, len);

    if (otherterminal_info_mirrorused){
        BDEBUG(LV_INFO, "ask mirror to forward the data pkt.");
        //ask mirror to forward the data pkt
        if (otherterminal_info_mirroraddr){
            //send pkt to mirrors
            char szMirrorIP[32];
            char szPort[16];
            bool bgetport = true;
            memset(szMirrorIP,0,sizeof(szMirrorIP));
            memset(szPort,0,sizeof(szPort));
            for (int i=0; i<strlen(otherterminal_info_mirroraddr); i++){
                char c = otherterminal_info_mirroraddr[i];
                if ('@'==c){
                    bgetport = false;
                }
                else if(';'==c){
                    bgetport = true;
                    send_udp_pkg(szMirrorIP, atoi(szPort), (char*)pkg, pkg_len(pkg));
                }
                else{
                    if (bgetport)
                        szPort[strlen(szPort)] = c;
                    else
                        szMirrorIP[strlen(szMirrorIP)] =c;
                }
            }

        }
        else{
            BDEBUG(LV_NORMAL, "need mirror, but there is no mirror.");
        }

    }
    else {
        //send ring data pkt to other side
        send_udp_pkg(otherterminal_info_ip, otherterminal_info_port, (char*)pkg, pkg_len(pkg));
    }

}


void hangUp()
{
    bCanCallback = false;
    setStateMachineCall(SMCALL_HANGUP, "User hangup the calling.", __FUNCTION__, __LINE__);

}

void sessionAccept()
{
    retry_calleraudio_timeout.reset(1);
    setStateMachineCall(SMCALL_CALLEEAUDIO, "User accepted the invitation.", __FUNCTION__, __LINE__);
    bCanCallback = true;
    if ((0!=cbfuncs) && 0!=cbfuncs->OnReady /*&& bCanCallback*/ ){
        cbfuncs->OnReady();
    }
}

void onalterRegInfoAck(F2FPKG* recv_data)
{
    F2FPKG_HEAD* hd = (F2FPKG_HEAD*) recv_data;
    if (hd->errorCode == 0) {
        if ((0!=cbfuncs) && 0!=cbfuncs->OnalterRegInfoAck /*&& bCanCallback*/ ){
            cbfuncs->OnalterRegInfoAck(0, "Alter register data succeeded.");
        }
    }
    else {
        char* errmsg = getErrorMsg(recv_data);
        if (errmsg){
            BDEBUG(LV_NORMAL, "%s pkg returns with error_msg: %s\n", get_cmd_str(hd->cmd), errmsg);
            if ((0!=cbfuncs) && 0!=cbfuncs->OnalterRegInfoAck /*&& bCanCallback*/ ){
                cbfuncs->OnalterRegInfoAck(-1, errmsg );
            }
            free(errmsg); errmsg=0;
        }

    }
}

void onRegInfoGetAck(F2FPKG* recv_data)
{
    kv_build((F2FPKG*)recv_data);
    int i = 0;
    char *mac = 0;
    char *nickname= 0;
    char *mobileno= 0;
    pthread_mutex_lock(&mutex_kv);
    while (kv_pair[i] && i<KV_MAX){
        switch (kv_pair[i]->key){
        case KEY_ID:
            mac = strdup(kv_pair[i]->value);
            break;
        case KEY_NICKNAME:
            nickname = strdup(kv_pair[i]->value);
            break;
        case KEY_MOBILENO:
            mobileno = strdup(kv_pair[i]->value);
            break;
        }
        i++;
    }
    pthread_mutex_unlock(&mutex_kv);
    kv_clear();

    F2FPKG_HEAD* hd = (F2FPKG_HEAD*) recv_data;
    if (hd->errorCode == 0) {
        if ((0!=cbfuncs) && 0!=cbfuncs->OnGetRegInfoAck)
            cbfuncs->OnGetRegInfoAck(0,mac,nickname,mobileno);
        setStateMachineReginfoGet(SMREGINFOGET_NOTIFIED, "received CMD_REGINFOGET_ACK", __FUNCTION__,__LINE__);
    }
    else {
        char* errmsg = getErrorMsg(recv_data);
        if (errmsg){
            BDEBUG(LV_CRITICAL, "%s pkg returns with error_msg: %s\n", get_cmd_str(hd->cmd), errmsg);
            free(errmsg); errmsg=0;
        }
        setStateMachineReginfoGet(SMREGINFOGET_NOTIFIED, "received CMD_REGINFOGET_ACK with error", __FUNCTION__,__LINE__);
        if ((0!=cbfuncs) && 0!=cbfuncs->OnGetRegInfoAck)
            cbfuncs->OnGetRegInfoAck(-1,mac,nickname,mobileno);
    }


    /* clean up variables */
    if (mac){free(mac);mac=0;}
    if (nickname){free(nickname);nickname=0;}
    if (mobileno){free(mobileno);mobileno=0;}

}

void onRegInfoGetByMobileAck(F2FPKG* recv_data)
{
    /** get terminal nat property */
    kv_build((F2FPKG*)recv_data);
    int i = 0;
    char *mobileno= 0;
    char *buddy= 0;
    pthread_mutex_lock(&mutex_kv);
    while (kv_pair[i] && i<KV_MAX){
        switch (kv_pair[i]->key){
        case KEY_MOBILENO:
            mobileno = strdup(kv_pair[i]->value);
            break;
        case KEY_BUDDY:
            buddy = strdup(kv_pair[i]->value);
            break;
        }
        i++;
    }
    pthread_mutex_unlock(&mutex_kv);
    kv_clear();

    /** check if all key-value pairs are available */

    F2FPKG_HEAD* hd = (F2FPKG_HEAD*) recv_data;
    if (hd->errorCode == 0) {
        if (buddy && mobileno) {
            if ((0!=cbfuncs) && 0!=cbfuncs->OnBuddyReceived/*&& bCanCallback*/ ){
                setStateMachineReginfoGetByMobile(SMREGINFOGETBYMOBILE_NOTIFIED, "received CMD_REGINFOGETBYMOBILE_ACK", __FUNCTION__,__LINE__);
                if ((0!=cbfuncs) && 0!=cbfuncs->OnGetRegInfoByMobileAck)
                    cbfuncs->OnGetRegInfoByMobileAck(0,mobileno,(char*)buddy);
            }
        }
    }
    else {
        char* errmsg = getErrorMsg(recv_data);
        if (errmsg){
            BDEBUG(LV_CRITICAL, "%s pkg returns with error_msg: %s\n", get_cmd_str(hd->cmd), errmsg);
            free(errmsg); errmsg=0;
        }
        setStateMachineReginfoGetByMobile(SMREGINFOGETBYMOBILE_NOTIFIED, "received CMD_REGINFOGETBYMOBILE_ACK with error", __FUNCTION__,__LINE__);
        if ((0!=cbfuncs) && 0!=cbfuncs->OnGetRegInfoByMobileAck)
            cbfuncs->OnGetRegInfoByMobileAck(-1,mobileno,0);
    }

    /** clean up the variables */
    if (buddy!=0){free(buddy); buddy=0;}
    if (mobileno){free(mobileno);mobileno=0;}


}

void onLBLoginserverAck(F2FPKG* recv_data)
{
	F2FPKG_HEAD* hd = (F2FPKG_HEAD*) recv_data;
	int cmd = hd->cmd;
	if (state_machine!=SM_LOGINSVRIPREQ) {
        BDEBUG(LV_NORMAL, "Cannot process cmd %s in state %s", get_cmd_str(cmd), get_sm_str(state_machine));
		return;
	}
	if (hd->errorCode == 0) {
        kv_build((F2FPKG*)recv_data);
		char* login_server_ip=0, *login_server_port=0;
		int i = 0;
        pthread_mutex_lock(&mutex_kv);
        while (kv_pair[i] && i<KV_MAX){
			if (kv_pair[i]->key == KEY_LOGSVRIP){
				login_server_ip = strdup(kv_pair[i]->value);
			}
			else if (kv_pair[i]->key == KEY_LOGSVRPORT){
				login_server_port = strdup(kv_pair[i]->value);
			}
			i++;
		}
        pthread_mutex_unlock(&mutex_kv);
        kv_clear();

		if (0!=login_server_ip && 0!=login_server_port){
            BDEBUG(LV_CRITICAL, "login_server_ip: %s, login_server_port:%s", login_server_ip,login_server_port);
            strcpy(loginserver_ip, strstr(login_server_ip,"localhost")?"127.0.0.1":login_server_ip);
			loginserver_port = atoi(login_server_port);

			struct hostent *host;
            host = (struct hostent*) gethostbyname(loginserver_ip);
            if (!host){
                BDEBUG(LV_CRITICAL, "error when call to gethostbyname(\"%s\")", login_server_ip);
				if (login_server_ip) {free(login_server_ip); login_server_ip=0;}
                if (login_server_port) {free(login_server_port); login_server_port=0;}
				return;
			}
			login_svr_addr.sin_family = AF_INET;
			login_svr_addr.sin_port = htons((unsigned short)atoi(login_server_port));
            login_svr_addr.sin_addr = *((struct in_addr*)host->h_addr);
			setStateMachine(SM_LOGINREQ, "Received login server's ip and port.");
            retry_ctrl_224488.reset(10);
		}
		if (login_server_ip) {free(login_server_ip); login_server_ip=0;}
        if (login_server_port) {free(login_server_port); login_server_port=0;}

	}
	else {
        char* errmsg = getErrorMsg(recv_data);
		if (errmsg){
            BDEBUG(LV_CRITICAL, "%s pkg returns with error_msg: %s\n", get_cmd_str(hd->cmd), errmsg);
			free(errmsg); errmsg=0;
		}

	}
}

void onHeartBeatAck(F2FPKG* recv_data)
{
    show_package(recv_data, __LINE__);

    /** get terminal nat property */
    kv_build((F2FPKG*)recv_data);
    int i = 0;
    char *buddy= 0;
    pthread_mutex_lock(&mutex_kv);
    while (kv_pair[i] && i<KV_MAX){
        if (kv_pair[i]->key == KEY_BUDDY){
            buddy = strdup(kv_pair[i]->value);
            break;
        }
        i++;
    }
    pthread_mutex_unlock(&mutex_kv);
    kv_clear();

    /** check if all key-value pairs are available */
    if (buddy) {
        if ((0!=cbfuncs) && 0!=cbfuncs->OnBuddyReceived/*&& bCanCallback*/ ){
            cbfuncs->OnBuddyReceived((unsigned char*)buddy, strlen(buddy));
        }
    }

    /** clean up the variables */
    if (buddy!=0){free(buddy); buddy=0;}

}

void onLoginAck(F2FPKG* recv_data)
{
	F2FPKG_HEAD* hd = (F2FPKG_HEAD*) recv_data;
	int cmd = hd->cmd;

	if (SM_LOGINREQ!=state_machine) {
        BDEBUG(LV_NORMAL, "Cannot process cmd %s in state %s", get_cmd_str(cmd), get_sm_str(state_machine));
		return;
	}
	if (hd->errorCode == 0) {
		session_id = hd->sessionId;
		kv_build((F2FPKG*)recv_data);
		int i = 0;
        pthread_mutex_lock(&mutex_kv);
        while (kv_pair[i] && i<KV_MAX){
			if (kv_pair[i]->key == KEY_WANIP){
				strcpy(terminal_wanip, kv_pair[i]->value);
			}
			else if (kv_pair[i]->key == KEY_WANPORT){
				terminal_wanport = atoi(kv_pair[i]->value);
			}
			if (kv_pair[i]->key == KEY_NATSVRIP){
				strcpy(natdetectserver_ip, kv_pair[i]->value);
			}
			else if (kv_pair[i]->key == KEY_NATSVRPORT){
				natdetectserver_port = atoi(kv_pair[i]->value);
			}
			i++;
		}
    pthread_mutex_unlock(&mutex_kv);
        kv_clear();

		if (0==terminal_wanip[0] || 0==terminal_wanport || 0==natdetectserver_ip[0] || 0==natdetectserver_port){
            BDEBUG(LV_CRITICAL, "Not all key-value pairs are available.");
		}
		else {
			setStateMachine(SM_WAITINGFWACK, "Received login ack.");
			retry_ctrl_222222.reset(5);
		}
	}

}

void onNatDetectAck(F2FPKG* recv_data)
{
    show_package(recv_data, __LINE__);

	/** get terminal nat property */
	kv_build((F2FPKG*)recv_data);
	int i = 0;
    char *terminal_iscone = 0;
    char *terminal_nickname = 0;
    char *terminal_mobileno= 0;
    char *terminal_hasfw = 0;
    pthread_mutex_lock(&mutex_kv);
    while (kv_pair[i] && i<KV_MAX){
        switch (kv_pair[i]->key){
        case KEY_ISCONE:
            terminal_iscone = strdup(kv_pair[i]->value);
            break;
        case KEY_HASFW:
            terminal_hasfw = strdup(kv_pair[i]->value);
            break;
        case KEY_NICKNAME:
            terminal_nickname = strdup(kv_pair[i]->value);
            break;
        case KEY_MOBILENO:
            terminal_mobileno= strdup(kv_pair[i]->value);
            break;
        }

		i++;
	}
    pthread_mutex_unlock(&mutex_kv);
    kv_clear();

	/** check if all key-value pairs are available */
	if (terminal_iscone==0) {
        BDEBUG(LV_CRITICAL, "error: pkt is incorrect.");
        if (terminal_iscone!=0){free(terminal_iscone); terminal_iscone=0;}
        if (terminal_nickname !=0){free(terminal_nickname ); terminal_nickname =0;}
        if (terminal_mobileno!=0){free(terminal_mobileno); terminal_mobileno=0;}
        if (terminal_hasfw !=0){free(terminal_hasfw ); terminal_hasfw =0;}
        return;
	}

    bTerminal_isCone = strcmp(terminal_iscone, "Y")==0;
    char msg[256];
    sprintf(msg, "%d%d -- Firewall:%s, Symmetric:%s", bTerminalHas_firewall?1:0, bTerminal_isCone?0:1,
            bTerminalHas_firewall?"Yes":"No", bTerminal_isCone?"No":"Yes");
    setStateMachine(SM_HEARTBEATING, msg);
    retry_ctrl_heatbeat.reset(3);


    if ((0!=cbfuncs) && 0!=cbfuncs->OnRegInfoReceived){
        sprintf(msg, "%01d%01d", *terminal_hasfw=='Y'?1:0, *terminal_iscone=='Y'?0:1);
        cbfuncs->OnRegInfoReceived(terminal_nickname, terminal_mobileno, msg);
    }

	/** clean up the variables */
    if (terminal_iscone!=0){free(terminal_iscone); terminal_iscone=0;}
    if (terminal_nickname !=0){free(terminal_nickname ); terminal_nickname =0;}
    if (terminal_mobileno!=0){free(terminal_mobileno); terminal_mobileno=0;}
    if (terminal_hasfw !=0){free(terminal_hasfw ); terminal_hasfw =0;}

}

void onCallAddressingAck(F2FPKG* recv_data)
{
    show_package(recv_data, __LINE__);
    F2FPKG_HEAD* hd = (F2FPKG_HEAD*) recv_data;
	/** if error, change eCallStateMachine */
	if (hd->errorCode != 0) {
        char* errmsg = getErrorMsg(recv_data);
        if (errmsg)
            setStateMachineCall(SMCALL_LINEBUSY,errmsg, __FUNCTION__, __LINE__);
        else
            setStateMachineCall(SMCALL_LINEBUSY,"Receive addressing ack with an unknown error.", __FUNCTION__, __LINE__);
        if (errmsg){
            BDEBUG(LV_NORMAL, "%s pkg returns with error_msg: %s\n", get_cmd_str(hd->cmd), errmsg);
			free(errmsg); errmsg=0;
		}
		return;
    }

    if (SMCALL_ADDRESSINGREQ != state_machine_call) {
        BDEBUG(LV_NORMAL, "Cannot process %s in state %s", get_cmd_str(hd->cmd), get_smcall_str(state_machine_call));
        return;
    }

	/** get terminal nat property */
	kv_build((F2FPKG*)recv_data);
	int i = 0;
	char *callerid=0, *callerip=0, *callerport=0, *callerlip=0, *callerlport=0, *callernat=0;
	char *calleeid=0, *calleeip=0, *calleeport=0, *calleelip=0, *calleelport=0, *calleenat=0;
	char *calleeon=0, *calleebusy=0, *mirrorused=0, *mirroraddr=0;
    pthread_mutex_lock(&mutex_kv);
    while (kv_pair[i] && i<KV_MAX){
		switch(kv_pair[i]->key){
			case KEY_CALLERID:
				callerid = strdup(kv_pair[i]->value); break;
			case KEY_CALLERIP:
				callerip = strdup(kv_pair[i]->value); break;
			case KEY_CALLERPORT:
				callerport = strdup(kv_pair[i]->value); break;
			case KEY_CALLERLIP:
				callerlip = strdup(kv_pair[i]->value); break;
			case KEY_CALLERLPORT:
				callerlport = strdup(kv_pair[i]->value); break;
			case KEY_CALLERNAT:
				callernat = strdup(kv_pair[i]->value); break;
			case KEY_CALLEEID:
				calleeid = strdup(kv_pair[i]->value); break;
			case KEY_CALLEEIP:
				calleeip = strdup(kv_pair[i]->value); break;
			case KEY_CALLEEPORT:
				calleeport = strdup(kv_pair[i]->value); break;
			case KEY_CALLEELIP:
				calleelip = strdup(kv_pair[i]->value); break;
			case KEY_CALLEELPORT:
				calleelport = strdup(kv_pair[i]->value); break;
			case KEY_CALLEENAT:
				calleenat = strdup(kv_pair[i]->value); break;
			case KEY_CALLEEON:
				calleeon = strdup(kv_pair[i]->value); break;
			case KEY_CALLEEBUSY:
				calleebusy= strdup(kv_pair[i]->value); break;
			case KEY_MIRRORUSED:
				mirrorused = strdup(kv_pair[i]->value); break;
			case KEY_MIRRORADDR:
				mirroraddr = strdup(kv_pair[i]->value); break;
			default:
				break;
		}
		i++;
	}
    pthread_mutex_unlock(&mutex_kv);
    kv_clear();

	/** check if all key-value pairs are available */
	if (callerid==0||callerip==0||callerport==0||callerlip==0||callerlport==0||callernat==0||
			calleeid==0||calleeip==0||calleeport==0||calleelip==0||calleelport==0||calleenat==0||
			calleeon==0||calleebusy==0||mirrorused==0||mirroraddr==0)
	{
        BDEBUG(LV_CRITICAL, "Error: not all key-value pairs are available!");
		/* clean up variables */
		if(callerid!=0){free(callerid);callerid=0;}
		if(callerip!=0){free(callerip);callerip=0;}
		if(callerport!=0){free(callerport);callerport=0;}
		if(callerlip!=0){free(callerlip);callerlip=0;}
		if(callerlport!=0){free(callerlport);callerlport=0;}
		if(callernat!=0){free(callernat);callernat=0;}
		if(calleeid!=0){free(calleeid);calleeid=0;}
		if(calleeip!=0){free(calleeip);calleeip=0;}
		if(calleeport!=0){free(calleeport);calleeport=0;}
		if(calleelip!=0){free(calleelip);calleelip=0;}
		if(calleelport!=0){free(calleelport);calleelport=0;}
		if(calleenat!=0){free(calleenat);calleenat=0;}
		if(calleeon!=0){free(calleeon);calleeon=0;}
		if(calleebusy!=0){free(calleebusy);calleebusy=0;}
		if(mirrorused!=0){free(mirrorused);mirrorused=0;}
		if(mirroraddr!=0){free(mirroraddr);mirroraddr=0;}
		return;
    }

    {
        char szmsg[256];
        sprintf(szmsg, "   **** Caller--- id:%s, Wip:%s, Wport:%s, Lip:%s, Lport:%s, nat:%s"
                , callerid, callerip, callerport, callerlip,callerlport,callernat);
        statusNotify(szmsg);
        sprintf(szmsg, "   **** Callee--- id:%s, Wip:%s, Wport:%s, Lip:%s, Lport:%s, nat:%s"
                , calleeid, calleeip, calleeport, calleelip,calleelport,calleenat);
        statusNotify(szmsg);
        sprintf(szmsg, "   **** Mirror--- %s%s", (mirrorused && 'Y'==*mirrorused)?"Need":"Not need",mirroraddr?mirroraddr:"");
        statusNotify(szmsg);
    }

	/** if callee offline, change eCallStateMachine */
	if ('Y' != calleeon[0]){

        BDEBUG(LV_NORMAL, "%s pkg returns with callee offline", get_cmd_str(hd->cmd));
        setStateMachineCall(SMCALL_LINEBUSY,"Receive addressing ack with callee offline.", __FUNCTION__, __LINE__);

		/* clean up variables */
		if(callerid!=0){free(callerid);callerid=0;}
		if(callerip!=0){free(callerip);callerip=0;}
		if(callerport!=0){free(callerport);callerport=0;}
		if(callerlip!=0){free(callerlip);callerlip=0;}
		if(callerlport!=0){free(callerlport);callerlport=0;}
		if(callernat!=0){free(callernat);callernat=0;}
		if(calleeid!=0){free(calleeid);calleeid=0;}
		if(calleeip!=0){free(calleeip);calleeip=0;}
		if(calleeport!=0){free(calleeport);calleeport=0;}
		if(calleelip!=0){free(calleelip);calleelip=0;}
		if(calleelport!=0){free(calleelport);calleelport=0;}
		if(calleenat!=0){free(calleenat);calleenat=0;}
		if(calleeon!=0){free(calleeon);calleeon=0;}
		if(calleebusy!=0){free(calleebusy);calleebusy=0;}
		if(mirrorused!=0){free(mirrorused);mirrorused=0;}
		if(mirroraddr!=0){free(mirroraddr);mirroraddr=0;}
		return;
	}

	/** if callee busy, change eCallStateMachine */
	if ('Y' == calleebusy[0]){
        BDEBUG(LV_NORMAL, "%s pkg returns with callee busy", get_cmd_str(hd->cmd));
        setStateMachineCall(SMCALL_LINEBUSY,"Receive addressing ack with callee busy.", __FUNCTION__, __LINE__);
		/* clean up variables */
		if(callerid!=0){free(callerid);callerid=0;}
		if(callerip!=0){free(callerip);callerip=0;}
		if(callerport!=0){free(callerport);callerport=0;}
		if(callerlip!=0){free(callerlip);callerlip=0;}
		if(callerlport!=0){free(callerlport);callerlport=0;}
		if(callernat!=0){free(callernat);callernat=0;}
		if(calleeid!=0){free(calleeid);calleeid=0;}
		if(calleeip!=0){free(calleeip);calleeip=0;}
		if(calleeport!=0){free(calleeport);calleeport=0;}
		if(calleelip!=0){free(calleelip);calleelip=0;}
		if(calleelport!=0){free(calleelport);calleelport=0;}
		if(calleenat!=0){free(calleenat);calleenat=0;}
		if(calleeon!=0){free(calleeon);calleeon=0;}
		if(calleebusy!=0){free(calleebusy);calleebusy=0;}
		if(mirrorused!=0){free(mirrorused);mirrorused=0;}
		if(mirroraddr!=0){free(mirroraddr);mirroraddr=0;}
		return;
	}

	/** if mirror not used, store callee's info, and change eCallStateMachine */
	if ('N'==mirrorused[0]) {
		if (otherterminal_info_id) {free(otherterminal_info_id); otherterminal_info_id=0;}
		if (otherterminal_info_ip) {free(otherterminal_info_ip); otherterminal_info_ip=0;}
		if (otherterminal_info_lip) {free(otherterminal_info_lip); otherterminal_info_lip=0;}
		if (otherterminal_info_mirroraddr) {free(otherterminal_info_mirroraddr); otherterminal_info_mirroraddr=0; }
		otherterminal_info_id = strdup(calleeid);
		otherterminal_info_ip = strdup(calleeip);
		otherterminal_info_lip = strdup(calleelip);
		otherterminal_info_port = atoi(calleeport);
		otherterminal_info_lport = atoi(calleelport);
		retry_calling_req.reset(10);
        setStateMachineCall(SMCALL_CALLINGREQ, "Receive addressing ack, callee online, not busy, and no mirror used.", __FUNCTION__, __LINE__);

        /* clean up variables */
		if(callerid!=0){free(callerid);callerid=0;}
		if(callerip!=0){free(callerip);callerip=0;}
		if(callerport!=0){free(callerport);callerport=0;}
		if(callerlip!=0){free(callerlip);callerlip=0;}
		if(callerlport!=0){free(callerlport);callerlport=0;}
		if(callernat!=0){free(callernat);callernat=0;}
		if(calleeid!=0){free(calleeid);calleeid=0;}
		if(calleeip!=0){free(calleeip);calleeip=0;}
		if(calleeport!=0){free(calleeport);calleeport=0;}
		if(calleelip!=0){free(calleelip);calleelip=0;}
		if(calleelport!=0){free(calleelport);calleelport=0;}
		if(calleenat!=0){free(calleenat);calleenat=0;}
		if(calleeon!=0){free(calleeon);calleeon=0;}
		if(calleebusy!=0){free(calleebusy);calleebusy=0;}
		if(mirrorused!=0){free(mirrorused);mirrorused=0;}
		if(mirroraddr!=0){free(mirroraddr);mirroraddr=0;}
		return;
	}

	/** if mirror used, store callee's info and store mirroraddr in the mirrorlis array,
	  set mirrorused flag, if wanip is same try lan first */

	{
		if (otherterminal_info_id) {free(otherterminal_info_id); otherterminal_info_id=0;}
		if (otherterminal_info_ip) {free(otherterminal_info_ip); otherterminal_info_ip=0;}
		if (otherterminal_info_lip) {free(otherterminal_info_lip); otherterminal_info_lip=0;}
		if (otherterminal_info_mirroraddr) {free(otherterminal_info_mirroraddr); otherterminal_info_mirroraddr=0; }
		otherterminal_info_id = strdup(calleeid);
		otherterminal_info_ip = strdup(calleeip);
		otherterminal_info_lip = strdup(calleelip);
		otherterminal_info_port = atoi(calleeport);
		otherterminal_info_lport = atoi(calleelport);
		otherterminal_info_mirrorused = true;
		otherterminal_info_mirroraddr = strdup(mirroraddr);
		if (0==strcmp(otherterminal_info_ip,callerip)) {
			retry_calling_req.reset(2);
            setStateMachineCall(SMCALL_CALLINGREQ_LAN, "Receive addressing ack, not busy, mirror used, wanip is same.", __FUNCTION__, __LINE__);
		}
		else{
			retry_calling_req.reset(10);
            setStateMachineCall(SMCALL_CALLINGREQ, "Receive addressing ack, not busy, mirror used, wanip is not same.", __FUNCTION__, __LINE__);
		}

    }


	/** clean up variables */
	if(callerid!=0){free(callerid);callerid=0;}
	if(callerip!=0){free(callerip);callerip=0;}
	if(callerport!=0){free(callerport);callerport=0;}
	if(callerlip!=0){free(callerlip);callerlip=0;}
	if(callerlport!=0){free(callerlport);callerlport=0;}
	if(callernat!=0){free(callernat);callernat=0;}
	if(calleeid!=0){free(calleeid);calleeid=0;}
	if(calleeip!=0){free(calleeip);calleeip=0;}
	if(calleeport!=0){free(calleeport);calleeport=0;}
	if(calleelip!=0){free(calleelip);calleelip=0;}
	if(calleelport!=0){free(calleelport);calleelport=0;}
	if(calleenat!=0){free(calleenat);calleenat=0;}
	if(calleeon!=0){free(calleeon);calleeon=0;}
	if(calleebusy!=0){free(calleebusy);calleebusy=0;}
	if(mirrorused!=0){free(mirrorused);mirrorused=0;}
	if(mirroraddr!=0){free(mirroraddr);mirroraddr=0;}
}

void onCallCalleeinform(F2FPKG*recv_data)
{
	/** get key-values */
	kv_build((F2FPKG*)recv_data);
	int i = 0;
	char *callerid=0, *callerip=0, *callerport=0, *callerlip=0, *callerlport=0, *callernat=0;
	char *calleeid=0, *calleeip=0, *calleeport=0, *calleelip=0, *calleelport=0, *calleenat=0;
	char *scallerip=0, *scallerport=0, *mirrorused=0, *mirroraddr=0;
    pthread_mutex_lock(&mutex_kv);
    while (kv_pair[i] && i<KV_MAX){
		switch(kv_pair[i]->key){
			case KEY_CALLERID:
				callerid = strdup(kv_pair[i]->value); break;
			case KEY_CALLERIP:
				callerip = strdup(kv_pair[i]->value); break;
			case KEY_CALLERPORT:
				callerport = strdup(kv_pair[i]->value); break;
			case KEY_CALLERLIP:
				callerlip = strdup(kv_pair[i]->value); break;
			case KEY_CALLERLPORT:
				callerlport = strdup(kv_pair[i]->value); break;
			case KEY_CALLERNAT:
				callernat = strdup(kv_pair[i]->value); break;
			case KEY_CALLEEID:
				calleeid = strdup(kv_pair[i]->value); break;
			case KEY_CALLEEIP:
				calleeip = strdup(kv_pair[i]->value); break;
			case KEY_CALLEEPORT:
				calleeport = strdup(kv_pair[i]->value); break;
			case KEY_CALLEELIP:
				calleelip = strdup(kv_pair[i]->value); break;
			case KEY_CALLEELPORT:
				calleelport = strdup(kv_pair[i]->value); break;
			case KEY_CALLEENAT:
				calleenat = strdup(kv_pair[i]->value); break;
			case KEY_SCALLERIP:
				scallerip = strdup(kv_pair[i]->value); break;
			case KEY_SCALLERPORT:
				scallerport = strdup(kv_pair[i]->value); break;
			case KEY_MIRRORUSED:
				mirrorused = strdup(kv_pair[i]->value); break;
			case KEY_MIRRORADDR:
				mirroraddr = strdup(kv_pair[i]->value); break;
			default:
				break;
		}
		i++;
	}
    pthread_mutex_unlock(&mutex_kv);
    kv_clear();

	/** check if all key-value pairs are available */
	if (callerid==0||callerip==0||callerport==0||callerlip==0||callerlport==0||callernat==0||
			calleeid==0||calleeip==0||calleeport==0||calleelip==0||calleelport==0||calleenat==0||
			scallerip==0||scallerport==0||mirrorused==0||mirroraddr==0)
	{
        BDEBUG(LV_CRITICAL, "Error: not all key-value pairs are available!");
		/* clean up variables */
		if(callerid!=0){free(callerid);callerid=0;}
		if(callerip!=0){free(callerip);callerip=0;}
		if(callerport!=0){free(callerport);callerport=0;}
		if(callerlip!=0){free(callerlip);callerlip=0;}
		if(callerlport!=0){free(callerlport);callerlport=0;}
		if(callernat!=0){free(callernat);callernat=0;}
		if(calleeid!=0){free(calleeid);calleeid=0;}
		if(calleeip!=0){free(calleeip);calleeip=0;}
		if(calleeport!=0){free(calleeport);calleeport=0;}
		if(calleelip!=0){free(calleelip);calleelip=0;}
		if(calleelport!=0){free(calleelport);calleelport=0;}
		if(calleenat!=0){free(calleenat);calleenat=0;}
		if(scallerip!=0){free(scallerip);scallerip=0;}
		if(scallerport!=0){free(scallerport);scallerport=0;}
		if(mirrorused!=0){free(mirrorused);mirrorused=0;}
		if(mirroraddr!=0){free(mirroraddr);mirroraddr=0;}
		return;
	}

	/** if not busy, change eCallStateMachine to send fwtraversing */
	if (SMCALL_IDLE == state_machine_call) {
		if (otherterminal_info_id) {free(otherterminal_info_id); otherterminal_info_id=0;}
		if (otherterminal_info_ip) {free(otherterminal_info_ip); otherterminal_info_ip=0;}
		if (otherterminal_info_lip) {free(otherterminal_info_lip); otherterminal_info_lip=0;}
		if (otherterminal_info_mirroraddr) {free(otherterminal_info_mirroraddr); otherterminal_info_mirroraddr=0; }
		otherterminal_info_id = strdup(callerid);
		otherterminal_info_ip = strdup(callerip);
		otherterminal_info_lip = strdup(callerlip);
		otherterminal_info_port = atoi(callerport);
        otherterminal_info_lport = atoi(callerlport);
        otherterminal_info_mirrorused = (0!=mirrorused && 0==strncmp(mirrorused,"Y",1))?true:false;
        if(mirroraddr) otherterminal_info_mirroraddr = strdup(mirroraddr);
        if (('Y'==mirrorused[0]) && (0==strcmp(callerip, calleeip))){
			retry_fwtraversing.reset(3);
            setStateMachineCall(SMCALL_CALLEEINFORM_LAN, "Received calleeinform, .", __FUNCTION__, __LINE__);
		}
		else {
            retry_fwtraversing.reset(10);
            setStateMachineCall(SMCALL_CALLEEINFORM, "Received calleeinform.", __FUNCTION__, __LINE__);
        }
        char szmsg[256];
        sprintf(szmsg, "   **** Caller--- id:%s, Wip:%s, Wport:%s, Lip:%s, Lport:%s, nat:%s"
                , callerid, callerip, callerport, callerlip,callerlport,callernat);
        statusNotify(szmsg);
        sprintf(szmsg, "   **** Callee--- id:%s, Wip:%s, Wport:%s, Lip:%s, Lport:%s, nat:%s"
                , calleeid, calleeip, calleeport, calleelip,calleelport,calleenat);
        statusNotify(szmsg);
        sprintf(szmsg, "   **** Mirror--- %s%s", (mirrorused && 'Y'==*mirrorused)?"Need":"Not need",mirroraddr?mirroraddr:"");
        statusNotify(szmsg);
    }

    /** retrun CALLEEINFORMACK to loginserver  */
    F2FPKG *pkg = (F2FPKG*)send_data_threadudp;
    pkg_init(pkg, CMD_CALL_CALLEEINFORMACK, 0);
    pkg_append(pkg, KEY_CALLERID, (char*)callerid);
    pkg_append(pkg, KEY_CALLEEID, (char*)calleeid);
    pkg_append(pkg, KEY_CALLERIP, (char*)callerip);
    pkg_append(pkg, KEY_CALLERPORT, (char*)callerport);
    pkg_append(pkg, KEY_CALLERLIP, (char*)callerlip);
    pkg_append(pkg, KEY_CALLERLPORT, (char*)callerlport);
    pkg_append(pkg, KEY_CALLERNAT, (char*)callernat);
    pkg_append(pkg, KEY_CALLEEIP, (char*)calleeip);
    pkg_append(pkg, KEY_CALLEEPORT, (char*)calleeport);
    pkg_append(pkg, KEY_CALLEELIP, (char*)calleelip);
    pkg_append(pkg, KEY_CALLEELPORT, (char*)calleelport);
    pkg_append(pkg, KEY_CALLEENAT, (char*)calleenat);
    if (SMCALL_IDLE == state_machine_call
            || ( otherterminal_info_id && 0==strcmp(otherterminal_info_id, callerid))) {
        pkg_append(pkg, KEY_CALLEEBUSY, "N");
    }
    else {
        pkg_append(pkg, KEY_CALLEEBUSY, "Y");
    }
    pkg_append(pkg, KEY_SCALLERIP, scallerip);
    pkg_append(pkg, KEY_SCALLERPORT, scallerport);
    pkg_append(pkg, KEY_MIRRORUSED, mirrorused);
    pkg_append(pkg, KEY_MIRRORADDR, mirroraddr);
    send_udp_pkg_loginserver((char*)pkg, pkg_len(pkg));


	/* clean up variables */
	if(callerid!=0){free(callerid);callerid=0;}
	if(callerip!=0){free(callerip);callerip=0;}
	if(callerport!=0){free(callerport);callerport=0;}
	if(callerlip!=0){free(callerlip);callerlip=0;}
	if(callerlport!=0){free(callerlport);callerlport=0;}
	if(callernat!=0){free(callernat);callernat=0;}
	if(calleeid!=0){free(calleeid);calleeid=0;}
	if(calleeip!=0){free(calleeip);calleeip=0;}
	if(calleeport!=0){free(calleeport);calleeport=0;}
	if(calleelip!=0){free(calleelip);calleelip=0;}
	if(calleelport!=0){free(calleelport);calleelport=0;}
	if(calleenat!=0){free(calleenat);calleenat=0;}
	if(scallerip!=0){free(scallerip);scallerip=0;}
	if(scallerport!=0){free(scallerport);scallerport=0;}
	if(mirrorused!=0){free(mirrorused);mirrorused=0;}
	if(mirroraddr!=0){free(mirroraddr);mirroraddr=0;}

}

void onCallFwTraversing(F2FPKG*recv_data, struct sockaddr_in* remoteaddr)
{
	/** get key-values */
	kv_build((F2FPKG*)recv_data);
	char* sourceid=0, *destid=0;
	int i = 0;
    pthread_mutex_lock(&mutex_kv);
    while (kv_pair[i] && i<KV_MAX){
		switch(kv_pair[i]->key){
			case KEY_SOURCEID:
				sourceid = strdup(kv_pair[i]->value);
				break;
			case KEY_DESTID:
				destid = strdup(kv_pair[i]->value);
				break;
		}
		i++;
	}
    pthread_mutex_unlock(&mutex_kv);
    kv_clear();

	if (0==sourceid || 0==destid){
		/* clean up variables */
		if (sourceid){free(sourceid);sourceid=0;}
		if (destid){free(destid);destid=0;}
		return;
	}

	bool bIsCaller = false;
	bool bIsMirror = false;
	/** am i the caller?  */
	// if state_machine_call!=SMCALL_IDLE, and the callerid and calleeid are equal to those in recv_data, it
	//  means i'm the caller.
	if ((SMCALL_IDLE != state_machine_call) && (0==strcmp(sourceid, test_calleeid)) && (0==strcmp(destid, terminal_id)) ){
		bIsCaller = true;
	}
	/** am i the mirror? */
	// when received CMD_CALL_MIRRORINFORM, terminal store the ids of two sides terminal. if state_machine_mirror!=
	//  SMMIRROR_STANDBY, and stored ids are equal to those in recv_data, it means i'm the irror
	else if ((SMMIRROR_STANDBY != state_machine_mirror) &&
			(0==strcmp(sourceid, mirror_fwd_calleeid)) && (0==strcmp(destid, mirror_fwd_callerid))){
		bIsMirror = true;
	}


	/** if i'm the caller, renew callee's ip (may be mirror's), and change eCallStateMachine to send CMD_CALL_CALLINGREQ */
	if (bIsCaller) {
		if (otherterminal_info_ip){free(otherterminal_info_ip); otherterminal_info_ip=0;}
		if (otherterminal_info_id){free(otherterminal_info_id); otherterminal_info_id=0;}
		otherterminal_info_id = strdup(sourceid);
		otherterminal_info_ip = strdup(inet_ntoa(remoteaddr->sin_addr));
		otherterminal_info_port = ntohs(remoteaddr->sin_port);
        if (SMCALL_CALLINGREQ!=state_machine_call){
            if ((SMCALL_CALLINGREQ_LAN == state_machine_call)
                    //&& (0==strcmp(otherterminal_info_lip, otherterminal_info_ip))){
                    && !isPktFromMirror(otherterminal_info_ip, otherterminal_info_mirroraddr)){
                //if firewall traversing pkt is from Lan, then we don't need the mirror.
                otherterminal_info_mirrorused = false;
            }
            retry_calling_req.reset(10);
            char sztmp[256];
            sprintf(sztmp, "Receive fwtraversing. <== %s:%d", otherterminal_info_ip, otherterminal_info_port);
            setStateMachineCall(SMCALL_CALLINGREQ, sztmp, __FUNCTION__, __LINE__);
		}

	}
	/** else if i'm the mirror, renew callee's ip, and forward fwtravering to caller */
	else if (bIsMirror) {
		if (mirror_fwd_calleeip){free(mirror_fwd_calleeip);mirror_fwd_calleeip=0;}
		mirror_fwd_calleeip = strdup(inet_ntoa(remoteaddr->sin_addr));
		mirror_fwd_calleeport = ntohs(remoteaddr->sin_port);

		F2FPKG *pkg = (F2FPKG*)send_data_threadudp;
		pkg_init(pkg, CMD_CALL_FWTRAVERSING, 0);
		pkg_append(pkg, KEY_SOURCEID, (char*)sourceid);
		pkg_append(pkg, KEY_DESTID, (char*)destid);
		send_udp_pkg(mirror_fwd_callerip, mirror_fwd_callerport, (char*)pkg, pkg_len(pkg));
	}

	/* clean up variables */
	if (sourceid){free(sourceid);sourceid=0;}
	if (destid){free(destid);destid=0;}
}

void onCallCallingReq(F2FPKG*recv_data, sockaddr_in *remoteaddr)
{
	/** get key-values */
	kv_build((F2FPKG*)recv_data);
    char* sourceid=0, *destid=0, *st=0;
	int i = 0;
    pthread_mutex_lock(&mutex_kv);
    while (kv_pair[i] && i<KV_MAX){
		switch(kv_pair[i]->key){
			case KEY_SOURCEID:
				sourceid = strdup(kv_pair[i]->value);
				break;
			case KEY_DESTID:
				destid = strdup(kv_pair[i]->value);
                break;
            case KEY_SESSIONTYPE:
                st = strdup(kv_pair[i]->value);
                break;
		}
		i++;
	}
    pthread_mutex_unlock(&mutex_kv);
    kv_clear();

    if (0==sourceid || 0==destid || 0==st){
		/* clean up variables */
		if (sourceid){free(sourceid);sourceid=0;}
		if (destid){free(destid);destid=0;}
        if (st){free(st);st=0;}
        return;
	}

	bool bIsCallee = false;
	bool bIsMirror = false;
	/** am i the callee?  */
	// if state_machine_call!=SMCALL_IDLE, and the callerid and calleeid are equal to those in recv_data, it
	//  means i'm the callee.
	if ((SMCALL_IDLE != state_machine_call) && (0==strcmp(sourceid, otherterminal_info_id)) && (0==strcmp(destid, terminal_id)) ){
		bIsCallee = true;
	}

	/** am i the mirror? */
	// when received CMD_CALL_MIRRORINFORM, terminal store the ids of two sides terminal. if state_machine_mirror!=
	//  SMMIRROR_STANDBY, and stored ids are equal to those in recv_data, it means i'm the irror
	else if ((SMMIRROR_STANDBY != state_machine_mirror) &&
			(0==strcmp(sourceid, mirror_fwd_callerid)) && (0==strcmp(destid, mirror_fwd_calleeid))){
		bIsMirror = true;
	}



	/** if i'm the callee, renew caller's ip (may be mirror's), send CMD_CALL_CALLINGACK (to simplified, do not send at present.)
	  and change eCallStateMachine to send CMD_CALL_RING */
	if (bIsCallee) {
		if (otherterminal_info_ip){free(otherterminal_info_ip); otherterminal_info_ip=0;}
		otherterminal_info_ip = strdup(inet_ntoa(remoteaddr->sin_addr));
		otherterminal_info_port = ntohs(remoteaddr->sin_port);
		retry_ringOrRingback.reset(5);
        if ((SMCALL_CALLEEINFORM_LAN == state_machine_call)
                //&& (0==strcmp(otherterminal_info_lip, otherterminal_info_ip))){
                && !isPktFromMirror(otherterminal_info_ip, otherterminal_info_mirroraddr)){
            //if calling req pkt is from Lan, then we don't need the mirror.
            otherterminal_info_mirrorused = false;
        }
        char sztmp[256];
        if (otherterminal_info_mirrorused){
            if (otherterminal_info_mirroraddr)
                sprintf(sztmp, "Receive calling req. <==%s(mirrors)"
                        , otherterminal_info_mirroraddr);
            else
                sprintf(sztmp, "Need mirror, but no mirror is found.");
        }
        else
            sprintf(sztmp, "Receive calling req.  <==%s:%d"
                    , otherterminal_info_ip, otherterminal_info_port);
        setStateMachineCall(SMCALL_RING, sztmp, __FUNCTION__, __LINE__);

        //callback to ui
        if ((0!=cbfuncs) && 0!=cbfuncs->OnCalleeNotify){
            cbfuncs->OnCalleeNotify(sourceid, (SESSIONTYPE)atoi(st));
        }


	}
	/** else if i'm the mirror, renew caller's ip, and forward fwtravering to callee */
	else if (bIsMirror) {
		if (mirror_fwd_callerip){free(mirror_fwd_callerip);mirror_fwd_callerip=0;}
		mirror_fwd_callerip = strdup(inet_ntoa(remoteaddr->sin_addr));
		mirror_fwd_callerport = ntohs(remoteaddr->sin_port);

		F2FPKG *pkg = (F2FPKG*)send_data_threadudp;
		pkg_init(pkg, CMD_CALL_CALLINGREQ, 0);
		pkg_append(pkg, KEY_SOURCEID, (char*)sourceid);
		pkg_append(pkg, KEY_DESTID, (char*)destid);
        pkg_append(pkg, KEY_SESSIONTYPE, (char*)st);
        send_udp_pkg(mirror_fwd_calleeip, mirror_fwd_calleeport, (char*)pkg, pkg_len(pkg));
	}

	/* clean up variables */
	if (sourceid){free(sourceid);sourceid=0;}
	if (destid){free(destid);destid=0;}
    if (st){free(st);st=0;}

}

void onCallCallingAck(F2FPKG*recv_data)
{
	/** am i the caller?  */
	// if state_machine_call!=SMCALL_IDLE, and the callerid and calleeid are equal to those in recv_data, it
	//  means i'm the caller.


	/** am i the mirror? */
	// when received CMD_CALL_MIRRORINFORM, terminal store the ids of two sides terminal. if state_machine_mirror!=
	//  SMMIRROR_STANDBY, and stored ids are equal to those in recv_data, it means i'm the irror

}

void onCallRing(F2FPKG*recv_data, sockaddr_in *remoteaddr)
{
	/** get key-values */
	kv_build((F2FPKG*)recv_data);
	char* sourceid=0, *destid=0;
	int i = 0;
    pthread_mutex_lock(&mutex_kv);
    while (kv_pair[i] && i<KV_MAX){
		switch(kv_pair[i]->key){
			case KEY_SOURCEID:
				sourceid = strdup(kv_pair[i]->value);
				break;
			case KEY_DESTID:
				destid = strdup(kv_pair[i]->value);
				break;
		}
		i++;
	}
    pthread_mutex_unlock(&mutex_kv);
    kv_clear();

	if (0==sourceid || 0==destid){
		/* clean up variables */
		if (sourceid){free(sourceid);sourceid=0;}
		if (destid){free(destid);destid=0;}
		return;
	}

    bool bIsCaller = false;
	bool bIsMirror = false;
    /** am i the caller?  */
	// if state_machine_call!=SMCALL_IDLE, and the callerid and calleeid are equal to those in recv_data, it
    //  means i'm the caller.
    if ((SMCALL_IDLE != state_machine_call) && (0==strcmp(sourceid, test_calleeid)) && (0==strcmp(destid, terminal_id)) ){
        bIsCaller = true;
	}
	/** am i the mirror? */
	// when received CMD_CALL_MIRRORINFORM, terminal store the ids of two sides terminal. if state_machine_mirror!=
	//  SMMIRROR_STANDBY, and stored ids are equal to those in recv_data, it means i'm the irror
    else if ((SMMIRROR_STANDBY != state_machine_mirror) &&
            (0==strcmp(sourceid, mirror_fwd_calleeid)) && (0==strcmp(destid, mirror_fwd_callerid))){
        bIsMirror = true;
	}


    /** if i'm the caller, and change eCallStateMachine to send CMD_CALL_RINGBACK */
    if (bIsCaller) {
        if (otherterminal_info_ip){free(otherterminal_info_ip); otherterminal_info_ip=0;}
        if (otherterminal_info_id){free(otherterminal_info_id); otherterminal_info_id=0;}
        otherterminal_info_id = strdup(sourceid);
        otherterminal_info_ip = strdup(inet_ntoa(remoteaddr->sin_addr));
        otherterminal_info_port = ntohs(remoteaddr->sin_port);
        if (SMCALL_RINGBACK!=state_machine_call){
            if ((SMCALL_CALLINGREQ_LAN == state_machine_call)
                    //&& (0==strcmp(otherterminal_info_lip, otherterminal_info_ip))){
                    && !isPktFromMirror(otherterminal_info_ip, otherterminal_info_mirroraddr)){
                //if firewall traversing pkt is from Lan, then we don't need the mirror.
                otherterminal_info_mirrorused = false;
            }
            retry_ringOrRingback.reset(20);
            char sztmp[256];
            if (otherterminal_info_mirrorused){
                if (otherterminal_info_mirroraddr)
                    sprintf(sztmp, "Received ring. <==%s(mirrors)"
                            , otherterminal_info_mirroraddr);
                else
                    sprintf(sztmp, "Need mirror, but no mirror is found.");
            }
            else
                sprintf(sztmp, "Received ring. <==%s:%d(wan)"
                        , otherterminal_info_ip, otherterminal_info_port);
            setStateMachineCall(SMCALL_RINGBACK, sztmp, __FUNCTION__, __LINE__);
        }
	}
    /** else if i'm the mirror, forward ring to caller */
	else if (bIsMirror) {
        if (mirror_fwd_calleeip){free(mirror_fwd_calleeip);mirror_fwd_calleeip=0;}
        mirror_fwd_calleeip = strdup(inet_ntoa(remoteaddr->sin_addr));
        mirror_fwd_calleeport = ntohs(remoteaddr->sin_port);

        F2FPKG *pkg = (F2FPKG*)send_data_threadudp;
		pkg_init(pkg, CMD_CALL_RING, 0);
		pkg_append(pkg, KEY_SOURCEID, (char*)sourceid);
		pkg_append(pkg, KEY_DESTID, (char*)destid);
		send_udp_pkg(mirror_fwd_callerip, mirror_fwd_callerport, (char*)pkg, pkg_len(pkg));
	}

	/* clean up variables */
	if (sourceid){free(sourceid);sourceid=0;}
	if (destid){free(destid);destid=0;}
}

void onCallRingback(F2FPKG*recv_data, sockaddr_in *remoteaddr)
{
	//----------------- seemed nothing to do.

	/** am i the callee?  */
	// if state_machine_call!=SMCALL_IDLE, and the callerid and calleeid are equal to those in recv_data, it
	//  means i'm the callee.


	/** am i the mirror? */
	// when received CMD_CALL_MIRRORINFORM, terminal store the ids of two sides terminal. if state_machine_mirror!=
	//  SMMIRROR_STANDBY, and stored ids are equal to those in recv_data, it means i'm the irror

}


void onMirrorinform(F2FPKG* recv_data)
{
    /** get key-values */
    kv_build((F2FPKG*)recv_data);
    int i = 0;
    pthread_mutex_lock(&mutex_kv);
    while (kv_pair[i] && i<KV_MAX){
        switch(kv_pair[i]->key){
        case KEY_CALLERID:
            if (mirror_fwd_callerid) {free(mirror_fwd_callerid); mirror_fwd_callerid=0;}
            mirror_fwd_callerid = strdup(kv_pair[i]->value);
            break;
        case KEY_CALLERIP:
            if (mirror_fwd_callerip) {free(mirror_fwd_callerip); mirror_fwd_callerip=0;}
            mirror_fwd_callerip = strdup(kv_pair[i]->value);
            break;
        case KEY_CALLERPORT:
            mirror_fwd_callerport = atoi(kv_pair[i]->value);
            break;
        case KEY_CALLEEID:
            if (mirror_fwd_calleeid) {free(mirror_fwd_calleeid); mirror_fwd_calleeid=0;}
            mirror_fwd_calleeid = strdup(kv_pair[i]->value);
            break;
        case KEY_CALLEEIP:
            if (mirror_fwd_calleeip) {free(mirror_fwd_calleeip); mirror_fwd_calleeip=0;}
            mirror_fwd_calleeip = strdup(kv_pair[i]->value);
            break;
        case KEY_CALLEEPORT:
            mirror_fwd_calleeport = atoi(kv_pair[i]->value);
            break;
        }
        i++;
    }
    pthread_mutex_unlock(&mutex_kv);
    kv_clear();
    retry_mirrorfwding_timeout.reset(2);
    setStateMachineMirror(SMMIRROR_FORWARDING, "Received mirror inform.");

}

void onCallHangup(F2FPKG*recv_data)
{
	/** get key-values */
	kv_build((F2FPKG*)recv_data);
	char* sourceid=0, *destid=0;
	int i = 0;
    pthread_mutex_lock(&mutex_kv);
    while (kv_pair[i] && i<KV_MAX){
		switch(kv_pair[i]->key){
			case KEY_SOURCEID:
				sourceid = strdup(kv_pair[i]->value);
				break;
			case KEY_DESTID:
				destid = strdup(kv_pair[i]->value);
				break;
		}
		i++;
	}
    pthread_mutex_unlock(&mutex_kv);
    kv_clear();

	if (0==sourceid || 0==destid){
		/* clean up variables */
		if (sourceid){free(sourceid);sourceid=0;}
		if (destid){free(destid);destid=0;}
		return;
	}

	bool bIsCallerOrCallee = false;
	bool bIsMirror = false;
	/** am i the caller or callee?  */
	// if state_machine_call!=SMCALL_IDLE, and the callerid and calleeid are equal to those in recv_data, it
	//  means i'm the callee.
	if ((SMCALL_IDLE != state_machine_call)
			&& (0==strcmp(sourceid, otherterminal_info_id) || 0==strcmp(sourceid, terminal_id))
			&& (0==strcmp(destid, otherterminal_info_id) || 0==strcmp(destid, terminal_id)) ){
		bIsCallerOrCallee = true;
	}


	/** am i the mirror? */
	// when received CMD_CALL_MIRRORINFORM, terminal store the ids of two sides terminal. if state_machine_mirror!=
	//  SMMIRROR_STANDBY, and stored ids are equal to those in recv_data, it means i'm the irror
	else if ((SMMIRROR_STANDBY != state_machine_mirror)
			&& (0==strcmp(sourceid, mirror_fwd_calleeid) || 0==strcmp(sourceid, mirror_fwd_callerid))
			&& (0==strcmp(destid, mirror_fwd_callerid)  || 0==strcmp(destid, mirror_fwd_calleeid))){
		bIsMirror = true;
	}

	/** if i'm the caller or callee, change eCallStateMachine to hangup */
	if (bIsCallerOrCallee){
        setStateMachineCall(SMCALL_OTHERSIDE_HANGUP, "Received hangup.", __FUNCTION__, __LINE__);
	}
	/** if i'm the mirror, forward hangup to other side */
	else if (bIsMirror) {
		F2FPKG *pkg = (F2FPKG*)send_data_threadudp;
		pkg_init(pkg, CMD_CALL_RING, 0);
		pkg_append(pkg, KEY_SOURCEID, (char*)sourceid);
		pkg_append(pkg, KEY_DESTID, (char*)destid);
		if (0==strcmp(sourceid, mirror_fwd_calleeid))
			send_udp_pkg(mirror_fwd_callerip, mirror_fwd_callerport, (char*)pkg, pkg_len(pkg));
		else
			send_udp_pkg(mirror_fwd_calleeip, mirror_fwd_calleeport, (char*)pkg, pkg_len(pkg));
	}
    /* clean up variables */
    if (sourceid){free(sourceid);sourceid=0;}
    if (destid){free(destid);destid=0;}

}

void onCallerMedia(F2FPKG*recv_data)
{
    int data=0;
    int len=0;
    kv_build_nodata(recv_data, data, len);
    char* sourceid=0, *destid=0;
    int i = 0;
    pthread_mutex_lock(&mutex_kv);
    while (kv_pair[i] && i<KV_MAX){
        switch(kv_pair[i]->key){
            case KEY_SOURCEID:
                sourceid = strdup(kv_pair[i]->value);
                break;
            case KEY_DESTID:
                destid = strdup(kv_pair[i]->value);
                break;
        }
        i++;
    }
    pthread_mutex_unlock(&mutex_kv);
    kv_clear();

    if (0==sourceid || 0==destid){
        /* clean up variables */
        if (sourceid){free(sourceid);sourceid=0;}
        if (destid){free(destid);destid=0;}
        return;
    }

    bool bIsCallee = false;
    bool bIsMirror = false;

    /** am i the callee?  */
    if ((SMCALL_IDLE != state_machine_call) && (0==strcmp(sourceid, otherterminal_info_id)) && (0==strcmp(destid, terminal_id)) ){
        bIsCallee = true;
    }
    /** am i the mirror? */
    else if ((SMMIRROR_STANDBY != state_machine_mirror) &&
             (0==strcmp(sourceid, mirror_fwd_callerid)) && (0==strcmp(destid, mirror_fwd_calleeid))){
        bIsMirror = true;
    }

    /** if i'm callee, callback to UI */
    if (bIsCallee){
#if 0
        int data=0;
        int len=0;
        getDataFromPkt(recv_data, data, len);
#endif
        if (0!=data && 0!=len){

            if ((0!=cbfuncs) && 0!=cbfuncs->OnCallerVideo /*&& bCanCallback */){
                cbfuncs->OnCallerVideo((unsigned char*)data,len);
            }
        }
        retry_calleraudio_timeout.reset(1);
    }
    else if (bIsMirror){
        /** if i'm mirror, forward it to callee */
        send_udp_pkg(mirror_fwd_calleeip, mirror_fwd_calleeport, (char*)recv_data, pkg_len(recv_data));
        retry_mirrorfwding_timeout.reset(2);
    }
    else {
        /** should not be here */
    }

    /* clean up variables */
    if (sourceid){free(sourceid);sourceid=0;}
    if (destid){free(destid);destid=0;}

}


void onCalleeMedia(F2FPKG*recv_data)
{
    int data=0;
    int len=0;
    kv_build_nodata(recv_data, data, len);
    char* sourceid=0, *destid=0;
    int i = 0;
    pthread_mutex_lock(&mutex_kv);
    while (kv_pair[i] && i<KV_MAX){
        switch(kv_pair[i]->key){
        case KEY_SOURCEID:
            sourceid = strdup(kv_pair[i]->value);
            break;
        case KEY_DESTID:
            destid = strdup(kv_pair[i]->value);
            break;
        }
        i++;
    }
    pthread_mutex_unlock(&mutex_kv);
    kv_clear();

    if (0==sourceid || 0==destid){
        /* clean up variables */
        if (sourceid){free(sourceid);sourceid=0;}
        if (destid){free(destid);destid=0;}
        return;
    }

    bool bIsCaller = false;
    bool bIsMirror = false;

    /** am i the caller?  */
    // if state_machine_call!=SMCALL_IDLE, and the callerid and calleeid are equal to those in recv_data, it
    //  means i'm the caller.
    if ((SMCALL_IDLE != state_machine_call) && (0==strcmp(sourceid, test_calleeid)) && (0==strcmp(destid, terminal_id)) ){
        bIsCaller = true;
    }
    /** am i the mirror? */
    // when received CMD_CALL_MIRRORINFORM, terminal store the ids of two sides terminal. if state_machine_mirror!=
    //  SMMIRROR_STANDBY, and stored ids are equal to those in recv_data, it means i'm the irror
    else if ((SMMIRROR_STANDBY != state_machine_mirror) &&
             (0==strcmp(sourceid, mirror_fwd_calleeid)) && (0==strcmp(destid, mirror_fwd_callerid))){
        bIsMirror = true;
    }

    /** if i'm caller, callback to UI */
    // test: change eStateMachine, to send calleraudio
    if (bIsCaller) {

        if ((SMCALL_CALLINGREQ_LAN==state_machine_call) ||
                (SMCALL_CALLINGREQ==state_machine_call) ||
                (SMCALL_RINGBACK==state_machine_call)){
            if (SMCALL_CALLINGREQ_LAN==state_machine_call && otherterminal_info_mirrorused) {
                //received lan calleeaudio, so do not need mirror
                otherterminal_info_mirrorused = false;
            }
            setStateMachineCall(SMCALL_CALLERAUDIO, "Received callee audio", __FUNCTION__, __LINE__);
            retry_calleeaudio_timeout.reset(1);
        }
        if (!bOnReadyCalled){
            bOnReadyCalled = true;
            if ((0!=cbfuncs) && 0!=cbfuncs->OnReady /*&& bCanCallback*/ ){
                cbfuncs->OnReady();
            }
        }
#if 0
        int data=0;
        int len=0;
        getDataFromPkt(recv_data, data, len);
#endif
        if (0!=data && 0!=len){
            if ((0!=cbfuncs) && 0!=cbfuncs->OnCalleeVideo /*&& bCanCallback*/ ){
                cbfuncs->OnCalleeVideo((unsigned char*)data,len);
            }
        }
        retry_calleeaudio_timeout.reset(1);
    }
    else if (bIsMirror){
        /** if i'm mirror, forward it to caller */
        send_udp_pkg(mirror_fwd_callerip, mirror_fwd_callerport, (char*)recv_data, pkg_len(recv_data));
        retry_mirrorfwding_timeout.reset(2);
    }
    else {
        /** should not be here. */
    }
    /* clean up variables */
    if (sourceid){free(sourceid);sourceid=0;}
    if (destid){free(destid);destid=0;}

}


void statusNotify(char* str)
{
    pthread_mutex_lock(&mutex_log);
    list_logtoui.push_back(strdup(str));
    pthread_mutex_unlock(&mutex_log);
}
