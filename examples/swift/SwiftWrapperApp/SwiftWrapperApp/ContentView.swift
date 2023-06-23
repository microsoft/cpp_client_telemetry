//
//  ContentView.swift
//  SwiftWrapperApp
//
//  Created by Abhinav Saroj on 6/22/23.
//

import OneDSSwift
import SwiftUI

struct ContentView: View {
	@State private var enteredText = ""
	
    var body: some View {
		VStack {
			TextField("Enter text", text: $enteredText)
				.textFieldStyle(RoundedBorderTextFieldStyle())
				.padding()

			Button(action: {
				// Perform an action with the captured text
				print("Entered text: \(enteredText)")
				sendTelemetry(enteredText)
				print("Telemetry Sent.")
			}) {
				Text("Send Telemtery")
					.padding()
					.foregroundColor(.white)
					.background(Color.accentColor)
					.cornerRadius(10)
			}
		}
		.padding()
	}
	
	/// Method accessing Swift Wrapper package OneDSSwift telemetry classes
	func sendTelemetry(_ text:String) {
		let event:EventProperties = EventProperties(name: "SetProps_Swift_Wrapper_Example_Proj")
		event.setProperty("enteredtext", withValue: text)
		event.setProperty("from", withValue: "Swift_Wrappers_Package_Example_Project")
		
		let logger: Logger? = LogManager.loggerForSource(source: "exmaple_proj_source")
		if (logger != nil) {
			logger!.logEvent(properties: event)
		}
		
		LogManager.uploadNow()
		_ = LogManager.flushAndTeardown()
	}
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
