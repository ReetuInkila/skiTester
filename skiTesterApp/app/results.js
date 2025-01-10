import React, { useState, useMemo } from 'react';
import { View, Text, StyleSheet, ScrollView, TouchableOpacity, Button, Platform } from 'react-native';
import { DataTable } from 'react-native-paper';
import { useLocalSearchParams } from 'expo-router';
import { useSelector } from 'react-redux';
import * as Print from 'expo-print';
import * as Sharing from 'expo-sharing';

export default function Results() {
  const { data } = useSelector((state) => state.app);

  const calculateSkiData = () => {
    const skis = data.results.reduce((acc, result) => {
      if (!acc[result.name]) {
        acc[result.name] = [];
      }
      acc[result.name].push(result);
      return acc;
    }, {});

    const skiData = Object.keys(skis).map((name) => {
      const skiResults = skis[name];

      const averageT1 = skiResults.reduce((sum, result) => sum + result.t1, 0) / skiResults.length;
      const averageT2 = skiResults.reduce((sum, result) => sum + result.t2, 0) / skiResults.length;
      const averageTotal = skiResults.reduce((sum, result) => sum + (result.t1 + result.t2), 0) / skiResults.length;

      return { name, skiResults, averageT1, averageT2, averageTotal };
    });

    return skiData;
  };

  const [skiData, setSkiData] = useState(calculateSkiData());
  const [expandedRows, setExpandedRows] = useState({});
  const [isDescending, setIsDescending] = useState(false);
  const date = new Date().toLocaleString();

  const toggleRow = (name) => {
    setExpandedRows((prev) => ({
      ...prev,
      [name]: !prev[name],
    }));
  };

  const sortData = (key) => {
    const sortedData = [...skiData].sort((a, b) => {
      if (a[key] < b[key]) return isDescending ? 1 : -1;
      if (a[key] > b[key]) return isDescending ? -1 : 1;
      return 0;
    });
    setIsDescending(!isDescending);
    setSkiData(sortedData);
  };

  const generatePDF = async () => {
    const htmlContent = `
      <html>
        <body>
          <h1>Testitulokset</h1>
          <p>Päivämäärä: ${date}</p>
          <p>Lämpötila: ${data.temperature}°C</p>
          <p>Lumi: ${data.snowQuality}</p>
          <p>Pohja: ${data.baseHardness}</p>
          <table border="1" style="width: 100%; border-collapse: collapse;">
            <thead>
              <tr>
                <th>Nimi</th>
                <th>T1</th>
                <th>T2</th>
                <th>Yhteensä</th>
              </tr>
            </thead>
            <tbody>
              ${skiData
                .map(
                  (ski) => `
                    <tr>
                      <td>${ski.name}</td>
                      <td>${ski.averageT1.toFixed(2)}</td>
                      <td>${ski.averageT2.toFixed(2)}</td>
                      <td>${ski.averageTotal.toFixed(2)}</td>
                    </tr>
                  `
                )
                .join('')}
            </tbody>
          </table>
        </body>
      </html>
    `;

    try {
      // Luo PDF-tiedosto `expo-print` avulla
      const { uri } = await Print.printToFileAsync({ html: htmlContent });

      // Jaa PDF-tiedosto
      if (await Sharing.isAvailableAsync()) {
        await Sharing.shareAsync(uri);
      } else {
        alert('Jakaminen ei ole käytettävissä laitteellasi.');
      }
    } catch (error) {
      console.log('Failed to generate pdf', error.message);
    }
  };

  return (
    <View style={styles.container}>
      <View style={styles.header}>
        <Text>{date}</Text>
        <Text>Lämpötila: {data.temperature}°C</Text>
        <Text>Lumi: {data.snowQuality}</Text>
        <Text>Pohja: {data.baseHardness}</Text>
      </View>
      
      <DataTable>
        <DataTable.Header>
          <DataTable.Title onPress={() => sortData('name')}>Nimi</DataTable.Title>
          <DataTable.Title onPress={() => sortData('averageT1')}>T1</DataTable.Title>
          <DataTable.Title onPress={() => sortData('averageT2')}>T2</DataTable.Title>
          <DataTable.Title onPress={() => sortData('averageTotal')}>Yhteensä</DataTable.Title>
        </DataTable.Header>
      </DataTable>
      <ScrollView style={styles.scrollView}>
        <DataTable style={styles.dataTable}>
          {skiData.map((ski, index) => (
            <React.Fragment key={index}>
              <TouchableOpacity onPress={() => toggleRow(ski.name)}>
                <DataTable.Row>
                  <DataTable.Cell>{ski.name}</DataTable.Cell>
                  <DataTable.Cell>{ski.averageT1.toFixed(2)}</DataTable.Cell>
                  <DataTable.Cell>{ski.averageT2.toFixed(2)}</DataTable.Cell>
                  <DataTable.Cell>{ski.averageTotal.toFixed(2)}</DataTable.Cell>
                </DataTable.Row>
              </TouchableOpacity>

              {expandedRows[ski.name] &&
                ski.skiResults.map((result, nestedIndex) => (
                  <DataTable.Row key={`nested-${nestedIndex}`} style={styles.nestedRow}>
                    <DataTable.Cell>{`Kierros ${result.round}`}</DataTable.Cell>
                    <DataTable.Cell>{result.t1.toFixed(2)}</DataTable.Cell>
                    <DataTable.Cell>{result.t2.toFixed(2)}</DataTable.Cell>
                    <DataTable.Cell>{(result.t1 + result.t2).toFixed(2)}</DataTable.Cell>
                  </DataTable.Row>
                ))}
            </React.Fragment>
          ))}
        </DataTable>
        <Button title="Jaa PDF" onPress={generatePDF} />
      </ScrollView>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#f9f9f9',
    paddingTop: 10,
  },
  header: {
    paddingHorizontal: 20,
    marginBottom: 20,
  },
  scrollView: {
    flex: 1,
    marginHorizontal: 10,
    marginBottom: 20,
    overflow: 'auto',
  },
  nestedRow: {
    backgroundColor: '#f1f1f1',
  },
});
