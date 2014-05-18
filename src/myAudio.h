//
//  myAudio.h
//  Test
//
//  Created by MacBook on 12-7-30.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>
//#import <MediaPlayer/MediaPlayer.h>
#import <AudioToolbox/AudioToolbox.h>

#import <OpenAL/al.h>

#import <OpenAL/alc.h>

#import "AsyncSocket.h"
#import "spcaframe.h"
//#import "DjSocket.h"

#import "NetUtils.h"

#import <AudioToolbox/AudioQueue.h>
#import <AudioToolbox/AudioFile.h>

#define NUM_BUFFERS 20
#define DOCUMENTS_FOLDER [NSHomeDirectory() stringByAppendingPathComponent:@"Documents"]

typedef struct
{
    AudioFileID                 audioFile;
    AudioStreamBasicDescription dataFormat;
    AudioQueueRef               queue;
    AudioQueueBufferRef         buffers[NUM_BUFFERS];
    UInt32                      bufferByteSize;
    SInt64                      currentPacket;
    BOOL                        recording;
} RecordState;


@interface myAudio : NSObject<NetUtilsDelegate> {
	RecordState recordState;
	ALCcontext *mContext;
    ALCdevice *mDevice;
	ALuint outSourceID;
	
	SInt16 *data2;
	int end2;
	int pos2;
	
	//NSMutableData *mData;
//	AsyncSocket *socket;
    
    //DjSocket *mV;
	
	
	int position;
	char header[9];
	unsigned char buff[1200];
	
}

@property (nonatomic) ALCcontext *mContext;

@property (nonatomic) ALCdevice *mDevice;
//@property (nonatomic,retain) AsyncSocket *socket;
//@property (nonatomic,retain) DjSocket *mV;
+ (instancetype)sharedInstance;
-(void)startPlay;
-(void)stopPlay;

-(void)initOpenAL;

-(void)playSound;

-(void)stopSound;

//-(void)cleanUpOpenALID;

-(void)cleanUpOpenAL;

@end
