import React, { useState, useEffect } from 'react';
import { View, Button, Text, StyleSheet } from 'react-native';
import { DataTable } from 'react-native-paper';

export default function HomeScreen() {
  const [pairs, setPairs] = useState(null);
  const [rounds, setRounds] = useState(null);
  const [data, setData] = useState({});
  const [loading, setLoading] = useState(false);

  const fetchData = async () => {
    setLoading(true);
    try {
      const response = await fetch('http://192.168.4.1/ski');
      if (!response.ok) {
        throw new Error(`HTTP error! Status: ${response.status}`);
      }
      const jsonData = await response.json();
      setData(jsonData);
    } catch (error) {
      console.error('Error fetching data:', error);
      setPairs("Error");
      setRounds("Error");
      setData([]);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchData();
  }, []);

  return (
    <View>
      {loading ? (
        <Text>Loading...</Text>
      ) : (
        <>
          <DataTable>
            <DataTable.Header>
              <DataTable.Title>Pari</DataTable.Title>
              <DataTable.Title>Kierros</DataTable.Title>
              <DataTable.Title>T1</DataTable.Title>
              <DataTable.Title>T2</DataTable.Title>
              <DataTable.Title>T Tot</DataTable.Title>
            </DataTable.Header>


            <DataTable.Row>
              <DataTable.Cell>{1}</DataTable.Cell>
              <DataTable.Cell>{1}</DataTable.Cell>
              <DataTable.Cell>{data.t1}</DataTable.Cell>
              <DataTable.Cell>{data.t2}</DataTable.Cell>
              <DataTable.Cell>{data.t1 + data.t2}</DataTable.Cell>
            </DataTable.Row>
          </DataTable>
        </>
      )}
      <Button title="Reload" onPress={fetchData} />
    </View>
  );
}