//
//  Logging.swift
//  OpenRenderBox

@_transparent
package func _openRenderBoxUnimplementedFailure(_ function: String = #function, file: StaticString = #fileID, line: UInt = #line) -> Never {
    preconditionFailure("Unimplemented yet", file: file, line: line)
}

