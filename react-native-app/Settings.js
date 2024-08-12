// Settings.js
import React from 'react';
import { StyleSheet, Text, View, TextInput, Button } from 'react-native';

export default function SettingsScreen({ pairs, setPairs, rounds, setRounds }) {
  return (
    <View style={styles.container}>
      <Text style={styles.label}>Set Pairs:</Text>
      <TextInput
        style={styles.input}
        keyboardType="numeric"
        value={String(pairs)}
        onChangeText={(text) => setPairs(Number(text))}
      />
      <Text style={styles.label}>Set Rounds:</Text>
      <TextInput
        style={styles.input}
        keyboardType="numeric"
        value={String(rounds)}
        onChangeText={(text) => setRounds(Number(text))}
      />
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: 'white',
    alignItems: 'center',
    justifyContent: 'center',
    padding: 20,
  },
  label: {
    fontSize: 18,
    marginVertical: 10,
  },
  input: {
    width: '100%',
    padding: 10,
    borderColor: 'gray',
    borderWidth: 1,
    borderRadius: 5,
    marginBottom: 20,
  },
});
