import React, { useEffect } from 'react';
import { View, ActivityIndicator, StyleSheet } from 'react-native';
import * as SplashScreen from 'expo-splash-screen';
import { useRouter } from 'expo-router';
import { loadDataFromStorage } from './storage';

export default function InitialScreen() {
  const router = useRouter();
  
  useEffect(() => {
    const initializeApp = async () => {
      try {
        // Estä splash screenin automaattinen piilottaminen
        await SplashScreen.preventAutoHideAsync();

        // Lataa tallennetut tiedot
        const storedData = await loadDataFromStorage();

        if (storedData) {
          // Data löytyi, navigoi aloitusnäyttöön
          router.replace({ pathname: '/start', params: { oldResults: 1 } });
        } else {
          // Dataa ei löytynyt, navigoi aloitusnäyttöön
          router.replace({ pathname: '/start', params: { oldResults: 0 } });
        }
      } catch (error) {
        console.error('Error during initialization:', error);
        router.replace('/start'); // Navigoi silti aloitusnäyttöön, vaikka virhe tapahtui
      } finally {
        // Piilota splash screen
        await SplashScreen.hideAsync();
      }
    };

    initializeApp();
  }, [router]);

  return (
    <View style={styles.container}>
      <ActivityIndicator size="large" color="#0000ff" />
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    backgroundColor: '#ffffff',
  },
});
