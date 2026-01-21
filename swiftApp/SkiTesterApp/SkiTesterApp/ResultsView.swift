import SwiftUI
import PDFKit

struct ResultsView: View {
    @EnvironmentObject var store: AppStore
    @State private var expandedNames: Set<String> = []

    private let dateString = DateFormatter.localizedString(
        from: Date(),
        dateStyle: .medium,
        timeStyle: .short
    )

    var body: some View {
        VStack(alignment: .leading) {

            // Header
            VStack(alignment: .leading, spacing: 4) {
                Text(dateString)
                Text("Lämpötila: \(store.state.settings?.temperature ?? 0)°C")
                Text("Lumi: \(store.state.settings?.snowQuality ?? "-")")
                Text("Pohja: \(store.state.settings?.baseHardness ?? "-")")
            }
            .padding()

            // Taulukon otsikko
            HStack {
                Text("Nimi").bold()
                Spacer()
                Text("Aika").bold()
                Spacer()
                Text("Kiihtyvyys").bold()
            }
            .padding(.horizontal)

            ScrollView {
                ForEach(groupedResults.keys.sorted(), id: \.self) { name in
                    let results = groupedResults[name] ?? []
                    let avgTime = results.map(\.time).reduce(0, +) / Double(results.count)
                    let avgMag  = results.map(\.mag_avg).reduce(0, +) / Double(results.count)

                    VStack(spacing: 0) {

                        // Päärivi
                        HStack {
                            Text(name)
                            Spacer()
                            Text(avgTime.formatted(.number.precision(.fractionLength(3))))
                            Spacer()
                            Text(avgMag.formatted(.number.precision(.fractionLength(3))))
                        }
                        .padding()
                        .background(Color.white)
                        .onTapGesture {
                            toggle(name)
                        }

                        // Kierrokset
                        if expandedNames.contains(name) {
                            ForEach(results.indices, id: \.self) { i in
                                let r = results[i]
                                HStack {
                                    Text("Kierros \(r.round)")
                                    Spacer()
                                    Text(r.time.formatted(.number.precision(.fractionLength(3))))
                                    Spacer()
                                    Text(r.mag_avg.formatted(.number.precision(.fractionLength(3))))
                                }
                                .padding(.horizontal)
                                .padding(.vertical, 6)
                                .background(Color.gray.opacity(0.15))
                            }
                        }
                    }
                }

                VStack(spacing: 12) {
                    Button("Jaa PDF") {
                        generatePDF()
                    }
                    Button("Jaa CSV") {
                        generateCSV()
                    }
                }
                .padding()
            }
        }
        .background(Color(.systemGroupedBackground))
    }

    // MARK: - Data

    private var groupedResults: [String: [ResultModel]] {
        Dictionary(grouping: store.state.results, by: \.name)
    }

    // MARK: - Actions

    private func toggle(_ name: String) {
        if expandedNames.contains(name) {
            expandedNames.remove(name)
        } else {
            expandedNames.insert(name)
        }
    }

    private func generatePDF() {
        let html = buildHTML()

        let renderer = UIGraphicsPDFRenderer(
            bounds: CGRect(x: 0, y: 0, width: 595, height: 842)
        )

        let data = renderer.pdfData { ctx in
            ctx.beginPage()
            let dataHTML = html.data(using: .utf8)!
            let attr = try? NSAttributedString(
                data: dataHTML,
                options: [.documentType: NSAttributedString.DocumentType.html],
                documentAttributes: nil
            )
            attr?.draw(in: CGRect(x: 20, y: 20, width: 555, height: 800))
        }

        let url = FileManager.default.temporaryDirectory.appendingPathComponent("results.pdf")
        try? data.write(to: url)

        let vc = UIActivityViewController(activityItems: [url], applicationActivities: nil)
        if let windowScene = UIApplication.shared.connectedScenes.first as? UIWindowScene,
           let window = windowScene.windows.first,
           let root = window.rootViewController {
            root.present(vc, animated: true)
        }
    }

    private func buildHTML() -> String {
        let rows = groupedResults.map { name, results -> String in
            let avgTime = results.map(\.time).reduce(0, +) / Double(results.count)
            let avgMag  = results.map(\.mag_avg).reduce(0, +) / Double(results.count)

            return """
            <tr>
              <td>\(name)</td>
              <td>\(String(format: "%.3f", avgTime))</td>
              <td>\(String(format: "%.3f", avgMag))</td>
            </tr>
            """
        }.joined()

        return """
        <html>
        <body>
        <h1>Testitulokset</h1>
        <p>Päivämäärä: \(dateString)</p>
        <p>Lämpötila: \(store.state.settings?.temperature ?? 0)°C</p>
        <p>Lumi: \(store.state.settings?.snowQuality ?? "-")</p>
        <p>Pohja: \(store.state.settings?.baseHardness ?? "-")</p>
        <table border="1" width="100%">
        <tr><th>Nimi</th><th>Aika</th><th>Kiihtyvyys</th></tr>
        \(rows)
        </table>
        </body>
        </html>
        """
    }

    private func generateCSV() {
        func escapeCSVField(_ field: String) -> String {
            let escaped = field.replacingOccurrences(of: "\"", with: "\"\"")
            return "\"\(escaped)\""
        }

        let temperature = store.state.settings?.temperature ?? 0
        let snowQuality = store.state.settings?.snowQuality ?? "-"
        let baseHardness = store.state.settings?.baseHardness ?? "-"

        var csv = ""
        csv += "\"Nimi\",\"Kierros\",\"Aika\",\"Kiihtyvyys\",\"Päivämäärä\",\"Lämpötila\",\"Lumi\",\"Pohja\"\n"

        for r in store.state.results {
            let name = escapeCSVField(r.name)
            let round = escapeCSVField(String(r.round))
            let time = escapeCSVField(String(format: "%.3f", r.time))
            let mag = escapeCSVField(String(format: "%.3f", r.mag_avg))
            let date = escapeCSVField(dateString)
            let temp = escapeCSVField(String(temperature))
            let snow = escapeCSVField(snowQuality)
            let base = escapeCSVField(baseHardness)

            csv += "\(name),\(round),\(time),\(mag),\(date),\(temp),\(snow),\(base)\n"
        }

        let url = FileManager.default.temporaryDirectory.appendingPathComponent("results.csv")
        do {
            try csv.write(to: url, atomically: true, encoding: .utf8)
        } catch {
            return
        }

        let vc = UIActivityViewController(activityItems: [url], applicationActivities: nil)
        if let windowScene = UIApplication.shared.connectedScenes.first as? UIWindowScene,
           let window = windowScene.windows.first,
           let root = window.rootViewController {
            root.present(vc, animated: true)
        }
    }
}
