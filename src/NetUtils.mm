//
//  NetUtils.m
//  Test
//
//  Created by wangbo on 4/17/14.
//
//

#import "NetUtils.h"

#ifdef __cplusplus
#import "f2fdefs.h"
#endif

static NSString *_calleeid;
static BOOL _firstCall;
static BOOL bReadyToSendVideo=false;
static BOOL bCallReady=false;

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
    return 1;
}

void OnCalleeVideo(unsigned char *data, int len)
{
//	printf("[%s]UI received video data, len: %d, data: \"%s\"\n",__FUNCTION__, len, data);
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
    
    if (_calleeid.length > 0 && strstr(msg, "(4) SM_NATDETECTREQ --->(5) SM_HEARTBEATING")){
        //            bCallReady = true;
        bCallReady=true;
        
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
    });
    return sharedNetUtilsInstance;
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

- (void)sendData:(NSData *)data {
    sendData((unsigned char *)data.bytes, (int)data.length);
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
    f2fInit((char *)[_ip cStringUsingEncoding:NSUTF8StringEncoding], _port, (char *)[_pwd cStringUsingEncoding:NSUTF8StringEncoding], &cbfuncs, (char *)[_callerid cStringUsingEncoding:NSUTF8StringEncoding], (char *)[_calleeid cStringUsingEncoding:NSUTF8StringEncoding], (char *)[_callerid cStringUsingEncoding:NSUTF8StringEncoding]);
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
