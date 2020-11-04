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
#include "NSAlertPanel.hpp"
#include "Tinyphone-C-Interface.h"

static const CFStringEncoding kNarrowStringEncoding = kCFStringEncodingUTF8;

// Given an STL string |in| with an encoding specified by |in_encoding|,
// return it as a CFStringRef.  Returns NULL on failure.
template<typename StringType>
static CFStringRef STLStringToCFStringWithEncodingsT(
    const StringType& in,
    CFStringEncoding in_encoding) {
  typename StringType::size_type in_length = in.length();
  if (in_length == 0)
    return CFSTR("");

  return CFStringCreateWithBytes(kCFAllocatorDefault,
                                 reinterpret_cast<const UInt8*>(in.data()),
                                 in_length *
                                   sizeof(typename StringType::value_type),
                                 in_encoding,
                                 false);
}

CFStringRef SysUTF8ToCFStringRef(const std::string& utf8) {
  return STLStringToCFStringWithEncodingsT(utf8, kNarrowStringEncoding);
}


@implementation TPAlert

- (void)ShowAlert: (NSString *) dataPayload {

  // Your logic here
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText: @"Error"];
    [alert addButtonWithTitle:@"OK"];
    [alert setInformativeText:dataPayload];
//     [alert setAlertStyle:NSAlertStyleWarning];
    [alert setAlertStyle:NSAlertStyleCritical];
    [alert runModal];
}

@end

void ShowOSXAlert(const char *dataPayload) {
    NSString *convertedString = [[NSString alloc] initWithCString: dataPayload encoding:NSUTF8StringEncoding];
    TPAlert *myInstance = [[TPAlert alloc] init];
    // [myInstance ShowAlert: convertedString];
    [myInstance performSelectorOnMainThread:@selector(ShowAlert:) withObject:convertedString waitUntilDone:NO];
}
