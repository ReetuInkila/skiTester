import React, { useState, useMemo } from 'react';
import { View, Text, StyleSheet, ScrollView, TouchableOpacity } from 'react-native';
import { DataTable } from 'react-native-paper';
import { useLocalSearchParams } from 'expo-router';

export default function Results() {
  // Receiving the object in the target component
  const { results } = useLocalSearchParams();
  // Parse the results string back into an array
  const parsedResults = useMemo(() => JSON.parse(results || '[]'), [results]);
  console.log(parsedResults);

  const calculateSkiData = () => {
    const skis = parsedResults.reduce((acc, result) => {
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
  const [expandedRows, setExpandedRows] = useState({});
  const [isDescending, setIsDescending] = useState(false);

  const toggleRow = (pair) => {
    setExpandedRows((prev) => ({
      ...prev,
      [pair]: !prev[pair],
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

  return (
    <View style={styles.container}>
      <DataTable>
        <DataTable.Header>
          <DataTable.Title onPress={() => sortData('pair')}>Pari</DataTable.Title>
          <DataTable.Title onPress={() => sortData('averageT1')}>T1</DataTable.Title>
          <DataTable.Title onPress={() => sortData('averageT2')}>T2</DataTable.Title>
          <DataTable.Title onPress={() => sortData('averageTotal')}>Yhteensä</DataTable.Title>
        </DataTable.Header>
      </DataTable>
      <ScrollView style={styles.scrollView}>
        <DataTable style={styles.dataTable}>
          {skiData.map((ski, index) => (
            <React.Fragment key={index} >
              <TouchableOpacity onPress={() => toggleRow(ski.pair)}>
                <DataTable.Row>
                  <DataTable.Cell>{ski.pair}</DataTable.Cell>
                  <DataTable.Cell>{ski.averageT1.toFixed(2)}</DataTable.Cell>
                  <DataTable.Cell>{ski.averageT2.toFixed(2)}</DataTable.Cell>
                  <DataTable.Cell>{ski.averageTotal.toFixed(2)}</DataTable.Cell>
                </DataTable.Row>
              </TouchableOpacity>

              {expandedRows[ski.pair] &&
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
