//
//  QRScannerView.swift
//  SkiTesterApp
//
//  Extracted for reuse.
//

import SwiftUI
import AVFoundation

struct QRScannerView: UIViewControllerRepresentable {
    enum ScanError: Error { case notAuthorized, setupFailed, noCamera }
    typealias Completion = (Result<String, Error>) -> Void

    let completion: Completion

    func makeUIViewController(context: Context) -> ScannerViewController {
        let vc = ScannerViewController()
        vc.completion = completion
        return vc
    }

    func updateUIViewController(_ uiViewController: ScannerViewController, context: Context) {}

    final class ScannerViewController: UIViewController, AVCaptureMetadataOutputObjectsDelegate {
        var completion: Completion?
        private let session = AVCaptureSession()
        private var previewLayer: AVCaptureVideoPreviewLayer?

        override func viewDidLoad() {
            super.viewDidLoad()
            view.backgroundColor = .black
            setupCamera()
        }

        override func viewDidLayoutSubviews() {
            super.viewDidLayoutSubviews()
            previewLayer?.frame = view.bounds
        }

        private func setupCamera() {
            switch AVCaptureDevice.authorizationStatus(for: .video) {
            case .authorized:
                configureSession()
            case .notDetermined:
                AVCaptureDevice.requestAccess(for: .video) { granted in
                    DispatchQueue.main.async {
                        if granted { self.configureSession() }
                        else { self.completion?(.failure(ScanError.notAuthorized)) }
                    }
                }
            default:
                completion?(.failure(ScanError.notAuthorized))
            }
        }

        private func configureSession() {
            guard let device = AVCaptureDevice.default(for: .video) else {
                completion?(.failure(ScanError.noCamera)); return
            }
            do {
                let input = try AVCaptureDeviceInput(device: device)
                if session.canAddInput(input) { session.addInput(input) }

                let output = AVCaptureMetadataOutput()
                if session.canAddOutput(output) {
                    session.addOutput(output)
                    output.setMetadataObjectsDelegate(self, queue: DispatchQueue.main)
                    output.metadataObjectTypes = [.qr]
                }

                let preview = AVCaptureVideoPreviewLayer(session: session)
                preview.videoGravity = .resizeAspectFill
                view.layer.addSublayer(preview)
                self.previewLayer = preview

                session.startRunning()
            } catch {
                completion?(.failure(ScanError.setupFailed))
            }
        }

        func metadataOutput(_ output: AVCaptureMetadataOutput, didOutput metadataObjects: [AVMetadataObject], from connection: AVCaptureConnection) {
            guard let obj = metadataObjects.first as? AVMetadataMachineReadableCodeObject,
                  obj.type == .qr,
                  let string = obj.stringValue else { return }
            session.stopRunning()
            completion?(.success(string))
            dismiss(animated: true)
        }

        deinit {
            if session.isRunning { session.stopRunning() }
        }
    }
}
