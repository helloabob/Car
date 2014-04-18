#ifndef F2FDEFS_H
#define F2FDEFS_H


/*
 1. In the F2F system, all packets are UDP protocol, including signaling and media
 2. F2F UDP, has a F2F header, followed by F2F data (payload)
 3. The payload, is composed by <key,value> pairs, there is a delimiter between key and value.
 4. Delimiter is two bytes: 0xA8F0
 5. In media or data payload, media or data is the value of KEY_AUDIO/KEY_VIDEO/KEY_FILE, the Timestamp,
    fragment id, frame id, etc., are defined in media or data.
*/
//#define USING_LOCALHOST
enum eCMD
{
    CMD_LB_LOGINSERVERREQ           = 0x0001,           //Req for LS addr
    CMD_LB_LOGINSERVERACK           = 0x0002,           //Ack for LS addr
    CMD_NAT_LOGINREQ                = 0x0011,           //Req for login
    CMD_NAT_LOGINACK                = 0x0012,           //Ack for login
    CMD_NAT_FWPORTRESTRICT          = 0x0013,           //P-ltd NAT detect
    CMD_NAT_FWDETECTREQ             = 0x0014,           //Req for FW detect
    CMD_NAT_FWDETECTACK             = 0x0015,           //Ack for FW detect
    CMD_NAT_NATDETECTREQ            = 0x0016,           //Req for NAT detect
    CMD_NAT_NATDETECTRPT            = 0x0017,           //Rep for NAT detect
    CMD_NAT_NATDETECTACK            = 0x0018,           //Ack for NAT detect
    CMD_HEARTBEAT_REQ               = 0x0019,           //Req for heart-beat
    CMD_HEARTBEAT_ACK               = 0x001A,           //Ack for heart-beat
    CMD_HEARTBEAT_STAT              = 0x001B,           //Note for state change
    CMD_HEARTBEAT_STATACK           = 0x001C,           //Ack for state change
    CMD_CALL_ADDRESSINGREQ          = 0x0021,           //Req for addressing
    CMD_CALL_MIRRORQUERY            = 0x0022,           //Query for mirror
    CMD_CALL_MIRRORNEEDED           = 0x0023,           //Mirror needed
    CMD_CALL_MIRRORNOTNEEDED        = 0x0024,           //Mirror not needed
    CMD_CALL_MIRRORNOTFOUND         = 0x0025,           //Mirror not found
    CMD_CALL_CALLEEOFFLINE          = 0x0026,           //Callee offline
    CMD_CALL_CALLEEINFORM           = 0x0027,           //Callee inform
    CMD_CALL_CALLEEINFORMACK        = 0x0028,           //Ack for Callee inform
    CMD_CALL_MIRRORINFORM           = 0x0029,           //Mirror inform
    CMD_CALL_ADDRESSINGACK          = 0x002A,           //Ack for addressing
    CMD_CALL_FWTRAVERSING           = 0x002B,           //Firewall traversing
    CMD_CALL_CALLINGREQ             = 0x002C,           //Req for calling
    CMD_CALL_CALLINGACK             = 0x002D,           //Ack for calling
    CMD_CALL_RING                   = 0x002E,           //Ring state
    CMD_CALL_RINGBACK               = 0x002F,           //Ringback state
    CMD_CALL_CALLEEAUDIO            = 0x0030,           //Audio from callee
    CMD_CALL_CALLERAUDIO            = 0x0031,           //Audio from caller
    CMD_CALL_CALLEEVIDEO            = 0x0032,           //Video from callee
    CMD_CALL_CALLERVIDEO            = 0x0033,           //Video from caller
    CMD_CALL_CALLEEPICTURE          = 0x0034,           //Picture from callee
    CMD_CALL_CALLERPICTURE          = 0x0035,           //Picture from caller
    CMD_CALL_CALLEEFILE             = 0x0036,           //File from callee
    CMD_CALL_CALLERFILE             = 0x0037,           //File from caller
    CMD_CALL_HANGUP                 = 0x0038,           //Hangup
    CMD_REGINFOALTER_REQ            = 0x0039,           //Req for altering register info
    CMD_REGINFOALTER_ACK            = 0x003A,           //Ack for altering register info
    CMD_REGINFOGET_REQ              = 0x003B,           //Req for get register info
    CMD_REGINFOGET_ACK              = 0x003C,           //Ack for get register info
    CMD_REGINFOGETBYMOBILE_REQ      = 0x003D,           //Req for get register info by mobileno
    CMD_REGINFOGETBYMOBILE_ACK      = 0x003E,           //Ack for get register info by mobileno
};


enum eKEY
{
    KEY_ID              = 1,                //Terminal id
    KEY_PASSWD          = 2,                //Terminal passwd
    KEY_LOGSVRIP        = 3,                //Login server IP
    KEY_LOGSVRPORT      = 4,                //Login server Port
    KEY_LANIP           = 5,                //Terminal lan IP
    KEY_LANPORT         = 6,                //Terminal lan Port
    KEY_CAPABILITY      = 7,                //Session capability
    KEY_WANIP           = 8,                //Terminal wan IP
    KEY_WANPORT         = 9,                //Terminal wan Port
    KEY_NATSVRIP        = 10,               //NAT detect svr IP
    KEY_NATSVRPORT      = 11,               //NAT detect svr Port
    KEY_HASFW           = 12,               //Terminal has FW
    KEY_ISCONE          = 13,               //Terminal is cone NAT
    KEY_FWDING          = 14,               //Forwarding state
    KEY_MEETING         = 15,               //Meeting state
    KEY_CALLING         = 16,               //Calling state
    KEY_CALLERID        = 17,               //Caller terminal ID
    KEY_CALLEEID        = 18,               //Callee terminal ID
    KEY_CALLERIP        = 19,               //Caller IP (wan)
    KEY_CALLERPORT      = 20,               //Caller Port (wan)
    KEY_CALLERLIP       = 21,               //Caller lan IP
    KEY_CALLERLPORT     = 22,               //Caller lan Port
    KEY_CALLERNAT       = 23,               //Caller NAT flag
    KEY_CALLEEIP        = 24,               //Callee IP (wan)
    KEY_CALLEEPORT      = 25,               //Callee Port (wan)
    KEY_CALLEELIP       = 26,               //Callee lan IP
    KEY_CALLEELPORT     = 27,               //Callee lan Port
    KEY_CALLEENAT       = 28,               //Callee NAT flag
    KEY_MIRRORADDR      = 29,               //Mirror addresses
    KEY_SCALLERIP       = 30,               //Caller server IP
    KEY_SCALLERPORT     = 31,               //Caller server Port
    KEY_SCALLEEIP       = 32,               //Callee server IP
    KEY_SCALLEEPORT     = 33,               //Callee server Port
    KEY_MIRRORUSED      = 34,               //Is mirror used
    KEY_CALLEEBUSY      = 35,               //Is mirror busy
    KEY_CALLEEON        = 36,               //Is mirror online
    KEY_SOURCEID        = 37,               //Source ID
    KEY_DESTID          = 38,               //Destination ID
    KEY_AUDIO           = 39,               //Audio data
    KEY_VIDEO           = 40,               //Video data
    KEY_PICTURE         = 41,               //Picture data
    KEY_FILE            = 42,               //File data
    KEY_DATA            = 43,               //user data
    KEY_NICKNAME        = 44,               //nickname of user
    KEY_MOBILENO        = 45,               //mobile of user
    KEY_BUDDY           = 46,               //buddy of user
    KEY_SESSIONTYPE     = 47,               //session type
    KEY_DEBUGMSG        = 101,			    //Debug message
    KEY_ERRORMSG        = 1001,			    //Error message
};
enum CALLSTATUS
{
    CALL_INIT           = 0,
    CALL_IDLE           = 1,
    CALL_CALLER         = 2,
    CALL_CALLEE         = 3,
    CALL_WAIT           = 4,
    CALL_WAIT_ACK       = 5,
    CALL_HANGUP         = 10,

};

enum SESSIONTYPE
{
    SESSION_VIDEONORMAL           = 0,      // normal video conversation call
    SESSION_VIDEOAUTOANSWER       = 1,      // autoanswer video conversation call
    SESSION_TWOTERMINANLDATA      = 2,      // data transmission between 2 terminals
    SESSION_P4PDATA               = 3,      // data transmission in P4P way
};

typedef struct _tagF2FPKG_HEAD
{
    unsigned int code;              //'F2FS' -- stands for "F2F System"
    unsigned short version;         //protocol version
    unsigned short cmd;             //command
    unsigned int   sessionId;       //session id, sometimes it's used to recognize a procedure, such as heart-beat
    unsigned short errorCode;       //for debug only
    unsigned short payloadLen;      //length of data after F2FPKG_HEAD
}F2FPKG_HEAD;

typedef struct _tagF2FPKG
{
    F2FPKG_HEAD pkgHead;
    unsigned char payload[0];
}F2FPKG;

typedef struct _call_status
{
    int status;
    char mac[30];
    SESSIONTYPE st;
}call_status;

/* callback function ponters struct */
//状态通知
// len: msg的长度，msg: 包含状态机(自身状态、呼叫状态、中转状态)的变化信息
typedef void (*cb_OnStatusNotify)(int len, char* msg);   
//被叫通知
typedef void (*cb_OnCalleeNotify)(char* caller, SESSIONTYPE st);
//准备好发送数据，包括音频、视频、文件、图片、激活信息等等
typedef int (*cb_OnReady)();  
//收到被叫数据，包括音频、视频、文件、图片、激活信息等等
typedef void (*cb_OnCalleeVideo)(unsigned char* data, int len);
//收到主叫数据，包括音频、视频、文件、图片、激活信息等等
typedef void (*cb_OnCallerVideo)(unsigned char* data, int len);
// 对方挂机、或者网络问题20秒无对方数据，则自动挂机，将回调OnHangUp
typedef void (*cb_OnHangUp)();		
//收到好友列表
typedef void (*cb_OnBuddyReceived)(unsigned char* str, int len); 
//更改注册信息返回
typedef void (*cb_OnalterRegInfoAck)(int errCode, char* errMsg);
//自身注册信息回调
typedef void (*cb_OnRegInfoReceived)(char* nickname, char* mobileno, char* fwnat);
//根据mac取注册信息回调, errCode: 0 -- no error, -1 -- no such a person, -2 -- timeout, -3 -- unknown
typedef void (*cb_OnGetRegInfoAck)(int errCode, char* mac, char* nickname, char* mobileno);
//根据mobileno取注册信息回调 errCode: 0 -- no error, -1 -- no such a person, -2 -- timeout, -3 -- unknown
typedef void (*cb_OnGetRegInfoByMobileAck)(int errCode, char* mobileno, char* buddy);

typedef struct _tagF2FCBFUNCTIONS
{
    _tagF2FCBFUNCTIONS(){
        OnStatusNotify = 0;
        OnCalleeNotify = 0;
        OnReady = 0;
        OnCalleeVideo = 0;
        OnCallerVideo = 0;
        OnHangUp = 0;
        OnBuddyReceived = 0;
        OnalterRegInfoAck = 0;
        OnRegInfoReceived = 0;
        OnGetRegInfoAck = 0;
        OnGetRegInfoByMobileAck = 0;
    }
    cb_OnStatusNotify OnStatusNotify;
    cb_OnCalleeNotify OnCalleeNotify;
    cb_OnReady OnReady;
    cb_OnCalleeVideo OnCalleeVideo;
    cb_OnCallerVideo OnCallerVideo;
    cb_OnHangUp OnHangUp;
    cb_OnBuddyReceived OnBuddyReceived;
    cb_OnalterRegInfoAck OnalterRegInfoAck;
    cb_OnRegInfoReceived OnRegInfoReceived;
    cb_OnGetRegInfoAck OnGetRegInfoAck;
    cb_OnGetRegInfoByMobileAck OnGetRegInfoByMobileAck;
}F2FCBFUNCTIONS;

//--------------------------------------- codec call to f2f_net_SDK
/* functions implemented by f2f net module */
//初始化f2f网络模块
void f2fInit(char* lanip, unsigned short lanport, char* passwd, F2FCBFUNCTIONS* cb, 
        char* mac,/*mac[0] type 0:NULL 1:eth0 2:wlan0 3:chip id*/
        char* calleeid=0, char* test_terminalid=0);
//发起呼叫
int startCall(char* calleeid, SESSIONTYPE st=SESSION_VIDEONORMAL);  // return 0 -- OK, -1 -- Cannot call at present.
//发送数据
void sendData(unsigned char* data, int len);
//接受邀请
void sessionAccept();
//挂机
void hangUp();
//更改注册信息, 成功时返回0，失败时返回负数表示的错误代码
int alterRegInfo(char* nickname, char* mobileno);
//根据mac地址获取对应的注册信息
int getRegInfo(char* mac);
//根据mobileno获取对应的注册信息
int getRegInfoByMobile(char* mobileno);
//卸载f2f网络模块
void f2fUnInit();

//--------------------------------------------
/* 
 jni调用函数接口                                         
 *                                           */
//初始化F2F，登陆服务器
int f2f_connect(char *ip, char *mac);
//设置视频显示区域
void VideoInit(int full_w, int full_h,
        int local_w, int local_h,
        int remote_w, int remote_h,
        int frame_rate, int bit_rate);
int getMsg(unsigned char *data);
int getSysInfo(unsigned char *data);
void setSysInfo(char *data, int len);
int getUserList(unsigned char *data);
int getUserInfo(unsigned char *data);
int getSyncUserInfo(unsigned char *data);
void setSyncUserInfo(char *data, int len);
int getUserSearch(unsigned char *data);

//呼叫
call_status getCallStatus();
//被叫
void setCallStatus(int status, char* data, SESSIONTYPE st = SESSION_VIDEONORMAL);
//获取本地图像
unsigned char * getCallerVideo();
//获取对方图像
unsigned char * getCalleeVideo();
//关闭视频
void video_exit();
//关闭连接
void f2f_exit();

void setPCMToBuf(short int *pcm, int len);
int getPCMFromBuf(short int *pcm);
void setVideo(int cmd);

#endif // F2FDEFS_H
