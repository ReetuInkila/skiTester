import React, { useState, useEffect } from 'react';
import { ScrollView, Text, StyleSheet, View } from 'react-native';
import { DataTable } from 'react-native-paper';
import Ionicons from 'react-native-vector-icons/Ionicons';

export default function ResultScreen({ results }) {
  // Function to calculate average for each ski (pair)
  const calculateSkiAverages = () => {
    // Group results by pair (Ski)
    const skis = results.reduce((acc, result) => {
      if (!acc[result.pair]) {
        acc[result.pair] = [];
      }
      acc[result.pair].push(result);
      return acc;
    }, {});

    // Calculate average for each ski
    const skiAverages = Object.keys(skis).map((pair) => {
      const skiResults = skis[pair];

      // Average for each ski (t1 and t2)
      const averageT1 = skiResults.reduce((sum, result) => sum + result.t1, 0) / skiResults.length;
      const averageT2 = skiResults.reduce((sum, result) => sum + result.t2, 0) / skiResults.length;
      const averageTotal = skiResults.reduce((sum, result) => sum + (result.t1 + result.t2), 0) / skiResults.length;

      return { pair, averageT1, averageT2, averageTotal };
    });

    return skiAverages;
  };

  const [skiAverages, setSkiAverages] = useState(calculateSkiAverages);
  const [sortConfig, setSortConfig] = useState({ key: 'pair', direction: 'asc' });

  // Function to sort data
  const sortData = (key) => {
    const sortedData = [...skiAverages].sort((a, b) => {
      if (a[key] < b[key]) {
        return -1;
      }
      if (a[key] > b[key]) {
        return 1;
      }
      return 0;
    });

    setSkiAverages(sortedData);
    setSortConfig({ key, direction: 'asc' }); // Always set to ascending order
  };

  return (
    <View style={styles.container}>
      <Text style={styles.title}>Tulokset</Text>
      <ScrollView style={styles.scrollView}>
        <DataTable style={styles.dataTable}>
          <DataTable.Header>
            <DataTable.Title
              onPress={() => sortData('pair')}
            >
              Pari
              {sortConfig.key === 'pair' && (
                <Ionicons name='arrow-up' size={16} color="black" />
              )}
            </DataTable.Title>

            <DataTable.Title
              onPress={() => sortData('averageT1')}
            >
              Keskiarvo T1
              {sortConfig.key === 'averageT1' && (
                <Ionicons name='arrow-up' size={16} color="black" />
              )}
            </DataTable.Title>

            <DataTable.Title
              onPress={() => sortData('averageT2')}
            >
              Keskiarvo T2
              {sortConfig.key === 'averageT2' && (
                <Ionicons name='arrow-up' size={16} color="black" />
              )}
            </DataTable.Title>

            <DataTable.Title
              onPress={() => sortData('averageTotal')}
            >
              Keskiarvo Yhteensä
              {sortConfig.key === 'averageTotal' && (
                <Ionicons name='arrow-up' size={16} color="black" />
              )}
            </DataTable.Title>
          </DataTable.Header>

          {skiAverages.map((avg, index) => (
            <DataTable.Row key={index}>
              <DataTable.Cell>{avg.pair}</DataTable.Cell>
              <DataTable.Cell>{avg.averageT1.toFixed(2)}</DataTable.Cell>
              <DataTable.Cell>{avg.averageT2.toFixed(2)}</DataTable.Cell>
              <DataTable.Cell>{avg.averageTotal.toFixed(2)}</DataTable.Cell>
            </DataTable.Row>
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
    width: '100%', // Ensures the ScrollView takes the full width
  },
  dataTable: {
    width: '100%', // Ensures the DataTable takes the full width
  },
});
