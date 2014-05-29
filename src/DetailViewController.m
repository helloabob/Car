//
//  DetailViewController.m
//  Test
//
//  Created by mac0001 on 5/29/14.
//
//

#import "DetailViewController.h"
#import "ActionSheetPicker.h"
#import "MBProgressHUD.h"

@interface DetailViewController (){
    UIView *p_panel;
    UITextField *p_ssid;
    UITextField *p_sec;
    UITextField *p_net;
    UITextField *p_pwd;
    UIButton *p_confirm;
    int i_sec;
    int i_net;
}

@end

@implementation DetailViewController

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
    
    self.view.backgroundColor=[UIColor lightGrayColor];
    
    p_panel=[[UIView alloc]initWithFrame:CGRectMake(0, 10, 300, 220)];
    if (IsIOS7System2) {
        p_panel.frame=CGRectMake(0, 74, 300, 220);
    }
//    p_panel.center=self.view.center;
//    p_panel.backgroundColor=[UIColor lightGrayColor];
    [self.view addSubview:p_panel];
    UILabel *lbl=[[[UILabel alloc]initWithFrame:CGRectMake(0, 0, 80, 30)]autorelease];
    lbl.textColor=[UIColor grayColor];
    lbl.text=@"SSID:";
    UITextField *txt=[[[UITextField alloc]initWithFrame:CGRectMake(24, 20, 252, 30)]autorelease];
    txt.backgroundColor=[UIColor whiteColor];
    txt.leftView=lbl;
    txt.leftViewMode=UITextFieldViewModeAlways;
    txt.text=self.ssid;
    txt.enabled=NO;
    [p_panel addSubview:txt];
    p_ssid=txt;
    lbl=[[[UILabel alloc]initWithFrame:CGRectMake(0, 0, 80, 30)]autorelease];
    lbl.textColor=[UIColor grayColor];
    lbl.text=@"加密方式:";
    txt=[[[UITextField alloc]initWithFrame:CGRectMake(24, 58, 252, 30)]autorelease];
    txt.backgroundColor=[UIColor whiteColor];
    txt.placeholder=@"请点击选择加密方式";
    txt.leftView=lbl;
    txt.leftViewMode=UITextFieldViewModeAlways;
    txt.delegate=self;
    UIButton *btn=[[[UIButton alloc]initWithFrame:CGRectMake(0, 0, 252, 30)]autorelease];
    [btn addTarget:self action:@selector(selectsec) forControlEvents:UIControlEventTouchUpInside];
    [txt addSubview:btn];
    [p_panel addSubview:txt];
    p_sec=txt;
    lbl=[[[UILabel alloc]initWithFrame:CGRectMake(0, 0, 80, 30)]autorelease];
    lbl.textColor=[UIColor grayColor];
    lbl.text=@"身份验证:";
    txt=[[[UITextField alloc]initWithFrame:CGRectMake(24, 96, 252, 30)]autorelease];
    txt.backgroundColor=[UIColor whiteColor];
    txt.placeholder=@"请点击选择身份验证";
    txt.leftView=lbl;
    txt.leftViewMode=UITextFieldViewModeAlways;
    txt.delegate=self;
    btn=[[[UIButton alloc]initWithFrame:CGRectMake(0, 0, 252, 30)]autorelease];
    [btn addTarget:self action:@selector(selectnet) forControlEvents:UIControlEventTouchUpInside];
    [txt addSubview:btn];
    [p_panel addSubview:txt];
    p_net=txt;
    lbl=[[[UILabel alloc]initWithFrame:CGRectMake(0, 0, 80, 30)]autorelease];
    lbl.textColor=[UIColor grayColor];
    lbl.text=@"网络密码:";
    txt=[[[UITextField alloc]initWithFrame:CGRectMake(24, 134, 252, 30)]autorelease];
    txt.backgroundColor=[UIColor whiteColor];
    txt.placeholder=@"请输入网络密码";
    txt.leftView=lbl;
    txt.leftViewMode=UITextFieldViewModeAlways;
    txt.returnKeyType=UIReturnKeyDone;
    txt.delegate=self;
    [p_panel addSubview:txt];
    p_pwd=txt;
    btn = [[[UIButton alloc] initWithFrame:CGRectMake(24, 181, 252, 40)] autorelease];
    [btn setTitle:@"确定" forState:UIControlStateNormal];
    btn.layer.cornerRadius = 10;
    [btn addTarget:self action:@selector(confirm) forControlEvents:UIControlEventTouchUpInside];
    btn.backgroundColor = [UIColor colorWithRed:0 green:0.6 blue:0 alpha:1];
    [p_panel addSubview:btn];
    i_net=0;
    i_sec=0;
    
}

-(void)onresp:(BOOL)flag{
    [MBProgressHUD hideHUDForView:self.view animated:YES];
    NSString *body=flag?@"保存成功":@"保存失败";
    UIAlertView *alert=[[UIAlertView alloc]initWithTitle:@"提示" message:body delegate:nil cancelButtonTitle:@"确定" otherButtonTitles:nil, nil];
    [alert show];
    [alert release];
}

-(void)confirm{
    NSMutableData *mdata=[NSMutableData data];
    unsigned char sec=i_sec;
    unsigned char net=i_net;
    [mdata appendBytes:&sec length:1];
    [mdata appendBytes:&net length:1];
    NSString *str=[NSString stringWithFormat:@"%@ %@",_ssid,p_pwd.text];
    [mdata appendBytes:[str cStringUsingEncoding:NSUTF8StringEncoding] length:str.length];
    MBProgressHUD *hud=[MBProgressHUD showHUDAddedTo:self.view animated:YES];
    hud.labelText=@"保存中...";
    [_parent setWIFI:mdata];
}

-(void)selectsec{
    ActionStringDoneBlock done = ^(ActionSheetStringPicker *picker, NSInteger selectedIndex, id selectedValue) {
        p_sec.text=selectedValue;
        i_sec=selectedIndex;
    };
    ActionStringCancelBlock cancel = ^(ActionSheetStringPicker *picker) {
    };
    NSArray *array=@[@"None",@"TKIP",@"AES"];
    [ActionSheetStringPicker showPickerWithTitle:@"选择" rows:array initialSelection:0 doneBlock:done cancelBlock:cancel origin:self.view];
}

-(void)selectnet{
    ActionStringDoneBlock done = ^(ActionSheetStringPicker *picker, NSInteger selectedIndex, id selectedValue) {
        p_net.text=selectedValue;
        i_net=selectedIndex;
    };
    ActionStringCancelBlock cancel = ^(ActionSheetStringPicker *picker) {
    };
    NSArray *array=@[@"开放式",@"共享式",@"WPA",@"WPA-PSK",@"WPA2",@"WPA2-PSK"];
    [ActionSheetStringPicker showPickerWithTitle:@"选择" rows:array initialSelection:0 doneBlock:done cancelBlock:cancel origin:self.view];
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField{
    [textField resignFirstResponder];
    return YES;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
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
