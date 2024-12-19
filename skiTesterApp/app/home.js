import React, { useEffect, useRef, useState } from 'react';
import { ScrollView, Text, StyleSheet, View } from 'react-native';
import { DataTable } from 'react-native-paper';
import { router, useLocalSearchParams} from 'expo-router'

export default function HomeScreen() {
  const local = useLocalSearchParams(); // Retrieve query parameters
  const [pairs, setPairs] = useState(Number(local.pairs) || 5);;
  const [rounds, setRounds] = useState(Number(local.rounds) || 5);;
  const [data, setData] = useState({ pairs: pairs, rounds: rounds, results: [] });
  const [serverState, setServerState] = useState('Loading...');
  const [order, setOrder] = useState([]);
  const indexRef = useRef(0);
  const ws = useRef(new WebSocket('ws://192.168.4.1/ws')).current;

  useEffect(() => {
    ws.onopen = () => setServerState('Connected to the server');
    ws.onclose = () => setServerState('Disconnected. Check internet or server.');
    ws.onerror = (e) => setServerState(`WebSocket error: ${e.message}`);

    return () => ws.close();
  }, []);

  useEffect(() => {
    const wsListener = (e) => {
      if (order.length > 0) {
        handleWebSocketMessage(e.data);
      } else {
        console.warn('Order not populated yet; message ignored.');
      }
    };

    ws.onmessage = wsListener;

    return () => {
      ws.onmessage = null;
    };
  }, [order]);

  useEffect(() => {
    const newOrder = [];
    for (let round = 1; round <= rounds; round++) {
      if (round % 2 !== 0) {
        for (let pair = 1; pair <= pairs; pair++) {
          newOrder.push({ round, pair });
        }
      } else {
        for (let pair = pairs; pair > 0; pair--) {
          newOrder.push({ round, pair });
        }
      }
    }
    setOrder(newOrder);
  }, [pairs, rounds]);

  const handleWebSocketMessage = (jsonData) => {
    try {
      const parsedData = JSON.parse(jsonData);
      setData((prevData) => {
        const updatedResults = [
          ...prevData.results,
          {
            round: order[indexRef.current].round,
            pair: order[indexRef.current].pair,
            ...parsedData,
          },
        ];
        if (indexRef.current < order.length - 1) {
          indexRef.current += 1;
        } else {
          router.push({
            pathname: '/results',
            params: { results: JSON.stringify(updatedResults) },
          });
        }
        return { ...prevData, results: updatedResults };
      });
    } catch (error) {
      console.error('Error parsing WebSocket data:', error);
    }
  };
  

  return (
    <View style={styles.container}>
      <Text style={styles.status}>
        Seuraavaksi pari: {order[indexRef.current]?.pair || 'N/A'}, kierros:{' '}
        {order[indexRef.current]?.round || 'N/A'}
      </Text>
      <DataTable>
        <DataTable.Header>
          <DataTable.Title>Pari</DataTable.Title>
          <DataTable.Title>Kierros</DataTable.Title>
          <DataTable.Title>T1</DataTable.Title>
          <DataTable.Title>T2</DataTable.Title>
          <DataTable.Title>Tot</DataTable.Title>
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
              <DataTable.Cell>
                {Number(result.t1) + Number(result.t2) || 0}
              </DataTable.Cell>
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
    paddingTop: 10,
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
    overflow: 'auto',
  },
});
