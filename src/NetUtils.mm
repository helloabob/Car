//
//  NetUtils.m
//  Test
//
//  Created by wangbo on 4/17/14.
//
//

#import "NetUtils.h"
#import "DjSocket.h"
#import "myAudio.h"

#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>

#ifdef __cplusplus
#import "f2fdefs.h"
#endif

static NSString *_calleeid;
static BOOL _firstCall;
static BOOL bReadyToSendVideo=false;
static BOOL bCallReady=false;

F2FCBFUNCTIONS *cbfuncs__=NULL;

#define kMaxBytes 539

typedef struct {
    unsigned char flag;
    unsigned short pkg_len;
    unsigned int serial;
    unsigned char type;
}UDPHeader;

@interface NetUtils()
@end

@implementation NetUtils {
    BOOL isInited;
    BOOL canStop;
    
    NSString *_ip;
    uint32_t _port;
    NSString *_pwd;
    NSString *_callerid;
}

int OnReady()
{
	printf("[%s]UI OnReady is called.\n",__FUNCTION__);
    bReadyToSendVideo = true;
    kPostNotif(@"stateChange", @"onReady");
    return 1;
}

void OnCalleeVideo(unsigned char *data, int len)
{
//	printf("[%s]UI received video data, len: %d, data: \"%s\"\n",__FUNCTION__, len, data);
    if (len < 9) {
        return;
    }
    if (data[0] != 0x7E) {
        return;
    }
    if (data[7]==0x0e) {
        if (bReadyToSendVideo==false) {
            return;
        }
        [[NetUtils sharedInstance].videoDelegate onReceivedData:data length:len];
        
    } else if (data[7]==0x0f) {
        if (bReadyToSendVideo==false) {
            return;
        }
        [[NetUtils sharedInstance].audioDelegate onReceivedData:data length:len];
        
    }
}

void OnCallerVideo(unsigned char *data, int len)
{
	printf("[%s]UI received video data, len: %d, data: \"%s\"\n",__FUNCTION__, len, data);
}

void OnStatusNotify(int st, char* msg)
{
    //    printf("OnStatusNotify\n");
    //#if 0
//    if (msg)
//        printf("[%s]---------callback:(%d)\"%s\"---------\n",__FUNCTION__, st, msg);
//    else
//        printf("[%s]UI received status callback:(%d)\n",__FUNCTION__, st);
    //#endif
    
    //SM_HEARTBEATING			= 5,		// Heart-beating
    
    NSLog(@"--------------status------------\n%s\n", msg);
    
    if (strstr(msg, "(4) SM_NATDETECTREQ --->(5) SM_HEARTBEATING")){
        //            bCallReady = true;
        bCallReady=true;
        printf("callReay\n");
        kPostNotif(@"stateChange",@"callReady");
    }
}

void OnCalleeNotify(char* caller, SESSIONTYPE st)
{
    //auto answer
    printf("[%s]UI received OnCalleeNotify callback:(caller: %s, st: %d)\n",__FUNCTION__, caller, st);
    sessionAccept();
}

//void* send_thread(void* ctx){
    //	char data[256];
    //	sprintf(data, "%s", "video data");
    //	while(!bQuitMainThread){
    //        usleep(1000*1000);
    //		if (bReadyToSendVideo)
    //			sendData((unsigned char*)data, (int)strlen(data));
    //	}
//	return NULL;
    
//}

void OnHangUp ()
{
    printf("[%s]UI OnHangUp is called.\n",__FUNCTION__);
    bReadyToSendVideo = false;
    kPostNotif(@"stateChange", @"onHangup");
}

void OnBuddyReceived(unsigned char* str, int len)
{
    //printf("[%d]UI buddy received : %s\n", len, str);
}

void OnRegInfoReceived(char* nickname, char* mobileno, char* fwnat)
{
    //printf("UI received register info: nickname: %s, mobileno: %s, fwnat: %s\n", nickname, mobileno, fwnat);
}


void OnGetRegInfoAck(int errCode, char* mac, char* nickname, char* mobileno)
{
    if (errCode==0){
        printf("*** OnGetRegInfoAck, mac: %s, nickname: %s, mobileno: %s\n", mac, nickname, mobileno);
    }
    else{
        printf("*** OnGetRegInfoAck, errorcode: %d\n", errCode);
        
    }
}

void OnGetRegInfoByMobileAck(int errCode, char* mobileno, char* buddy)
{
    if (errCode==0){
        printf("*** OnGetRegInfoByMobileAck, mobileno: %s, buddy: %s\n", mobileno, buddy);
        
    }
    else{
        printf("*** OnGetRegInfoByMobileAck, errorcode: %d\n", errCode);
    }
}

+ (instancetype)sharedInstance {
    static NetUtils *sharedNetUtilsInstance = nil;
    static dispatch_once_t predicate; dispatch_once(&predicate, ^{
        sharedNetUtilsInstance = [[self alloc] init];
//        signal(SIGPIPE,SIG_IGN);
//        struct sigaction sa;
//        sa.sa_handler = new_sa_handler;
//        sigaction(SIGPIPE, &sa, 0);
    });
    return sharedNetUtilsInstance;
}

- (void)abortNetwork {
    bReadyToSendVideo=false;
    if (cbfuncs__!=NULL) {
        f2fUnInit();
        free(cbfuncs__);
        cbfuncs__=NULL;
        NSLog(@"deactive");
    }
}

//void new_sa_handler(int){
//    NSLog(@"--------catch_pipe_signal---------");
//    bReadyToSendVideo=false;
//    if (cbfuncs__!=NULL) {
//        f2fUnInit();
//        free(cbfuncs__);
//        cbfuncs__ = NULL;
//    }
//    kPostNotif(@"stateChange", @"sigpipe");
//}

//- (void)dispose {
//    free(cbfuncs);
//}

-(NSString *)localIPAddress
{
    NSString *localIP = nil;
    struct ifaddrs *addrs;
    if (getifaddrs(&addrs)==0) {
        const struct ifaddrs *cursor = addrs;
        NSMutableDictionary *dict = [NSMutableDictionary dictionary];
        while (cursor != NULL) {
            if (cursor->ifa_addr->sa_family == AF_INET && (cursor->ifa_flags & IFF_LOOPBACK) == 0)
            {
                NSString *name = [NSString stringWithUTF8String:cursor->ifa_name];
                //                if ([name isEqualToString:@"en0"]) // Wi-Fi adapter
                //                {
                [dict setObject:[NSString stringWithUTF8String:inet_ntoa(((struct sockaddr_in *)cursor->ifa_addr)->sin_addr)] forKey:name];
                //                    localIP = [NSString stringWithUTF8String:inet_ntoa(((struct sockaddr_in *)cursor->ifa_addr)->sin_addr)];
                //                    break;
                //                }
            }
            cursor = cursor->ifa_next;
        }
        freeifaddrs(addrs);
        if ([dict objectForKey:@"en0"]) {
            localIP = [dict objectForKey:@"en0"];
        } else if (dict.count > 0) {
            localIP = [dict objectForKey:[dict allKeys].firstObject];
        }
    }
    return localIP;
}

- (void)initNetwork {
//    _ip = [[NSString alloc] initWithString:[CommonUtils localIPAddress]];
    
    self.videoDelegate = [DjSocket sharedInstance];
    self.audioDelegate = [myAudio sharedInstance];
    
    _ip = [self localIPAddress];
    _port = 10241;
    _pwd = @"888888";
//    _pwd = [[NSString alloc] initWithString:@"888888"];
    _callerid = @"0A:0A:0A:0A:0A:0A";
//    _callerid = @"12345678901234567";
    
    cbfuncs__=(F2FCBFUNCTIONS *)malloc(sizeof(F2FCBFUNCTIONS));
    cbfuncs__->OnReady = OnReady;
    cbfuncs__->OnCalleeVideo = OnCalleeVideo;
    cbfuncs__->OnCallerVideo = OnCallerVideo;
    cbfuncs__->OnCalleeNotify = OnCalleeNotify;
    cbfuncs__->OnStatusNotify = OnStatusNotify;
    cbfuncs__->OnHangUp = OnHangUp;
    cbfuncs__->OnBuddyReceived = OnBuddyReceived ;
    cbfuncs__->OnRegInfoReceived = OnRegInfoReceived;
    cbfuncs__->OnGetRegInfoAck = OnGetRegInfoAck;
    cbfuncs__->OnGetRegInfoByMobileAck = OnGetRegInfoByMobileAck;
    f2fInit((char *)[_ip cStringUsingEncoding:NSUTF8StringEncoding], _port, (char *)[_pwd cStringUsingEncoding:NSUTF8StringEncoding], cbfuncs__, (char *)[_callerid cStringUsingEncoding:NSUTF8StringEncoding]);
}

- (int)startCall:(NSString *)cid {
    return startCall((char *)[cid cStringUsingEncoding:NSUTF8StringEncoding]);
}

- (void)connectWithIP:(NSString *)ip
          withPort:(uint32_t)port
           withPwd:(NSString *)pwd
         withCallerId:(NSString *)callerId
      withCalleeId:(NSString *)cid {
    _ip = [ip retain];
    _port = port;
    _pwd = [pwd retain];
    _callerid = [callerId retain];
    _calleeid = [cid retain];
    if (isInited) {
        canStop = YES;
        usleep(200000);
        f2fUnInit();
    }
    [self performSelectorInBackground:@selector(thread_job) withObject:nil];
}

- (unsigned char)convertCommandTypeToBytes:(CommandType)type {
    if (type == CommandTypeAudio) {
        return 0x0d;
    } else if (type == CommandTypeDirection) {
        return 0x0a;
    } else if (type == CommandTypeRedMode) {
        return 0x06;
    } else if (type == CommandTypeRedSwitch) {
        return 0x07;
    } else if (type == CommandTypeState) {
        return 0x08;
    } else if (type == CommandTypeSpeech) {
        return 0x09;
    } else {
        return 0x00;
    }
}

- (void)startSendData:(int)cmd withType:(CommandType)type forLength:(int)length {
    NSData *data = [NSData dataWithBytes:&cmd length:length];
    return [self startSendData:data withType:type];
}

- (void)startSendData:(NSData *)data withType:(CommandType)type {
    static unsigned int _serial = 0;
    unsigned long long bytesSent = 0;
    while (data.length-bytesSent > 0) {
        unsigned int bytesThisTime = 0;
        if (data.length-bytesSent < kMaxBytes) {
            bytesThisTime = data.length-bytesSent;
        } else {
            bytesThisTime = kMaxBytes;
        }
        NSRange range;
        range.location = bytesSent;
        range.length = bytesThisTime;
        NSData *newData = [data subdataWithRange:range];
        NSMutableData *_mdata = [NSMutableData data];
        unsigned char flag = 0x7E;
        unsigned short len = bytesThisTime+6;
        unsigned int serial = _serial++;
        unsigned char cmd = [self convertCommandTypeToBytes:type];
        unsigned char cs = 0x00;
        [_mdata appendBytes:&flag length:1];
        [_mdata appendBytes:&len length:2];
        [_mdata appendBytes:&serial length:4];
        [_mdata appendBytes:&cmd length:1];
        [_mdata appendBytes:newData.bytes length:bytesThisTime];
        [_mdata appendBytes:&cs length:1];
        
        char *d=(char *)_mdata.bytes;
        for(int i=0,f=_mdata.length-1;i<f;i++){
            d[f]+=d[i];
        }
//        NSLog(@"sendData:%@", _mdata);
        sendData((unsigned char *)_mdata.bytes, _mdata.length);
        bytesSent += bytesThisTime;
    }
}


- (void)send_thread {
    while (!canStop) {
        if (bReadyToSendVideo) {
//            char data[]="video data22";
//            sendData((unsigned char *)data, (int)strlen(data));
//            static int count = 0;
//            count ++;
//            if (count >= 10) {
//                bReadyToSendVideo = false;
//            }
        }
        usleep(1000*1000);
    }
}

- (void)thread_job {
//    kAddObserver(@selector(<#selector#>), @"init");
    isInited = YES;
    canStop = NO;
    _firstCall = YES;
    F2FCBFUNCTIONS cbfuncs;
    cbfuncs.OnReady = OnReady;
    cbfuncs.OnCalleeVideo = OnCalleeVideo;
    cbfuncs.OnCallerVideo = OnCallerVideo;
    cbfuncs.OnCalleeNotify = OnCalleeNotify;
    cbfuncs.OnStatusNotify = OnStatusNotify;
    cbfuncs.OnHangUp = OnHangUp;
    cbfuncs.OnBuddyReceived = OnBuddyReceived ;
    cbfuncs.OnRegInfoReceived = OnRegInfoReceived;
    cbfuncs.OnGetRegInfoAck = OnGetRegInfoAck;
    cbfuncs.OnGetRegInfoByMobileAck = OnGetRegInfoByMobileAck;
    f2fInit((char *)[_ip cStringUsingEncoding:NSUTF8StringEncoding], _port, (char *)[_pwd cStringUsingEncoding:NSUTF8StringEncoding], &cbfuncs, (char *)[_callerid cStringUsingEncoding:NSUTF8StringEncoding]);
//    char *lanip=(char *)[_ip cStringUsingEncoding:NSUTF8StringEncoding];
    
    [self performSelectorInBackground:@selector(send_thread) withObject:nil];
    
    while (!canStop) {
        if (_firstCall==YES && bCallReady==true) {
            _firstCall = NO;
            startCall((char *)[_calleeid cStringUsingEncoding:NSUTF8StringEncoding]);
//            kPostNotif(@"init");
        }
        usleep(200*1000);
    }
}

//- (void)test {
//    
//    F2FCBFUNCTIONS cbfuncs;
//    cbfuncs.OnReady = OnReady;
//    cbfuncs.OnCalleeVideo = OnCalleeVideo;
//    cbfuncs.OnCallerVideo = OnCallerVideo;
//    cbfuncs.OnCalleeNotify = OnCalleeNotify;
//    cbfuncs.OnStatusNotify = OnStatusNotify;
//    cbfuncs.OnHangUp = OnHangUp;
//    cbfuncs.OnBuddyReceived = OnBuddyReceived ;
//    cbfuncs.OnRegInfoReceived = OnRegInfoReceived;
//    cbfuncs.OnGetRegInfoAck = OnGetRegInfoAck;
//    cbfuncs.OnGetRegInfoByMobileAck = OnGetRegInfoByMobileAck;
//    /*test*/
//    f2fInit("127.0.0.1", 8000, "aaa", &cbfuncs, "terminal_id", NULL, NULL);
//    
//    CFRunLoopRun();
//    
//}

@end
