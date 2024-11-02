// Settings.js
import React, { useState } from 'react';
import { StyleSheet, Text, View, TextInput, Button, Alert } from 'react-native';

export default function SettingsScreen({ pairs, setPairs, rounds, setRounds, navigation }) {
  // Local state for temporary values
  const [tempPairs, setTempPairs] = useState(String(pairs));
  const [tempRounds, setTempRounds] = useState(String(rounds));

  const handleSave = async () => {
    try {
      // Update the main state
      const newPairs = Number(tempPairs);
      const newRounds = Number(tempRounds);

      setPairs(newPairs);
      setRounds(newRounds);

      // Make the HTTP POST request
      const response = await fetch('http://192.168.4.1/settings', {
        method: 'GET',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ pairs: newPairs, rounds: newRounds }),
      });

      if (!response.ok) {
        throw new Error('Network response was not ok');
      }

      const data = await response.json();
      console.log('Response:', data);

      Alert.alert('Success', 'Settings saved successfully!');
      navigation.goBack(); // Navigate back to the Home screen after saving
    } catch (error) {
      console.error('Error:', error);
      Alert.alert('Error', 'Failed to save settings.');
    }
  };

  return (
    <View style={styles.container}>
      <Text style={styles.label}>Set Pairs:</Text>
      <TextInput
        style={styles.input}
        keyboardType="numeric"
        value={tempPairs}
        onChangeText={setTempPairs}
      />
      <Text style={styles.label}>Set Rounds:</Text>
      <TextInput
        style={styles.input}
        keyboardType="numeric"
        value={tempRounds}
        onChangeText={setTempRounds}
      />
      <Button title="Save" onPress={handleSave} />
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
