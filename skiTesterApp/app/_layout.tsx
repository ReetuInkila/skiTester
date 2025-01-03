import React from 'react';
import { Stack } from "expo-router";
import { TouchableOpacity, StyleSheet } from 'react-native';
import { useRouter } from 'expo-router';
import { Ionicons } from '@expo/vector-icons';
import StartScreen from './start';
import HomeScreen from './home'
import InfoModal from './info';

export default function RootLayout() {
  const router = useRouter();

  const [isInfoVisible, setInfoVisible] = React.useState(false);

  const openInfoModal = () => setInfoVisible(true);
  const closeInfoModal = () => setInfoVisible(false);

  return (
    <>
      <Stack>
        <Stack.Screen name="index" options={{ headerShown: false }} />
        <Stack.Screen 
          name="start" 
          options={{
            headerShown: true,
            title:"",
            headerBackVisible: false,
            headerRight: () => (
              <TouchableOpacity onPress={openInfoModal}>
                <Ionicons name="information-circle-outline" size={30} color="black" />
              </TouchableOpacity>
            ),
          }}
        />
        <Stack.Screen name="settings" options={{ title: 'Asetukset' }} />
        <Stack.Screen
          name="home"
          options={{
            headerShown: true,
            title:"",
            headerLeft: () => (
              <Button
                title="Keskeytä mittaus"
                onPress={() => router.push({ pathname: '/start', params: { oldResults: 1 } })}
              />
            ),
          }}
        />
        <Stack.Screen name="results" options={{ headerShown: false }} />
      </Stack>
      {/* InfoModal */}
      <InfoModal visible={isInfoVisible} onClose={closeInfoModal} />
    </>
  );
}
