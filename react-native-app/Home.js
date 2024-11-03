import React, { useState, useEffect } from 'react';
import { View, Button, Text, StyleSheet } from 'react-native';
import { DataTable } from 'react-native-paper';

export default function HomeScreen(props) {
  const [serverState, setServerState] = React.useState('Loading...');
  const [data, setData] = useState({pairs:props.pairs, rouds:props.rouds, results:[]});
  var ws = React.useRef(new WebSocket('ws://192.168.4.1/ws')).current;


  React.useEffect(() => {
    console.log(props.pairs);
    ws.onopen = () => {
      setServerState('Connected to the server')
    };
    ws.onclose = (e) => {
      setServerState('Disconnected. Check internet or server.')
    };
    ws.onerror = (e) => {
      setServerState(e.message);
    };
    ws.onmessage = (e) => {
      const jsonData = JSON.parse(e.data);
      // Use functional update to avoid direct mutation of state
      setData(prevData => ({
        ...prevData,
        results: [...prevData.results, jsonData]
      }));
    };
  }, [])

  return (
    <View>
        <>
          <DataTable>
            <DataTable.Header>
              <DataTable.Title>Pari</DataTable.Title>
              <DataTable.Title>Kierros</DataTable.Title>
              <DataTable.Title>T1</DataTable.Title>
              <DataTable.Title>T2</DataTable.Title>
              <DataTable.Title>T Tot</DataTable.Title>
            </DataTable.Header>

            {data.results.map((result, index) => (
              <DataTable.Row key={index}>
                <DataTable.Cell>1</DataTable.Cell>
                <DataTable.Cell>1</DataTable.Cell>
                <DataTable.Cell>{result.t1}</DataTable.Cell>
                <DataTable.Cell>{result.t2}</DataTable.Cell>
                <DataTable.Cell>{result.t1 + result.t2}</DataTable.Cell>
              </DataTable.Row>
            ))}
          </DataTable>
        </>
    </View>
  );
}