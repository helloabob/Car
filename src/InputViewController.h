//
//  InputViewController.h
//  Test
//
//  Created by wangbo on 4/17/14.
//
//

#import <UIKit/UIKit.h>

@interface InputViewController : UIViewController<UITextFieldDelegate>

@property (nonatomic, retain) IBOutlet UITextField *txtTerminal;
@property (nonatomic, retain) IBOutlet UITextField *txtIp;
@property (nonatomic, retain) IBOutlet UITextField *txtPort;
@property (nonatomic, retain) IBOutlet UITextField *txtPwd;
@property (nonatomic, retain) IBOutlet UITextField *txtCallee;
@property (nonatomic, retain) IBOutlet UIButton *btnStart;
@property (nonatomic, retain) IBOutlet UIButton *btnSend;

@end
