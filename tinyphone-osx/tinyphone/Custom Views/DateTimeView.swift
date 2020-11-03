//
//  DateTimeView.swift
//  WorldTime
//
//  Created by Gabriel Theodoropoulos.
//  Copyright © 2020 AppCoda. All rights reserved.
//
import Cocoa

class DateTimeView: NSView, LoadableView {

    // MARK: - IBOutlet Properties
    
    @IBOutlet weak var areaLabel: NSTextField!
    
    @IBOutlet weak var dateLabel: NSTextField!
    
    @IBOutlet weak var timeLabel: NSTextField!
    
    @IBOutlet weak var timezoneLabel: NSTextField!
    
    
    // MARK: - Properties
    
    var timer: Timer?
    
    var preferredTimezoneID: String?
    
    
    // MARK: - Init
    
    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        _ = load(fromNIBNamed: "DateTimeView")
    }
    
    
    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
    }
    
    
    
    // MARK: - Custom Fileprivate Methods
    
    fileprivate func getPreferredTimezoneID() {
        preferredTimezoneID = UserDefaults.standard.string(forKey: "timezoneID")
    }
    
    
    @objc fileprivate func showDateAndTimeInfo() {
        let date = Date()
        let formatter = DateFormatter()
        
        if let id = preferredTimezoneID {
            formatter.timeZone = TimeZone(identifier: id)
        } else {
            formatter.timeZone = TimeZone.current
        }
        areaLabel.stringValue = formatter.timeZone.identifier
        
        formatter.dateFormat = "EEEE, MMMM dd, yyyy"
        dateLabel.stringValue = formatter.string(from: date)
        
        formatter.dateFormat = "ZZZZ"
        timezoneLabel.stringValue = formatter.string(from: date)
        
        formatter.timeStyle = .medium
        timeLabel.stringValue = formatter.string(from: date)
    }
    
    
    // MARK: - Internal Methods
    
    func startTimer() {
        getPreferredTimezoneID()
        
        timer = Timer.scheduledTimer(timeInterval: 1.0, target: self, selector: #selector(showDateAndTimeInfo), userInfo: nil, repeats: true)
        timer?.fire()
        
        RunLoop.current.add(timer!, forMode: .common)
    }
    
    
    func stopTimer() {
        timer?.invalidate()
        timer = nil
    }
}
