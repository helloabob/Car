//
//  WIFITableViewController.m
//  Test
//
//  Created by wangbo on 5/28/14.
//
//

#import "WIFITableViewController.h"
#import "MBProgressHUD.h"
#import "DetailViewController.h"

NSString *host=@"192.168.16.12";
//NSString *host=@"192.168.0.103";
unsigned short port=8000;

@interface WIFITableViewController (){
    AsyncUdpSocket *socket;
    DetailViewController *vc;
    BOOL canresp;
    
}

@property(nonatomic,retain)NSMutableArray *ssidArray;

@end

@implementation WIFITableViewController

-(void)dealloc{
    NSLog(@"wifi_dealloc");
    self.ssidArray=nil;
    kRemoveNotif(UIApplicationDidEnterBackgroundNotification);
    kRemoveNotif(UIApplicationDidBecomeActiveNotification);
    socket.delegate=nil;
    [socket close];
    [socket release];
    [super dealloc];
}

- (id)initWithStyle:(UITableViewStyle)style
{
    self = [super initWithStyle:style];
    if (self) {
        // Custom initialization
    }
    return self;
}


-(void)senddata:(NSData *)data withType:(unsigned char)cmd{
    static unsigned int _serial;
    NSMutableData *_mdata = [NSMutableData data];
    unsigned char flag = 0x7E;
    unsigned short len = data.length+6;
    unsigned int serial = _serial++;
    unsigned char cs = 0x00;
    [_mdata appendBytes:&flag length:1];
    [_mdata appendBytes:&len length:2];
    [_mdata appendBytes:&serial length:4];
    [_mdata appendBytes:&cmd length:1];
    [_mdata appendBytes:data.bytes length:data.length];
    [_mdata appendBytes:&cs length:1];
    
    char *d=(char *)_mdata.bytes;
    for(int i=0,f=_mdata.length-1;i<f;i++){
        d[f]+=d[i];
    }
    NSLog(@"send_data:%@",_mdata);
    BOOL fg=[socket sendData:_mdata toHost:host port:port withTimeout:5 tag:2];
    NSLog(@"udp_send:%d",fg);
}


-(void)setWIFI:(NSData *)data{
    [self senddata:data withType:0x0c];
    [socket receiveWithTimeout:5 tag:2];
}

-(void)viewDidAppear:(BOOL)animated{
    [super viewDidAppear:animated];
    [self senddata:nil withType:0x0b];
    [socket receiveWithTimeout:5 tag:1];
    MBProgressHUD *mb=[MBProgressHUD showHUDAddedTo:self.view animated:YES];
    mb.labelText=@"查询SSID列表中...";
    canresp=NO;
}

-(void)didEnterBackground{
//    [socket close];
}
-(void)didBecomeActive{
    socket=[[AsyncUdpSocket alloc]initWithDelegate:self];
    [socket bindToPort:10242 error:nil];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.ssidArray=[NSMutableArray array];
    
    socket=[[AsyncUdpSocket alloc]initWithDelegate:self];
    [socket bindToPort:10242 error:nil];
    
    self.navigationController.navigationBarHidden=NO;
    
    
    kAddObserver(@selector(didEnterBackground), UIApplicationDidEnterBackgroundNotification);
    kAddObserver(@selector(didBecomeActive), UIApplicationDidBecomeActiveNotification);
    
    
    
//    socket sendData:<#(NSData *)#> toHost:<#(NSString *)#> port:<#(UInt16)#> withTimeout:<#(NSTimeInterval)#> tag:<#(long)#>
    
    // Uncomment the following line to preserve selection between presentations.
    // self.clearsSelectionOnViewWillAppear = NO;
    
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
}

- (BOOL)onUdpSocket:(AsyncUdpSocket *)sock didReceiveData:(NSData *)data withTag:(long)tag fromHost:(NSString *)host port:(UInt16)port
{
    unsigned char *dt=(unsigned char *)data.bytes;
    if (dt[0]!=0x7e) {
        return NO;
    }
    if (dt[7]==0x0b) {
        unsigned short len;
        memcpy(&len, &dt[1], 2);
        NSString *ssids=[[NSString alloc]initWithBytes:&dt[8] length:len-6 encoding:NSUTF8StringEncoding];
        NSLog(@"ssids:%@",ssids);
        
//        NSString *body=ssids;
//        UIAlertView *alert=[[UIAlertView alloc]initWithTitle:@"提示" message:body delegate:nil cancelButtonTitle:@"确定" otherButtonTitles:nil, nil];
//        [alert show];
//        [alert release];
        
        self.ssidArray = [NSMutableArray arrayWithArray:[ssids componentsSeparatedByString:@" "]];
        
        [MBProgressHUD hideHUDForView:self.view animated:YES];
        
        [self.tableView reloadData];
        
        
    }else if(dt[7]==0x0c){
        unsigned char result=dt[8];
        BOOL flag=result>0?YES:NO;
        if (canresp&&vc!=nil) {
            [vc onresp:flag];
        }
//        return YES;
    }
    
    return YES;
//    NSString *s = [[[NSString alloc] initWithData: data encoding: NSUTF8StringEncoding] autorelease];
//    NSLog(@"didReceiveData, host = %@, tag = %ld, s = %@", host, tag, s);
    
//    return NO;
}

- (void)onUdpSocket:(AsyncUdpSocket *)sock didNotReceiveDataWithTag:(long)tag dueToError:(NSError *)error{
    NSLog(@"not receive");
    if (tag==1) {
        [MBProgressHUD hideHUDForView:self.view animated:YES];
        [self.navigationController popViewControllerAnimated:YES];
    }else if(tag==2){
        if (canresp&&vc!=nil) {
            [vc onresp:NO];
        }
    }
}

- (void)onUdpSocketDidClose:(AsyncUdpSocket *)sock{
    NSLog(@"close");
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
//#warning Potentially incomplete method implementation.
    // Return the number of sections.
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
//#warning Incomplete method implementation.
    // Return the number of rows in the section.
    return _ssidArray.count;
}


- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *identifier=@"aaaaa";
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:identifier];
    
    if (!cell) {
        cell=[[[UITableViewCell alloc]initWithStyle:UITableViewCellStyleDefault reuseIdentifier:identifier]autorelease];
    }
    
//    static int aa=0;
    
    cell.textLabel.text=[_ssidArray objectAtIndex:indexPath.row];
    // Configure the cell...
    
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath{
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
    
    vc=[[[DetailViewController alloc]init]autorelease];
    vc.parent=self;
    vc.ssid=[tableView cellForRowAtIndexPath:indexPath].textLabel.text;
    [self.navigationController pushViewController:vc animated:YES];
    canresp=YES;
    
}


/*
// Override to support conditional editing of the table view.
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Return NO if you do not want the specified item to be editable.
    return YES;
}
*/

/*
// Override to support editing the table view.
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source
        [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationFade];
    } else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view
    }   
}
*/

/*
// Override to support rearranging the table view.
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath
{
}
*/

/*
// Override to support conditional rearranging of the table view.
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Return NO if you do not want the item to be re-orderable.
    return YES;
}
*/

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
