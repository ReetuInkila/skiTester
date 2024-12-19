import { Stack } from "expo-router";

export default function RootLayout() {
  return (
    <Stack>
      <Stack.Screen name="index" options={{ headerShown: false }} />
      <Stack.Screen name="settings" options={{ title: 'Asetukset' }} />
      <Stack.Screen name="home" options={{ title: 'Mittaa' }} />
      <Stack.Screen name="results" options={{ title: 'Tulokset' }} />
    </Stack>
  );
}
