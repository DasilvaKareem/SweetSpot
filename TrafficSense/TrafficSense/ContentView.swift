//
//  ContentView.swift
//  TrafficSense
//
//  Created by Kareem Dasilva on 4/7/23.
//

import SwiftUI
import Charts

struct ContentView: View {
    var body: some View {
        VStack{
            HStack{
                Button(action: {}) {
                    Image("menu").renderingMode(.template)
                }
            }
            Chart {
                BarMark(
                    x: .value("Shape Type", "Triangle"),
                     y: .value("Total Count", 5)
                 )
                 BarMark(
                      x: .value("Shape Type", "Circle"),
                      y: .value("Total Count", 3)
                 )
                 BarMark(
                      x: .value("Shape Type", "Square"),
                      y: .value("Total Count", 8)
                 )
            }
        }.background(Color(.blue).ignoresSafeArea(.all, edges: .all))
 
    }

}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
