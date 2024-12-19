import React from 'react';
import { View, Text, Button, StyleSheet } from 'react-native';
import { useRouter } from 'expo-router';

export default function StartScreen() {
  const router = useRouter();

  return (
    <View style={styles.container}>
      <Text style={styles.question}>Aloitetaanko uusi mittaus?</Text>
      <View style={styles.buttons}>
        <View style={styles.button}>
          <Button title="Kyllä" onPress={() => router.push('/settings')} />
        </View>
        <View style={styles.button}>
          <Button title="Ei" onPress={() => router.push('/home')} />
        </View>
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
  button: {
    flex: 1,
    marginHorizontal: 10,
  },
});
