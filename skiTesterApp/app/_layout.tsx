import React from 'react';
import { Stack } from "expo-router";
import { Button } from 'react-native';
import { useRouter } from 'expo-router';
import StartScreen from './start';
import HomeScreen from './home'

export default function RootLayout() {
  const router = useRouter();
  return (
    <Stack>
      <Stack.Screen name="index" options={{ headerShown: false }} />
      <Stack.Screen name="start" options={{ title: 'Aloitus', headerShown: false }} />
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
  );
}
