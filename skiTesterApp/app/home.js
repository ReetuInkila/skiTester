import React, { useEffect, useRef, useState, useCallback } from 'react';
import { ScrollView, Text, StyleSheet, View } from 'react-native';
import { DataTable } from 'react-native-paper';
import { useLocalSearchParams, router } from 'expo-router';
import { saveDataToStorage, loadDataFromStorage } from './storage';

export default function HomeScreen() {
  const params = useLocalSearchParams(); // Fetch query params
  const { temperature, snowQuality, baseHardness, pairs, rounds, names} = params; // Destructure specific params

  const [data, setData] = useState({
    order:[],
    names: JSON.parse(names|| '[]'),
    temperature: temperature || 0, 
    snowQuality: snowQuality || 'unknown', 
    baseHardness: baseHardness || 'unknown', 
    pairs: pairs || 5, 
    rounds: rounds || 5, 
    results: [] 
  });

  const [serverState, setServerState] = useState('Yhdistää...');

  const indexRef = useRef(0);
  const wsRef = useRef(null);

  // WebSocket-yhteyden muodostaminen
  const connectWebSocket = useCallback(() => {
    if (wsRef.current) return; // Estetään uuden WebSocket-olion luonti, jos yhteys on jo olemassa

    const ws = new WebSocket('ws://192.168.4.1/ws');
    wsRef.current = ws; // Tallennetaan viite WebSocket-olioon

    ws.onopen = () => setServerState('Yhdistetty.');
    ws.onclose = () => {
      setServerState('Ei yhteyttä.');
      wsRef.current = null; // Nollataan viite, jos yhteys katkeaa
    };
    ws.onerror = (e) => setServerState(`Virhe yhteydessä: ${e.message}`);
    ws.onmessage = (e) => {
      const parsedData = JSON.parse(e.data);
      if (data.order.length > 0) {
        handleWebSocketMessage(parsedData);
        ws.send(JSON.stringify({ id: parsedData.id }));
      } else {
        console.warn('Order not populated yet; message ignored.');
      }
    };
  }, [data.order]);

  const handleWebSocketMessage = (parsedData) => {
    try {
      if (parsedData.error) {
        alert(`Virheilmoitus laitteelta: ${parsedData.error}`);
        return;
      }

      setData((prevData) => {
        const updatedResults = [
          ...prevData.results,
          {
            round: prevData.order[indexRef.current].round,
            name: prevData.order[indexRef.current].name,
            ...parsedData,
          },
        ];
        indexRef.current += 1;
  
        // Tallennetaan data jokaisen päivityksen jälkeen
        const updatedData = { ...prevData, results: updatedResults };
        saveDataToStorage(updatedData);
  
        return updatedData;
      });
    } catch (error) {
      console.error('Error parsing WebSocket data:', error);
    }
  };

  useEffect(() => {
    if (indexRef.current === data.order?.length && data.order.length>0) {
      router.push({
        pathname: '/results',
        params: {
          results: JSON.stringify(data.results),
          temperature: data.temperature,
          baseHardness: data.baseHardness,
          snowQuality: data.snowQuality,
        },
      });
    }
  }, [data.results]);
  
  // Ladataan vanhat tulokset vain, jos parametri `loadOldResults` on true
  useEffect(() => {
    const loadData = async () => {
      try {
        const oldData = await loadDataFromStorage();
        if (oldData) {
          console.log('Loaded data:', oldData);
          setData(oldData);
          indexRef.current = oldData.results?.length || 0; // Safeguard for `results`
        }
      } catch (error) {
        console.error('Error loading data:', error);
      }
    };

    if (params?.loadOldResults) { // Safeguard for `params`
      loadData();
    }
  }, [params?.loadOldResults]);

  // WebSocket-yhteyden hallinta
  useEffect(() => {
    const reconnectInterval = setInterval(() => {
      if (!wsRef.current || wsRef.current.readyState === WebSocket.CLOSED) {
        setServerState('Yritetään yhdistää uudelleen...');
        connectWebSocket();
      }
    }, 5000);

    return () => clearInterval(reconnectInterval);
  }, [connectWebSocket]);

  // Järjestyksen laskeminen
  useEffect(() => {
    setData((prevData) => {
      const newOrder = [];
      for (let round = 1; round <= prevData.rounds; round++) {
        if (round % 2 !== 0) {
          for (let i = 0; i < prevData.pairs; i++) {
            newOrder.push({ round, name: prevData.names[i] || `Pari ${i + 1}` });
          }
        } else {
          for (let i = prevData.pairs - 1; i >= 0; i--) {
            newOrder.push({ round, name: prevData.names[i] || `Pari ${i + 1}` });
          }
        }
      }
      // Avoid re-setting the same value to prevent unnecessary renders
      if (JSON.stringify(newOrder) !== JSON.stringify(prevData.order)) {
        return { ...prevData, order: newOrder };
      }
      return prevData; // Return the same object if order hasn't changed
    });
  }, [data.pairs, data.rounds, data.names]);

  return (
    <View style={styles.container}>
      <Text style={styles.status}>
        Seuraavaksi: {data.order[indexRef.current]?.name || 'N/A'} kierros: {data.order[indexRef.current]?.round || 'N/A'}
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
    backgroundColor: 'white',
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
