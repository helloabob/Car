//
//  IndexViewController.m
//  Test
//
//  Created by wangbo on 5/28/14.
//
//

#import "IndexViewController.h"
#import "MainViewController.h"
#import "WIFITableViewController.h"

@interface IndexViewController ()

@end

@implementation IndexViewController

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
    
    UIButton *btn = [[[UIButton alloc] initWithFrame:CGRectMake(20, 200, 280, 40)] autorelease];
    [btn setTitle:@"查看视频" forState:UIControlStateNormal];
    btn.layer.cornerRadius = 10;
    [btn addTarget:self action:@selector(call) forControlEvents:UIControlEventTouchUpInside];
    btn.backgroundColor = [UIColor colorWithRed:0 green:0.6 blue:0 alpha:1];
    [self.view addSubview:btn];
    
    btn = [[[UIButton alloc] initWithFrame:CGRectMake(20, 280, 280, 40)] autorelease];
    [btn setTitle:@"设备管理" forState:UIControlStateNormal];
    btn.layer.cornerRadius = 10;
    [btn addTarget:self action:@selector(scan) forControlEvents:UIControlEventTouchUpInside];
    btn.backgroundColor = [UIColor colorWithRed:0 green:0.6 blue:0 alpha:1];
    [self.view addSubview:btn];
}

-(void)viewWillAppear:(BOOL)animated{
    [super viewWillAppear:animated];
    self.navigationController.navigationBarHidden=YES;
}

-(void)call{
    MainViewController *vc=[[[MainViewController alloc]init]autorelease];
    [self.navigationController pushViewController:vc animated:YES];
}
-(void)scan{
    WIFITableViewController *vc=[[[WIFITableViewController alloc]init]autorelease];
    [self.navigationController pushViewController:vc animated:YES];
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
