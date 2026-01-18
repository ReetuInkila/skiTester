import React from 'react';
import { SafeAreaView, Text, TouchableOpacity, StyleSheet, Image} from 'react-native';
import { useLocalSearchParams, useRouter } from 'expo-router';

export default function StartScreen() {
  const router = useRouter();
  const params = useLocalSearchParams(); // Fetch query params
  const { oldResults } = params;

  return (
    <SafeAreaView style={styles.container}>
      <Image 
        source={require('./../assets/images/logo.png')} // Korvaa polku omalla logosi polulla
        style={styles.logo} 
        resizeMode="contain"
      />
      <TouchableOpacity
        style={[styles.button, styles.newMeasurementButton]}
        onPress={() => router.push('/settings')}
      >
        <Text style={styles.buttonText}>Uusi mittaus</Text>
      </TouchableOpacity>
      {oldResults == 1 && ( // Näytetään "Jatka edellistä" vain, jos dataa on
        <TouchableOpacity
          style={[styles.button, styles.continueButton]}
          onPress={() =>
            router.push({ pathname: '/home', params: { loadOldResults: true } })
          }
        >
          <Text style={styles.buttonText}>Jatka edellistä</Text>
        </TouchableOpacity>
      )}
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  container:{
    alignItems: 'center',
    backgroundColor: "white",
    minHeight: "100%",
  },
  logo: {
    width: '90%', // Skaalaa suhteessa näytön leveyteen
    height: undefined, // Korkeus lasketaan suhteessa leveys-korkeus-suhteeseen
    aspectRatio: 1, // Pitää kuvan mittasuhteet 1:1
  },
  button: {
    width: '90%',
    paddingVertical: 15,
    marginVertical: 15,
    borderRadius: 10,
    alignItems: 'center',
    justifyContent: 'center',
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.25,
    shadowRadius: 3.84,
    elevation: 5,
    backgroundColor: '#91d1f5'
  },
  buttonText: {
    fontSize: 40,
    fontWeight: 'bold',
  },
});
