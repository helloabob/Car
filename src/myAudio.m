//
//  myAudio.m
//  Test
//
//  Created by MacBook on 12-7-30.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#import "myAudio.h"


#define rate 8000

@implementation myAudio

@synthesize mContext;
@synthesize mDevice;
//@synthesize socket;
//int ret;

BOOL isPlay=YES;

-(unsigned char *)getBuff{
	return &buff;
}

-(void)startPlay{
	isPlay=YES;
}
-(void)stopPlay{
	[self stopSound];
	isPlay=NO;
}

void interruptionListenerCallback(void  *inUserData ,UInt32 interruptionState){
    
    myAudio *controller = (myAudio *) inUserData;
	
	if (interruptionState==kAudioSessionBeginInterruption) {
		[controller _haltOpenALSession];
	} else if (interruptionState==kAudioSessionEndInterruption) {
		[controller _resumeOpenALSession];
	}
}

-(void)_haltOpenALSession{
    AudioSessionSetActive(NO);
    // set the current context to NULL will 'shutdown' openAL
    alcMakeContextCurrent(NULL);
    // now suspend your context to 'pause' your sound world
    alcSuspendContext(mContext);
}

-(void)_resumeOpenALSession{
    // Reset audio session
    UInt32 category = kAudioSessionCategory_AmbientSound;
    AudioSessionSetProperty ( kAudioSessionProperty_AudioCategory, sizeof (category), &category );
	
    // Reactivate the current audio session
    AudioSessionSetActive(YES);
	
    // Restore open al context
    alcMakeContextCurrent(mContext);
    // 'unpause' my context
    alcProcessContext(mContext);
}

-(void)initOpenAL

{
	
    OSStatus result = AudioSessionInitialize(NULL, NULL, interruptionListenerCallback, self);
    UInt32 category = kAudioSessionCategory_AmbientSound;
    result = AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(category), &category);
    
    mDevice=alcOpenDevice(NULL);
	
    if (mDevice) {
		
        mContext=alcCreateContext(mDevice, NULL);
		
        alcMakeContextCurrent(mContext);
		
    }
	
	
	alGenSources(1, &outSourceID);
	
	alSpeedOfSound(1.0);
	
	alDopplerVelocity(1.0);
	
	alDopplerFactor(1.0);
	
	alSourcef(outSourceID, AL_PITCH, 1.0f);
	
	alSourcef(outSourceID, AL_GAIN, 1.0f);
	
	alSourcei(outSourceID, AL_LOOPING, AL_FALSE);
	
	alSourcef(outSourceID, AL_SOURCE_TYPE, AL_STREAMING);
	
	
	//[NSTimer scheduledTimerWithTimeInterval: 1/1000.0
    // target:self
    // selector:@selector(updataQueueBuffer)
    // userInfo: nil
    
    //repeats:YES];
	
}




- (void) openAudioFromQueue:(unsigned char*)data dataSize:(UInt32)dataSize

{
	
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	
	NSCondition* ticketCondition= [[NSCondition alloc] init];
	
	[ticketCondition lock];
	
	
	ALuint bufferID = 0;
	
	alGenBuffers(1, &bufferID);
	
    
	
	NSData * tmpData = [NSData dataWithBytes:data length:dataSize];
	
	alBufferData(bufferID, AL_FORMAT_MONO16, (char*)[tmpData bytes], (ALsizei)[tmpData length], 8000);
	
	alSourceQueueBuffers(outSourceID, 1, &bufferID);
	
	
	[self updataQueueBuffer];
	
	
	ALint stateVaue;
	
	alGetSourcei(outSourceID, AL_SOURCE_STATE, &stateVaue);
	
	
	[ticketCondition unlock];
	
	[ticketCondition release];
	
	ticketCondition = nil;
	alDeleteBuffers(1, &bufferID);
	
	[pool release];
	
	pool = nil;
	
}




- (BOOL) updataQueueBuffer

{
	
	ALint stateVaue;
	
	int processed, queued;
	
	
	alGetSourcei(outSourceID, AL_SOURCE_STATE, &stateVaue);
	
	
	if (stateVaue == AL_STOPPED ||
		
		stateVaue == AL_PAUSED ||
		stateVaue == AL_INITIAL)
	{
		
		[self playSound];
		
		return NO;
		
	}
	
	
    alGetSourcei(outSourceID, AL_BUFFERS_PROCESSED, &processed);
	
	alGetSourcei(outSourceID, AL_BUFFERS_QUEUED, &queued);
	
	
	NSLog(@"Processed = %d\n", processed);
	
	NSLog(@"Queued = %d\n", queued);
	
	
    while(processed--)
		
    {
		
        ALuint buff;
		
        alSourceUnqueueBuffers(outSourceID, 1, &buff);
		
		alDeleteBuffers(1, &buff);
		
	}
	
	
	return YES;
	
}

-(void)playSound{
	alSourcePlay(outSourceID);
}

-(void)stopSound{
	alSourceStop(outSourceID);
	
}

-(void)cleanUpOpenAL{
	alDeleteSources(1, &outSourceID);
	alcDestroyContext(mContext);
	alcCloseDevice(mDevice);
}
- (void)clearBuffer{
    int processed;
    alGetSourcei(outSourceID, AL_BUFFERS_PROCESSED, &processed);
    
    
    while(processed--)
        
    {
        
        ALuint buff;
        
        alSourceUnqueueBuffers(outSourceID, 1, &buff);
        
        alDeleteBuffers(1, &buff);
        
    }
}
/*
-(void)on_Recv:(char*)data length:(int)length{
	static short requestLength=0;
	int pos=0;
	
	while (YES) {
		//∂¡header
		while (position<sizeof(struct Tphead) && length-pos>0) {
			header[position]=data[pos];
			position++;
			pos++;
		}
		//header √ª∂¡ÕÍ
		if (position<sizeof(struct Tphead)) {
			break;
		}
		
		requestLength=((struct Tphead*)header)->bodylen;
        //比较是否延迟2秒
        int timestamp=((struct Tphead*)header)->serial;
        int videotimestamp=[mV getTimestamp];
        if(videotimestamp-timestamp>2){
            position=0;
            break;
        }
		
		//label.text  = [NSString stringWithFormat:@"requestLength=%d",requestLength];
		
		
		//bodyŒ™0
		if (requestLength==0) {
			position=0;
			continue;
		}
		
		//∂¡body
		while (position<requestLength+sizeof(struct Tphead) && length-pos>0) {
			buff[position-sizeof(struct Tphead)]=data[pos];
			position++;
			pos++;
		}
		
		if (position==requestLength+sizeof(struct Tphead)) {
			//label.text=@"decodeData";
			//[self decodeData:buff length:requestLength];
			if (isPlay) {
				[self openAudioFromQueue:buff dataSize:(UInt32)requestLength];
			}else{
                //memset(buff, 0, sizeof(buff));
                [self clearBuffer];
                
            }
			
			//[self pcmBuffer:buff size:];
			position=0;
		}else {
			break;
		}
	}
	
}  
*/
-(void)on_Recv:(char*)data length:(int)length{
	static short requestLength=0;
	int pos=0;
    
	position = 0;
	if (isPlay == 0)
	{
        [self clearBuffer];
		return;
	}
	if(length > 1200)
	{
        [self clearBuffer];
		return;
	}
	while (YES) {
		//∂¡header
		if(data[pos] != 0x7e)	return;
		position = 0;
		while (position<sizeof(struct Tphead) && length-pos>0) {
			header[position]=data[pos];
			position++;
			pos++;
		}
		//header √ª∂¡ÕÍ
		if (position<sizeof(struct Tphead)) {
			return;
		}
		requestLength=((struct Tphead*)header)->bodylen;
		if (requestLength == 0) {
			continue;
		}
		
		//∂¡body
		while (position<requestLength+sizeof(struct Tphead) && length-pos>0) {
			buff[position-sizeof(struct Tphead)]=data[pos];
			position++;
			pos++;
		}
		if (position==requestLength+sizeof(struct Tphead)){
			[self openAudioFromQueue:buff dataSize:(UInt32)requestLength];
			position=0;
		}
		else
			return;
	}
}

#pragma mark socket

-(void)connect{
	NSLog(@"8001 connecting...");
	NSError *err = [[NSError alloc]init];
	BOOL b=[socket connectToHost:@"192.168.1.11" onPort:8001 withTimeout:-1 error:&err];
 //   BOOL b=[socket connectToHost:@"163.125.84.233" onPort:8001 withTimeout:-1 error:&err];
	if (!b){
		NSLog(@"failed err=%@",err);
	}
	[err release];
}


- (id)init{
    if (self = [super init]){
        socket = [[[[AsyncSocket alloc] initWithDelegate:self] autorelease]retain];
        NSLog(@"myaudio init: socket=%@",socket);
		[self connect];
		
		
        
		
		pos2=0;
		[self initOpenAL];
		
		
    }
    return self;
}

- (void)onSocket:(AsyncSocket *)sock didConnectToHost:(NSString *)host port:(UInt16)port{
	NSLog(@"8001 connect success");
    [socket readDataWithTimeout:-1 tag:0];
}
- (void)onSocketDidDisconnect:(AsyncSocket *)sock{
	NSLog(@"8001 faild");
   [self stopSound];
   [self clearBuffer];
   [self connect];
    
}

- (void)onSocket:(AsyncSocket *)sock didReadData:(NSData *)data withTag:(long)tag{
	[self on_Recv:(char*)[data bytes] length:[data length]];
    [socket readDataWithTimeout:-1 tag:0];
}

-(void)dealloc{
	//[self stopSound];
	
	[self cleanUpOpenAL];
    NSLog(@"myAudio dealloc release开始:socket:%@",socket);
    [socket release];
    socket = nil;
    [super dealloc];
}

@end
