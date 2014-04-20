//
//  NetUtils.h
//  Test
//
//  Created by wangbo on 4/17/14.
//
//

#import <Foundation/Foundation.h>

typedef enum {
    CommandTypeAudio,
    CommandTypeDirection,
    CommandTypeRedMode,
    CommandTypeRedSwitch,
    CommandTypeState,
}CommandType;

@interface NetUtils : NSObject
+ (instancetype)sharedInstance;

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
