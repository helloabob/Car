#import "AsyncSocket.h"
#import "spcaframe.h"
#import "NetUtils.h"


@class DjSocket;


@protocol DjSocketDelegate <NSObject>
@required
-(void)djSocket:(DjSocket*)djSocket connectedStatusChange:(bool)isConnected;
@end

@class KKScrollView;


@interface DjSocket : NSObject<NetUtilsDelegate>{
//    AsyncSocket *socket;
//	id<DjSocketDelegate> delegate;
    
    NSMutableDictionary *jpgDict;
    
	NSMutableData *jpgData;
	UIImageView *imgView;
    KKScrollView *scrollView;
    
	UIImage *image;
	UILabel *label;
	
	int position;
	char header[8];
	char buff[512*512];
	unsigned char abuff[1200];
    NSString *fullPath;
}
@property(nonatomic,assign)UIImageView *imgView;
@property(nonatomic,assign)UILabel*label;
//@property(nonatomic,assign)id<DjSocketDelegate> delegate;
@property(nonatomic,retain)UIImage *image;
@property(nonatomic,assign)BOOL shouldReceive;
@property(nonatomic,retain)NSString *fullPath;

+ (instancetype)sharedInstance;

@property (nonatomic, retain) KKScrollView *scrollView;

- (void)startShow;

-(void)test:(int)cmd;
-(int)getTimestamp;

@end