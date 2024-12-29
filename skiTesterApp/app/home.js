import React, { useEffect, useRef, useState } from 'react';
import { ScrollView, Text, StyleSheet, View } from 'react-native';
import { DataTable } from 'react-native-paper';
import { router, useLocalSearchParams } from 'expo-router';

export default function HomeScreen() {
  const local = useLocalSearchParams(); // Retrieve query parameters
  const [pairs, setPairs] = useState(Number(local.pairs) || 5);
  const [rounds, setRounds] = useState(Number(local.rounds) || 5);
  const [names, setNames] = useState(JSON.parse(local.names || '[]'));
  const [temperature, setTemperature] = useState(local.temperature || 'N/A');
  const [snowQuality, setSnowQuality] = useState(local.snowQuality || 'N/A');
  const [baseHardness, setBaseHardness] = useState(local.baseHardness || 'N/A');
  const [data, setData] = useState({ pairs: pairs, rounds: rounds, results: [] });
  const [serverState, setServerState] = useState('Yhdistää...');
  const [order, setOrder] = useState([]);
  const indexRef = useRef(0);
  const ws = useRef(new WebSocket('ws://192.168.4.1/ws')).current;

  useEffect(() => {
    ws.onopen = () => setServerState('Yhdistetty.');
    ws.onclose = () => setServerState('Ei yhteyttä.');
    ws.onerror = (e) => setServerState(`Virhe yhteydessä: ${e.message}`);

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
        for (let i = 0; i < pairs; i++) {
          newOrder.push({ round, name: names[i] || `Pari ${i + 1}` });
        }
      } else {
        for (let i = pairs - 1; i >= 0; i--) {
          newOrder.push({ round, name: names[i] || `Pari ${i + 1}` });
        }
      }
    }
    setOrder(newOrder);
  }, [pairs, rounds, names]);

  const handleWebSocketMessage = (jsonData) => {
    try {
      const parsedData = JSON.parse(jsonData);

      if (parsedData.error) {
        alert(`Virheilmoitus laitteelta: ${parsedData.error}`);
        return;
      }

      setData((prevData) => {
        const updatedResults = [
          ...prevData.results,
          {
            round: order[indexRef.current].round,
            name: order[indexRef.current].name,
            ...parsedData,
          },
        ];
        if (indexRef.current < order.length - 1) {
          indexRef.current += 1;
        } else {
          router.push({
            pathname: '/results',
            params: {
              results: JSON.stringify(updatedResults),
              temperature,
              snowQuality,
              baseHardness,
            },
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
        Seuraavaksi: {order[indexRef.current]?.name || 'N/A'} kierros: {order[indexRef.current]?.round || 'N/A'}
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
          {[...data.results].reverse().map((result, index) => (
            <DataTable.Row key={index}>
              <DataTable.Cell>{result.name}</DataTable.Cell>
              <DataTable.Cell>{result.round}</DataTable.Cell>
              <DataTable.Cell>{result.t1.toFixed(3)}</DataTable.Cell>
              <DataTable.Cell>{result.t2.toFixed(3)}</DataTable.Cell>
              <DataTable.Cell>
                {(Number(result.t1) + Number(result.t2)).toFixed(3) || 0}
              </DataTable.Cell>
            </DataTable.Row>
          ))}
        </DataTable>
      </ScrollView>
      <View style={styles.footer}>
        <Text style={styles.footerText}>Yhteyden tila: {serverState}</Text>
      </View>
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
  status: {
    fontSize: 16,
    fontWeight: 'bold',
    marginVertical: 10,
    color: '#333',
  },
  footer: {
    height: 50,
    backgroundColor: '#eee',
    justifyContent: 'center',
    alignItems: 'center',
    borderTopWidth: 1,
    borderTopColor: '#ccc',
  },
  footerText: {
    fontSize: 14,
    color: '#555',
  },
});
