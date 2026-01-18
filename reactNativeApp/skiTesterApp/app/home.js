import React, { useEffect, useRef, useCallback } from 'react';
import { ScrollView, Text, StyleSheet, View } from 'react-native';
import { DataTable } from 'react-native-paper';
import { useLocalSearchParams, router } from 'expo-router';
import { useDispatch, useSelector } from 'react-redux';
import { setData, addResult, setServerState} from './store';
import { saveDataToStorage, loadDataFromStorage } from './storage';

export default function HomeScreen() {
  const params = useLocalSearchParams();

  const dispatch = useDispatch();
  const { data, serverState} = useSelector((state) => state.app);

  const wsRef = useRef(null);
  const indexRef = useRef(0);

  const connectWebSocket = useCallback(() => {
    if (wsRef.current) return;

    const ws = new WebSocket('ws://192.168.4.1/ws');
    wsRef.current = ws;

    ws.onopen = () => {
      dispatch(setServerState('Yhdistetty.'));
    };

    ws.onclose = () => {
      dispatch(setServerState('Ei yhteyttä.'));
      wsRef.current = null;
    };

    ws.onerror = (e) => dispatch(setServerState(`Virhe yhteydessä: ${e.message}`));

    ws.onmessage = (e) => {
      const parsedData = JSON.parse(e.data);
      ws.send(JSON.stringify({ id: parsedData.id }));
      handleWebSocketMessage(parsedData);
    };
  }, [dispatch]);

  useEffect(() => {
    const reconnectInterval = setInterval(() => {
      if (!wsRef.current || wsRef.current.readyState === WebSocket.CLOSED) {
        dispatch(setServerState('Yhdistetään...'));
        connectWebSocket();
      }
    }, 200);
    return () => clearInterval(reconnectInterval);
  }, [connectWebSocket, dispatch]);

  const handleWebSocketMessage = (parsedData) => {
    if (data.order.length === 0) {
      console.warn('Order not populated yet; message ignored.');
      return;
    }

    if (indexRef >= data.order.length) {
      console.warn('Index out of bounds; message ignored.');
      return;
    }

    if (parsedData.error) {
      alert(`Virheilmoitus laitteelta: ${parsedData.error}`);
      return;
    }

    const newResult = {
      round: data.order[indexRef.current].round,
      name: data.order[indexRef.current].name,
      ...parsedData,
    };

    dispatch(addResult(newResult));

    indexRef.current += 1;
  };

  useEffect(() => {
    // Tallennetaan tulokset laitteelle
    if(data.order.length > 0){
      console.log("Saving");
      saveDataToStorage(data);
    }else{
      return;
    }
    // Jos kaikki sukset testattu siirretään tulos sivulle
    if (indexRef.current === data.order?.length) {
      indexRef.current = 0;
      console.log("->Results")
      router.push('/results');
    }
  }, [data]);

  
  // Ladataan vanhat tulokset laitteelta vain, jos parametri `loadOldResults` on true
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

  return (
    <View style={styles.container}>
      <Text style={styles.status}>
        Seuraavaksi: {data.order[indexRef.current]?.name || 'N/A'} kierros: {data.order[indexRef.current]?.round || 'N/A'}
      </Text>
      <DataTable>
        <DataTable.Header>
          <DataTable.Title>Pari</DataTable.Title>
          <DataTable.Title>Kierros</DataTable.Title>
          <DataTable.Title>Kiihtyvyys</DataTable.Title>
          <DataTable.Title>Aika</DataTable.Title>
        </DataTable.Header>
      </DataTable>
      <ScrollView style={styles.scrollView}>
        <DataTable>
          {[...data.results].reverse().map((result, index) => (
            <DataTable.Row key={index}>
              <DataTable.Cell>{result.name}</DataTable.Cell>
              <DataTable.Cell>{result.round}</DataTable.Cell>
              <DataTable.Cell>{result.mag_avg.toFixed(3)}</DataTable.Cell>
              <DataTable.Cell>{result.time.toFixed(3)}</DataTable.Cell>
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
