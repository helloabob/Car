//
//  KKScrollView.m
//  MWPhotoBrowser
//
//  Created by Li xuechuan on 12-8-16.
//  Copyright (c) 2012年 SNDA. All rights reserved.
//

#import "KKScrollView.h"
#import "TapDetectingImageView.h"

const float maxScale = 6.0f;
const float minScale = 1.0f;
const float scaleValue = 0.4f;
const float defaultScale = 1.0f; // 默认原始图大小

@implementation KKScrollView{
    BOOL done;
}
@synthesize scale;

- (id)initWithFrame:(CGRect)frame {
	if ((self = [super initWithFrame:frame])) {
        
        _imageView = [[TapDetectingImageView alloc] initWithFrame:CGRectZero];
		_imageView.delegate = self;
		_imageView.contentMode = UIViewContentModeCenter;
		_imageView.backgroundColor = [UIColor blackColor];
		[self addSubview:_imageView];
         
        ////////////////////////////
        self.backgroundColor = [UIColor blackColor];
		self.delegate = self;
		self.showsHorizontalScrollIndicator = NO;
		self.showsVerticalScrollIndicator = NO;
		self.decelerationRate = UIScrollViewDecelerationRateFast;
		self.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        
        //self.contentSize = CGSizeMake(self.bounds.size.width, self.bounds.size.height);
        //self.contentOffset = CGPointMake(0, 0);
        
    }
    return self;
}
- (void)dealloc{
    [_imageView release];
    
    [super dealloc];
}
- (void)setupCenter{
    // Center the image as it becomes smaller than the size of the screen
    CGSize boundsSize = self.bounds.size;
    CGRect frameToCenter = _imageView.frame;
    
    // Horizontally
    if (frameToCenter.size.width < boundsSize.width) {
        frameToCenter.origin.x = floorf((boundsSize.width - frameToCenter.size.width) / 2.0);
	} else {
        frameToCenter.origin.x = 0;
	}
    
    // Vertically
    if (frameToCenter.size.height < boundsSize.height) {
        frameToCenter.origin.y = floorf((boundsSize.height - frameToCenter.size.height) / 2.0);
	} else {
        frameToCenter.origin.y = 0;
	}
    
	// Center
	if (!CGRectEqualToRect(_imageView.frame, frameToCenter))
		_imageView.frame = frameToCenter;
}
- (void)layoutSubviews{
    [super layoutSubviews];
    
 
    [self setupCenter];
    if(scale <= minScale){
        CGRect imgRect = _imageView.frame;
        imgRect.origin = CGPointMake(imgRect.origin.x, 0);
        
        _imageView.frame = imgRect;
    }
}
- (void)setMaxMinZoomScalesForCurrentBounds {
	

	self.maximumZoomScale = maxScale;
	self.minimumZoomScale = minScale;
	self.zoomScale = scaleValue;
	
	if (_imageView.image == nil) return;

    CGSize boundsSize = self.bounds.size;
    CGSize imageSize = _imageView.frame.size;

    CGFloat xScale = boundsSize.width / imageSize.width;    // the scale needed to perfectly fit the image width-wise
    CGFloat yScale = boundsSize.height / imageSize.height;  // the scale needed to perfectly fit the image height-wise
    CGFloat minScale = MIN(xScale, yScale);                 // use minimum of these to allow the image to become fully visible
	
	// If image is smaller than the screen then ensure we show it at
	// min scale of 1
	if (xScale > 1 && yScale > 1) {
		minScale = 1.0;
	}
	// Calculate Max
	CGFloat maxScale = 2.0; // Allow double scale
    // on high resolution screens we have double the pixel density, so we will be seeing every pixel if we limit the
    // maximum zoom scale to 0.5.
	if ([UIScreen instancesRespondToSelector:@selector(scale)]) {
		maxScale = maxScale / [[UIScreen mainScreen] scale];
	}
	
	// Set
	self.maximumZoomScale = maxScale;
	self.minimumZoomScale = minScale;
	self.zoomScale = scaleValue;
	
	// Reset position
	_imageView.frame = CGRectMake(0, 0, _imageView.frame.size.width, _imageView.frame.size.height);
	[self setNeedsLayout];
    
}
- (BOOL)canAddScale{
    if(self.zoomScale + scaleValue > self.maximumZoomScale)return FALSE;
    return TRUE;
    
}
- (BOOL)addScale{
    return NO;
    if (done) {
        return NO;
    }
    BOOL flag = TRUE;
    if([self canAddScale])flag = FALSE;
    [self setScale:scaleValue];
    done=YES;
    return flag;

}
- (BOOL)canDeleteScale{
    if(self.zoomScale -scaleValue < minScale)return FALSE;
    return TRUE;
    
}
- (BOOL)deleteScale{
    BOOL flag = TRUE;
    if([self canDeleteScale])flag = FALSE;
    
    [self setScale:-scaleValue];
    return flag;
}
- (void)setScale:(float)scales{
    
    self.zoomScale += scales;
    
    printf("%lf %lf %lf\n", self.minimumZoomScale, self.maximumZoomScale, self.zoomScale);
    
    if(self.zoomScale >= self.maximumZoomScale || self.zoomScale <= self.minimumZoomScale){
//        self.zoomScale = self.minimumZoomScale;
//        
//        [self setZoomScale:self.minimumZoomScale animated:YES];
    }
    else{
        //[self zoomToRect:CGRectMake(touchPoint.x, touchPoint.y, 1, 1) animated:YES];
		
        [self setZoomScale:self.zoomScale animated:YES];
    }
    
    
}
- (void)setDisplayImage:(UIImage*)image{
    if(!image )return;
   
    _imageView.image = image;
    _imageView.hidden = NO;
  
    if(first){
        return; 
    }
    first = YES;
    
    self.maximumZoomScale = maxScale;
    self.minimumZoomScale = minScale;
    //self.zoomScale = 0.4;
    [self setZoomScale:defaultScale animated:YES];
    
    self.contentSize = CGSizeMake(0, 0);
    
    
    CGRect photoImageViewFrame;
    photoImageViewFrame.origin = CGPointZero;
    photoImageViewFrame.size = image.size;
    _imageView.frame = photoImageViewFrame;
    self.contentSize = photoImageViewFrame.size;
    
    // Set zoom to minimum zoom
    //[self setMaxMinZoomScalesForCurrentBounds];
    
   // [self setNeedsLayout];
}
- (void)handleDoubleTap:(CGPoint)touchPoint {
	
    
    if(self.zoomScale >= self.maximumZoomScale){
        self.zoomScale = self.minimumZoomScale;
        
        [self setZoomScale:self.minimumZoomScale animated:YES];
    }else{
        [self setZoomScale:self.zoomScale animated:YES];
    }
    return;
    ////////////////
	// Zoom
	if (self.zoomScale == self.maximumZoomScale) {
		
		// Zoom out
		[self setZoomScale:self.minimumZoomScale animated:YES];
		
	} else {
		
		// Zoom in
		[self zoomToRect:CGRectMake(touchPoint.x, touchPoint.y, 1, 1) animated:YES];
		
	}
}
#pragma mark -
#pragma mark UIScrollViewDelegate

- (UIView *)viewForZoomingInScrollView:(UIScrollView *)scrollView {
	return _imageView;
}
- (void)scrollViewWillBeginDragging:(UIScrollView *)scrollView {
	
}

- (void)scrollViewWillBeginZooming:(UIScrollView *)scrollView withView:(UIView *)view {
}
- (void)scrollViewDidEndZooming:(UIScrollView *)scrollView withView:(UIView *)view atScale:(float)scale{
    if(scale <= minScale){
      [self setupCenter];
        CGRect imgRect = _imageView.frame;
        imgRect.origin = CGPointMake(imgRect.origin.x, 0);
    
        _imageView.frame = imgRect;
    }
}
- (void)scrollViewDidEndDragging:(UIScrollView *)scrollView willDecelerate:(BOOL)decelerate {
	//[photoBrowser hideControlsAfterDelay];
}
#pragma mark tapDelegate
- (void)tapDetectingImageView:(TapDetectingImageView *)view gotSingleTapAtPoint:(CGPoint)tapPoint{
   
    
}
- (void)tapDetectingImageView:(TapDetectingImageView *)view gotDoubleTapAtPoint:(CGPoint)tapPoint{
    // [self handleDoubleTap:tapPoint];
}
- (void)tapDetectingImageView:(TapDetectingImageView *)view gotTwoFingerTapAtPoint:(CGPoint)tapPoint{
    
}
@end
