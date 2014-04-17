//
//  AppDelegate.h
//  LocalPush
//

#import <UIKit/UIKit.h>

@interface AppDelegate : NSObject <UIApplicationDelegate,UIAlertViewDelegate> {
    UIWindow *window;
}

@property (nonatomic, retain) UIWindow *window;

-(BOOL)connectedToNetwork;

@end
