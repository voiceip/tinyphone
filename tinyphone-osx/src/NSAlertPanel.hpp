//
//  NSRunAlertPanel.hpp
//  Tinyphone
//
//  Created by Kinshuk  Bairagi on 04/11/20.
//  Copyright © 2020 Kinshuk  Bairagi. All rights reserved.
//
#import "Tinyphone-C-Interface.h"
#include <string>
 
//// An Objective-C class that needs to be accessed from C++
@interface TPAlert : NSObject
{
   
}

// The Objective-C member function you want to call from C++
- (void) ShowAlert:(NSString *) message;

- (const char *) GetAppSupportDirectory;

@end

 
