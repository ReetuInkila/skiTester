import React, { useState, useEffect } from 'react';
import { StyleSheet, Text, KeyboardAvoidingView, TextInput, Button, Platform, ScrollView, View } from 'react-native';
import { router } from 'expo-router';
import { useDispatch} from 'react-redux';
import { setData, setOrder, clearData } from './store';
import { clearStorage } from './storage';

export default function SettingsScreen() {
  const [pairs, setPairs] = useState(5); // Tallennetaan numeroina
  const [rounds, setRounds] = useState(5); // Tallennetaan numeroina
  const [names, setNames] = useState(Array(5).fill('')); // Alustetaan viidellä tyhjällä nimellä
  const [temperature, setTemperature] = useState(''); // Lämpötila
  const [snowQuality, setSnowQuality] = useState(''); // Lumen laatu
  const [baseHardness, setBaseHardness] = useState(''); // Pohjan kovuus

  const dispatch = useDispatch();

  // Päivitä names-listan pituus vastaamaan pairs-muuttujaa
  useEffect(() => {
    if (pairs > names.length) {
      setNames((prevNames) => [...prevNames, ...Array(pairs - prevNames.length).fill('')]);
    } else if (pairs < names.length) {
      setNames((prevNames) => prevNames.slice(0, pairs));
    }
  }, [pairs]);

  const handleNumberInput = (value, setter) => {
    const filteredValue = value.replace(/[^0-9]/g, '');
    setter(filteredValue ? Number(filteredValue) : 0);
  };

  const handleNameChange = (value, index) => {
    setNames((prevNames) => {
      const updatedNames = [...prevNames];
      updatedNames[index] = value;
      return updatedNames;
    });
  };


  const handleSave = () => {
    // Tallenna tiedot Redux-storeen
    dispatch(clearData());

    dispatch(
      setData({
        pairs,
        rounds,
        names,
        temperature,
        snowQuality,
        baseHardness,
      })
    );

    // Luo järjestys
    const newOrder = [];
    for (let round = 1; round <= rounds; round++) {
      if (round % 2 !== 0) {
        for (let i = 0; i < pairs; i++) {
          newOrder.push({ round, name: names[i] || `Pari ${i + 1}` });
        }
      } else {
        for (let i = pairs - 1; i >= 0; i--) {
          newOrder.push({ round, name: names[i] || `Pari ${i + 1}` });
        }
      }
    }

    dispatch(setOrder(newOrder));

    clearStorage();
    router.push('/home');
  };

  return (
    <KeyboardAvoidingView
      style={styles.container}
      behavior={Platform.OS === 'ios' ? 'padding' : 'height'}
    >
      <ScrollView>
        <View style={styles.row}>
          {/* Vasen sarake */}
          <View style={styles.column}>
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
            <Text style={styles.label}>Lämpötila (°C):</Text>
            <TextInput
              style={styles.input}
              keyboardType="numbers-and-punctuation"
              placeholder="Esim. -5"
              value={temperature}
              onChangeText={setTemperature}
            />
            <Text style={styles.label}>Lumen laatu:</Text>
            <TextInput
              style={styles.input}
              placeholder="Esim. Uusi lumi"
              value={snowQuality}
              onChangeText={setSnowQuality}
            />
            <Text style={styles.label}>Pohjan kovuus:</Text>
            <TextInput
              style={styles.input}
              placeholder="Esim. Pehmeä"
              value={baseHardness}
              onChangeText={setBaseHardness}
            />
          </View>

          {/* Oikea sarake */}
          <View style={styles.column}>
            <Text style={styles.label}>Sukset:</Text>
            {names.map((name, index) => (
              <TextInput
                key={index}
                style={styles.input}
                placeholder={`Pari ${index + 1}`}
                value={name}
                onChangeText={(value) => handleNameChange(value, index)}
              />
            ))}
          </View>
        </View>
        <Button title="Aloita mittaus" onPress={handleSave} />
      </ScrollView>
    </KeyboardAvoidingView>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: 'white',
    padding: 20,
  },
  row: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    marginBottom: 20,
  },
  column: {
    flex: 1,
    paddingHorizontal: 10,
  },
  label: {
    fontSize: 16,
    marginBottom: 5,
    textAlign: 'left',
  },
  input: {
    width: '100%',
    padding: 10,
    borderColor: 'gray',
    borderWidth: 1,
    borderRadius: 5,
    marginBottom: 10,
    textAlign: 'center',
  },
});
