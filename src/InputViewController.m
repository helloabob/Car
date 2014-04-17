//
//  InputViewController.m
//  Test
//
//  Created by wangbo on 4/17/14.
//
//

#import "InputViewController.h"
#import "NetUtils.h"

@interface InputViewController ()

@end

@implementation InputViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    // Do any additional setup after loading the view from its nib.
    
    self.btnStart.backgroundColor = [UIColor darkGrayColor];
    [self.btnStart addTarget:self action:@selector(btnStartTapped) forControlEvents:UIControlEventTouchUpInside];
    
}

- (void)btnStartTapped {
    kAddObserver(@selector(receiNotif:), @"init");
    [[NetUtils sharedInstance] connectWithIP:_txtIp.text withPort:[_txtPort.text intValue] withPwd:_txtPwd.text withCallerId:_txtTerminal.text withCalleeId:_txtCallee.text];
}

- (void)receiNotif:(NSNotification *)notif {
    NSLog(@"connected");
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField {
    [textField resignFirstResponder];
    return YES;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
