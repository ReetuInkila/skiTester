import { Stack } from "expo-router";

export default function RootLayout() {
  return (
    <Stack>
      <Stack.Screen name="index" options={{ headerShown: false }} />
      <Stack.Screen name="start" options={{ title: 'Aloitus', headerShown: false }} />
      <Stack.Screen name="settings" options={{ title: 'Asetukset' }} />
      <Stack.Screen name="home" options={{headerShown: false }} />
      <Stack.Screen name="results" options={{ headerShown: false }} />
    </Stack>
  );
}
