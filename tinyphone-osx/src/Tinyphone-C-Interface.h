//
//  Tinyphone-C-Interface.h
//  Tinyphone
//
//  Created by Kinshuk  Bairagi on 04/11/20.
//  Copyright © 2020 Kinshuk  Bairagi. All rights reserved.
//

#ifndef Tinyphone_C_Interface_h
#define Tinyphone_C_Interface_h

// This is the C "ShowOSXAlert" function that will be used
// to invoke a specific Objective-C method FROM C++
void ShowOSXAlert (const char *message, bool blocking);

const char* GetAppSupportDirectory();

const char* GetOSXProductVersion();

const char* GetResourceFilePath(const char * name);

const char* GetAppSupportFilePath(const char * name);

const char* GetLogsDirectory();

#endif /* Tinyphone_C_Interface_h */
