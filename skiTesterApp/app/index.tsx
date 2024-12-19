import React from 'react';
import { View, Text, Button, StyleSheet } from 'react-native';
import { Link } from 'expo-router';

export default function StartScreen() {
  return (
    <View style={styles.container}>
      <Text style={styles.question}>Aloitetaanko uusi mittaus?</Text>
      <View style={styles.buttons}>
        <Link href="/settings">Kyllä</Link>
        <Link href="/home">Ei</Link>
      </View>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    padding: 20,
  },
  question: {
    fontSize: 20,
    marginBottom: 30,
    textAlign: 'center',
  },
  buttons: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    width: '80%',
  },
});