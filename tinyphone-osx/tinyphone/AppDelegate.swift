//
//  AppDelegate.swift
//  tinyphone
//
//  Created by Kinshuk  Bairagi on 03/11/20.
//  Copyright © 2020 Kinshuk  Bairagi. All rights reserved.
//

import Cocoa
import AVFoundation

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {

    var statusItem: NSStatusItem?
    
    @IBOutlet weak var menu: NSMenu?
    
    //implement https://www.appcoda.com/macos-status-bar-apps/
    @IBOutlet weak var firstMenuItem: NSMenuItem?

    var dateTimeView: DateTimeView?

    
    override func awakeFromNib() {
        
        statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)
        let itemImage = NSImage(named: "MenuIcon")
        itemImage?.isTemplate = true
        statusItem?.button?.image = itemImage
     
        if let menu = menu {
            statusItem?.menu = menu
            menu.delegate = self
        }
        
        if let item = firstMenuItem {
            dateTimeView = DateTimeView(frame: NSRect(x: 0.0, y: 0.0, width: 250.0, height: 150.0))
            item.view = dateTimeView
        }
    }
    
    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // Insert code here to initialize your application
        
        let dispatchQueue = DispatchQueue(label: "HTTPServerQueue", qos: .background)
        dispatchQueue.async{
            Start()
        }
        checkPermissions()
    }

    func applicationWillTerminate(_ aNotification: Notification) {
        // Insert code here to tear down your application
    }


}

func checkPermissions(){
    switch AVCaptureDevice.authorizationStatus(for: .audio) {
        case .authorized:
            // The user has previously granted access to the camera.
            //all good :)
        return
        case .notDetermined: // The user has not yet been asked for camera access.
            AVCaptureDevice.requestAccess(for: .audio) { granted in
                if granted {
                    print("Microphone Permission Granted")
                } else {
                    print("Microphone Permission Denined")
                }
            }
        case .denied: // The user has previously denied access.
            return
        case .restricted: // The user can't grant access due to restrictions.
            return
    }
}

extension AppDelegate: NSMenuDelegate {
    func menuWillOpen(_ menu: NSMenu) {
        dateTimeView?.startTimer()
    }
    
    
    func menuDidClose(_ menu: NSMenu) {
        dateTimeView?.stopTimer()
    }
}
