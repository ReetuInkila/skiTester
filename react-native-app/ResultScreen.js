import React, { useState, useEffect, useRef } from 'react';
import { ScrollView, Text, StyleSheet, View } from 'react-native';
import { DataTable } from 'react-native-paper';

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

  const skiAverages = calculateSkiAverages();

  return (
    <View style={styles.container}>
      <Text style={styles.title}>Tulokset</Text>
      <ScrollView>
        <DataTable>
          <DataTable.Header>
            <DataTable.Title>Pari</DataTable.Title>
            <DataTable.Title>Keskiarvo T1</DataTable.Title>
            <DataTable.Title>Keskiarvo T2</DataTable.Title>
            <DataTable.Title>Keskiarvo Yhteensä</DataTable.Title>
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
});
