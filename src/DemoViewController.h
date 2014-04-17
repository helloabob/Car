 #import "DjSocket.h"
#import "myAudio.h"
@interface DemoViewController : UIViewController<DjSocketDelegate,UIAccelerometerDelegate,UIAlertViewDelegate>
{  
	UILabel*label;
	DjSocket *djSocket;
	myAudio *mA;
}
@property (nonatomic,retain) DjSocket *djSocket;
@property (nonatomic,retain) myAudio *mA;

-(IBAction) butClick:(id)sender;
@end  