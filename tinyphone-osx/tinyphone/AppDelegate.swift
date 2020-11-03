//
//  AppDelegate.swift
//  tinyphone
//
//  Created by Kinshuk  Bairagi on 03/11/20.
//  Copyright © 2020 Kinshuk  Bairagi. All rights reserved.
//

import Cocoa

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
        
        
    }

    func applicationWillTerminate(_ aNotification: Notification) {
        // Insert code here to tear down your application
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
