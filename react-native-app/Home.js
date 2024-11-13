import React, { useState, useEffect, useRef } from 'react';
import { ScrollView, Text, StyleSheet, View } from 'react-native';
import { DataTable } from 'react-native-paper';

export default function HomeScreen(props) {
  const [serverState, setServerState] = useState('Loading...');
  const [data, setData] = useState({ pairs: props.pairs, rounds: props.rounds, results: [] });
  const [order, setOrder] = useState([]);
  const indexRef = useRef(0);
  const ws = useRef(new WebSocket('ws://192.168.4.1/ws')).current;

  useEffect(() => {
    ws.onopen = () => setServerState('Connected to the server');
    ws.onclose = () => setServerState('Disconnected. Check internet or server.');
    ws.onerror = (e) => setServerState(`WebSocket error: ${e.message}`);
    ws.onmessage = (e) => newResult(e.data);

    return () => ws.close();
  }, []);

  useEffect(() => {
    ws.onmessage = (e) => {
      if (order.length > 0) {
        newResult(e.data);
      } else {
        console.warn('Order not populated yet; message ignored.');
      }
    };
    console.log('WebSocket message handler updated based on new order:', order);
  }, [order]);

  useEffect(() => {
    const newOrder = [];
    for (let round = 1; round <= props.rounds; round++) {
      if (round % 2 !== 0) {
        for (let pair = 1; pair <= props.pairs; pair++) {
          newOrder.push({ round, pair });
        }
      } else {
        for (let pair = props.pairs; pair > 0; pair--) {
          newOrder.push({ round, pair });
        }
      }
    }
    setOrder(newOrder);
  }, [props.pairs, props.rounds]);

  function newResult(jsonData) {
    const currentIndex = indexRef.current;
    console.log(order);
    if (currentIndex < order.length) {
      try {
        const parsedData = JSON.parse(jsonData);
        setData((prevData) => ({
          ...prevData,
          results: [
            ...prevData.results,
            {
              round: order[currentIndex].round,
              pair: order[currentIndex].pair,
              ...parsedData,
            },
          ],
        }));
        indexRef.current += 1;
      } catch (error) {
        console.error('Error parsing WebSocket data:', error);
      }
    }
  }

  useEffect(() => {
    console.log('Data updated:', data);
  }, [data]);

  return (
    <View style={styles.container}>
      <Text style={styles.status}>{serverState}</Text>
      <DataTable>
        <DataTable.Header>
          <DataTable.Title>Pair</DataTable.Title>
          <DataTable.Title>Round</DataTable.Title>
          <DataTable.Title>T1</DataTable.Title>
          <DataTable.Title>T2</DataTable.Title>
          <DataTable.Title>Total</DataTable.Title>
        </DataTable.Header>
      </DataTable>
      <ScrollView style={styles.scrollView}>
        <DataTable>
          {data.results.map((result, index) => (
            <DataTable.Row key={index}>
              <DataTable.Cell>{result.pair}</DataTable.Cell>
              <DataTable.Cell>{result.round}</DataTable.Cell>
              <DataTable.Cell>{result.t1}</DataTable.Cell>
              <DataTable.Cell>{result.t2}</DataTable.Cell>
              <DataTable.Cell>{result.t1 + result.t2}</DataTable.Cell>
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
    backgroundColor: '#f9f9f9',
    paddingTop: 10, // Adding a little padding to avoid issues on Web
  },
  status: {
    fontSize: 16,
    fontWeight: 'bold',
    margin: 10,
    color: '#333',
  },
  scrollView: {
    flex: 1,
    marginHorizontal: 10,
    marginBottom: 20,
    overflow: 'auto', // Ensure scroll is enabled
    maxHeight: '80vh', // Limit height to 80% of the viewport height (useful for web)
  },
});
