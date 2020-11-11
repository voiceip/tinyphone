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
    //@IBOutlet weak var statusMenuItem: NSMenuItem?

    //implement https://www.appcoda.com/macos-status-bar-apps/
    @IBOutlet weak var firstMenuItem: NSMenuItem?

    var accountView: AccountsView?

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
            accountView = AccountsView(frame: NSRect(x: 0.0, y: 0.0, width: 250.0, height: 70.0))
            item.view = accountView
        }

        //let editMenuItem = NSMenuItem()
        //editMenuItem.title = "Edit"
        //menu?.insertItem(editMenuItem, at: 0)
    }
    
    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // Insert code here to initialize your application
        let dispatchQueue = DispatchQueue(label: "HTTPServerQueue", qos: .background)
        dispatchQueue.async{
            Start()
        }
        if #available(OSX 10.14, *) {
            checkPermissions()
        }
    }

    func applicationWillTerminate(_ aNotification: Notification) {
        // Insert code here to tear down your application
        Stop()
    }
}

@available(OSX 10.14, *)
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
        @unknown default:
            return
    }
}

extension AppDelegate: NSMenuDelegate {
    func menuWillOpen(_ menu: NSMenu) {
        accountView?.startTimer()
    }
    
    
    func menuDidClose(_ menu: NSMenu) {
        accountView?.stopTimer()
    }
}


/// Generates array form a tuple. Given tuple's elements must have homogenous type.
///
/// - Parameter tuple: a (homogenous) tuple
/// - Returns: array of tuple elements
func makeArray<Tuple, Value>(from tuple: Tuple) -> [Value] {
    let tupleMirror = Mirror(reflecting: tuple)
    assert(tupleMirror.displayStyle == .tuple, "Given argument is no tuple")
    assert(tupleMirror.superclassMirror == nil, "Given tuple argument must not have a superclass (is: \(tupleMirror.superclassMirror!)")
    assert(!tupleMirror.children.isEmpty, "Given tuple argument has no value elements")
    func convert(child: Mirror.Child) -> Value? {
        let valueMirror = Mirror(reflecting: child.value)
        assert(valueMirror.subjectType == Value.self, "Given tuple argument's child type (\(valueMirror.subjectType)) does not reflect expected return value type (\(Value.self))")
        return child.value as? Value
    }
    return tupleMirror.children.compactMap(convert)
}
