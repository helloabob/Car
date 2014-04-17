//
//  KKScrollView.h
//  MWPhotoBrowser
//
//  Created by Li xuechuan on 12-8-16.
//  Copyright (c) 2012年 SNDA. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "TapDetectingImageView.h"

@interface KKScrollView : UIScrollView <UIScrollViewDelegate,TapDetectingImageViewDelegate>{
        
    TapDetectingImageView *_imageView;
        
    float scale;
    BOOL first;
    BOOL firstLayout;
}
@property (nonatomic)float scale;

- (void)setDisplayImage:(UIImage*)image;
- (BOOL)addScale;
- (BOOL)deleteScale;

- (BOOL)canAddScale;
- (BOOL)canDeleteScale;

@end
