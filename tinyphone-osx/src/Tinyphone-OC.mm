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

- (void)ShowAlert: (NSString *) dataPayload  {
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
    // NSString *identifier = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleDisplayName"];
    NSURL *folder = [applicationSupport URLByAppendingPathComponent:identifier];
    [manager createDirectoryAtURL:folder withIntermediateDirectories:true attributes:nil error:&error];
    NSString *myString = folder.path;
    return myString.UTF8String;
}

- (const char *) GetProductVersion {
    NSDictionary* infoDict = [[NSBundle mainBundle] infoDictionary];
    NSString* version = [infoDict objectForKey:@"CFBundleShortVersionString"];
    return version.UTF8String;
}

@end

void ShowOSXAlert(const char *dataPayload, bool blocking) {
    NSString *convertedString = [[NSString alloc] initWithCString: dataPayload encoding:NSUTF8StringEncoding];
    TinyphoneOC *myInstance = [[TinyphoneOC alloc] init];
    // [myInstance ShowAlert: convertedString];
    [myInstance performSelectorOnMainThread:@selector(ShowAlert:) withObject:convertedString waitUntilDone: blocking ? YES : NO];
}


const char* GetAppSupportDirectory(){
    TinyphoneOC *myInstance = [[TinyphoneOC alloc] init];
    return [myInstance GetAppSupportDirectory];
}

const char* GetOSXProductVersion(){
    TinyphoneOC *myInstance = [[TinyphoneOC alloc] init];
    return [myInstance GetProductVersion];
}

const char* GetAppSupportFilePath(const char * name){
    NSString* file = @(name);
    NSString* basePath = @(GetAppSupportDirectory());
    NSString* finalPath = [basePath stringByAppendingPathComponent:file];
    return [finalPath UTF8String];;
}

const char* GetResourceFilePath(const char * name){
    CFURLRef appUrlRef = CFBundleCopyResourceURL(CFBundleGetMainBundle(), CFSTR("config.json"), NULL, NULL);
    if (appUrlRef != nullptr){
        CFStringRef filePathRef = CFURLCopyPath(appUrlRef);
        const char* filePath = CFStringGetCStringPtr(filePathRef, kCFStringEncodingUTF8);
        CFRelease(filePathRef);
        CFRelease(appUrlRef);
        return filePath;
    }
    return NULL;
}

const char* GetLogsDirectory(){
	NSString* bundleName = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleName"];
	NSString* logsPath = [NSString stringWithFormat:@"Library/Logs/%@",bundleName];
	NSString* libraryPath = [NSHomeDirectory() stringByAppendingPathComponent:logsPath];
	NSError * error = nil;
	[[NSFileManager defaultManager] createDirectoryAtPath:libraryPath  withIntermediateDirectories:YES attributes:nil error:&error];
	if (error != nil) {
		NSLog(@"error creating directory: %@", error);
	}
	return [libraryPath UTF8String];
}
