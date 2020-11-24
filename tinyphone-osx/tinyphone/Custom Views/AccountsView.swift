//
//  DateTimeView.swift
//  WorldTime
//
//  Created by Gabriel Theodoropoulos.
//  Copyright © 2020 AppCoda. All rights reserved.
//
import Cocoa

class AccountsView: NSView, LoadableView {

    @IBOutlet weak var titleLabel: NSTextField!
    @IBOutlet weak var accountNameLabel: NSTextField!
    @IBOutlet weak var accountStatusLabel: NSTextField!
    
    var timer: Timer?
         
    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        _ = load(fromNIBNamed: "AccountsView")
    }
    
    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
    }
    
    @objc fileprivate func showDateAndTimeInfo() {
        
        let accountsInfo = Accounts();
        //print("Accounts Count ", accountsInfo.count)
        let accounts: [UIAccountInfo] = makeArray(from: accountsInfo.accounts)
        titleLabel.stringValue = "Accounts"
        if (accountsInfo.count > 0 ){
            var primaryAccount: UIAccountInfo?
            for  i in 0 ..< accountsInfo.count{
                let ac = accounts[Int(i)]
                if ac.primary == 1 {
                    primaryAccount = ac
                } else {
                    if ac.name != nil {
                        free(ac.name)
                    }
                    if ac.status != nil {
                        free(ac.status)
                    }
                }
            }
    
            if primaryAccount?.name != nil {
                let name = String(cString: primaryAccount!.name)
                           accountNameLabel.stringValue = name
                free(primaryAccount?.name)
            }
            if primaryAccount?.status != nil {
                let status = String(cString: primaryAccount!.status)
                accountStatusLabel.stringValue = "Status: " + status
                free(primaryAccount?.status)
            }
        } else {
            accountNameLabel.stringValue = "No Accounts Registered"
            accountStatusLabel.stringValue = "----"
        }
    }
    
    // MARK: - Internal Methods
    func startTimer() {
        timer = Timer.scheduledTimer(timeInterval: 3.0, target: self, selector: #selector(showDateAndTimeInfo), userInfo: nil, repeats: true)
        timer?.fire()
        RunLoop.current.add(timer!, forMode: .common)
    }
    
    func stopTimer() {
        timer?.invalidate()
        timer = nil
    }
}
