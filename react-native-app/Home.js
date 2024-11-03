import React, { useState, useEffect } from 'react';
import { View, Button, Text, StyleSheet } from 'react-native';
import { DataTable } from 'react-native-paper';

export default function HomeScreen() {
  const [serverState, setServerState] = React.useState('Loading...');
  const [pairs, setPairs] = useState(null);
  const [rounds, setRounds] = useState(null);
  const [data, setData] = useState({});
  var ws = React.useRef(new WebSocket('ws://192.168.4.1/ws')).current;


  React.useEffect(() => {
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
      console.log(e.data);
      setData(jsonData)
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

            <DataTable.Row>
              <DataTable.Cell>{1}</DataTable.Cell>
              <DataTable.Cell>{1}</DataTable.Cell>
              <DataTable.Cell>{data.t1}</DataTable.Cell>
              <DataTable.Cell>{data.t2}</DataTable.Cell>
              <DataTable.Cell>{data.t1 + data.t2}</DataTable.Cell>
            </DataTable.Row>
          </DataTable>
        </>
    </View>
  );
}