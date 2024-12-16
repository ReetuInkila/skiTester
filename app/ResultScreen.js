import React, { useState } from 'react';
import { View, Text, StyleSheet, ScrollView, TouchableOpacity } from 'react-native';
import { DataTable } from 'react-native-paper';
import Ionicons from 'react-native-vector-icons/Ionicons';

export default function ResultScreen({ results }) {
  // Group the results by ski (pair) to calculate the averages and nested rounds
  const calculateSkiData = () => {
    const skis = results.reduce((acc, result) => {
      if (!acc[result.pair]) {
        acc[result.pair] = [];
      }
      acc[result.pair].push(result);
      return acc;
    }, {});

    const skiData = Object.keys(skis).map((pair) => {
      const skiResults = skis[pair];

      const averageT1 = skiResults.reduce((sum, result) => sum + result.t1, 0) / skiResults.length;
      const averageT2 = skiResults.reduce((sum, result) => sum + result.t2, 0) / skiResults.length;
      const averageTotal = skiResults.reduce((sum, result) => sum + (result.t1 + result.t2), 0) / skiResults.length;

      return { pair, skiResults, averageT1, averageT2, averageTotal };
    });

    return skiData;
  };

  const [skiData, setSkiData] = useState(calculateSkiData());
  const [expandedRows, setExpandedRows] = useState({}); // Track expanded rows

  const toggleRow = (pair) => {
    setExpandedRows((prev) => ({
      ...prev,
      [pair]: !prev[pair], // Toggle the expanded state of the selected pair
    }));
  };

  const sortData = (key) => {
    const sortedData = [...skiData].sort((a, b) => {
      if (a[key] < b[key]) {
        return -1;
      }
      if (a[key] > b[key]) {
        return 1;
      }
      return 0;
    });
    setSkiData(sortedData);
  };

  return (
    <View style={styles.container}>
      <Text style={styles.title}>Tulokset</Text>
      <ScrollView style={styles.scrollView}>
        <DataTable style={styles.dataTable}>
          <DataTable.Header>
            <DataTable.Title onPress={() => sortData('pair')}>Pari</DataTable.Title>
            <DataTable.Title onPress={() => sortData('averageT1')}>Keskiarvo T1</DataTable.Title>
            <DataTable.Title onPress={() => sortData('averageT2')}>Keskiarvo T2</DataTable.Title>
            <DataTable.Title onPress={() => sortData('averageTotal')}>Keskiarvo Yhteensä</DataTable.Title>
          </DataTable.Header>

          {skiData.map((ski, index) => (
            <React.Fragment key={index}>
              <TouchableOpacity onPress={() => toggleRow(ski.pair)}>
                <DataTable.Row>
                  <DataTable.Cell>{ski.pair}</DataTable.Cell>
                  <DataTable.Cell>{ski.averageT1.toFixed(2)}</DataTable.Cell>
                  <DataTable.Cell>{ski.averageT2.toFixed(2)}</DataTable.Cell>
                  <DataTable.Cell>{ski.averageTotal.toFixed(2)}</DataTable.Cell>
                </DataTable.Row>
              </TouchableOpacity>

              {expandedRows[ski.pair] &&
                ski.skiResults.map((result, index) => (
                  <DataTable.Row key={`nested-${index}`} style={styles.nestedRow}>
                    <DataTable.Cell>{`Kierros ${result.round}`}</DataTable.Cell>
                    <DataTable.Cell>{result.t1.toFixed(2)}</DataTable.Cell>
                    <DataTable.Cell>{result.t2.toFixed(2)}</DataTable.Cell>
                    <DataTable.Cell>{(result.t1 + result.t2).toFixed(2)}</DataTable.Cell>
                  </DataTable.Row>
                ))}
            </React.Fragment>
          ))}
        </DataTable>
      </ScrollView>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    backgroundColor: '#f9f9f9',
    padding: 10,
  },
  title: {
    fontSize: 24,
    fontWeight: 'bold',
    color: '#333',
    marginBottom: 20,
  },
  scrollView: {
    width: '100%',
  },
  dataTable: {
    width: '100%',
  },
  nestedRow: {
    backgroundColor: '#f1f1f1',
  },
});
