//
//  myAudio.m
//  Test
//
//  Created by MacBook on 12-7-30.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#import "myAudio.h"

#define DOCUMENTS_FOLDER [NSHomeDirectory() stringByAppendingPathComponent:@"Documents"]
#define FILEPATH [DOCUMENTS_FOLDER stringByAppendingPathComponent:[self dateString]]

#define rate 8000

@implementation myAudio{
    dispatch_queue_t serial_queue;
    ALenum al_error;
    dispatch_queue_t serial_send_audio;
}
static myAudio *sharedNetUtilsInstance = nil;

@synthesize mContext;
@synthesize mDevice;
//@synthesize socket;
//int ret;

BOOL isPlay=YES;

-(unsigned char *)getBuff{
	return &buff;
}

- (NSString *) dateString
{
	NSDateFormatter *formatter = [[[NSDateFormatter alloc] init] autorelease];
	formatter.dateFormat = @"ddMMYY_hhmmssa";
	return [[formatter stringFromDate:[NSDate date]] stringByAppendingString:@".aif"];
}

-(void)startPlay{
	isPlay=YES;
    [self startRecording:FILEPATH];
}
-(void)stopPlay{
	[self stopSound];
	isPlay=NO;
    [self stopRecording];
}
+ (instancetype)sharedInstance {
    
    static dispatch_once_t predicate; dispatch_once(&predicate, ^{
        sharedNetUtilsInstance = [[self alloc] init];
        sharedNetUtilsInstance->serial_queue=dispatch_queue_create("serial_audio", NULL);
        sharedNetUtilsInstance->serial_send_audio=dispatch_queue_create("serial_send", NULL);
    });
    return sharedNetUtilsInstance;
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
	
//	alDopplerVelocity(1.0);
	
//	alDopplerFactor(1.0);
	
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
    al_error=0;
    if ((al_error = alGetError()) != AL_NO_ERROR)
    {
        NSLog(@"error1:%d", al_error);
//        alDeleteBuffers(1, &bufferID);
//        al_error=0;
//        if ((al_error = alGetError()) != AL_NO_ERROR)
//        {
//            NSLog(@"error1-1:%d", al_error);
//        }else{
//            NSLog(@"solved");
//        }
//        return;
    }
	
    
//	NSData * tmpData = [NSData dataWithBytes:data length:dataSize];
	
	alBufferData(bufferID, AL_FORMAT_MONO16, (char*)data, (ALsizei)dataSize, 8000);
    
    al_error=0;
    if ((al_error = alGetError()) != AL_NO_ERROR)
    {
        NSLog(@"error2:%d", al_error);
    }
	
	alSourceQueueBuffers(outSourceID, 1, &bufferID);
	al_error=0;
    if ((al_error = alGetError()) != AL_NO_ERROR)
    {
        NSLog(@"error3:%d", al_error);
    }
	
	[self updataQueueBuffer];
	al_error=0;
    if ((al_error = alGetError()) != AL_NO_ERROR)
    {
        NSLog(@"error4:%d", al_error);
    }
	
//	ALint stateVaue;
	
//	alGetSourcei(outSourceID, AL_SOURCE_STATE, &stateVaue);
	
	
	[ticketCondition unlock];
	
	[ticketCondition release];
	
	ticketCondition = nil;
//	alDeleteBuffers(1, &bufferID);
	al_error=0;
    if ((al_error = alGetError()) != AL_NO_ERROR)
    {
        NSLog(@"error5:%d", al_error);
    }
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
	
	
//	NSLog(@"Processed = %d\n", processed);
//	NSLog(@"Queued = %d\n", queued);
	
    al_error=0;
    if ((al_error = alGetError()) != AL_NO_ERROR)
    {
        NSLog(@"error7:%d", al_error);
    }
    
	
    while(processed--)
		
    {
		
        ALuint buffD;
		
        alSourceUnqueueBuffers(outSourceID, 1, &buffD);
		
		alDeleteBuffers(1, &buffD);
		
	}
    al_error=0;
    if ((al_error = alGetError()) != AL_NO_ERROR)
    {
        NSLog(@"error8:%d", al_error);
    }
	
	
	return YES;
	
}

-(void)playSound{
	alSourcePlay(outSourceID);
    al_error=0;
    if ((al_error = alGetError()) != AL_NO_ERROR)
    {
        NSLog(@"error6:%d", al_error);
    }
}

-(void)stopSound{
	alSourceStop(outSourceID);
}

-(void)cleanUpOpenAL{
    NSLog(@"cleanUpOpenAL");
    ALint queue_num;
    alGetSourcei(&outSourceID, AL_BUFFERS_QUEUED, &queue_num);
    while (queue_num--) {
        ALuint processedBuffer;
        alSourceUnqueueBuffers(&outSourceID, 1, &processedBuffer);
        alDeleteBuffers(1, &processedBuffer);
    }
	alDeleteSources(1, &outSourceID);
	alcDestroyContext(mContext);
	alcCloseDevice(mDevice);
}
- (void)clearBuffer{
    int processed;
    alGetSourcei(outSourceID, AL_BUFFERS_PROCESSED, &processed);
    while(processed--)
    {
        ALuint buffD;
        alSourceUnqueueBuffers(outSourceID, 1, &buffD);
        alDeleteBuffers(1, &buffD);
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

- (void)onReceivedData:(unsigned char*)data length:(int)length {
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
//    unsigned char *new_data=data;
    unsigned char *new_data=malloc(length);
    memcpy(new_data, data, length);
    dispatch_async(serial_queue, ^(){
        if (isPlay == 0)
        {
            [self clearBuffer];
            free(new_data);
            return;
        }
//    NSLog(@"recei_audio_data");
//    static short requestLength=0;
//	int pos=0;
    
	position = 0;
	
//	while (YES) {
		//∂¡header
//		if(new_data[pos] != 0x7e)	return;
		position = 0;
        unsigned short requestLength;
        memcpy(&requestLength, &new_data[1], 2);
        [self openAudioFromQueue:&new_data[8] dataSize:requestLength-6];
//        
//		while (position<sizeof(struct Tphead) && length-pos>0) {
//			header[position]=data[pos];
//			position++;
//			pos++;
//		}
//		//header √ª∂¡ÕÍ
//		if (position<sizeof(struct Tphead)) {
//			return;
//		}
//		requestLength=((struct Tphead*)header)->bodylen;
//		if (requestLength == 0) {
//			continue;
//		}
//		
//		//∂¡body
//		while (position<requestLength+sizeof(struct Tphead) && length-pos>0) {
//			buff[position-sizeof(struct Tphead)]=data[pos];
//			position++;
//			pos++;
//		}
//		if (position==requestLength+sizeof(struct Tphead)){
//			[self openAudioFromQueue:buff dataSize:(UInt32)requestLength];
//			position=0;
//		}
//		else
//			return;
//	}
        free(new_data);
    });
}
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

static void HandleInputBuffer (void *aqData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer, const AudioTimeStamp *inStartTime,
							   UInt32 inNumPackets, const AudioStreamPacketDescription *inPacketDesc)
{
    RecordState *pAqData = (RecordState *) aqData;
    if (pAqData->recording==NO) {
        return;
    }
    static int count=0;
    count++;
    if (count%10==0) {
        NSLog(@"send_audio_data");
        count=0;
    }
    
    NSData *audioData = [NSData dataWithBytes:inBuffer->mAudioData length:inBuffer->mAudioDataByteSize];
    dispatch_async(sharedNetUtilsInstance->serial_send_audio, ^(){
        [[NetUtils sharedInstance] startSendData:audioData withType:CommandTypeAudio];
    });
    AudioQueueEnqueueBuffer(pAqData->queue, inBuffer, 0, NULL);
}

- (BOOL)startRecording: (NSString *) filePath
{
    
    AVAudioSession * audioSession = [AVAudioSession sharedInstance];
    [audioSession setCategory:AVAudioSessionCategoryPlayAndRecord error: nil];
    UInt32 audioRouteOverride = kAudioSessionOverrideAudioRoute_Speaker;
    AudioSessionSetProperty (kAudioSessionProperty_OverrideAudioRoute,
                             sizeof (audioRouteOverride),
                             &audioRouteOverride);
    [audioSession setActive:YES error: nil];
    AudioStreamBasicDescription *format = &recordState.dataFormat;
    format->mSampleRate = 8000.0;
    format->mFormatID = kAudioFormatLinearPCM;
    format->mFormatFlags = kAudioFormatFlagIsSignedInteger;
    format->mChannelsPerFrame = 1;
    format->mBitsPerChannel = 16;
    format->mFramesPerPacket = 1;
    format->mBytesPerPacket = 2;
    format->mBytesPerFrame = 2;
    format->mReserved = 0;
//    CFURLRef fileURL =  CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *) [filePath UTF8String], [filePath length], NO);
    // recordState.currentPacket = 0;
    
	// new input queue
    OSStatus status;
    status = AudioQueueNewInput(&recordState.dataFormat, HandleInputBuffer, &recordState, CFRunLoopGetCurrent(),kCFRunLoopCommonModes, 0, &recordState.queue);
    if (status) {NSLog(@"Could not establish new queue");return NO;}
//    if (status) {CFRelease(fileURL); printf("Could not establish new queue\n"); return NO;}
	// create new audio file
//    status = AudioFileCreateWithURL(fileURL, kAudioFileAIFFType, &recordState.dataFormat, kAudioFileFlags_EraseFile, &recordState.audioFile);
//	CFRelease(fileURL); // thanks august joki
//    if (status) {printf("Could not create file to record audio\n"); return NO;}
    
	// figure out the buffer size
//    DeriveBufferSize(recordState.queue, recordState.dataFormat, 0.5, &recordState.bufferByteSize);
	
    recordState.bufferByteSize=8000;
//    recordState.bufferByteSize=1024;
    
	// allocate those buffers and enqueue them
    for(int i = 0; i < NUM_BUFFERS; i++)
    {
        status = AudioQueueAllocateBuffer(recordState.queue, recordState.bufferByteSize, &recordState.buffers[i]);
        if (status) {printf("Error allocating buffer %d\n", i); return NO;}
        
        status = AudioQueueEnqueueBuffer(recordState.queue, recordState.buffers[i], 0, NULL);
        if (status) {printf("Error enqueuing buffer %d\n", i); return NO;}
    }
	
	// enable metering
    UInt32 enableMetering = YES;
    status = AudioQueueSetProperty(recordState.queue, kAudioQueueProperty_EnableLevelMetering, &enableMetering,sizeof(enableMetering));
    if (status) {printf("Could not enable metering\n"); return NO;}
    
	// start recording
    status = AudioQueueStart(recordState.queue, NULL);
    if (status) {printf("Could not start Audio Queue\n"); return NO;}
    recordState.currentPacket = 0;
    recordState.recording = YES;
    return YES;
}

- (void)stopRecording
{
    [self performSelector:@selector(reallyStopRecording) withObject:NULL afterDelay:0.5f];
}

- (void)reallyStopRecording
{
    AudioQueueFlush(recordState.queue);
    AudioQueueStop(recordState.queue, NO);
    recordState.recording = NO;
    
    for(int i = 0; i < NUM_BUFFERS; i++)
		AudioQueueFreeBuffer(recordState.queue, recordState.buffers[i]);
    
    AudioQueueDispose(recordState.queue, YES);
//    AudioFileClose(recordState.audioFile);
}

#pragma mark socket

-(void)connect{
	NSLog(@"8001 connecting...");
	NSError *err = [[NSError alloc]init];
//	BOOL b=[socket connectToHost:@"192.168.1.11" onPort:8001 withTimeout:-1 error:&err];
 //   BOOL b=[socket connectToHost:@"163.125.84.233" onPort:8001 withTimeout:-1 error:&err];
//	if (!b){
//		NSLog(@"failed err=%@",err);
//	}
	[err release];
}


- (id)init{
    if (self = [super init]){
//        socket = [[[[AsyncSocket alloc] initWithDelegate:self] autorelease]retain];
//        NSLog(@"myaudio init: socket=%@",socket);
//		[self connect];
		
		
        
		
		pos2=0;
		[self initOpenAL];
		
    }
    return self;
}

- (void)onSocket:(AsyncSocket *)sock didConnectToHost:(NSString *)host port:(UInt16)port{
	NSLog(@"8001 connect success");
//    [socket readDataWithTimeout:-1 tag:0];
}
- (void)onSocketDidDisconnect:(AsyncSocket *)sock{
	NSLog(@"8001 faild");
   [self stopSound];
   [self clearBuffer];
   [self connect];
    
}

- (void)onSocket:(AsyncSocket *)sock didReadData:(NSData *)data withTag:(long)tag{
	[self on_Recv:(char*)[data bytes] length:[data length]];
//    [socket readDataWithTimeout:-1 tag:0];
}

-(void)dealloc{
	//[self stopSound];
	[self stopPlay];
	[self cleanUpOpenAL];
//    NSLog(@"myAudio dealloc release开始:socket:%@",socket);
//    [socket release];
//    socket = nil;
    [super dealloc];
}

@end
