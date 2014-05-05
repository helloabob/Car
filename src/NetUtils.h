//
//  NetUtils.h
//  Test
//
//  Created by wangbo on 4/17/14.
//
//

#import <Foundation/Foundation.h>

@protocol NetUtilsDelegate <NSObject>
- (void)onReceivedData:(unsigned char*)data length:(int)length;
@end

typedef enum {
    CommandTypeAudio,
    CommandTypeDirection,
    CommandTypeRedMode,
    CommandTypeRedSwitch,
    CommandTypeState,
    CommandTypeSpeech,
}CommandType;

@interface NetUtils : NSObject
+ (instancetype)sharedInstance;
@property (nonatomic, assign) id<NetUtilsDelegate>videoDelegate;
@property (nonatomic, assign) id<NetUtilsDelegate>audioDelegate;
-(NSString *)localIPAddress;
- (void)connectWithIP:(NSString *)ip
             withPort:(uint32_t)port
              withPwd:(NSString *)pwd
         withCallerId:(NSString *)callerId
         withCalleeId:(NSString *)cid;
- (void)initNetwork;
- (int)startCall:(NSString *)cid;
- (void)startSendData:(NSData *)data withType:(CommandType)type;
- (void)startSendData:(int)cmd withType:(CommandType)type forLength:(int)length;

@end
