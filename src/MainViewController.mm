//
//  MainViewController.m
//  Test
//
//  Created by wangbo on 4/18/14.
//
//

#import "MainViewController.h"
#import "NetUtils.h"
#import "MBProgressHUD.h"
#import <AVFoundation/AVFoundation.h>
#import "DemoViewController.h"

#import <ZXingWidgetController.h>
#import <QRCodeReader.h>

#define WAVE_UPDATE_FREQUENCY   0.05

typedef enum ActionState{
    ActionStateIniting,
    ActionStateInited,
    ActionStateCalling,
    ActionStateCalled,
}ActionState;

@interface MainViewController ()<AVAudioRecorderDelegate> {
    UITextField *txtCalleeId;
    UIView *viewDial;
    UIView *viewSpeak;
    NSTimer * timer_;
    BOOL shouldInitNetwork;
    ActionState actionState_;
    UITextView *txtLog;
}

@property(nonatomic,retain) AVAudioRecorder * recorder;
@property(nonatomic,retain) NSString * recordPath;
@property(nonatomic) float recordTime;



@end

@implementation MainViewController
-(void)dealloc{
    NSLog(@"main_dealloc");
    kRemoveNotif(UIApplicationDidEnterBackgroundNotification);
    kRemoveNotif(UIApplicationDidBecomeActiveNotification);
    kRemoveNotif(@"stateChange");
    [super dealloc];
}
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
    // Do any additional setup after loading the view.
    self.view.backgroundColor = [UIColor lightGrayColor];
    
    viewDial = [[[UIView alloc] initWithFrame:self.view.bounds]autorelease];
    viewDial.backgroundColor = [UIColor lightGrayColor];
    [self.view addSubview:viewDial];
    viewSpeak = [[[UIView alloc] initWithFrame:self.view.bounds]autorelease];
    viewSpeak.backgroundColor = [UIColor lightGrayColor];
    [self.view addSubview:viewSpeak];
    [self.view bringSubviewToFront:viewDial];
    
    
    UILabel *lbl = [[[UILabel alloc] initWithFrame:CGRectMake(0, 0, 80, 40)]autorelease];
    lbl.text = @"终端号:";
    lbl.textAlignment = NSTextAlignmentCenter;
    lbl.font = [UIFont systemFontOfSize:14];
    txtCalleeId = [[[UITextField alloc] initWithFrame:CGRectMake(20, 120, 280, 40)] autorelease];
    txtCalleeId.leftView = lbl;
    txtCalleeId.leftViewMode = UITextFieldViewModeAlways;
    txtCalleeId.contentVerticalAlignment = UIControlContentVerticalAlignmentCenter;
    txtCalleeId.clearButtonMode = UITextFieldViewModeAlways;
    txtCalleeId.font = [UIFont systemFontOfSize:14];
    txtCalleeId.backgroundColor = [UIColor whiteColor];
    txtCalleeId.textColor = [UIColor blackColor];
    txtCalleeId.delegate = self;
    txtCalleeId.returnKeyType = UIReturnKeyDone;
    txtCalleeId.placeholder = @"对方终端号";
    txtCalleeId.text = @"00:AA:BB:CC:DD:EE";
    [viewDial addSubview:txtCalleeId];
    
    UIButton *btn = [[[UIButton alloc] initWithFrame:CGRectMake(20, 200, 280, 40)] autorelease];
    [btn setTitle:@"寻址" forState:UIControlStateNormal];
    btn.layer.cornerRadius = 10;
    [btn addTarget:self action:@selector(call) forControlEvents:UIControlEventTouchUpInside];
    btn.backgroundColor = [UIColor colorWithRed:0 green:0.6 blue:0 alpha:1];
    [viewDial addSubview:btn];
    
    btn = [[[UIButton alloc] initWithFrame:CGRectMake(20, 280, 280, 40)] autorelease];
    [btn setTitle:@"二维码扫描" forState:UIControlStateNormal];
    btn.layer.cornerRadius = 10;
    [btn addTarget:self action:@selector(scan) forControlEvents:UIControlEventTouchUpInside];
    btn.backgroundColor = [UIColor colorWithRed:0 green:0.6 blue:0 alpha:1];
    [viewDial addSubview:btn];
    
    btn = [[[UIButton alloc] initWithFrame:CGRectMake(20, 360, 280, 40)] autorelease];
    [btn setTitle:@"返回" forState:UIControlStateNormal];
    btn.layer.cornerRadius = 10;
    [btn addTarget:self action:@selector(goback) forControlEvents:UIControlEventTouchUpInside];
    btn.backgroundColor = [UIColor colorWithRed:0 green:0.6 blue:0 alpha:1];
    [viewDial addSubview:btn];
    
    btn = [[[UIButton alloc] initWithFrame:CGRectMake(110, 180, 100, 100)] autorelease];
    [btn addTarget:self action:@selector(startRecord) forControlEvents:UIControlEventTouchDown];
    [btn addTarget:self action:@selector(recordEnd) forControlEvents:UIControlEventTouchUpInside];
    // Set record cancel action for UIControlEventTouchUpOutside
    [btn addTarget:self action:@selector(recordCancel) forControlEvents:UIControlEventTouchUpOutside];
    [btn setBackgroundImage:[UIImage imageNamed:@"record_up"] forState:UIControlStateNormal];
    [btn setBackgroundImage:[UIImage imageNamed:@"record_down"] forState:UIControlStateHighlighted];
    [viewSpeak addSubview:btn];
    
#ifdef logShow
    txtLog = [[[UITextView alloc] initWithFrame:CGRectMake(0, 0, 320, 250)] autorelease];
    txtLog.backgroundColor = [UIColor whiteColor];
    txtLog.alpha=0.7f;
    txtLog.textColor=[UIColor blackColor];
    txtLog.text=@"";
    [self.view addSubview:txtLog];
    
    kAddObserver(@selector(receiLog:), @"log2");
#endif
    kAddObserver(@selector(didEnterBackground), UIApplicationDidEnterBackgroundNotification);
    kAddObserver(@selector(didBecomeActive), UIApplicationDidBecomeActiveNotification);
    kAddObserver(@selector(stateChange:), @"stateChange");
    shouldInitNetwork=YES;
    [self initNetwork];
}

-(void)goback{
    [self.navigationController popViewControllerAnimated:YES];
}

- (void)zxingController:(ZXingWidgetController*)controller didScanResult:(NSString *)result{
    NSLog(@"got:%@",result);
    [controller dismissViewControllerAnimated:NO completion:nil];
    txtCalleeId.text=result;
    [self call];
}
- (void)zxingControllerDidCancel:(ZXingWidgetController*)controller{
    [controller dismissViewControllerAnimated:NO completion:nil];
}

- (void)scan {
    ZXingWidgetController *widController = [[ZXingWidgetController alloc] initWithDelegate:self showCancel:YES OneDMode:NO];
    NSMutableSet *readers = [[NSMutableSet alloc ] init];
    QRCodeReader* qrcodeReader = [[QRCodeReader alloc] init];
    [readers addObject:qrcodeReader];
    [qrcodeReader release];
    widController.readers = readers;
    [readers release];
    [self presentViewController:widController animated:YES completion:nil];
    [widController release];
}

- (void)receiLog:(NSNotification *)notif {
    dispatch_async(dispatch_get_main_queue(), ^(){
        txtLog.text=[NSString stringWithFormat:@"%@%@",txtLog.text,notif.object];
    });
}

- (void)initNetwork {
    if (shouldInitNetwork==YES) {
        shouldInitNetwork=NO;
        actionState_=ActionStateIniting;
        MBProgressHUD *hud = [MBProgressHUD showHUDAddedTo:self.view animated:YES];
        hud.labelText = @"网络初始化...";
        [[NetUtils sharedInstance] initNetwork];
    }
}

- (void)startRecord {
    if ([[NSFileManager defaultManager] fileExistsAtPath:[NSString stringWithFormat:@"%@/Documents/MySound.caf", NSHomeDirectory()]]) {
        [[NSFileManager defaultManager] removeItemAtPath:[NSString stringWithFormat:@"%@/Documents/MySound.caf", NSHomeDirectory()] error:nil];
    }
    [self startRecordWithPath:[NSString stringWithFormat:@"%@/Documents/MySound.caf", NSHomeDirectory()]];
}
- (void)recordEnd {
    [self stopRecordWithCompletionBlock:^{
        NSData *data = [NSData dataWithContentsOfFile:[NSString stringWithFormat:@"%@/Documents/MySound.caf", NSHomeDirectory()]];
        MBProgressHUD *hud = [MBProgressHUD showHUDAddedTo:self.view animated:YES];
        hud.labelText = @"发送语音...";
        [[NetUtils sharedInstance] startSendData:data withType:CommandTypeAudio];
        [self performSelector:@selector(hideHUD) withObject:nil afterDelay:1.0f];
    }];
}
- (void)recordCancel {
    [self cancelled];
}

- (void)hideHUD {
    [MBProgressHUD hideHUDForView:self.view animated:YES];
}

- (void)didBecomeActive {
    if (shouldInitNetwork==YES) {
        [self initNetwork];
    }
}

- (void)didEnterBackground {
    if (shouldInitNetwork==NO) {
        shouldInitNetwork=YES;
        [MBProgressHUD hideHUDForView:self.view animated:NO];
        [[NetUtils sharedInstance] abortNetwork];
        [self returnPage1];
    }
    
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    NSLog(@"viewDidAppear");
}

- (void)stateChange:(NSNotification *)notfi {
    NSString *state = notfi.object;
    if ([state isEqualToString:@"callReady"]) {
        dispatch_async(dispatch_get_main_queue(), ^(){actionState_=ActionStateInited;[MBProgressHUD hideHUDForView:self.view animated:YES];});
    } else if ([state isEqualToString:@"onReady"]) {
        dispatch_async(dispatch_get_main_queue(), ^(){actionState_=ActionStateCalled;[MBProgressHUD hideHUDForView:self.view animated:YES];[self gotoPage2];});
    } else if ([state isEqualToString:@"onHangup"]) {
        dispatch_async(dispatch_get_main_queue(), ^(){if(actionState_==ActionStateCalling){[MBProgressHUD hideHUDForView:self.view animated:YES];}if(actionState_==ActionStateCalled){[self returnPage1];}});
    } else if ([state isEqualToString:@"sigpipe"]) {
        dispatch_async(dispatch_get_main_queue(), ^(){[self returnPage1];[self initNetwork];});
    }
}
- (void)gotoPage2{
//    [self.view bringSubviewToFront:viewSpeak];
    DemoViewController *vc = [[[DemoViewController alloc] init] autorelease];
    [self.navigationController pushViewController:vc animated:YES];
    [DjSocket sharedInstance].shouldReceive=YES;
}

- (void)returnPage1{
//    [self.view bringSubviewToFront:viewDial];
    if (self.navigationController.visibleViewController!=self) {
        [[myAudio sharedInstance] stopPlay];
        [DjSocket sharedInstance].shouldReceive=NO;
        [self.navigationController popToRootViewControllerAnimated:NO];
    }
}

- (void)call {
    MBProgressHUD *hud = [MBProgressHUD showHUDAddedTo:self.view animated:YES];
    hud.labelText = @"寻址中...";
    actionState_=ActionStateCalling;
    int result = [[NetUtils sharedInstance] startCall:txtCalleeId.text];
    [[DjSocket sharedInstance] startShow];
//    NSLog(@"call_result:%d", result);
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

#pragma mark - Publick Function

-(void)startRecordWithPath:(NSString *)path
{
    NSError * err = nil;
    
	AVAudioSession *audioSession = [AVAudioSession sharedInstance];
	[audioSession setCategory :AVAudioSessionCategoryPlayAndRecord error:&err];
    
	if(err){
        NSLog(@"audioSession: %@ %d %@", [err domain], [err code], [[err userInfo] description]);
        return;
	}
    
	[audioSession setActive:YES error:&err];
    
	err = nil;
	if(err){
        NSLog(@"audioSession: %@ %d %@", [err domain], [err code], [[err userInfo] description]);
        return;
	}
	
	NSMutableDictionary * recordSetting = [NSMutableDictionary dictionary];
	[recordSetting setValue :[NSNumber numberWithInt:kAudioFormatLinearPCM] forKey:AVFormatIDKey];
	[recordSetting setValue:[NSNumber numberWithFloat:8000.0] forKey:AVSampleRateKey];
	[recordSetting setValue:[NSNumber numberWithInt:1] forKey:AVNumberOfChannelsKey];
    [recordSetting setValue:[NSNumber numberWithInt:16] forKeyPath:AVLinearPCMBitDepthKey];
    [recordSetting setValue :[NSNumber numberWithBool:NO] forKey:AVLinearPCMIsBigEndianKey];
    [recordSetting setValue :[NSNumber numberWithBool:NO] forKey:AVLinearPCMIsFloatKey];
	
    /*
     [recordSetting setValue :[NSNumber numberWithInt:16] forKey:AVLinearPCMBitDepthKey];
     [recordSetting setValue :[NSNumber numberWithBool:NO] forKey:AVLinearPCMIsBigEndianKey];
     [recordSetting setValue :[NSNumber numberWithBool:NO] forKey:AVLinearPCMIsFloatKey];
     */
    
	self.recordPath = path;
	NSURL * url = [NSURL fileURLWithPath:self.recordPath];
	
	err = nil;
	
	NSData * audioData = [NSData dataWithContentsOfFile:[url path] options: 0 error:&err];
    
	if(audioData)
	{
		NSFileManager *fm = [NSFileManager defaultManager];
		[fm removeItemAtPath:[url path] error:&err];
	}
	
	err = nil;
    
    if(self.recorder){[self.recorder stop];self.recorder = nil;}
    
	self.recorder = [[[AVAudioRecorder alloc] initWithURL:url settings:recordSetting error:&err] autorelease];
    
	if(!_recorder){
        NSLog(@"recorder: %@ %d %@", [err domain], [err code], [[err userInfo] description]);
        UIAlertView *alert =
        [[UIAlertView alloc] initWithTitle: @"Warning"
								   message: [err localizedDescription]
								  delegate: nil
						 cancelButtonTitle:@"OK"
						 otherButtonTitles:nil];
        [alert show];
        return;
	}
	
	[_recorder setDelegate:self];
	[_recorder prepareToRecord];
	_recorder.meteringEnabled = YES;
	
	BOOL audioHWAvailable = audioSession.inputIsAvailable;
	if (! audioHWAvailable) {
        UIAlertView *cantRecordAlert =
        [[UIAlertView alloc] initWithTitle: @"Warning"
								   message: @"Audio input hardware not available"
								  delegate: nil
						 cancelButtonTitle:@"OK"
						 otherButtonTitles:nil];
        [cantRecordAlert show];
        return;
	}
	
	[_recorder recordForDuration:(NSTimeInterval) 60];
    
    self.recordTime = 0;
    [self resetTimer];
    
	timer_ = [NSTimer scheduledTimerWithTimeInterval:WAVE_UPDATE_FREQUENCY target:self selector:@selector(updateMeters) userInfo:nil repeats:YES];
    
//    [self showVoiceHudOrHide:YES];
    
}

-(void) stopRecordWithCompletionBlock:(void (^)())completion
{
    dispatch_async(dispatch_get_main_queue(),completion);
    [self cancelRecording];
    [self resetTimer];
//    [self showVoiceHudOrHide:NO];
}

- (void)cancelled {
    
//    [self showVoiceHudOrHide:NO];
    [self resetTimer];
    [self cancelRecording];
}

-(void) cancelRecording
{
    if (self.recorder.isRecording) {
        [self.recorder stop];
    }
    
    self.recorder = nil;
}

-(void) resetTimer
{
    if (timer_) {
        [timer_ invalidate];
        timer_ = nil;
    }
}

- (void)updateMeters {
    
    self.recordTime += WAVE_UPDATE_FREQUENCY;
    
//    if (voiceHud_)
//    {
//        /*  发送updateMeters消息来刷新平均和峰值功率。
//         *  此计数是以对数刻度计量的，-160表示完全安静，
//         *  0表示最大输入值
//         */
//        
//        if (_recorder) {
//            [_recorder updateMeters];
//        }
//        
//        float peakPower = [_recorder averagePowerForChannel:0];
//        double ALPHA = 0.05;
//        double peakPowerForChannel = pow(10, (ALPHA * peakPower));
//        
//        [voiceHud_ setProgress:peakPowerForChannel];
//    }
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
