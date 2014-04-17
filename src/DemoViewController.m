#import "DemoViewController.h"
//#import "gmeNSF/Test.h"
// #import "MmsPlayer.h"

#import "KKScrollView.h"

//#define kFilteringFactor   0.03

@implementation DemoViewController;

@synthesize djSocket;
@synthesize mA;

NSString *strArr[]={
	@"CAR_FONT",
	@"CAR_BACK",
	@"CAR_R_FONT",
	@"CAR_L_FONT",
	@"CAR_R_BACK",
	@"CAR_L_BACK",
	@"CAR_L_ROUND",
	@"CAR_R_ROUND",
	@"CAR_STOP",
	@"CAR_IR_IN",
	@"CAR_IR_OFF",
	@"CAR_REC_START",
	@"CAR_REC_END"
};

int *intArr[]={
	CAR_FONT,
	CAR_BACK,
	CAR_R_FONT,
	CAR_L_FONT,
	CAR_R_BACK,
	CAR_L_BACK,
	CAR_L_ROUND,
	CAR_R_ROUND,
	CAR_STOP,
	CAR_IR_IN,
	CAR_IR_OFF,
	CAR_REC_START,
	CAR_REC_END
};

//int curTag=0;
//float accelX=0.0f,accelY=0.0f,accelZ=0.0f;
long starttime;
BOOL gravity=NO;
BOOL isRecord=NO;
int deta=32;
//MmsPlayer *mmsPlayer;


-(void)showButton:(int)func image1:(NSString*)str1 image2:(NSString*)str2 x:(int)x y:(int)y Hidden:(BOOL)isHidden{
	UIImage *img1 = [UIImage imageNamed:str1];
	UIImage *img2 = [UIImage imageNamed:str2];
	
	UIButton *btn = [UIButton buttonWithType:UIButtonTypeCustom];
	if(x<0){
		x=320-img1.size.width/2+x;
	}
	[btn setFrame:CGRectMake(x,y, img1.size.width/2, img1.size.height/2)];
	[btn setTag:func];
	
	[btn addTarget:self action:@selector(butClick:) forControlEvents:UIControlEventTouchUpInside];
	
	[btn setBackgroundImage:img1 forState:UIControlStateNormal];
	[btn setBackgroundImage:img2 forState:UIControlStateHighlighted];
	[btn setHidden:isHidden];
	[self.view addSubview:btn];
	//[btn release];
}

-(void)showButtonEx:(int)func image1:(NSString*)str1 image2:(NSString*)str2 x:(int)x y:(int)y{
	UIImage *img1 = [UIImage imageNamed:str1];
	UIImage *img2 = [UIImage imageNamed:str2];
	
	UIButton *btn = [UIButton buttonWithType:UIButtonTypeCustom];
	if(x<0){
		x=320-img1.size.width/2+x;
	}
	[btn setFrame:CGRectMake(x,y, img1.size.width*2/3, img1.size.height*2/3)];
	[btn setTag:func];
	[btn addTarget:self action:@selector(butClickEx:) forControlEvents:UIControlEventTouchDown];
	[btn addTarget:self action:@selector(butClickUp:) forControlEvents:UIControlEventTouchUpInside];
	[btn setBackgroundImage:img1 forState:UIControlStateNormal];
	[btn setBackgroundImage:img2 forState:UIControlStateHighlighted];
	//[btn setBackgroundImage:img2 forState:UIControlState];
	[self.view addSubview:btn];
	//[btn release];
}


- (void)viewDidLoad
{
	[super viewDidLoad];
	[[UIAccelerometer sharedAccelerometer] setUpdateInterval:0.05];
    [[UIAccelerometer sharedAccelerometer] setDelegate:self];
	
	djSocket=[[DjSocket alloc]init];
	djSocket.delegate=self;
    
    mA=[[myAudio alloc] init];
    [mA startPlay];
	
	//[djSocket setAudioData:[mA getBuff]];
	
	//[djSocket prepareVideoSetting];
    
	djSocket.label = [[UILabel alloc]initWithFrame:CGRectMake(0, 20, 300, 60)];
	label=djSocket.label;
    label.text = @"test";
    label.textAlignment = UITextAlignmentCenter;
    label.textColor = [UIColor redColor];
    label.font = [UIFont systemFontOfSize:20];
    [self.view addSubview:label];
	
	/*
     for(int i=0;i<13;i++)
     {
     UIButton *button1 = [UIButton buttonWithType:UIButtonTypeRoundedRect];
     button1.frame = CGRectMake(40+(i%2)*120, 80+(i/2)*50, 80, 40);
     button1.tag=i;
     [button1 setTitle:strArr[i] forState:UIControlStateNormal];
     [button1 addTarget:self action:@selector(butClick:) forControlEvents:UIControlEventTouchUpInside];
     [self.view addSubview:button1];
     }*/
	
	//[self.view addSubview:djSocket.imgView];
    
    [djSocket.scrollView setFrame:self.view.bounds];
    //   [djSocket.scrollView setDisplayImage:[UIImage imageNamed:@"ttt1.jpg"]];
    
	[self.view addSubview:djSocket.scrollView];
    
	
	//[self showButton:40 image1:@"icon_r2_c4_s1.png" image2:@"icon_r2_c12_s1.png" x:10 y:5 Hidden:NO];//放大
	//[self showButton:41 image1:@"icon_r2_c21_s1.png" image2:@"icon_r2_c27_s1.png" x:-10 y:5 Hidden:NO];//缩小
	
	[self showButton:42 image1:@"icon_r9_c5_s1.png" image2:@"icon_r11_c5_s1.png" x:100 y:400 Hidden:NO];//重力感应
	[self showButton:43 image1:@"icon_r13_c5_s1.png" image2:@"icon_r15_c5_s1.png" x:100 y:400 Hidden:YES];//1选中
	
	
	[self showButton:34 image1:@"icon_r9_c30_s1.png" image2:@"icon_r11_c30_s1.png" x:60 y:400  Hidden:NO];//红外等已关闭
	[self showButton:35 image1:@"icon_r13_c30_s1.png" image2:@"icon_r15_c30_s1.png" x:60 y:400  Hidden:YES];//红外灯已开启
	
    
	
	[self showButton:31 image1:@"icon_r9_c18_s1.png" image2:@"icon_r11_c18_s1.png" x:140 y:400 Hidden:NO];//录像
	[self showButton:32 image1:@"icon_r13_c13_s1.png" image2:@"icon_r15_c13_s1.png" x:140 y:400  Hidden:YES];
	[self showButton:33 image1:@"icon_r13_c40_s1.png" image2:@"icon_r15_c40_s1.png" x:140 y:400 Hidden:YES];
	//[self showButton:30 image1:@"icon_r13_c18_s1.png" image2:@"icon_r15_c18_s1.png" x:140 y:400 Hidden:NO];//
	
	
	
	[self showButton:12 image1:@"icon_r9_c24_s1.png" image2:@"icon_r11_c24_s1.png" x:180 y:400 Hidden:YES];//已关闭音频
	[self showButton:13 image1:@"icon_r13_c24_s1.png" image2:@"icon_r15_c24_s1.png" x:180 y:400 Hidden:NO];//已开启音频
	
	
	
	
	[self showButton:10 image1:@"icon_r9_c33_s1.png" image2:@"icon_r11_c33_s1.png" x:220 y:400 Hidden:NO];//车灯开
	[self showButton:11 image1:@"icon_r13_c33_s1.png" image2:@"icon_r15_c33_s1.png" x:220 y:400 Hidden:YES];//车灯关
	
	
	
	[self showButton:44 image1:@"icon_r18_c34_s1.png" image2:@"icon_r18_c39_s1.png" x:-10 y:360 Hidden:NO];//拍照
	
	
	
	[self showButton:45 image1:@"icon_r19_c3_s1.png" image2:@"icon_r19_c19_s1.png" x:10 y:400 Hidden:NO];//设置
	[self showButton:46 image1:@"icon_r19_c11_s1.png" image2:@"icon_r19_c24_s1.png" x:-10 y:400 Hidden:NO];//图片
	[self showButton:47 image1:@"icon_r17_c44_s1.png" image2:@"icon_r17_c46_s1.png" x:10 y:360 Hidden:NO];//路径
	
	
	//[self showButton:0 image1:@"icon_r5_c20_s1.png" image2:@"icon_r5_c26_s1.png" x:-14 y:100];//太阳
	//[self showButton:0 image1:@"icon_r5_c2_s1.png" image2:@"icon_r5_c7_s1.png" x:-14 y:100];//太阳选中
	[self showButtonEx:7 image1:@"icon_r4_c32_s1.png" image2:@"icon_r4_c37_s1.png" x:260 y:230];//上
	[self showButtonEx:5 image1:@"icon_r6_c32_s1.png" image2:@"icon_r6_c37_s1.png" x:260 y:290];//下
	[self showButtonEx:8 image1:@"icon_r4_c32_s1.png" image2:@"icon_r4_c37_s1.png" x:10 y:230];//上
	[self showButtonEx:6 image1:@"icon_r6_c32_s1.png" image2:@"icon_r6_c37_s1.png" x:10 y:290];//下
    
    
    BOOL flag1 = [djSocket.scrollView canDeleteScale];
    ////////////////////////////
    BOOL flag2 = [djSocket.scrollView canAddScale];
    
    UIButton *Addbutton = (UIButton*)[self.view viewWithTag:40];
    Addbutton.enabled = flag2;
    UIButton *deleBtn = (UIButton*)[self.view viewWithTag:41];
    deleBtn.enabled = flag1;
    if(flag2){
        [djSocket.scrollView addScale];
    }
    
}
- (void)viewWillAppear:(BOOL)animated{
    //if([[self.view viewWithTag:12] isHidden]){
        
      //  mA=[[myAudio alloc] init];
       // [mA startPlay];
    //}
    switch ([[UIApplication sharedApplication] statusBarOrientation]){
       case UIInterfaceOrientationPortrait:
        
            djSocket.imgView.frame = CGRectMake(djSocket.imgView.frame.origin.x, djSocket.imgView.frame.origin.y, self.view.bounds.size.width, self.view.bounds.size.height);
            
            [self layoutViewsWithTag:40 point:CGPointMake(10, 5)];
            [self layoutViewsWithTag:41 point:CGPointMake(274, 5)];
            [self layoutViewsWithTag:42 point:CGPointMake(100, 400)];
            [self layoutViewsWithTag:43 point:CGPointMake(100, 400)];
            [self layoutViewsWithTag:34 point:CGPointMake(60, 400)];
            [self layoutViewsWithTag:35 point:CGPointMake(60, 400)];
            
            [self layoutViewsWithTag:31 point:CGPointMake(140, 400)];
            [self layoutViewsWithTag:32 point:CGPointMake(140, 400)];
            [self layoutViewsWithTag:33 point:CGPointMake(140, 400)];
            
            [self layoutViewsWithTag:12 point:CGPointMake(180, 400)];
            [self layoutViewsWithTag:13 point:CGPointMake(180, 400)];
            
            [self layoutViewsWithTag:10 point:CGPointMake(220, 400)];
            [self layoutViewsWithTag:11 point:CGPointMake(220, 400)];
            
            [self layoutViewsWithTag:44 point:CGPointMake(278, 360)];
            
            [self layoutViewsWithTag:45 point:CGPointMake(10, 400)];
            [self layoutViewsWithTag:46 point:CGPointMake(274, 400)];
            [self layoutViewsWithTag:47 point:CGPointMake(10, 360)];
            
            [self layoutViewsWithTag:7 point:CGPointMake(260, 230)];
            [self layoutViewsWithTag:5 point:CGPointMake(260, 290)];
            [self layoutViewsWithTag:8 point:CGPointMake(10, 230)];
            [self layoutViewsWithTag:6 point:CGPointMake(10, 290)];
            
            break;
        case UIInterfaceOrientationPortraitUpsideDown:
            djSocket.imgView.frame = CGRectMake(djSocket.imgView.frame.origin.x, djSocket.imgView.frame.origin.y, self.view.bounds.size.width, self.view.bounds.size.height);
            
            [self layoutViewsWithTag:40 point:CGPointMake(10, 5)];
            UIButton *button = (UIButton*)[self.view viewWithTag:41];
            CGRect buttonFrame = button.frame;
            [self layoutViewsWithTag:41 point:CGPointMake(self.view.bounds.size.width - buttonFrame.size.width, 5)];
            
            //////////
            button = (UIButton*)[self.view viewWithTag:45];
            buttonFrame = button.frame;
            float offY = self.view.bounds.size.height - buttonFrame.size.height;
            [self layoutViewsWithTag:45 point:CGPointMake(10, offY)];
            buttonFrame.origin = CGPointMake(buttonFrame.origin.x + 40 + 80, offY);
            ///////////////////////////////
            button = (UIButton*)[self.view viewWithTag:34];
            [self layoutViewsWithTag:34 point:buttonFrame.origin];
            button = (UIButton*)[self.view viewWithTag:35];
            [self layoutViewsWithTag:35 point:buttonFrame.origin];
            buttonFrame.origin = CGPointMake(buttonFrame.origin.x + 40 , offY);
            
            ///////////////////////////////
            button = (UIButton*)[self.view viewWithTag:42];
            [self layoutViewsWithTag:42 point:buttonFrame.origin];
            buttonFrame.origin = CGPointMake(buttonFrame.origin.x , offY);
            button = (UIButton*)[self.view viewWithTag:43];
            [self layoutViewsWithTag:43 point:buttonFrame.origin];
            
            buttonFrame.origin = CGPointMake(buttonFrame.origin.x + 40, offY);
            
            button = (UIButton*)[self.view viewWithTag:31];
            [self layoutViewsWithTag:31 point:buttonFrame.origin];
            button = (UIButton*)[self.view viewWithTag:32];
            [self layoutViewsWithTag:32 point:buttonFrame.origin];
            button = (UIButton*)[self.view viewWithTag:33];
            [self layoutViewsWithTag:33 point:buttonFrame.origin];
            buttonFrame.origin = CGPointMake(buttonFrame.origin.x + 40, offY);
            ///////////////////////////////
            button = (UIButton*)[self.view viewWithTag:12];
            [self layoutViewsWithTag:12 point:buttonFrame.origin];
            button = (UIButton*)[self.view viewWithTag:13];
            [self layoutViewsWithTag:13 point:buttonFrame.origin];
            buttonFrame.origin = CGPointMake(buttonFrame.origin.x + 40, offY);
            ///////////////////////////////    ///////////////////////////////
            button = (UIButton*)[self.view viewWithTag:10];
            [self layoutViewsWithTag:10 point:buttonFrame.origin];
            button = (UIButton*)[self.view viewWithTag:11];
            [self layoutViewsWithTag:11 point:buttonFrame.origin];
            buttonFrame.origin = CGPointMake(buttonFrame.origin.x + 40, offY);
            
            /////////////////////////
            //////////////////////////////////////////////////////////////
            button = (UIButton*)[self.view viewWithTag:46];
            [self layoutViewsWithTag:46 point:CGPointMake(self.view.bounds.size.width - button.frame.size.width, offY)];
            
            ///////////////////////////////    ///////////////////////////////
            button = (UIButton*)[self.view viewWithTag:44];
            buttonFrame = button.frame;
            
            [self layoutViewsWithTag:47 point:CGPointMake(10, self.view.bounds.size.height - 60 - 10)];
            
            [self layoutViewsWithTag:44 point:CGPointMake(self.view.bounds.size.width - buttonFrame.size.width, (self.view.bounds.size.height - 60 - 10))];
            
            
            //// 箭头button
            button = (UIButton*)[self.view viewWithTag:7];
            buttonFrame = button.frame;
            buttonFrame.origin = CGPointMake(self.view.bounds.size.width - buttonFrame.size.width, (self.view.bounds.size.height - 2*button.bounds.size.height)/2.0);
            [self layoutViewsWithTag:7 point:buttonFrame.origin];
            
            buttonFrame.origin = CGPointMake(buttonFrame.origin.x, buttonFrame.origin.y + buttonFrame.size.height);
            [self layoutViewsWithTag:5 point:buttonFrame.origin];
            buttonFrame.origin = CGPointMake(self.view.bounds.size.width - buttonFrame.size.width, (self.view.bounds.size.height - 2*button.bounds.size.height)/2.0);
            [self layoutViewsWithTag:8 point:CGPointMake(10, buttonFrame.origin.y)];
            [self layoutViewsWithTag:6 point:CGPointMake(10, buttonFrame.origin.y + buttonFrame.size.height)];
            break;
     }   
    [super viewWillAppear:animated];
    
    
}
-(void)djSocket:(DjSocket*)djSocket connectedStatusChange:(bool)isConnected{
	label.text =isConnected?@"连接成功":@"连接失败";
}


-(IBAction) butClick:(id)sender{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	int tag=((UIButton *)sender).tag;
	UIButton *myButton1;
	UIButton *myButton2;
	if (tag<14) {
		//label.text = strArr[tag];
		[djSocket test:tag];
		UIButton *myButton1,*myButton2;
		if (tag==10) {//车灯开
			myButton1 = (UIButton *)[self.view viewWithTag:10];
			myButton2 = (UIButton *)[self.view viewWithTag:11];
			[myButton1 setHidden:YES];
			[myButton2 setHidden:NO];
			//[myButton1 release];
			//[myButton2 release];
			
		}
		if (tag==11) {//车灯关
			myButton1 = (UIButton *)[self.view viewWithTag:11];
			myButton2 = (UIButton *)[self.view viewWithTag:10];
			[myButton1 setHidden:YES];
			[myButton2 setHidden:NO];
			//[myButton1 release];
			//[myButton2 release];
			
		}
		if (tag==12) {//开启音频
			//mmsPlayer=[[MmsPlayer alloc]init];
			//mA=[[myAudio alloc] init];
			[mA startPlay];
			myButton1 = (UIButton *)[self.view viewWithTag:12];
			myButton2 = (UIButton *)[self.view viewWithTag:13];
			[myButton1 setHidden:YES];
			[myButton2 setHidden:NO];
			//[myButton1 release];
			//[myButton2 release];
			
		}
		if (tag==13) { //关闭音频
			[mA stopPlay];
			/*
             if (mA!=nil) {
             [mA stopSound];
             [mA release];
             mA=nil;
             }
			 */
			[[self.view viewWithTag:13] setHidden:YES];
			[[self.view viewWithTag:12] setHidden:NO];
			//myButton1 = (UIButton *)[self.view viewWithTag:13];
			//myButton2 = (UIButton *)[self.view viewWithTag:12];
			//[myButton1 setHidden:YES];
			//[myButton2 setHidden:NO];
			//[myButton1 release];
			//[myButton2 release];
			
		}
		
		
	}else{
		if (tag==31) { //开启录像
			
			[djSocket startWitting];
			
			myButton1 = (UIButton *)[self.view viewWithTag:31];
			myButton2 = (UIButton *)[self.view viewWithTag:32];
			UIButton *myButton3=(UIButton *)[self.view viewWithTag:33];
			//UIButton *myButton4=(UIButton *)[self.view viewWithTag:33];
			[myButton1 setHidden:YES];
			[myButton2 setHidden:NO];
			[myButton3 setHidden:YES];
			//[myButton4 setHidden:YES];
            
			
			
		}
		
		if (tag==32) { //停止或暂停录像对话框询问
			
			UIAlertView* dialog = [[UIAlertView alloc] init];
			[dialog setDelegate:self];
			[dialog setTitle:@"暂停还是保存？"];
			[dialog setMessage:@" "];
			[dialog addButtonWithTitle:@"暂停"];
			
			[dialog addButtonWithTitle:@"保存"];
			
			isRecord=YES;
			CGAffineTransform moveUp = CGAffineTransformMakeTranslation(0.0, 100.0);
			[dialog setTransform: moveUp];
			[dialog show];
			
			[dialog release];
			
			
			
		}
		
		if (tag==33) { //
            
			[djSocket resumeRecord];
			[[self.view viewWithTag:31] setHidden:YES];
			[[self.view viewWithTag:32] setHidden:NO];
			[[self.view viewWithTag:33] setHidden:YES];
			
		}
		
		if (tag==34) { //开启红外灯
			
			[djSocket test:0x0E];
			
			myButton1 = (UIButton *)[self.view viewWithTag:34];
			myButton2 = (UIButton *)[self.view viewWithTag:35];
			
			[myButton1 setHidden:YES];
			[myButton2 setHidden:NO];
			
			
			
			
		}
		if (tag==35) { //关闭红外灯
			[djSocket test:0x0F];
			myButton1 = (UIButton *)[self.view viewWithTag:35];
			myButton2 = (UIButton *)[self.view viewWithTag:34];
			
			[myButton1 setHidden:YES];
			[myButton2 setHidden:NO];
			
			
			
		}
		if(tag==40){//放大
            
            
            BOOL flag2 = [djSocket.scrollView canAddScale];
            if(flag2){
                [djSocket.scrollView addScale];
            }
            UIButton *Addbutton = (UIButton*)[self.view viewWithTag:40];
            Addbutton.enabled = flag2;
            
            ////////////////////////////
            BOOL flag1 = [djSocket.scrollView canDeleteScale];
            ////////////////////////////
            
            UIButton *deleBtn = (UIButton*)[self.view viewWithTag:41];
            deleBtn.enabled = flag1;
            
			//启用动画移动
			[UIImageView beginAnimations:nil context:NULL];
			
			//移动时间1秒
			[UIImageView setAnimationDuration:1];
			
			//图片持续移动
			[UIImageView setAnimationBeginsFromCurrentState:YES];
            
            
            
            
            //////////// /////////////////////
			int width=djSocket.imgView.frame.size.width;
			int height=djSocket.imgView.frame.size.height;
			if (width<480) {
				//重新定义图片的位置和尺寸,位置
				djSocket.imgView.frame = CGRectMake(0, 0, width+deta, height+deta);
				djSocket.imgView.center= CGPointMake(160, 240);
				//完成动画移动
				[UIImageView commitAnimations];
			}
			
		}
		if(tag==41){//缩小
			
            ////////////////////////////
            BOOL flag1 = [djSocket.scrollView canDeleteScale];
            if(flag1){
                
                [djSocket.scrollView deleteScale];
            }
            UIButton *deleBtn = (UIButton*)[self.view viewWithTag:41];
            deleBtn.enabled = flag1;
            ////////////////////////////
			BOOL flag2 = [djSocket.scrollView canAddScale];
            
            UIButton *Addbutton = (UIButton*)[self.view viewWithTag:40];
            Addbutton.enabled = flag2;
            ///////////////////////
            
            //启用动画移动
			[UIImageView beginAnimations:nil context:NULL];
			
			//移动时间1秒
			[UIImageView setAnimationDuration:1];
			
			//图片持续移动
			[UIImageView setAnimationBeginsFromCurrentState:YES];
			
            
            
            
            
			int width=djSocket.imgView.frame.size.width;
			int height=djSocket.imgView.frame.size.height;
			if (width>320) {
				//重新定义图片的位置和尺寸,位置
				djSocket.imgView.frame = CGRectMake(0, 0, width-deta, height-deta);
				djSocket.imgView.center= CGPointMake(160, 240);
				//完成动画移动
				[UIImageView commitAnimations];
			}
		}
		if (tag==42) {//重力感应开
            [djSocket test:0x10];
			myButton1 = (UIButton *)[self.view viewWithTag:42];
			myButton2 = (UIButton *)[self.view viewWithTag:43];
			[myButton1 setHidden:YES];
			[myButton2 setHidden:NO];
			gravity=YES;
			
		}
		if (tag==43) {//重力感应关
            //[djSocket test:9];
            [djSocket test:0x11];
			myButton1 = (UIButton *)[self.view viewWithTag:43];
			myButton2 = (UIButton *)[self.view viewWithTag:42];
			[myButton1 setHidden:YES];
			[myButton2 setHidden:NO];
			gravity=NO;
            
            //  [djSocket test:0x09];
			
			
			
		}
		if(tag==44){//拍照
			
			//UIImage *img=[UIImage imageWithData:UIImageJPEGRepresentation(djSocket.imgView.image,1.0)];
			UIImageWriteToSavedPhotosAlbum(djSocket.image,self, @selector(image:didFinishSavingWithError:contextInfo:), nil);
			UIAlertView* dialog = [[UIAlertView alloc] init];
			[dialog setDelegate:self];
			[dialog setTitle:@"已保存到相册"];
			[dialog setMessage:@" "];
			[dialog addButtonWithTitle:@"确定"];
			//[dialog addButtonWithTitle:@"视频"];
			
			
			CGAffineTransform moveUp = CGAffineTransformMakeTranslation(0.0, 100.0);
			[dialog setTransform: moveUp];
			[dialog show];
			[dialog release];
			//[djSocket setCamera];
            
		}
		if(tag==45){//设置
		}
		if(tag==46){//相册
			/*
             UIAlertView* dialog = [[UIAlertView alloc] init];
             [dialog setDelegate:self];
             [dialog setTitle:@"照片还是视频？"];
             [dialog setMessage:@" "];
             [dialog addButtonWithTitle:@"照片"];
             [dialog addButtonWithTitle:@"视频"];
             
             
             CGAffineTransform moveUp = CGAffineTransformMakeTranslation(0.0, 100.0);
             [dialog setTransform: moveUp];
             [dialog show];
             [dialog release];
             
             */
            //关闭音频
         // UIButton *myButton = (UIButton *)[self.view viewWithTag:12];
         // if ([myButton isHidden]) {
           //   [mA stopSound];
           //   [mA release];
            //  mA=nil;

           //     [[self.view viewWithTag:13] setHidden:NO];
             //   [[self.view viewWithTag:12] setHidden:YES];
          //}
            
			
			UIImagePickerController *imagePickerController = [[UIImagePickerController alloc] init];
			//imagePickerController.sourceType =UIImagePickerControllerSourceTypePhotoLibrary;
			imagePickerController.sourceType =UIImagePickerControllerSourceTypeSavedPhotosAlbum;
			imagePickerController.mediaTypes=[UIImagePickerController availableMediaTypesForSourceType:imagePickerController.sourceType];
			imagePickerController.allowsImageEditing=YES;
			
			imagePickerController.delegate = self;
			
			[self presentModalViewController:imagePickerController animated:YES];
			
			[imagePickerController release];
            
			
		}
		if(tag==47){//路径
		}
		
	}
	[pool release];
	
	
}

-(IBAction) butClickEx:(id)sender{
	if (!gravity) {
		int tag=((UIButton *)sender).tag;
		
        //int tagplus=curTag+tag;
		if(tag==7){
			
			[djSocket test:5];
		}else if(tag==5){
			[djSocket test:7];
			//curTag=0;
		}else if(tag==8){
			[djSocket test:1];
			//curTag=0;
		}else{
			
			[djSocket test:3];
		}
	}
	
    
}
-(IBAction) butClickUp:(id)sender{
	if (!gravity) {
        int tag=((UIButton *)sender).tag;
		
        //int tagplus=curTag+tag;
		if(tag==7){
			
			[djSocket test:6];
		}else if(tag==5){
			[djSocket test:8];
			//curTag=0;
		}else if(tag==8){
			[djSocket test:2];
			//curTag=0;
		}else{
			
			[djSocket test:4];
		}
	}
	
}

-(void)viewDidUnload {
    [djSocket test:0X7D];
}

-(void)dealloc{
    [djSocket release];
    
	[mA release];
    [super dealloc];
}

-(void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration
{
    if (gravity) {
        //accelX=((acceleration.x*kFilteringFactor)+(accelX*(1-kFilteringFactor)));
        //accelY=((acceleration.y*kFilteringFactor)+(accelY*(1-kFilteringFactor)));
        //accelZ=((acceleration.z*kFilteringFactor)+(accelZ*(1-kFilteringFactor)));
        
        //float moveX=acceleration.x-accelX;
        //float moveY=acceleration.y-accelY;
        // float moveZ=acceleration.z-accelZ;
        /*
         float moveX=acceleration.x;
         float moveY=acceleration.y;
         float moveZ=acceleration.z;
         */
        float lastx = acceleration.x;
        float lasty = acceleration.y;
        float lastz = acceleration.z;
        
        
        double tmp = lastx;
        
        switch ([[UIApplication sharedApplication] statusBarOrientation])
        {
            case UIInterfaceOrientationLandscapeRight:
                lastx = -lasty;
                lasty = tmp;
                
                break;
                
            case UIInterfaceOrientationLandscapeLeft:
                lastx = lasty;
                lasty = -tmp;
                break;
                
            case UIInterfaceOrientationPortraitUpsideDown:
                lastx = -lastx;
                lasty = -lasty;
                break;
            case UIInterfaceOrientationPortrait:
                break;
        }
        float  moveX = lastx;
        float  moveY = lasty;
        float  moveZ = lastz;
        
        if (!starttime) {
            starttime=acceleration.timestamp;
        }
        
        if (acceleration.timestamp>starttime+1&&(fabs(moveX)>=.3||fabs(moveY)>=.3||fabs(moveZ)>=.3)) {
            
            if (moveX>=.3&&moveY>-.3) {//右上
                //[djSocket test:9];
                [djSocket test:0x14];
            }
            
            else if (moveX<=-.3&&moveY>-.3) {//左上
                //[djSocket test:9];
                [djSocket test:0x12];
            }
            else if (moveX>=.3&&moveY<=-.3) {//右下
                //[djSocket test:9];
                [djSocket test:0x17];
            }
            
            else if (moveX<=-.3&&moveY<=-.3) {//左下
                //[djSocket test:9];
                [djSocket test:0x15];
            }
            
            else if (fabs(moveX)<.3&&moveY>=.3) {//往前
                [djSocket test:0x13];
                
            }
            
            else if (fabs(moveX)<.3&&moveY<=-.3) {//往后
                [djSocket test:0x16];
                
            }
            else{//停止
            	[djSocket test:0x09];
            }
        }
    }
	
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex{
	switch (buttonIndex) {
		case 0:
		{
			if (isRecord) {
				//暂停
				isRecord=NO;
				[djSocket pauseRecord];
				[[self.view viewWithTag:31] setHidden:YES];
				[[self.view viewWithTag:32] setHidden:YES];
				[[self.view viewWithTag:33] setHidden:NO];
			}
			
		}
			break;
		case 1:
			
		{
			if (isRecord) {
				//保存
				isRecord=NO;
				[djSocket stopVideoWritting];
				[[self.view viewWithTag:31] setHidden:NO];
				[[self.view viewWithTag:32] setHidden:YES];
				[[self.view viewWithTag:33] setHidden:YES];
				UIAlertView* dialog = [[UIAlertView alloc] init];
				[dialog setDelegate:self];
				[dialog setTitle:@"已保存到相册"];
				[dialog setMessage:@" "];
				[dialog addButtonWithTitle:@"确定"];
				//[dialog addButtonWithTitle:@"视频"];
				
				
				CGAffineTransform moveUp = CGAffineTransformMakeTranslation(0.0, 100.0);
				[dialog setTransform: moveUp];
				[dialog show];
				[dialog release];
				
			}
		}
			
			break;
            
		default:
			break;
	}
	
}

- (void)image:(UIImage *)image didFinishSavingWithError:(NSError *)error
  contextInfo:(void *)contextInfo
{
    // Was there an error?
    if (error != NULL)
    {
		// Show error message...
		
    }
    else  // No errors
    {
		// Show message image successfully saved
    }
}
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return !gravity;
    return YES;
}

- (void)layoutViewsWithTag:(int)tag point:(CGPoint)point{
    UIButton *button = (UIButton *)[self.view viewWithTag:tag];
    CGRect rect = button.frame;
    rect.origin = point;
    button.frame = rect;
}
- (void)willAnimateRotationToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation duration:(NSTimeInterval)duration {
    
    
    //[djSocket.scrollView setFrame:self.view.bounds];
    //[djSocket.scrollView setMaxMinZoomScalesForCurrentBounds];
    
    
    if (interfaceOrientation == UIInterfaceOrientationPortrait
        || interfaceOrientation == UIInterfaceOrientationPortraitUpsideDown)
    {
        djSocket.imgView.frame = CGRectMake(djSocket.imgView.frame.origin.x, djSocket.imgView.frame.origin.y, self.view.bounds.size.width, self.view.bounds.size.height);
        
        [self layoutViewsWithTag:40 point:CGPointMake(10, 5)];
        [self layoutViewsWithTag:41 point:CGPointMake(274, 5)];
        [self layoutViewsWithTag:42 point:CGPointMake(100, 400)];
        [self layoutViewsWithTag:43 point:CGPointMake(100, 400)];
        [self layoutViewsWithTag:34 point:CGPointMake(60, 400)];
        [self layoutViewsWithTag:35 point:CGPointMake(60, 400)];
        
        [self layoutViewsWithTag:31 point:CGPointMake(140, 400)];
        [self layoutViewsWithTag:32 point:CGPointMake(140, 400)];
        [self layoutViewsWithTag:33 point:CGPointMake(140, 400)];
        
        [self layoutViewsWithTag:12 point:CGPointMake(180, 400)];
        [self layoutViewsWithTag:13 point:CGPointMake(180, 400)];
        
        [self layoutViewsWithTag:10 point:CGPointMake(220, 400)];
        [self layoutViewsWithTag:11 point:CGPointMake(220, 400)];
        
        [self layoutViewsWithTag:44 point:CGPointMake(278, 360)];
        
        [self layoutViewsWithTag:45 point:CGPointMake(10, 400)];
        [self layoutViewsWithTag:46 point:CGPointMake(274, 400)];
        [self layoutViewsWithTag:47 point:CGPointMake(10, 360)];
        
        [self layoutViewsWithTag:7 point:CGPointMake(260, 230)];
        [self layoutViewsWithTag:5 point:CGPointMake(260, 290)];
        [self layoutViewsWithTag:8 point:CGPointMake(10, 230)];
        [self layoutViewsWithTag:6 point:CGPointMake(10, 290)];
        
        
        
        
    }
    else
    {
        djSocket.imgView.frame = CGRectMake(djSocket.imgView.frame.origin.x, djSocket.imgView.frame.origin.y, self.view.bounds.size.width, self.view.bounds.size.height);
        
        [self layoutViewsWithTag:40 point:CGPointMake(10, 5)];
        UIButton *button = (UIButton*)[self.view viewWithTag:41];
        CGRect buttonFrame = button.frame;
        [self layoutViewsWithTag:41 point:CGPointMake(self.view.bounds.size.width - buttonFrame.size.width, 5)];
        
        //////////
        button = (UIButton*)[self.view viewWithTag:45];
        buttonFrame = button.frame;
        float offY = self.view.bounds.size.height - buttonFrame.size.height;
        [self layoutViewsWithTag:45 point:CGPointMake(10, offY)];
        buttonFrame.origin = CGPointMake(buttonFrame.origin.x + 40 + 80, offY);
        ///////////////////////////////
        button = (UIButton*)[self.view viewWithTag:34];
        [self layoutViewsWithTag:34 point:buttonFrame.origin];
        button = (UIButton*)[self.view viewWithTag:35];
        [self layoutViewsWithTag:35 point:buttonFrame.origin];
        buttonFrame.origin = CGPointMake(buttonFrame.origin.x + 40 , offY);
        
        ///////////////////////////////
        button = (UIButton*)[self.view viewWithTag:42];
        [self layoutViewsWithTag:42 point:buttonFrame.origin];
        buttonFrame.origin = CGPointMake(buttonFrame.origin.x , offY);
        button = (UIButton*)[self.view viewWithTag:43];
        [self layoutViewsWithTag:43 point:buttonFrame.origin];
        
        buttonFrame.origin = CGPointMake(buttonFrame.origin.x + 40, offY);
        
        button = (UIButton*)[self.view viewWithTag:31];
        [self layoutViewsWithTag:31 point:buttonFrame.origin];
        button = (UIButton*)[self.view viewWithTag:32];
        [self layoutViewsWithTag:32 point:buttonFrame.origin];
        button = (UIButton*)[self.view viewWithTag:33];
        [self layoutViewsWithTag:33 point:buttonFrame.origin];
        buttonFrame.origin = CGPointMake(buttonFrame.origin.x + 40, offY);
        ///////////////////////////////
        button = (UIButton*)[self.view viewWithTag:12];
        [self layoutViewsWithTag:12 point:buttonFrame.origin];
        button = (UIButton*)[self.view viewWithTag:13];
        [self layoutViewsWithTag:13 point:buttonFrame.origin];
        buttonFrame.origin = CGPointMake(buttonFrame.origin.x + 40, offY);
        ///////////////////////////////    ///////////////////////////////
        button = (UIButton*)[self.view viewWithTag:10];
        [self layoutViewsWithTag:10 point:buttonFrame.origin];
        button = (UIButton*)[self.view viewWithTag:11];
        [self layoutViewsWithTag:11 point:buttonFrame.origin];
        buttonFrame.origin = CGPointMake(buttonFrame.origin.x + 40, offY);
        
        /////////////////////////
        //////////////////////////////////////////////////////////////
        button = (UIButton*)[self.view viewWithTag:46];
        [self layoutViewsWithTag:46 point:CGPointMake(self.view.bounds.size.width - button.frame.size.width, offY)];
        
        ///////////////////////////////    ///////////////////////////////
        button = (UIButton*)[self.view viewWithTag:44];
        buttonFrame = button.frame;
        
        [self layoutViewsWithTag:47 point:CGPointMake(10, self.view.bounds.size.height - 60 - 10)];
        
        [self layoutViewsWithTag:44 point:CGPointMake(self.view.bounds.size.width - buttonFrame.size.width, (self.view.bounds.size.height - 60 - 10))];
        
        
        //// 箭头button
        button = (UIButton*)[self.view viewWithTag:7];
        buttonFrame = button.frame;
        buttonFrame.origin = CGPointMake(self.view.bounds.size.width - buttonFrame.size.width, (self.view.bounds.size.height - 2*button.bounds.size.height)/2.0);
        [self layoutViewsWithTag:7 point:buttonFrame.origin];
        
        buttonFrame.origin = CGPointMake(buttonFrame.origin.x, buttonFrame.origin.y + buttonFrame.size.height);
        [self layoutViewsWithTag:5 point:buttonFrame.origin];
        buttonFrame.origin = CGPointMake(self.view.bounds.size.width - buttonFrame.size.width, (self.view.bounds.size.height - 2*button.bounds.size.height)/2.0);
        [self layoutViewsWithTag:8 point:CGPointMake(10, buttonFrame.origin.y)];
        [self layoutViewsWithTag:6 point:CGPointMake(10, buttonFrame.origin.y + buttonFrame.size.height)];
        
        
    }
}
@end