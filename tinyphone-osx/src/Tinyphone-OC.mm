//
//  NSRunAlertPanel.mmm
//  Tinyphone
//
//  Created by Kinshuk  Bairagi on 04/11/20.
//  Copyright © 2020 Kinshuk  Bairagi. All rights reserved.
//


#include <Foundation/Foundation.h>
#import <CoreData/CoreData.h>
#import <AppKit/AppKit.h>
#include "Tinyphone-OC.hpp"
#include "Tinyphone-C-Interface.h"

@implementation TinyphoneOC

- (void)ShowAlert: (NSString *) dataPayload {
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText: @"Error"];
    [alert addButtonWithTitle:@"OK"];
    [alert setInformativeText:dataPayload];
    [alert setAlertStyle:NSAlertStyleCritical];
    [alert runModal];
}

- (const char *) GetAppSupportDirectory {
    NSError *error;
    NSFileManager *manager = [NSFileManager defaultManager];
    NSURL *applicationSupport = [manager URLForDirectory:NSApplicationSupportDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:false error:&error];
    NSString *identifier = [[NSBundle mainBundle] bundleIdentifier];
    NSURL *folder = [applicationSupport URLByAppendingPathComponent:identifier];
    [manager createDirectoryAtURL:folder withIntermediateDirectories:true attributes:nil error:&error];
	NSString *myString = folder.path;
    return myString.UTF8String;
}

@end

void ShowOSXAlert(const char *dataPayload) {
    NSString *convertedString = [[NSString alloc] initWithCString: dataPayload encoding:NSUTF8StringEncoding];
    TinyphoneOC *myInstance = [[TinyphoneOC alloc] init];
    // [myInstance ShowAlert: convertedString];
    [myInstance performSelectorOnMainThread:@selector(ShowAlert:) withObject:convertedString waitUntilDone:NO];
}


const char* GetAppSupportDirectory(){
    TinyphoneOC *myInstance = [[TinyphoneOC alloc] init];
    return [myInstance GetAppSupportDirectory];
}
