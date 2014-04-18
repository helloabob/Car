#import "AppDelegate.h"
#import "DemoViewController.h" 
//#import "DataVerifier.h"
#import "InputViewController.h"

#import <sys/utsname.h>
#import <SystemConfiguration/SCNetworkReachability.h>
#import <netinet/in.h>

@implementation AppDelegate
@synthesize window;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions { 
    
//	[NSThread sleepForTimeInterval:2.0];
//	if ([self connectedToNetwork]) {
//		self.window = [[[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]] autorelease];      
//		self.window.backgroundColor = [UIColor whiteColor];  
//		
//		DemoViewController *demo = [[DemoViewController alloc]init]; 
//		[self.window addSubview:demo.view];  
//	
//		[self.window makeKeyAndVisible];
//	}else{
//		UIAlertView * alertView = [[UIAlertView alloc] initWithTitle:@"提示" 
//															 message:@"未连接网络！" 
//															delegate:self 
//												   cancelButtonTitle:@"确定" 
//												   otherButtonTitles:nil];
//		[alertView show];
//		[alertView release];
//	}
    
    self.window = [[[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]] autorelease];
    self.window.backgroundColor = [UIColor whiteColor];
    
    InputViewController *demo = [[[InputViewController alloc] init] autorelease];
    UINavigationController *nav = [[[UINavigationController alloc] initWithRootViewController:demo] autorelease];
    nav.navigationBarHidden = YES;
    self.window.rootViewController = nav;
	
    [self.window makeKeyAndVisible];
    
    
    return YES;
}

-(BOOL)connectedToNetwork
{
	//创建零地址，0.0.0.0的地址表示查询本机的网络连接状态
	struct sockaddr_in zeroAddress;
	bzero(&zeroAddress, sizeof(zeroAddress));
	zeroAddress.sin_len = sizeof(zeroAddress);
	zeroAddress.sin_family = AF_INET;
	// Recover reachability flags
	SCNetworkReachabilityRef defaultRouteReachability = SCNetworkReachabilityCreateWithAddress(NULL, (struct sockaddr *)&zeroAddress);
	SCNetworkReachabilityFlags flags;
	
	//获得连接的标志
	BOOL didRetrieveFlags = SCNetworkReachabilityGetFlags(defaultRouteReachability, &flags);
	CFRelease(defaultRouteReachability);
	
	//如果不能获取连接标志，则不能连接网络，直接返回
	if (!didRetrieveFlags){
		return NO;
	}
	
	//根据获得的连接标志进行判断
	BOOL isReachable = flags & kSCNetworkFlagsReachable;
	BOOL needsConnection = flags & kSCNetworkFlagsConnectionRequired;
	return (isReachable && !needsConnection) ? YES:NO;
}


- (void)applicationWillResignActive:(UIApplication *)application {
    /*
     Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
     Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
     */
}


- (void)applicationDidEnterBackground:(UIApplication *)application {
    /*
     Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
     If your application supports background execution, called instead of applicationWillTerminate: when the user quits.
     */
}


- (void)applicationWillEnterForeground:(UIApplication *)application {
    /*
     Called as part of  transition from the background to the inactive state: here you can undo many of the changes made on entering the background.
     */
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    /*
     Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
     */
}


- (void)applicationWillTerminate:(UIApplication *)application {
    /*
     Called when the application is about to terminate.
     See also applicationDidEnterBackground:.
     */
}


#pragma mark -
#pragma mark Memory management

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application {
    /*
     Free up as much memory as possible by purging cached data objects that can be recreated (or reloaded from disk) later.
     */
}


- (void)dealloc {
    [window release];
    [super dealloc];
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex{
	switch (buttonIndex) {
		case 0:
		{
			abort();
		}
			break;
		case 1:
			
			break;
			
		default:
			break;
	}
	
}


@end
