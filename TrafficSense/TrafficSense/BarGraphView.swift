//
//  BarGraphView.swift
//  TrafficSense
//
//  Created by Kareem Dasilva on 4/7/23.
//

import Foundation
import SwiftUI
import Charts


struct BarChart: View {
    var body: some View {
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
    }
}
