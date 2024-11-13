// Settings.js
import React, { useState } from 'react';
import { StyleSheet, Text, View, TextInput, Button, Alert } from 'react-native';

export default function SettingsScreen({ pairs, setPairs, rounds, setRounds, navigation }) {
  // Local state for temporary values
  const [tempPairs, setTempPairs] = useState(String(pairs));
  const [tempRounds, setTempRounds] = useState(String(rounds));

  const handleSave = async () => {
    // Update the main state
    const newPairs = Number(tempPairs);
    const newRounds = Number(tempRounds);

    setPairs(newPairs);
    setRounds(newRounds);
    navigation.navigate('Home'); // Navigate back to Home
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
