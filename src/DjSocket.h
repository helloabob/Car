﻿#import "AsyncSocket.h"
#import "spcaframe.h"


@class DjSocket;


@protocol DjSocketDelegate <NSObject>
@required
-(void)djSocket:(DjSocket*)djSocket connectedStatusChange:(bool)isConnected;
@end

@class KKScrollView;


@interface DjSocket : NSObject{
//    AsyncSocket *socket;
//	id<DjSocketDelegate> delegate;
	NSMutableData *jpgData;
	UIImageView *imgView;
    KKScrollView *scrollView;
    
	UIImage *image;
	UILabel *label;
	
	int position;
	char header[8];
	char buff[512*512];
	unsigned char abuff[1200];
}
@property(nonatomic,assign)UIImageView *imgView;
@property(nonatomic,assign)UILabel*label;
//@property(nonatomic,assign)id<DjSocketDelegate> delegate;
@property(nonatomic,assign)UIImage *image;


@property (nonatomic, retain) KKScrollView *scrollView;

-(void)test:(int)cmd;
-(int)getTimestamp;

@end