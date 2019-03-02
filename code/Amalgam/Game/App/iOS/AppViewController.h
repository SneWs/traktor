#ifndef traktor_amalgam_AppViewController_H
#define traktor_amalgam_AppViewController_H

#import <UIKit/UIKit.h>

@interface AppViewController : UIViewController

- (BOOL) createApplication;

- (void) startAnimation;

- (void) stopAnimation;

- (void) suspend;

- (void) resume;

@end

#endif	// traktor_amalgam_AppViewController_H

