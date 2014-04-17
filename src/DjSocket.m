#import "DjSocket.h"
#import <MediaPlayer/MediaPlayer.h>
#import <QuartzCore/QuartzCore.h>
#import <CoreVideo/CoreVideo.h>
#import <CoreMedia/CoreMedia.h>
#import <AVFoundation/AVFoundation.h>
#import <AssetsLibrary/AssetsLibrary.h>
#import <UIKit/UIKit.h>
//#import "faac.h"
#import "KKScrollView.h"

@implementation DjSocket
@synthesize imgView;
@synthesize label;
@synthesize delegate;
@synthesize image;

@synthesize scrollView;


//BOOL isCamera=NO;
AVAssetWriter *videoWriter;
AVAssetWriterInput *writerInput;
AVAssetWriterInput *audioWriterInput;
AVAssetWriterInputPixelBufferAdaptor *adaptor;

unsigned char *myBuff;
//dispatch_queue_t dispatchQueue;
NSString *fullPath;
int  frame;
CGSize size;
int videoflag=0;
int timestamp;


-(void)setAudioData:(unsigned char*) buffer{
	
	myBuff=buffer;
	
}


#pragma mark 发送

-(void)test:(int)cmd{
	static int serial=0;
	
	struct Tphead tp;
	tp.signature=0x7E;
	tp.serial=serial++;
	tp.cmd=cmd;
	tp.bodylen=0;
	tp.checknum=0;
	
	NSData *sendData = [[NSData alloc]initWithBytes:&tp length:sizeof(tp)];
	
	char *d=(char *)sendData.bytes;
	for(int i=0,f=sendData.length-1;i<f;i++){
		d[f]+=d[i];
	}	
	[socket writeData:sendData withTimeout:-1 tag:0];
	
    [sendData release];
}




-(void)decodeData:(char*)data length:(int)length
{
	struct TVDATA *tvData=(struct TVDATA *)data;
	[jpgData appendBytes:(char*)tvData+8 length:length-8];
		
		
	//label.text=[NSString stringWithFormat:@"%d/%d",tvData->m_frag_cur+1,tvData->m_frag_totle];
		
	if(tvData->m_frag_totle-1==tvData->m_frag_cur)
	{
		image = [[UIImage alloc] initWithData:jpgData]; 
		//if (isCamera){
		//	sleep(1);
		//	[self saveScreen];
		//}
		[self performSelectorOnMainThread:@selector(aa) withObject:nil waitUntilDone:!NO];
		
		
		jpgData.length=0;			
	}
}

/*
-(void)aa
{
	//if (isCamera){
	//	sleep(1);
	//[self saveScreen];
	//}
	if(imgView.image!=nil){
		[imgView.image release];
	}
    imgView.image=image;
    
    
    [scrollView setDisplayImage:image];
    
	//if (isCamera){
	//	sleep(1);
	//[self saveScreen];
	//}
	if (videoflag==1) {
		[self writeVideo];
	}
	
	static int n=0;
	label.text=[NSString stringWithFormat:@"%d",n++];
}
*/
-(void)aa
{
	//if (isCamera){
	//	sleep(1);
	//[self saveScreen];
	//}
	if(imgView.image!=nil){
		[imgView.image release];
	}
    imgView.image=image;
    
    
    [scrollView setDisplayImage:image];
    
	//if (isCamera){
	//	sleep(1);
	//[self saveScreen];
	//}
	if (videoflag==1) {
        [self performSelectorOnMainThread:@selector(writeVideo) withObject:nil waitUntilDone:!NO];
        
		//[self writeVideo];
	}
	
	static int n=0;
	label.text=[NSString stringWithFormat:@"%d",n++];
}
-(void)pauseRecord{
	videoflag=0;
}

-(void)resumeRecord{
	videoflag=1;
}

-(int)getTimestamp{
    return timestamp;
}



-(void)on_Recv:(char*)data length:(int)length{
	static short requestLength=0;
	int pos=0;
	
	while (YES) {
			//读header
		while (position<sizeof(struct Tphead) && length-pos>0) {
			header[position]=data[pos];
			position++;
			pos++;
		}
			//header 没读完
		if (position<sizeof(struct Tphead)) {
			break;
		}
		
		requestLength=((struct Tphead*)header)->bodylen;
        
        timestamp=((struct Tphead*)header)->serial;
		
		//label.text  = [NSString stringWithFormat:@"requestLength=%d",requestLength];
		
		
			//body为0
		if (requestLength==0) {
			position=0;
			continue;
		}
		
			//读body
		while (position<requestLength+sizeof(struct Tphead) && length-pos>0) {
			buff[position-sizeof(struct Tphead)]=data[pos];
			position++;
			pos++;
		}
		
		if (position==requestLength+sizeof(struct Tphead)) {
			//label.text=@"decodeData";
			[self decodeData:buff length:requestLength];
			position=0;
		}else {
			break;
		}
	}
	
}




-(void)connect{
	NSError *err = [[NSError alloc]init];
//	BOOL b=[socket connectToHost:@"163.125.84.233" onPort:8000 withTimeout:-1 error:&err];
	BOOL b=[socket connectToHost:@"192.168.1.11" onPort:8000 withTimeout:-1 error:&err];
	if (!b){
		NSLog(@"failed err=%@",err);
	}
	[err release];
}

- (CVPixelBufferRef)pixelBufferFromCGImage:(CGImageRef)image size:(CGSize)size
{
	NSDictionary *options = [NSDictionary dictionaryWithObjectsAndKeys:
							 [NSNumber numberWithBool:YES], kCVPixelBufferCGImageCompatibilityKey,
							 [NSNumber numberWithBool:YES], kCVPixelBufferCGBitmapContextCompatibilityKey, nil];
	CVPixelBufferRef pxbuffer = NULL;
	CVReturn status = CVPixelBufferCreate(kCFAllocatorDefault, size.width, size.height, kCVPixelFormatType_32ARGB, (CFDictionaryRef) options, &pxbuffer);
	
	NSParameterAssert(status == kCVReturnSuccess && pxbuffer != NULL);
	
	CVPixelBufferLockBaseAddress(pxbuffer, 0);
	void *pxdata = CVPixelBufferGetBaseAddress(pxbuffer);
	NSParameterAssert(pxdata != NULL);
	
	CGColorSpaceRef rgbColorSpace = CGColorSpaceCreateDeviceRGB();
	CGContextRef context = CGBitmapContextCreate(pxdata, size.width, size.height, 8, 4*size.width, rgbColorSpace, kCGImageAlphaPremultipliedFirst);
	NSParameterAssert(context);
	
	CGContextDrawImage(context, CGRectMake(0, 0, CGImageGetWidth(image), CGImageGetHeight(image)), image);
	
	CGColorSpaceRelease(rgbColorSpace);
	CGContextRelease(context);
	
	CVPixelBufferUnlockBaseAddress(pxbuffer, 0);
	
	return pxbuffer;
}

-(void)prepareVideoSetting{
	NSMutableString *buildPath = [[NSMutableString alloc] init];
	[buildPath setString:@"Documents/"];
	int aNumber =arc4random()%8999 + 1000;
	
	[buildPath appendString:@"temporary"];
	[buildPath appendString:[NSString stringWithFormat:@"%d",aNumber]];
	[buildPath appendString:@".mov"];
    fullPath = [[NSHomeDirectory() stringByAppendingPathComponent:buildPath] retain];
	[buildPath release];
	
	//NSString *moviePath = [[NSBundle mainBundle] pathForResource:@"movie" ofType:@"mov"];
	
	size = CGSizeMake(320,240);//定义视频的大小
	
	NSError *error = nil;
	
	//unlink([moviePath UTF8String]);
	
	//—-initialize compression engine
	videoWriter = [[AVAssetWriter alloc] initWithURL:[NSURL fileURLWithPath:fullPath]
											fileType:AVFileTypeQuickTimeMovie
											   error:&error];
	NSParameterAssert(videoWriter);
	if(error)
		NSLog(@"error = %@", [error localizedDescription]);
	
	NSDictionary *videoSettings = [NSDictionary dictionaryWithObjectsAndKeys:AVVideoCodecH264, AVVideoCodecKey,
								   [NSNumber numberWithInt:size.width], AVVideoWidthKey,
								   [NSNumber numberWithInt:size.height], AVVideoHeightKey, nil];
	writerInput = [AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeVideo outputSettings:videoSettings];
	
	NSDictionary *sourcePixelBufferAttributesDictionary = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithInt:kCVPixelFormatType_32ARGB], kCVPixelBufferPixelFormatTypeKey, nil];
	
	adaptor = [[AVAssetWriterInputPixelBufferAdaptor
													 assetWriterInputPixelBufferAdaptorWithAssetWriterInput:writerInput sourcePixelBufferAttributes:nil] retain];
	NSParameterAssert(writerInput);
	NSParameterAssert([videoWriter canAddInput:writerInput]);
	
	if ([videoWriter canAddInput:writerInput])
		NSLog(@" ");
	else
		NSLog(@" ");
	
	[videoWriter addInput:writerInput];
    /*
	AudioChannelLayout acl;  
	
    bzero( &acl, sizeof(acl));  
	
    acl.mChannelLayoutTag = kAudioChannelLayoutTag_Mono;  
	NSDictionary* audioOutputSettings = nil;
	audioOutputSettings = [ NSDictionary dictionaryWithObjectsAndKeys:                         
						   
                          // [ NSNumber numberWithInt: kAudioFormatLinearPCM ], AVFormatIDKey,  
						   [ NSNumber numberWithInt: kAudioFormatMPEG4AAC ], AVFormatIDKey,  
						   
                           [ NSNumber numberWithInt:16], AVEncoderBitRateKey,  
						  // [ NSNumber numberWithInt:8], AVLinearPCMBitDepthKey, 
						  // [ NSNumber numberWithBool:NO], AVLinearPCMIsNonInterleaved, 
						  // [ NSNumber numberWithBool:NO], AVLinearPCMIsFloatKey, 
						  // [ NSNumber numberWithBool:NO],AVLinearPCMIsBigEndianKey,

						   
                           [ NSNumber numberWithFloat: 8000.0 ], AVSampleRateKey,  
						   
                           [ NSNumber numberWithInt: 1 ], AVNumberOfChannelsKey,                                        
						   
                           [ NSData dataWithBytes: &acl length: sizeof( acl ) ], AVChannelLayoutKey,  
						   
                           nil ];   
	
	
	
    audioWriterInput = [AVAssetWriterInput   
						 
						 assetWriterInputWithMediaType: AVMediaTypeAudio   
						 
						 outputSettings: audioOutputSettings];  
	
	
	
    audioWriterInput.expectsMediaDataInRealTime = YES;  
	
    // add input  
	NSParameterAssert(audioWriterInput);
	NSParameterAssert([videoWriter canAddInput:audioWriterInput]);
	
	if ([videoWriter canAddInput:audioWriterInput])
		NSLog(@" ");
	else
		NSLog(@" ");
	
    [videoWriter addInput:audioWriterInput]; 
     */
	
}

-(void)startWitting{
	[self prepareVideoSetting];
	[videoWriter startWriting];
	[videoWriter startSessionAtSourceTime:kCMTimeZero];
	//dispatchQueue = dispatch_queue_create("mediaInputQueue", NULL);
	frame = 0;
	videoflag=1;
}

- (void)writeVideo
{

	//合成多张图片为一个视频文件
	
	
	//[writerInput requestMediaDataWhenReadyOnQueue:dispatchQueue usingBlock:^{
		if([writerInput isReadyForMoreMediaData])
		{
			
			
			CVPixelBufferRef buffer = NULL;
			
			
			
			buffer = (CVPixelBufferRef)[self pixelBufferFromCGImage:[image CGImage] size:size];
			//CVPixelBufferPoolCreatePixelBuffer(NULL,adaptor.pixelBufferPool,&buffer);
			//[adaptor appendPixelBuffer:buffer withPresentationTime:kCMTimeZero];
            frame++;
			CMTime lastTime = CMTimeMake(frame,12.5);
			
			
			if (buffer)
			{
				
				if(![adaptor appendPixelBuffer:buffer withPresentationTime:lastTime])
					NSLog(@"FAIL");
				else
					CFRelease(buffer);
			}
			
			
			
			
		}
	//}];
	
	//unsigned char* aacBuff=[self convertFromPCMData:myBuff];
	
	//CMBlockBufferRef blockBuffer;
	//CMBlockBufferCreateWithMemoryBlock(NULL,aacBuff,2048,kCFAllocatorNull,NULL,0,2048,1,&blockBuffer);
	//CMSampleBufferRef audioBuffer;
	//CMSampleBufferCreate(kCFAllocatorDefault,blockBuffer,YES,NULL,NULL,NULL,0,0,NULL,0,NULL,&audioBuffer);
	
	/*

	CMSampleBufferRef audioBuffer;
	NSData* data = [NSData dataWithBytes:myBuff length:sizeof(unsigned char)*2048
					];
	[data getBytes:&audioBuffer length:sizeof(audioBuffer)];
	[data release];
	 */
	
	//if ([audioWriterInput isReadyForMoreMediaData])  
		
	//	if( ![audioWriterInput appendSampleBuffer:audioBuffer] )  
			
	//		NSLog(@"Unable to write to audio input");  
	
	//	else  
			
	//		NSLog(@"already write audio");  
}
 
-(void)stopVideoWritting{
	//[audioWriterInput markAsFinished];
	[writerInput markAsFinished];
	[videoWriter finishWriting];
	[videoWriter release];
	void(^completionBlock)(NSURL *, NSError *) =
    ^(NSURL *assetURL, NSError *error)
	{
		if(error != nil){
			//error
		}
		//remove temp movie file
		NSFileManager *fileMgr = [NSFileManager defaultManager];
		if([fileMgr removeItemAtPath:fullPath error:&error] != YES){
			//error
		}
	};
	ALAssetsLibrary *library = [[ALAssetsLibrary alloc] init];
	NSURL *filePathURL = [NSURL fileURLWithPath:fullPath isDirectory:NO];
	if([library videoAtPathIsCompatibleWithSavedPhotosAlbum:filePathURL]){
		
		[library writeVideoAtPathToSavedPhotosAlbum:filePathURL completionBlock:completionBlock];
	}
	//clean up
	[library release];
	/*
	NSFileManager *fileMgr = [NSFileManager defaultManager];
	if([fileMgr fileExistsAtPath:fullPath]){
		if([fileMgr removeItemAtPath:fullPath error:nil] != YES){
			//error
		}
	}
	 */
		videoflag=0;
		
	
}


/*
-(unsigned char*)convertFromPCMData:(unsigned char*)pcmbuff{
	
    unsigned long nSampleRate = 8000;  // 采样率
    unsigned int nChannels = 1;         // 声道数
    unsigned int nPCMBitSize = 16;      // 单样本位数
	unsigned long nInputSamples = 0;
    unsigned long nMaxOutputBytes = 0;
	
    int nRet;
    faacEncHandle hEncoder;
    faacEncConfigurationPtr pConfiguration; 
	
    
    int nPCMBufferSize=2048;
  //  BYTE* pbPCMBuffer;
	unsigned char* pbAACBuffer;
	
	hEncoder = faacEncOpen(nSampleRate, nChannels, &nInputSamples, &nMaxOutputBytes);
	//pbPCMBuffer = new BYTE [nPCMBufferSize];
	//unsigned char pbAACBuffer[nPCMBufferSize];
	
	pConfiguration = faacEncGetCurrentConfiguration(hEncoder);
    pConfiguration->inputFormat = FAAC_INPUT_16BIT;
	nRet = faacEncSetConfiguration(hEncoder, pConfiguration);
	
	nRet = faacEncEncode(hEncoder, (int*) pcmbuff, nInputSamples, pbAACBuffer, 2048);
	nRet = faacEncClose(hEncoder);
	return pbAACBuffer;
}

*/
#pragma mark socket
- (id)init{
    if (self = [super init]){
        socket = [[AsyncSocket alloc] initWithDelegate:self];		
		jpgData=[[NSMutableData alloc]init];
		
        imgView=[[UIImageView alloc] initWithFrame:CGRectMake(0,0,320,480)];
		imgView.backgroundColor=[UIColor redColor];
		imgView.image=nil;
        
        scrollView = [[KKScrollView alloc] initWithFrame:CGRectMake(0,0,320,480)];
        
        
        
		[self connect];
    }
    return self;
}

- (void)onSocket:(AsyncSocket *)sock didConnectToHost:(NSString *)host port:(UInt16)port{
	[delegate djSocket:self connectedStatusChange:YES];
    [socket readDataWithTimeout:-1 tag:0];
}


- (void)onSocketDidDisconnect:(AsyncSocket *)sock{
	[delegate djSocket:self connectedStatusChange:NO];
    position=0;
   	[self connect];
   

   
}


- (void)onSocket:(AsyncSocket *)sock didReadData:(NSData *)data withTag:(long)tag{
    
    NSLog(@"--did receive video ---\n");
    
	[self on_Recv:(char*)[data bytes] length:[data length]];
    [socket readDataWithTimeout:-1 tag:0];
}

-(void)dealloc{
    [socket release];
	[jpgData release];
	[imgView release];
    [scrollView release];
    
    [super dealloc];
}
@end