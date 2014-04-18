//
//  NetUtils.h
//  Test
//
//  Created by wangbo on 4/17/14.
//
//

#import <Foundation/Foundation.h>

@interface NetUtils : NSObject
+ (instancetype)sharedInstance;

- (void)connectWithIP:(NSString *)ip
             withPort:(uint32_t)port
              withPwd:(NSString *)pwd
         withCallerId:(NSString *)callerId
         withCalleeId:(NSString *)cid;
- (void)initNetwork;
- (int)startCall:(NSString *)cid;
- (void)startSendData:(NSData *)data;

@end
