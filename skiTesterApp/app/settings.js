import React, { useState } from 'react';
import { StyleSheet, Text, KeyboardAvoidingView, TextInput, Button, Platform } from 'react-native';
import { router } from 'expo-router';

export default function SettingsScreen() {
  const [pairs, setPairs] = useState(5); // Tallennetaan numeroina
  const [rounds, setRounds] = useState(5); // Tallennetaan numeroina

  const handleSave = () => {
    console.log(pairs);
    router.push({
      pathname: '/home',
      params: { pairs: pairs, rounds: rounds },
    });
  };

  const handleNumberInput = (value, setter) => {
    // Suodatetaan vain numerot ja päivitetään tila
    const filteredValue = value.replace(/[^0-9]/g, ''); // Salli vain numerot
    setter(filteredValue ? Number(filteredValue) : 0); // Päivitä tila numerona
  };

  return (
    <KeyboardAvoidingView
      style={styles.container}
      behavior={Platform.OS === 'ios' ? 'padding' : 'height'} // iOS: padding, Android: height
    >
      <Button title="Aloita mittaus" onPress={handleSave} />
      <Text style={styles.label}>Suksien lukumäärä:</Text>
      <TextInput
        style={styles.input}
        keyboardType="numeric"
        value={String(pairs)}
        onChangeText={(value) => handleNumberInput(value, setPairs)}
      />
      <Text style={styles.label}>Kierrosten lukumäärä:</Text>
      <TextInput
        style={styles.input}
        keyboardType="numeric"
        value={String(rounds)}
        onChangeText={(value) => handleNumberInput(value, setRounds)}
      />
      <Button title="Aloita mittaus" onPress={handleSave} />
    </KeyboardAvoidingView>
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
    textAlign: 'center',
  },
});
