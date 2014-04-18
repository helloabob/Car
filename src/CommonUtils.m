//
//  CommonUtils.m
//  Test
//
//  Created by mac0001 on 4/18/14.
//
//

#import "CommonUtils.h"
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>

@implementation CommonUtils

+ (NSString *)localIPAddress
{
    NSString *localIP = nil;
    struct ifaddrs *addrs;
    if (getifaddrs(&addrs)==0) {
        const struct ifaddrs *cursor = addrs;
        NSMutableDictionary *dict = [NSMutableDictionary dictionary];
        while (cursor != NULL) {
            if (cursor->ifa_addr->sa_family == AF_INET && (cursor->ifa_flags & IFF_LOOPBACK) == 0)
            {
                NSString *name = [NSString stringWithUTF8String:cursor->ifa_name];
                //                if ([name isEqualToString:@"en0"]) // Wi-Fi adapter
                //                {
                [dict setObject:[NSString stringWithUTF8String:inet_ntoa(((struct sockaddr_in *)cursor->ifa_addr)->sin_addr)] forKey:name];
                //                    localIP = [NSString stringWithUTF8String:inet_ntoa(((struct sockaddr_in *)cursor->ifa_addr)->sin_addr)];
                //                    break;
                //                }
            }
            cursor = cursor->ifa_next;
        }
        freeifaddrs(addrs);
        if ([dict objectForKey:@"en0"]) {
            localIP = [dict objectForKey:@"en0"];
        } else if (dict.count > 0) {
            localIP = [dict objectForKey:[dict allKeys].firstObject];
        }
    }
    return localIP;
}

@end
