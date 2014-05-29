//
//  DetailViewController.h
//  Test
//
//  Created by mac0001 on 5/29/14.
//
//

#import <UIKit/UIKit.h>
#import "WIFITableViewController.h"

@interface DetailViewController : UIViewController<UITextFieldDelegate>

@property(nonatomic,retain)NSString *ssid;
@property(nonatomic,assign)WIFITableViewController *parent;
-(void)onresp:(BOOL)flag;
@end
